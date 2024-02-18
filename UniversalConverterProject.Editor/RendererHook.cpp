#include "RendererHook.h"
#include "3dPatches.h"
#include "Kits.h"
#include "CustomShaders.h"
#include "UcpSettings.h"

unsigned int gOriginalLoadRenderer = 0;
unsigned int hLibrary = 0;

unsigned int RendererAddress(unsigned int addr) {
    return hLibrary + addr;
}

bool METHOD OnLoadRenderer(void **info, DUMMY_ARG, const wchar_t *libFileName, const char *procName) {
    bool result = false;
    if (gOriginalLoadRenderer) {
        result = CallMethodAndReturnDynGlobal<bool>(gOriginalLoadRenderer, info, libFileName, procName);
        if (result) {
            void *gfxCore = info[0];
            hLibrary = (unsigned int)info[1];

            //if (Settings::GetInstance().EnableCustomShaders)
            //    InstallCustomShaders();
            Install3dPatches();
            InstallKits_Renderer();

        }
    }
    return result;
}

void PatchRendererHook(FM::Version v) {
    if (v.id() == ID_ED_13_1000)
        gOriginalLoadRenderer = patch::RedirectCall(0x5410B5, OnLoadRenderer);
}
