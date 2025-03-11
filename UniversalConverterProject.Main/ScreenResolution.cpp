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

        // Screenshot - overlay
        patch::RedirectJump(0x9D3DF8, App_MakeScreenshot_Exe);
        // Screenshot - key
        patch::RedirectCall(0x9D74F6, Screenshot_GetAsyncKeyState);
        patch::Nop(0x9D74F6 + 5, 1);
        // Screenshot - filename
        patch::RedirectCall(0x9D3DBD, Screenshot_FormatFileName);
    }
}
