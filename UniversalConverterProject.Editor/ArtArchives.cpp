#include "ArtArchives.h"
#include "FifamTypes.h"
#include "shared.h"

using namespace plugin;

using CEABigDummy = Array<UChar, 0x1050>;

Vector<CEABigDummy> &ArtArchives() {
    static Vector<CEABigDummy> artArchives;
    return artArchives;
}

Int WAccessKitFile(WideChar const *filepath, Int mode) {
    void *app = CallAndReturn<void *, 0x4BE1A0>();
    if (app) {
        if (CallMethodAndReturn<Bool, 0x4BE640>(app, filepath, true))
            return 0;
    }
    return -1;
}

Vector<String> &ArtArchiveNames() {
    static Vector<String> artArchives;
    return artArchives;
}

Vector<String> &PortraitArtNames() {
    static Vector<String> portraitArchives;
    return portraitArchives;
}

void OnGetPortraitsArtFilename(wchar_t *dst, wchar_t const *fmt, unsigned int archiveId) {
    if (archiveId >= 1 && archiveId <= PortraitArtNames().size())
        wcscpy(dst, PortraitArtNames()[archiveId - 1].c_str());
    else
        swprintf(dst, fmt, archiveId);
}

void METHOD OnAddArchive(void *t, DUMMY_ARG, void *archive) {
    CallMethod<0x4C0FF0>(t, archive); // std::vector<CEABig>::push_back()
    for (UInt i = 0; i < ArtArchives().size(); i++) {
        CEABigDummy *archive = &ArtArchives()[i];
        CallMethod<0x409AD0>(archive); // CEABig::CEABig()
        *raw_ptr<UChar>(archive, 0x104A) = 1; // m_bSkipErrorMessages
        CallMethod<0x409B90>(archive, ArtArchiveNames()[i].c_str(), 7); // CEABig::Open()
        *raw_ptr<UChar>(archive, 0x104A) = 0; // m_bSkipErrorMessages
        CallMethod<0x4C0FF0>(t, &archive); // std::vector<CEABig>::push_back()
    }
}

void PatchArtArchives(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        auto allArchives = CollectArtArchives(FM::GetGameDir());
        for (auto const &i : allArchives) {
            if (i != "art_badges.big")
                ArtArchiveNames().push_back(Utils::AtoW(i));
        }
        ArtArchives().resize(ArtArchiveNames().size());
        for (auto const &i : ArtArchiveNames()) {
            if (i.find(L"portraits") != String::npos)
                PortraitArtNames().push_back(i);
        }
        patch::RedirectCall(0x435B0F, WAccessKitFile);
        patch::RedirectCall(0x435B25, WAccessKitFile);
        patch::RedirectCall(0x48710A, WAccessKitFile);
        patch::RedirectCall(0x4B6D32, WAccessKitFile);
        patch::RedirectCall(0x4AC8B9, WAccessKitFile);
        patch::RedirectCall(0x4C15BF, OnAddArchive);
        patch::Nop(0x4C15C4, 0x4C1618 - 0x4C15C4);
        patch::SetPointer(0x50C0C7 + 1, L"art_badges.big");
        patch::SetUChar(0x4B916B + 2, (UChar)(PortraitArtNames().size() + 1));
        patch::RedirectCall(0x4B900E, OnGetPortraitsArtFilename);
        // extended compresion format (16 MB+ files) for BIG files
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
    }
}

void ClearArtArchives() {
    for (CEABigDummy &i : ArtArchives())
        CallMethod<0x408E30>(&i); // CEABig::~CEABig()
}
