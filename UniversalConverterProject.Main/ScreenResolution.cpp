#include "ScreenResolution.h"
#include "Resolutions.h"
#include "GfxCoreHook.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"
#include "Utils.h"
#include "shared.h"
#include <d3dx9.h>
#include <regex>

using namespace plugin;

Bool GetBaseResolution(void *app, Float *outWidth, Float *outHeight, Bool minimal = false) {
    Bool result = false;
    if (minimal)
        result = CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD40), app, outWidth, outHeight);
    else
        result = CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD10), app, outWidth, outHeight);
    if (result && *outWidth > 2000) {
        *outWidth /= 2.0f;
        *outHeight /= 2.0f;
    }
    return result;
}

Bool GetBaseResolution(Float *outWidth, Float *outHeight) {
    return GetBaseResolution(GetApp(), outWidth, outHeight);
}

Bool METHOD App_GetScreenResolution(void *app, DUMMY_ARG, Float *outWidth, Float *outHeight) {
    return GetBaseResolution(app, outWidth, outHeight);
}

Bool GetCurrentScreenResolution(Float &width, Float &height) {
    return CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD10), GetApp(), &width, &height);
}

Bool METHOD GetCurrentScreenResolution_Method(void *, DUMMY_ARG, Float &width, Float &height) {
    return CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD10), GetApp(), &width, &height);
}

Bool METHOD App_GetMinimumScreenResolution(void *app, DUMMY_ARG, Float *outWidth, Float *outHeight) {
    return GetBaseResolution(app, outWidth, outHeight, true);
}

void METHOD GameScreen_AddResolutions(void *, DUMMY_ARG, CXgComboBox *cb) {
    UInt SelectedResolution = *(UInt *)0x30AA734;
    void *GfxInterface = *(void **)0x30ABBD0;
    cb->Clear();
    for (auto const &r : Resolutions) {
        if (CallVirtualMethodAndReturn<Bool, 23>(GfxInterface, r.width, r.height, 32)) { // CheckResolution
            String resText = Format(L"%u x %u", r.width, r.height);
            cb->AddItem(resText.c_str(), r.id);
        }
    }
    for (Int i = 0; i < cb->GetNumberOfItems(); i++) {
        if (cb->GetValueAtIndex(i) == SelectedResolution) {
            cb->SetCurrentIndex(i);
            break;
        }
    }
    if (cb->GetNumberOfItems() && cb->GetCurrentIndex() < 0)
        cb->SetCurrentIndex(0);
}

Bool METHOD App_MakeScreenshot(void *app, DUMMY_ARG, WideChar const *filePath) {
    Bool screenshotTaken = CallVirtualMethodAndReturn<Bool, 75>(app, filePath);
    return Settings::GetInstance().DisplayScreenshotOverlay && screenshotTaken;
}

void __declspec(naked) App_MakeScreenshot_Exe() {
    __asm {
        push eax
        mov ecx, esi
        call App_MakeScreenshot
        mov ecx, 0x9D3DFD
        jmp ecx
    }
}

SHORT __stdcall Screenshot_GetAsyncKeyState(int vKey) {
    if (Settings::GetInstance().WindowedModeStartValue && GetForegroundWindow() != *(HWND *)GfxCoreAddress(0x669064))
        return 0;
    return (GetAsyncKeyState(Settings::GetInstance().ScreenshotKey) & 0x8000) != 0;
}

WideChar const *Screenshot_WcharToMB(WideChar const *lpWideCharStr) {
    return lpWideCharStr;
}

HRESULT __stdcall Screenshot_SaveSurfaceToFile(LPCWSTR pDestFile, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DSURFACE9 pSrcSurface,
    const PALETTEENTRY *pSrcPalette, const RECT *pSrcRect)
{
    return D3DXSaveSurfaceToFileW(pDestFile, (D3DXIMAGE_FILEFORMAT)Settings::GetInstance().ScreenshotFormat, pSrcSurface, pSrcPalette, pSrcRect);
}

UInt GetNextScreenshotNumber(String const &prefix = String()) {
    path documentsPath = GetDocumentsPath();
    if (!documentsPath.empty()) {
        path screenshotsPath = documentsPath / "Data" / "ScreenShots";
        if (exists(screenshotsPath) && is_directory(screenshotsPath)) {
            wregex pattern(L"^" + prefix + L"(0\\d{6})\\.(bmp|jpg|png)$", std::regex::icase);
            UInt maxNumber = 0;
            for (const auto &entry : directory_iterator(screenshotsPath)) {
                if (entry.is_regular_file()) {
                    String filename = entry.path().filename().wstring();
                    wsmatch match;
                    if (regex_match(filename, match, pattern)) {
                        if (match.size() >= 2) {
                            try {
                                UInt number = std::stoul(match[1].str());
                                if (number > maxNumber)
                                    maxNumber = number;
                            }
                            catch (...) {}
                        }
                    }
                }
            }
            if (maxNumber >= 999999)
                return 999999;
            return maxNumber + 1;
        }
    }
    return 1;
}

void Screenshot_FormatFileName(WideChar *dst, UInt len, WideChar const *, UShort year, UShort month, UShort day,
    UShort hour, UShort minute, UShort second)
{
    if (Settings::GetInstance().ScreenshotName == SCREENSHOTNAME_NUMBER)
        swprintf_s(dst, len, L"%07u", GetNextScreenshotNumber());
    else if (Settings::GetInstance().ScreenshotName == SCREENSHOTNAME_TITLE_AND_NUMBER)
        swprintf_s(dst, len, L"FM%s_%07u", GetPatchName().c_str(), GetNextScreenshotNumber(L"FM" + GetPatchName() + L"_"));
    else if (Settings::GetInstance().ScreenshotName == SCREENSHOTNAME_TITLE_AND_TIME)
        swprintf_s(dst, len, L"FM%s %04d_%02d_%02d %02d_%02d_%02d", GetPatchName().c_str(), year, month, day, hour, minute, second);
    else
        swprintf_s(dst, len, L"%04d_%02d_%02d %02d_%02d_%02d", year, month, day, hour, minute, second);
    if (Settings::GetInstance().ScreenshotFormat == D3DXIFF_BMP)
        wcscat(dst, L".bmp");
    else if (Settings::GetInstance().ScreenshotFormat == D3DXIFF_JPG)
        wcscat(dst, L".jpg");
    else if (Settings::GetInstance().ScreenshotFormat == D3DXIFF_PNG)
        wcscat(dst, L".png");
}

void ModifyAppearanceFor3dRender(VisibleControlAppearance &a, Bool updatePosition = true) {
    Float width = 0.0f, height = 0.0f;
    if (GetCurrentScreenResolution(width, height) && width > 2000.0f && height > 0.0f) { // TODO: change to width > 2000.0f
        Float factorX = width / 1920.0f;
        Float factorY = height / 1080.0f;
        a.rect.width = (Short)((Float)a.rect.width * factorX);
        a.rect.height = (Short)((Float)a.rect.height * factorY);
        if (updatePosition) {
            Float offsetX = (1920.0f - 1280.0f) / 2.0f;
            Float offsetY = (1080.0f - 1024.0f) / 2.0f;
            a.rect.x = (Short)((Float)a.rect.x * factorX + offsetX);
            a.rect.y = (Short)((Float)a.rect.y * factorY + offsetY);
        }
    }
}

void ModifyAppearanceFor3dRender_Test(VisibleControlAppearance &a, Bool updatePosition = true) {
    a.rect.x = 1000;
    a.rect.y = 1000;
    a.rect.x = 200;
    a.rect.y = 200;
}

VisibleControlAppearance *METHOD GetAppearanceFor3dRender(CXgBaseButton *control) {
    static VisibleControlAppearance a;
    a = *control->GetAppearance();
    ModifyAppearanceFor3dRender(a);
    return &a;
}

VisibleControlAppearance *METHOD GetRectFor3dRender(CXgBaseButton *control) {
    static VisibleControlAppearance a;
    a.rect = *control->GetRect();
    ModifyAppearanceFor3dRender(a);
    a.blendCol = 0xFFFFFFFF;
    a.depth = 0.0f;
    return &a;
}

VisibleControlAppearance *METHOD GetAppearanceFor3dRender_Test(CXgBaseButton *control) {
    static VisibleControlAppearance a;
    a = *control->GetAppearance();
    a.rect.x = 1000;
    a.rect.y = 800;
    a.rect.width = 200;
    a.rect.height = 200;
    return &a;
}

VisibleControlAppearance *METHOD GetRectFor3dRender_Test(CXgBaseButton *control) {
    static VisibleControlAppearance a;
    a.rect = *control->GetRect();
    a.rect.width = 200;
    a.rect.height = 200;
    a.blendCol = 0xFFFFFFFF;
    a.depth = 0.0f;
    return &a;
}

void __declspec(naked) CClubShirtDesign_3DRenderGetAppearance_Exe() {
    __asm {
        call GetRectFor3dRender
        mov edx, 0x599DEC
        jmp edx
    }
}

void __declspec(naked) CMdSelectMatchModes_3DRenderGetAppearance_Kit1_1_Exe() {
    __asm {
        call GetRectFor3dRender
        mov edx, 0xADD03E
        jmp edx
    }
}

void __declspec(naked) CMdSelectMatchModes_3DRenderGetAppearance_Kit1_2_Exe() {
    __asm {
        call GetRectFor3dRender
        mov edx, 0xADD0F1
        jmp edx
    }
}

void __declspec(naked) CMdSelectMatchModes_3DRenderGetAppearance_Kit2_1_Exe() {
    __asm {
        call GetRectFor3dRender
        mov edx, 0xADD411
        jmp edx
    }
}

void __declspec(naked) CMdSelectMatchModes_3DRenderGetAppearance_Kit2_2_Exe() {
    __asm {
        call GetRectFor3dRender
        mov edx, 0xADD4BB
        jmp edx
    }
}

void __declspec(naked) GetScreenResolution_StadiumEditor1_Exe() {
    __asm {
        push ecx
        mov ecx, eax
        call GetCurrentScreenResolution_Method
        mov ecx, 0x689ADE
        jmp ecx
    }
}

void __declspec(naked) GetScreenResolution_StadiumEditor2_Exe() {
    __asm {
        push ecx
        mov ecx, eax
        call GetCurrentScreenResolution_Method
        mov ecx, 0x682AB7
        jmp ecx
    }
}

Rect *METHOD CClubStadium_3DRenderGetGlobalRect(CXgTextBox *tb, DUMMY_ARG, Rect *out) {
    VisibleControlAppearance *appearance = GetAppearanceFor3dRender_Test(tb);
    CallVirtualMethodAndReturn<VisibleControlAppearance *, 59>(tb, out, appearance);
    return out;
}

void __declspec(naked) CClubStadium_3DRenderGetGlobalRect_Exe() {
    __asm {
        lea eax, [esp + 8]
        push eax
        call CClubStadium_3DRenderGetGlobalRect
        mov ecx, 0x65D6A7
        jmp ecx
    }
}

void __declspec(naked) CClubStadium_3DRenderGetAppearance_Exe() {
    __asm {
        add edi, 0x2C
        call GetAppearanceFor3dRender_Test
        mov ecx, 0x65D8A3
        jmp ecx
    }
}

template<Bool UpdatePosition>
void *METHOD CXgFMPanel_3DRenderHead(CXgFMPanel *t, DUMMY_ARG, void *unk, Rect *rect, void *playerRenderData, void *kitRenderData, Bool bDisplayPortrait, Bool32 bHasPicture) {
    static VisibleControlAppearance a;
    a.rect = *rect;
    ModifyAppearanceFor3dRender(a, UpdatePosition);
    return CallMethodAndReturn<void *, 0xD39A70>(t, unk, &a.rect, playerRenderData, kitRenderData, bDisplayPortrait, bHasPicture);
}

void InstallScreenResolution_GfxCore() {
#if 0
    patch::SetPointer(GfxCoreAddress(0x54FF68), App_GetScreenResolution);
    patch::SetPointer(GfxCoreAddress(0x54FF6C), App_GetMinimumScreenResolution);
#endif
    // Screenshot - format
    patch::RedirectCall(GfxCoreAddress(0x3BA4D6), Screenshot_WcharToMB);
    patch::RedirectCall(GfxCoreAddress(0x3BA4DF), Screenshot_SaveSurfaceToFile);
}

void PatchScreenResolution(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
#if 0
        patch::RedirectJump(0x5005A0, GameScreen_AddResolutions);
        patch::RedirectJump(0x7AE720, GameScreen_AddResolutions);

        struct ResolutionWidthHeight { UInt width, height; };
        Vector<ResolutionDesc> TempResolutions(std::size(Resolutions));
        for (UInt i = 0; i < std::size(Resolutions); i++)
            TempResolutions[i] = Resolutions[i];
        Utils::Sort(TempResolutions, [](ResolutionDesc const &a, ResolutionDesc const &b) {
            return a.id < b.id;
        });
        static ResolutionWidthHeight ResolutionsSortedByID[std::size(Resolutions)];
        for (UInt i = 0; i < std::size(Resolutions); i++) {
            ResolutionsSortedByID[i].width = TempResolutions[i].width;
            ResolutionsSortedByID[i].height = TempResolutions[i].height;
        }
        // GameOptions::SetResolution
        patch::SetPointer(0x44EF95 + 3, &ResolutionsSortedByID[0].width);
        patch::SetPointer(0x44EF9E + 3, &ResolutionsSortedByID[0].height);
        patch::SetUChar(0x44EFA8 + 2, (UChar)std::size(Resolutions));
        // CDBGame::WriteResolutionToRegistry
        patch::SetPointer(0x44F101 + 3, &ResolutionsSortedByID[0].width);
        patch::SetPointer(0x44F108 + 3, &ResolutionsSortedByID[0].height);
        // TFMOptions::TFMOptions
        patch::SetPointer(0x4501F0 + 3, &ResolutionsSortedByID[0].width);
        patch::SetPointer(0x4501F9 + 3, &ResolutionsSortedByID[0].height);
        patch::SetUChar(0x450204 + 2, (UChar)std::size(Resolutions));

        // 3D renders
        patch::RedirectJump(0x599DE7, CClubShirtDesign_3DRenderGetAppearance_Exe);
        patch::RedirectJump(0xADD039, CMdSelectMatchModes_3DRenderGetAppearance_Kit1_1_Exe);
        patch::RedirectJump(0xADD0EC, CMdSelectMatchModes_3DRenderGetAppearance_Kit1_2_Exe);
        patch::RedirectJump(0xADD40C, CMdSelectMatchModes_3DRenderGetAppearance_Kit2_1_Exe);
        patch::RedirectJump(0xADD4B6, CMdSelectMatchModes_3DRenderGetAppearance_Kit2_2_Exe);
        patch::RedirectJump(0x689AD9, GetScreenResolution_StadiumEditor1_Exe);
        patch::RedirectJump(0x682AB2, GetScreenResolution_StadiumEditor2_Exe);
        patch::RedirectJump(0x65D6A0, CClubStadium_3DRenderGetGlobalRect_Exe); // generic stadium
        patch::RedirectJump(0x65D89E, CClubStadium_3DRenderGetAppearance_Exe); // custom stadium
        patch::RedirectCall(0x4FAD73, CXgFMPanel_3DRenderHead<false>);
        patch::RedirectCall(0x536F98, CXgFMPanel_3DRenderHead<true>);
        patch::RedirectCall(0x5478A7, CXgFMPanel_3DRenderHead<true>);
        patch::RedirectCall(0xCB5ACA, CXgFMPanel_3DRenderHead<true>);
        patch::RedirectCall(0xD39CB2, CXgFMPanel_3DRenderHead<true>);
#endif
        // Screenshot - overlay
        patch::RedirectJump(0x9D3DF8, App_MakeScreenshot_Exe);
        // Screenshot - key
        patch::RedirectCall(0x9D74F6, Screenshot_GetAsyncKeyState);
        patch::Nop(0x9D74F6 + 5, 1);
        // Screenshot - filename
        patch::RedirectCall(0x9D3DBD, Screenshot_FormatFileName);
    }
}
