#include "ScreenResolution.h"
#include "Resolutions.h"
#include "GfxCoreHook.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"
#include "Utils.h"
#include <d3dx9.h>

using namespace plugin;

Bool METHOD App_GetScreenResolution(void *app, DUMMY_ARG, Float *outWidth, Float *outHeight) {
    Bool result = CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD10), app, outWidth, outHeight);
    if (result && *outWidth > 2000) {
        *outWidth = 1920.0f;
        *outHeight = 1080.0f;
    }
    return result;
}

bool METHOD App_GetMinimumScreenResolution(void *app, DUMMY_ARG, Float *outWidth, Float *outHeight) {
    Bool result = CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD10), app, outWidth, outHeight);
    if (result && *outWidth > 2000) {
        *outWidth = 1920.0f;
        *outHeight = 1080.0f;
        return true;
    }
    return CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD40), app, outWidth, outHeight);
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

void Screenshot_FormatFileName(WideChar *dst, UInt len, WideChar const *format, UShort year, UShort month, UShort day,
    UShort hour, UShort minute, UShort second)
{
    swprintf_s(dst, len, L"%04d_%02d_%02d %02d_%02d_%02d", year, month, day, hour, minute, second);
    if (Settings::GetInstance().ScreenshotFormat == D3DXIFF_BMP)
        wcscat(dst, L".bmp");
    else if (Settings::GetInstance().ScreenshotFormat == D3DXIFF_JPG)
        wcscat(dst, L".jpg");
    else if (Settings::GetInstance().ScreenshotFormat == D3DXIFF_PNG)
        wcscat(dst, L".png");
}

void InstallScreenResolution_GfxCore() {
    patch::SetPointer(GfxCoreAddress(0x54FF68), App_GetScreenResolution);
    patch::SetPointer(GfxCoreAddress(0x54FF6C), App_GetMinimumScreenResolution);
    // Screenshot - format
    patch::RedirectCall(GfxCoreAddress(0x3BA4D6), Screenshot_WcharToMB);
    patch::RedirectCall(GfxCoreAddress(0x3BA4DF), Screenshot_SaveSurfaceToFile);
}

Bool GetCurrentScreenResolution(Float &width, Float &height) {
    void *xlibFactory = CallAndReturn<void *, 0x459720>();
    void *app = CallVirtualMethodAndReturn<void *, 33>(xlibFactory);
    return CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD10), app, &width, &height);
}

VisibleControlAppearance *METHOD GetImageAppearanceFor3dRender(CXgImage *image) {
    static VisibleControlAppearance a;
    a = *image->GetAppearance();
    Float width = 0.0f, height = 0.0f;
    if (GetCurrentScreenResolution(width, height) && width > 2000.0f && height > 0.0f) { // TODO: change to width > 2000.0f
        Float factorX = width / 1920.0f;
        Float factorY = height / 1080.0f;
        //factorX = 2.0f; // TODO: remove this
        //factorY = 2.0f; // TODO: remove this
        a.rect.x = (Short)((Float)a.rect.x * factorX); // TODO: uncomment
        a.rect.y = (Short)((Float)a.rect.y * factorY); // TODO: uncomment
        a.rect.width = (Short)((Float)a.rect.width * factorX);
        a.rect.height = (Short)((Float)a.rect.height * factorY);
    }
    return &a;
}

void __declspec(naked) CClubShirtDesign_3DRenderGetAppearance_Exe() {
    __asm {
        call GetImageAppearanceFor3dRender
        mov edx, 0x599DEC
        jmp edx
    }
}

void __declspec(naked) CMdSelectMatchModes_3DRenderGetAppearance_Kit1_1_Exe() {
    __asm {
        call GetImageAppearanceFor3dRender
        mov edx, 0xADD03E
        jmp edx
    }
}

void __declspec(naked) CMdSelectMatchModes_3DRenderGetAppearance_Kit1_2_Exe() {
    __asm {
        call GetImageAppearanceFor3dRender
        mov edx, 0xADD0F1
        jmp edx
    }
}

void __declspec(naked) CMdSelectMatchModes_3DRenderGetAppearance_Kit2_1_Exe() {
    __asm {
        call GetImageAppearanceFor3dRender
        mov edx, 0xADD411
        jmp edx
    }
}

void __declspec(naked) CMdSelectMatchModes_3DRenderGetAppearance_Kit2_2_Exe() {
    __asm {
        call GetImageAppearanceFor3dRender
        mov edx, 0xADD4BB
        jmp edx
    }
}

void PatchScreenResolution(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
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

        // Screenshot - overlay
        patch::RedirectJump(0x9D3DF8, App_MakeScreenshot_Exe);
        // Screenshot - key
        patch::RedirectCall(0x9D74F6, Screenshot_GetAsyncKeyState);
        patch::Nop(0x9D74F6 + 5, 1);
        // Screenshot - filename
        patch::RedirectCall(0x9D3DBD, Screenshot_FormatFileName);
    }
}
