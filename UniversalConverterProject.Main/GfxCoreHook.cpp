#include "GfxCoreHook.h"
#include "3dPatches.h"
#include "Kits.h"
#include "PlayerAccessories.h"
#include "WindowedMode.h"
#include "Pitch.h"
#include "GenericHeads.h"
#include "3dAdboardsAndBanners.h"
#include "CustomStadiums.h"
#include "3dModels.h"
#include "CustomShaders.h"
#include "ScreenResolution.h"
#include "UcpSettings.h"

unsigned int gOriginalLoadGfxCore = 0;
unsigned int hLibrary = 0;

unsigned int GfxCoreAddress(unsigned int addr) {
    return hLibrary + addr;
}

bool METHOD OnLoadGfxCore(void **info, DUMMY_ARG, const wchar_t *libFileName, const char *procName) {
    bool result = false;
    if (gOriginalLoadGfxCore) {
        result = CallMethodAndReturnDynGlobal<bool>(gOriginalLoadGfxCore, info, libFileName, procName);
        if (result) {
            void *gfxCore = info[0];
            hLibrary = (unsigned int)info[1];

            //if (Settings::GetInstance().EnableCustomShaders)
            //    InstallCustomShaders();
            Install3dPatches_FM13();
            InstallKits_FM13();
            InstallPlayerAccessoriesGfxPatches();
            InstallWindowedMode_GfxCore();
            InstallPitch3D();
            InstallGenericHeads_GfxCore();
            InstallAdBoardsAndBanners3dPatches();
            InstallCustomStadiums3DPatches();
            Install3DModelPatches();
            InstallScreenResolution_GfxCore();
        }
    }
    return result;
}

void PatchGfxCoreHook(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD)
        gOriginalLoadGfxCore = patch::RedirectCall(0x45BBEF, OnLoadGfxCore);
}
