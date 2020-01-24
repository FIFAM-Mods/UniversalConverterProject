#include "FixedArchivesReadingForEditor.h"
#include "FifamTypes.h"
#include "license_check/license_check.h"

using namespace plugin;

Bool METHOD CBL2002Adapter__FileExists(void *t, DUMMY_ARG, const WideChar *out, Char a3) {
    bool result = CallMethodAndReturn<bool, 0x4BEEB0>(t, out, a3);
    Message(L"%s\n%d", out, result);
    return result;
}

Int WAccessKitFile(WideChar const *filepath, Int mode) {
    void *app = CallAndReturn<void *, 0x4BE1A0>();
    if (app) {
        if (CallMethodAndReturn<Bool, 0x4BE640>(app, filepath, true))
            return 0;
    }
    return -1;
}

void PatchArchivesReadingForEditor(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::RedirectCall(0x435B0F, WAccessKitFile);
        patch::RedirectCall(0x435B25, WAccessKitFile);
        patch::RedirectCall(0x48710A, WAccessKitFile);
        patch::RedirectCall(0x4B6D32, WAccessKitFile);
        patch::RedirectCall(0x4AC8B9, WAccessKitFile);
        //patch::SetUChar(0x50C1E5 + 1, 3);
        //patch::SetPointer(0x6685B8, CBL2002Adapter__FileExists);
        static String art08name = Magic<'a','r','t','_','0','8','.','b','i','g'>(2619068061);
        patch::SetPointer(0x50C307 + 1, (void *)art08name.c_str());
        // fix badges reading
        using Cdecl1 = bool(__cdecl *)(unsigned char *);
        patch::RedirectJump(0x404F50, (Cdecl1)([](unsigned char *data) {
            return data && data[1] == 0xFB && (data[0] == 0x10 || data[0] == 0x90);
        }));
        using Cdecl2 = int(__cdecl *)(unsigned char *);
        patch::RedirectJump(0x404F70, (Cdecl2)([](unsigned char *data) {
            if (data && data[1] == 0xFB) {
                if (data[0] == 0x10)
                    return data[4] + ((data[3] + (data[2] << 8)) << 8);
                else if (data[0] == 0x90)
                    return data[5] + ((data[4] + ((data[3] + (data[2] << 8)) << 8)) << 8);
            }
            return 0;
        }));
        // add more archives for portraits
        patch::SetUChar(0x4B916B + 2, 8);
    }
    else if (v.id() == ID_ED_11_1003) {
        // fix badges reading
        using Cdecl1 = bool(__cdecl *)(unsigned char *);
        patch::RedirectJump(0x401020, (Cdecl1)([](unsigned char *data) {
            return data && data[1] == 0xFB && (data[0] == 0x10 || data[0] == 0x90);
        }));
        using Cdecl2 = int(__cdecl *)(unsigned char *);
        patch::RedirectJump(0x401040, (Cdecl2)([](unsigned char *data) {
            if (data && data[1] == 0xFB) {
                if (data[0] == 0x10)
                    return data[4] + ((data[3] + (data[2] << 8)) << 8);
                else if (data[0] == 0x90)
                    return data[5] + ((data[4] + ((data[3] + (data[2] << 8)) << 8)) << 8);
            }
            return 0;
        }));
        // add more archives for portraits
        patch::SetUChar(0x498FAC + 2, 10);
    }
}
