#include "ScreenResolution.h"
#include "GfxCoreHook.h"
#include "FifamTypes.h"

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

void METHOD App_GetScreenRatio(void *app, DUMMY_ARG, Float *outWidthRatio, Float *outHeightRatio) {
    *outWidthRatio = 2.0f;
    *outHeightRatio = 2.0f;
}

void InstallScreenResolution_GfxCore() {
    //patch::SetPointer(GfxCoreAddress(0x54FF68), App_GetScreenResolution);
    //patch::SetPointer(GfxCoreAddress(0x54FF6C), App_GetMinimumScreenResolution);
    //patch::SetPointer(GfxCoreAddress(0x54FF70), App_GetScreenRatio);
}

void * METHOD OnScaleControlsConstructor(void *sc, DUMMY_ARG, Float scaleWidth, Float scaleHeight) {
    //void *xLib = CallAndReturn<void *, 0x459720>();
    //void *app = CallVirtualMethodAndReturn<void *, 18>(xLib);
    //Float width = 0.0f, height = 0.0f;
    //Bool result = CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3BAD10), app, &width, &height);
    //if (result && width > 2000) {
        scaleWidth = 2.0f;
        scaleHeight = 2.0f;
    //}
    return CallMethodAndReturn<void *, 0x4F1570>(sc, 1.9f, 1.9f);
}

void PatchScreenResolution(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        //patch::RedirectJump(0x45035A, (void *)0x4505A1);
    //    patch::Nop(0xA02906, 5);
    //    patch::RedirectCall(0x4EF89E, OnScaleControlsConstructor);
    //    patch::Nop(0x4EF8D2, 2);
    //
    //    patch::Nop(0x4F1178, 2);
    //    patch::RedirectJump(0x4F102C, (void *)0x4F1020);
          //patch::RedirectCall(0x45BB67, DisplayResolutool);
    }
}
