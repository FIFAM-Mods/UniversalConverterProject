#include "FixedArchivesReadingForEditor.h"
#include "FifamTypes.h"
#include "license_check/license_check.h"

using namespace plugin;

static UChar gUpdateBig[0x1050] = {};

void DestroyUpdateBig() {
    CallMethod<0x408E30>(gUpdateBig);
}

const UChar *GetUpdateBig() {
    static Bool initialized = false;
    if (!initialized) {
        initialized = true;
        CallMethod<0x409AD0>(gUpdateBig);
        Call<0x5FBB38>(DestroyUpdateBig);
        *raw_ptr<UChar>(gUpdateBig, 0x104A) = 1;
        CallMethod<0x409B90>(gUpdateBig, L"update.big", 7);
        *raw_ptr<UChar>(gUpdateBig, 0x104A) = 0;
    }
    return gUpdateBig;
}

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

void OnGetPortraitsArtFilename(wchar_t *dst, wchar_t const *fmt, unsigned int archiveId) {
    switch (archiveId) {
    case 1:
        wcscpy(dst, L"update_portraits.big");
        break;
    case 2:
        wcscpy(dst, L"update.big");
        break;
    case 3:
        wcscpy(dst, L"art_02.big");
        break;
    case 4:
        wcscpy(dst, L"art_03.big");
        break;
    case 5:
        wcscpy(dst, L"art_05.big");
        break;
    case 6:
        wcscpy(dst, L"art_06.big");
        break;
    case 7:
        wcscpy(dst, L"art_07.big");
        break;
    default:
        wcscpy(dst, L"art_02.big");
        break;
    }
}

void METHOD OnAddArchive(void *t, DUMMY_ARG, void *archive) {
    CallMethod<0x4C0FF0>(t, archive);
    UChar const *updateArchive = GetUpdateBig();
    CallMethod<0x4C0FF0>(t, &updateArchive);
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
        patch::SetPointer(0x50C157 + 1, L"art_08.big");
        patch::SetPointer(0x50C307 + 1, L"art_09.big");
        patch::RedirectCall(0x4C1613, OnAddArchive);
        
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
        patch::RedirectCall(0x4B900E, OnGetPortraitsArtFilename);
    }
}
