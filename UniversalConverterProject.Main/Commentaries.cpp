#include "gfxcorehook.h"
#include "plugin-std.h"

using namespace plugin;
unsigned int gOriginalLoadGfxCore = 0;
unsigned int hLibrary = 0;

unsigned int GfxCoreAddress(unsigned int addr) {
    return hLibrary + addr;
}

bool METHOD OnLoadGfxCore(void** info, DUMMY_ARG, const wchar_t* libFileName, const char* procName) {
    bool result = false;
    if (gOriginalLoadGfxCore) {
        result = CallMethodAndReturnDynGlobal<bool>(gOriginalLoadGfxCore, info, libFileName, procName);
        if (result) {
            void* gfxCore = info[0];
            hLibrary = (unsigned int)info[1];

            patch::SetUInt(GfxCoreAddress(0x231a0f + 1), 0x1051d0a8);
            patch::SetUInt(GfxCoreAddress(0x231a14 + 2), 0x1051d0ac);
            patch::SetUInt(GfxCoreAddress(0x231a1a + 2), 0x1051d0b0);
            patch::SetPointer(GfxCoreAddress(0x235c9f + 1), "Commentary\\pbp.evt");
            patch::SetPointer(GfxCoreAddress(0x235de4 + 1), "Commentary\\hdr.big");
            patch::SetUInt(GfxCoreAddress(0x235e4d + 1), 17);
            patch::SetPointer(GfxCoreAddress(0x235e52 + 1), "data\\audio\\Commentary\\dat.big");
            patch::SetUInt(GfxCoreAddress(0x235e82 + 1), 18);
            patch::SetPointer(GfxCoreAddress(0x235e87 + 1), "data\\audio\\Commentary\\dat.big");
        }
    }
    return result;
}
void PatchGfxCoreHook(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD)
        gOriginalLoadGfxCore = patch::RedirectCall(0x45BBEF, OnLoadGfxCore);

}

class Commentary {
public:
    Commentary() {
        auto v = FM::GetAppVersion();
        PatchGfxCoreHook(v);
    }
};
Commentary commentary;