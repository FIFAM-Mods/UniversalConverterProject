#include "TheClubScreenExtended.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include <algorithm>

using namespace plugin;

const UInt DEFAULT_CCLUBHISTORY_SIZE = 0x1234;
void *gClubHistoryScreen = nullptr;

void *METHOD OnCreateClubHistoryUI(void *t, DUMMY_ARG, char const *name) {
    *raw_ptr<void *>(t, DEFAULT_CCLUBHISTORY_SIZE + 0x0) = CreateTextBox(t, "TbAddressImage");
    *raw_ptr<void *>(t, DEFAULT_CCLUBHISTORY_SIZE + 0x4) = CreateTextBox(t, "TbMascotImage");
    return CreateTextBox(t, name);
}

bool GetClubScreenExtendedImageFilename(String &out, String const &folder, String const &filename) {
    String filepath = folder + L"\\" + filename;
    out = filepath + L".tga";
    if (FmFileExists(out))
        return true;
    out = filepath + L".png";
    if (FmFileExists(out))
        return true;
    out = filepath + L".jpg";
    if (FmFileExists(out))
        return true;
    return false;
}

WideChar const *METHOD OnGetTeamAddress(CDBTeam *team) {
    if (gClubHistoryScreen) {
        static WideChar const *headquartersPath = L"art\\Lib\\Headquarters";
        static WideChar const *mascotPath = L"art\\Lib\\Mascot";
        static WideChar const *defaultFilename = L"00000000";
        static WideChar const *defaultFilenameHeadquarters = L"art\\Lib\\Headquarters\\00000000.tga";
        static WideChar const *defaultFilenameMascot = L"art\\Lib\\Mascot\\00000000.tga";
        String clubUid = Format(L"%08X", team->GetTeamUniqueID());
        String imgPath;
        void *addressImg = *raw_ptr<void *>(gClubHistoryScreen, DEFAULT_CCLUBHISTORY_SIZE + 0x0);
        if (GetClubScreenExtendedImageFilename(imgPath, headquartersPath, clubUid))
            Call<0xD32860>(addressImg, imgPath.c_str(), 4, 4);
        else if (GetClubScreenExtendedImageFilename(imgPath, headquartersPath, defaultFilename))
            Call<0xD32860>(addressImg, imgPath.c_str(), 4, 4);
        else
            Call<0xD32860>(addressImg, defaultFilenameHeadquarters, 4, 4);
        void *mascotImg = *raw_ptr<void *>(gClubHistoryScreen, DEFAULT_CCLUBHISTORY_SIZE + 0x4);
        if (GetClubScreenExtendedImageFilename(imgPath, mascotPath, clubUid))
            Call<0xD32860>(mascotImg, imgPath.c_str(), 4, 4);
        else if (GetClubScreenExtendedImageFilename(imgPath, mascotPath, defaultFilename))
            Call<0xD32860>(mascotImg, imgPath.c_str(), 4, 4);
        else
            Call<0xD32860>(mascotImg, defaultFilenameMascot, 4, 4);
    }
    String strAddr = CallMethodAndReturn<WideChar const *, 0xED23B0>(team);
    String strBuf;
    for (auto s : strAddr) {
        if (s != L'\r') {
            if (s == L'\n')
                strBuf += L", ";
            else
                strBuf += s;
        }
    }
    static WideChar buf[256];
    wcscpy_s(buf, strBuf.c_str());
    return buf;
}

void METHOD OnSetupClubHistoryScreen(void *t) {
    gClubHistoryScreen = t;
    CallMethod<0x6569B0>(t);
    gClubHistoryScreen = nullptr;
}

void METHOD OnTeamResolveLinks(CDBTeam *team) {
    CallMethod<0xEDC0C0>(team);
    Vector<CTeamIndex> vecTeamIDs;
    CTeamIndex *rivals = raw_ptr<CTeamIndex>(team, 0x438);
    for (UInt i = 0; i < 4; i++) {
        if (rivals[i].countryId != 0)
            vecTeamIDs.push_back(rivals[i]);
    }
    if (!vecTeamIDs.empty()) {
        for (UInt i = 0; i < 4; i++) {
            if (i < vecTeamIDs.size())
                rivals[i] = vecTeamIDs[i];
            else
                rivals[i] = CTeamIndex::make(0, 0, 0);
        }
    }
}

void PatchTheClubScreenExtended(FM::Version v) {
    static UInt NEW_CCLUBHISTORY_SIZE = DEFAULT_CCLUBHISTORY_SIZE + 8;
    patch::SetUInt(0x659AF4 + 1, NEW_CCLUBHISTORY_SIZE);
    patch::SetUInt(0x659AFB + 1, NEW_CCLUBHISTORY_SIZE);
    patch::RedirectCall(0x65565E, OnCreateClubHistoryUI);
    patch::RedirectCall(0x656B96, OnGetTeamAddress);
    patch::RedirectCall(0x6590F5, OnSetupClubHistoryScreen);
    patch::RedirectCall(0xFDC597, OnTeamResolveLinks);
}
