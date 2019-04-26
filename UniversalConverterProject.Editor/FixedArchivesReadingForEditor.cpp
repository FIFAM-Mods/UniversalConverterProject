#include "FixedArchivesReadingForEditor.h"

using namespace plugin;

void PatchArchivesReadingForEditor(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
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
        patch::SetUChar(0x4B916B + 2, 10);
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
