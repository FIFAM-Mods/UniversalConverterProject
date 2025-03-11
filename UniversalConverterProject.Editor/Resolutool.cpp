#include "Resolutool.h"
#include "FifamTypes.h"
#include "Resolutions.h"
#include "shared.h"

using namespace plugin;

void PrintResolutoolName(WideChar *buf, void *locale) {
    static WideChar const *localeAppNames[] = {
        L"FIFA Manager",
        L"Fussball Manager",
        L"FIFA Manager",
        L"LFP Manager",
    };
    UInt lang = *raw_ptr<UInt>(locale, 4);
    if (lang >= std::size(localeAppNames))
        lang = 0;
    String appName = ToUpper(localeAppNames[lang]) + L' ' + GetPatchName();
    wcscpy(buf, appName.c_str());
}

void PatchResolutool(FM::Version v) {
    if (v.id() == ID_RESOLUTOOL_13_1000) {
        UInt baseAddress = (UInt)GetModuleHandleA(NULL);
        auto ResolutoolAddress = [baseAddress](UInt address) { return baseAddress + (address - 0x400000); };
        struct ResolutoolResolution { UInt width, height, u0, u1; };
        static ResolutoolResolution ResolutoolResolutions[std::size(Resolutions)];
        for (UInt i = 0; i < std::size(Resolutions); i++) {
            ResolutoolResolutions[i].width = Resolutions[i].width;
            ResolutoolResolutions[i].height = Resolutions[i].height;
            ResolutoolResolutions[i].u0 = 0;
            ResolutoolResolutions[i].u1 = 0;
        }
        patch::SetPointer(ResolutoolAddress(0x402131 + 1), ResolutoolResolutions);
        patch::SetPointer(ResolutoolAddress(0x4022D5 + 1), ResolutoolResolutions);
        patch::SetPointer(ResolutoolAddress(0x4022F0 + 1), &ResolutoolResolutions[std::size(ResolutoolResolutions)]);
        patch::SetUChar(ResolutoolAddress(0x4027FB), 0x57); // push edi
        patch::RedirectCall(ResolutoolAddress(0x4027FB + 1), PrintResolutoolName);
        patch::Nop(ResolutoolAddress(0x4027FB + 6), 15);
        patch::SetUChar(ResolutoolAddress(0x402810 + 2), 8); // add esp, 8
    }
}
