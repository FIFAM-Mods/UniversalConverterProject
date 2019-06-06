#include "GfxCoreHook.h"
#include "3dPatches.h"
#include "Kits.h"

unsigned int gOriginalLoadGfxCore = 0;
unsigned int hLibrary = 0;

unsigned int GfxCoreAddress(unsigned int addr) {
    return hLibrary + addr;
}

bool METHOD OnLoadGfxCore(void **info, int, const wchar_t *libFileName, const char *procName) {
    bool result = false;
    if (gOriginalLoadGfxCore) {
        result = CallMethodAndReturnDynGlobal<bool>(gOriginalLoadGfxCore, info, libFileName, procName);
        if (result) {
            void *gfxCore = info[0];
            hLibrary = (unsigned int)info[1];

            Install3dPatches_FM13();
            InstallKits_FM13();

        }
    }
    return result;
}

void *OnLoadGfxCore_FM11() {
    //Error("GFX core loaded");
    void *result = nullptr;
    if (gOriginalLoadGfxCore) {
        result = CallAndReturnDynGlobal<void *>(gOriginalLoadGfxCore);
        if (result) {
            hLibrary = *(unsigned int *)0x146A104;

            Install3dPatches_FM11();
            //InstallKits_FM11();

        }
    }
    return result;
}

void PatchGfxCoreHook(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD)
        gOriginalLoadGfxCore = patch::RedirectCall(0x45BBEF, OnLoadGfxCore);
    else if (v.id() == ID_FM_11_1003)
        gOriginalLoadGfxCore = patch::RedirectCall(0x43C574, OnLoadGfxCore_FM11);
}
