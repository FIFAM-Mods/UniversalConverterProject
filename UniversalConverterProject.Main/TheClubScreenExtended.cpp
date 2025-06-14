#include "TheClubScreenExtended.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include <algorithm>

using namespace plugin;

const UInt DEFAULT_CCLUBHISTORY_SIZE = 0x1234;
CXgFMPanel *gClubHistoryScreen = nullptr;

struct ClubHistoryScreenExtension {
    CXgTextBox *TbAddressImage;
    CXgTextBox *TbMascotImage;
    CXgImage *ImgCupTitles[9];
    CXgImage *ImgCupTitlesLong[9];
    CXgTextBox *TbCupTitles[9];
    CXgTextBox *TbCupTitlesLong[9];
};

void *METHOD OnCreateClubHistoryUI(CXgFMPanel *screen, DUMMY_ARG, char const *name) {
    auto ext = raw_ptr<ClubHistoryScreenExtension>(screen, DEFAULT_CCLUBHISTORY_SIZE);
    ext->TbAddressImage = screen->GetTextBox("TbAddressImage");
    ext->TbMascotImage  = screen->GetTextBox("TbMascotImage");
    for (UInt i = 0; i < 9; i++) {
        ext->ImgCupTitles[i] = screen->GetImage(FormatStatic("ImgCupTitles%d", i + 1));
        ext->ImgCupTitlesLong[i] = screen->GetImage(FormatStatic("ImgCupTitlesLong%d", i + 1));
        ext->TbCupTitles[i] = screen->GetTextBox(FormatStatic("TbCupTitles%d", i + 1));
        ext->TbCupTitlesLong[i] = screen->GetTextBox(FormatStatic("TbCupTitlesLong%d", i + 1));
    }
    return screen->GetTextBox(name);
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
        auto ext = raw_ptr<ClubHistoryScreenExtension>(gClubHistoryScreen, DEFAULT_CCLUBHISTORY_SIZE);
        static WideChar const *headquartersPath = L"art\\Lib\\Headquarters";
        static WideChar const *mascotPath = L"art\\Lib\\Mascot";
        static WideChar const *defaultFilename = L"00000000";
        static WideChar const *defaultFilenameHeadquarters = L"art\\Lib\\Headquarters\\00000000.tga";
        static WideChar const *defaultFilenameMascot = L"art\\Lib\\Mascot\\00000000.tga";
        String clubUid = Format(L"%08X", team->GetTeamUniqueID());
        String imgPath;
        if (GetClubScreenExtendedImageFilename(imgPath, headquartersPath, clubUid))
            SetImageFilename(ext->TbAddressImage, imgPath.c_str(), 4, 4);
        else if (GetClubScreenExtendedImageFilename(imgPath, headquartersPath, defaultFilename))
            SetImageFilename(ext->TbAddressImage, imgPath.c_str(), 4, 4);
        else
            SetImageFilename(ext->TbAddressImage, defaultFilenameHeadquarters, 4, 4);
        if (GetClubScreenExtendedImageFilename(imgPath, mascotPath, clubUid))
            SetImageFilename(ext->TbMascotImage, imgPath.c_str(), 4, 4);
        else if (GetClubScreenExtendedImageFilename(imgPath, mascotPath, defaultFilename))
            SetImageFilename(ext->TbMascotImage, imgPath.c_str(), 4, 4);
        else
            SetImageFilename(ext->TbMascotImage, defaultFilenameMascot, 4, 4);
        auto vecWins = raw_ptr<FmVec<UShort>>(gClubHistoryScreen, 0x48C);
        for (UInt i = 0; i < 9; i++) {
            if (vecWins[i].empty()) {
                ext->ImgCupTitles[i]->SetVisible(false);
                ext->TbCupTitles[i]->SetVisible(false);
                ext->ImgCupTitlesLong[i]->SetVisible(false);
                ext->TbCupTitlesLong[i]->SetVisible(false);
            }
            else if (vecWins[i].size() < 100) {
                ext->ImgCupTitles[i]->SetVisible(true);
                ext->TbCupTitles[i]->SetVisible(true);
                ext->ImgCupTitlesLong[i]->SetVisible(false);
                ext->TbCupTitlesLong[i]->SetVisible(false);
                ext->TbCupTitles[i]->SetText(FormatStatic(L"%d", vecWins[i].size()));
                ext->TbCupTitlesLong[i]->SetText(L"");
            }
            else {
                ext->ImgCupTitles[i]->SetVisible(false);
                ext->TbCupTitles[i]->SetVisible(false);
                ext->ImgCupTitlesLong[i]->SetVisible(true);
                ext->TbCupTitlesLong[i]->SetVisible(true);
                ext->TbCupTitles[i]->SetText(L"");
                ext->TbCupTitlesLong[i]->SetText(FormatStatic(L"%d", vecWins[i].size()));
            }
        }
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

void METHOD OnSetupClubHistoryScreen(CXgFMPanel *screen) {
    gClubHistoryScreen = screen;
    CallMethod<0x6569B0>(screen);
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
    if (v.id() == ID_FM_13_1030_RLD) {
        static UInt NEW_CCLUBHISTORY_SIZE = DEFAULT_CCLUBHISTORY_SIZE + sizeof(ClubHistoryScreenExtension);
        patch::SetUInt(0x659AF4 + 1, NEW_CCLUBHISTORY_SIZE);
        patch::SetUInt(0x659AFB + 1, NEW_CCLUBHISTORY_SIZE);
        patch::RedirectCall(0x65565E, OnCreateClubHistoryUI);
        patch::RedirectCall(0x656B96, OnGetTeamAddress);
        patch::RedirectCall(0x6590F5, OnSetupClubHistoryScreen);
        patch::RedirectCall(0xFDC597, OnTeamResolveLinks);
    }
}
