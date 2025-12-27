#include "FixedArchivesReadingForEditor.h"
#include "FifamTypes.h"

using namespace plugin;

#define NUM_BIG_ARCHIVE_UPDATES 2

static UChar gUpdateBig[NUM_BIG_ARCHIVE_UPDATES][0x1050] = {};

template<UInt Id>
void DestroyUpdateBig() {
    CallMethod<0x408E30>(gUpdateBig[Id]);
}

template<UInt Id>
const UChar *GetUpdateBig() {
    static Bool initialized = false;
    if (!initialized) {
        initialized = true;
        CallMethod<0x409AD0>(gUpdateBig[Id]);
        Call<0x5FBB38>(&DestroyUpdateBig<Id>);
        *raw_ptr<UChar>(gUpdateBig[Id], 0x104A) = 1;
        CallMethod<0x409B90>(gUpdateBig[Id], Id == 0 ? L"update.big" : L"update2.big", 7);
        *raw_ptr<UChar>(gUpdateBig[Id], 0x104A) = 0;
    }
    return gUpdateBig[Id];
}

#define NUM_BIG_ARCHIVE_ARTS 3

static UChar gArtBig[NUM_BIG_ARCHIVE_ARTS][0x1050] = {};

template<UInt Id>
void DestroyArtBig() {
    CallMethod<0x408E30>(gArtBig[Id]);
}

template<UInt Id>
const UChar *GetArtBig() {
    static Bool initialized = false;
    if (!initialized) {
        initialized = true;
        CallMethod<0x409AD0>(gArtBig[Id]);
        Call<0x5FBB38>(&DestroyArtBig<Id>);
        *raw_ptr<UChar>(gArtBig[Id], 0x104A) = 1;
        static WideChar const *artName[] = { L"art_11.big", L"art_12.big", L"art_13.big" };
        CallMethod<0x409B90>(gArtBig[Id], artName[Id], 7);
        *raw_ptr<UChar>(gArtBig[Id], 0x104A) = 0;
    }
    return gArtBig[Id];
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

WideChar const *portraitsArts[] = {
    L"update_portraits2.big", L"update2.big", L"update_portraits.big", L"update.big",
    L"art_02.big", L"art_03.big", L"art_06.big", L"art_07.big", L"art_08.big", L"art_09.big", L"art_10.big",
    L"art_14.big", L"art_15.big", L"art_16.big", L"art_17.big", L"art_18.big"
};

void OnGetPortraitsArtFilename(wchar_t *dst, wchar_t const *fmt, unsigned int archiveId) {
    if (archiveId >= 1 && archiveId <= std::size(portraitsArts))
        wcscpy(dst, portraitsArts[archiveId - 1]);
    else
        wcscpy(dst, L"art_02.big");
}

void METHOD OnAddArchive(void *t, DUMMY_ARG, void *archive) {
    UChar const *update2Archive = GetUpdateBig<1>();
    CallMethod<0x4C0FF0>(t, &update2Archive);
    UChar const *update1Archive = GetUpdateBig<0>();
    CallMethod<0x4C0FF0>(t, &update1Archive);
	UChar const *art11 = GetArtBig<0>();
    CallMethod<0x4C0FF0>(t, &art11);
	UChar const *art12 = GetArtBig<1>();
    CallMethod<0x4C0FF0>(t, &art12);
    UChar const *art13 = GetArtBig<2>();
    CallMethod<0x4C0FF0>(t, &art13);
    CallMethod<0x4C0FF0>(t, archive);
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
        //patch::SetPointer(0x50C157 + 1, L"art_11.big");
        //patch::SetPointer(0x50C307 + 1, L"art_12.big");
        patch::RedirectCall(0x4C15BF, OnAddArchive);
        
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
        patch::SetUChar(0x4B916B + 2, (UChar)std::size(portraitsArts) + 1);
        patch::RedirectCall(0x4B900E, OnGetPortraitsArtFilename);
    }
}
