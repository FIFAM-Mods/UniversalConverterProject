#include "3dMatchStandings.h"
#include "FifamReadWrite.h"
#include "shared.h"
#include "license_check/license_check.h"
#include "GameInterfaces.h"
#include "Competitions.h"
#include "Color.h"

const UInt STANDINGS3D_ORIGINAL_STRUCT_SIZE = 0x4F8;
const UInt NEW_MAX_FONTS = 512;

UChar gNewFonts320[320 * NEW_MAX_FONTS];
UChar gNewFonts8[8 * NEW_MAX_FONTS];

struct PopupData {
    String paramStr;
    UChar paramInt1 = 0;
    UChar paramInt2 = 0;
};

struct PopupCompetitionData {
    UInt compId = 0;
    UShort year = 0;
    PopupData data;
};

Vector<PopupCompetitionData> &GetPopupKitColors() {
    static Vector<PopupCompetitionData> popupKitColors;
    return popupKitColors;
}

Vector<PopupCompetitionData> &GetPopupCustomBadge() {
    static Vector<PopupCompetitionData> popupCustomBadge;
    return popupCustomBadge;
}

Vector<PopupCompetitionData> &GetPopupRecolor() {
    static Vector<PopupCompetitionData> popupRecolor;
    return popupRecolor;
}

PopupData GetPopupDataForCompetition(Vector<PopupCompetitionData> &container, UInt compId) {
    if (compId != 0) {
        for (auto const &s : container) {
            if (s.year == 0 || s.year == GetCurrentYear()) {
                if (s.compId <= 0xFFFF) {
                    if (s.compId == ((compId >> 16) & 0xFFFF))
                        return s.data;
                }
                else if (s.compId == compId)
                    return s.data;
            }
        }
    }
    for (auto const &s : container) {
        if (s.compId == 0)
            return s.data;
    }
    return PopupData();
}

void ReadPopupDataSettingsFile(Vector<PopupCompetitionData> &container, Path const &filename, Bool directories) {
    FifamReader reader(filename);
    if (reader.Available()) {
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                PopupCompetitionData entry;
                reader.ReadLine(Hexadecimal(entry.compId), entry.year, entry.data.paramStr, entry.data.paramInt1, entry.data.paramInt2);
                if (!entry.data.paramStr.empty()) {
                    if (entry.data.paramStr != L"-") {
                        Utils::Replace(entry.data.paramStr, L"/", L"\\");
                        if (directories && !Utils::EndsWith(entry.data.paramStr, L"\\"))
                            entry.data.paramStr += L"\\";
                    }
                    container.push_back(entry);
                }
            }
            else
                reader.SkipLine();
        }
        if (container.size() > 1) {
            std::sort(container.begin(), container.end(), [](PopupCompetitionData const &a, PopupCompetitionData const &b) {
                if (a.year > b.year)
                    return true;
                if (b.year > a.year)
                    return false;
                return a.compId >= b.compId;
            });
        }
    }
}

struct Standings3dAdditionalData {
    void *mImgAddedTime;
    void *mTbAddedTime;
    void *mImgAggregate;
    void *mTbAggregate;
};

Standings3dAdditionalData *GetStandingsAdditionalData(void *standingsInterface) {
    return raw_ptr<Standings3dAdditionalData>(standingsInterface, STANDINGS3D_ORIGINAL_STRUCT_SIZE);
}

void *GetTextBox(void *panelInterface, char const *textBoxName) {
    return CallMethodAndReturn<void *, 0xD44240>(panelInterface, textBoxName);
}

void *GetImage(void *panelInterface, char const *imageName) {
    return CallMethodAndReturn<void *, 0xD44380>(panelInterface, imageName);
}

void *GetOptionalComponent(void *panelInterface, char const *componentName) {
    return CallMethodAndReturn<void *, 0xD34CE0>(panelInterface, componentName);
}

Bool FileExists(String const &filename, Bool makeAbsolutePath = true) {
    void *iSystem = *reinterpret_cast<void **>(0x30ABBC0);
    return CallVirtualMethodAndReturn<Bool, 0>(iSystem, filename.c_str(), makeAbsolutePath);
}

Bool IsSecondLegMatch() {
    void *match = *reinterpret_cast<void **>(0x3124748);
    return match && *raw_ptr<Bool>(match, 0xBB8);
}

void SetWidgetTeamColor(void *widget, String const &resourcePath, void *competition, void *team, Bool awayTeam) {

}

String GetGenericKitColorName(CDBTeamKit *kit, Bool home) {
    UChar kitId = (home == true) ? 0 : 1;
    UChar color1 = 0, color2 = 0;
    switch (kit->GetPartType(kitId, 0)) {
    case 2:
    case 4:
    case 5:
    case 8:
    case 9:
    case 18:
    case 26:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 35:
    case 54:
    case 56:
    case 58:
    case 61:
    case 62:
    case 66:
        color1 = 0;
        color2 = 0;
        break;
    case 1:
    case 6:
    case 10:
    case 11:
    case 13:
    case 20:
    case 27:
    case 33:
    case 34:
    case 37:
    case 38:
    case 39:
    case 40:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 50:
    case 52:
    case 64:
        color1 = 0;
        color2 = 2;
        break;
    case 22:
    case 53:
    case 57:
    case 59:
    case 60:
        color1 = 0;
        color2 = 1;
        break;
    case 23:
        color1 = 2;
        color2 = 0;
        break;
    case 3:
    case 7:
    case 14:
    case 16:
    case 17:
    case 19:
    case 21:
    case 24:
    case 25:
    case 36:
    case 41:
    case 48:
    case 55:
    case 65:
        color1 = 0;
        color2 = 2;
        break;
    case 15:
    case 63:
        color1 = 0;
        color2 = 1;
        break;
    case 12:
    case 49:
    case 51:
        color1 = 0;
        color2 = 1;
        break;
    }
    return Utils::Format(L"clr_%d_%d", kit->GetPartColor(kitId, 0, color1), kit->GetPartColor(kitId, 0, color2));
}

void Process3dMatchScreenExtensions(void *screen,
    char const *homeBadgeNode, char const *awayBadgeNode,
    char const *homeTeamNode, char const *awayTeamNode,
    char const *homeScoreNode, char const *awayScoreNode,
    CTeamIndex _homeTeamID, CTeamIndex _awayTeamID, UInt _compID, bool oneTeam)
{
    Bool reserveHome = false;
    Bool reserveAway = false;
    CDBTeam *homeTeam = nullptr;
    CDBTeam *awayTeam = nullptr;
    UInt compId = 0;
    if (_compID != 0)
        compId = _compID;
    if (!_homeTeamID.isNull()) {
        homeTeam = GetTeam(_homeTeamID);
        reserveHome = _homeTeamID.type == 1;
    }
    if (!oneTeam && !_awayTeamID.isNull()) {
        awayTeam = GetTeam(_awayTeamID);
        reserveAway = _awayTeamID.type == 1;
    }
    if (_compID == 0 || _homeTeamID.isNull()) {
        CDBOneMatch *match = GetCurrentMatch();
        if (match) {
            if (_homeTeamID.isNull()) {
                CTeamIndex homeTeamIndex = match->GetHomeTeamID();
                if (!homeTeamIndex.isNull()) {
                    homeTeam = GetTeam(homeTeamIndex);
                    reserveHome = homeTeamIndex.type == 1;
                }
            }
            if (!oneTeam && _awayTeamID.isNull()) {
                CTeamIndex awayTeamIndex = match->GetAwayTeamID();
                if (!awayTeamIndex.isNull()) {
                    awayTeam = GetTeam(awayTeamIndex);
                    reserveAway = awayTeamIndex.type == 1;
                }
            }
            if (_compID == 0)
                compId = match->GetCompIDInt();
        }
    }
    // kit colors
    if (homeTeam || (!oneTeam && awayTeam)) {
        void *imgKitColor1 = nullptr;
        void *imgKitColor2 = nullptr;
        if (homeTeam)
            imgKitColor1 = GetOptionalComponent(screen, "ImgColor1");
        if (!oneTeam && awayTeam)
            imgKitColor2 = GetOptionalComponent(screen, "ImgColor2");
        if (imgKitColor1 || imgKitColor2) {
            auto colorsData = GetPopupDataForCompetition(GetPopupKitColors(), compId);
            if (!colorsData.paramStr.empty()) {
                UInt homeTeamUId = 0;
                if (imgKitColor1)
                    homeTeamUId = homeTeam->GetTeamUniqueID();
                UInt awayTeamUId = 0;
                if (imgKitColor2)
                    awayTeamUId = awayTeam->GetTeamUniqueID();

                auto GetPathForTeamKitColor = [](String const &dir, UInt teamUId, UInt kitType) {
                    String kitSuffix;
                    if (kitType == 0)
                        kitSuffix = L"_h";
                    else if (kitType == 1)
                        kitSuffix = L"_a";
                    else if (kitType == 2)
                        kitSuffix = L"_t";
                    else
                        return String();
                    return dir + Utils::Format(L"%08X", teamUId) + kitSuffix + L".png";
                };

                // get home/away team kit type (home/away/third)
                void *gfxCoreInterface = *reinterpret_cast<void **>(0x30ABBD0);
                void *aiInterface = CallVirtualMethodAndReturn<void *, 7>(gfxCoreInterface);
                void *matchData = CallVirtualMethodAndReturn<void *, 65>(aiInterface);
                void *kitsData = raw_ptr<void *>(matchData, 0x30);
                UInt homeTeamKitId = *raw_ptr<UInt>(kitsData, 0xED8);
                UInt awayTeamKitId = *raw_ptr<UInt>(kitsData, 0xEDC);

                Bool homeSet = false;
                Bool awaySet = false;

                CDBTeamKit *homeTeamKit = homeTeam->GetKit();
                Bool homeGenericKit = (*raw_ptr<UInt>(homeTeamKit, 4) & 0xC0000000) == 0;
                CDBTeamKit *awayTeamKit = awayTeam->GetKit();
                Bool awayGenericKit = (*raw_ptr<UInt>(awayTeamKit, 4) & 0xC0000000) == 0;

                if (imgKitColor1 && !homeGenericKit) {
                    WideChar const *homeTeamKitPath = raw_ptr<WideChar const>(kitsData, 0x220 + 0x454);
                    if (homeTeamKitPath[0]) {
                        String kitPath = homeTeamKitPath;
                        auto dotPos = kitPath.find(L'.');
                        if (dotPos != String::npos) {
                            kitPath = kitPath.substr(0, dotPos);
                            if (kitPath.size() >= 2 && kitPath[kitPath.size() - 2] == L'_') {
                                if (kitPath[kitPath.size() - 1] == L'h')
                                    homeTeamKitId = 0;
                                else if (kitPath[kitPath.size() - 1] == L'a')
                                    homeTeamKitId = 1;
                                else if (kitPath[kitPath.size() - 1] == L't')
                                    homeTeamKitId = 2;
                            }
                        }
                    }
                    String homeTeamColorPath = GetPathForTeamKitColor(colorsData.paramStr, homeTeamUId, homeTeamKitId);
                    if (!homeTeamColorPath.empty() && FileExists(homeTeamColorPath)) {
                        SetImageFilename(imgKitColor1, homeTeamColorPath);
                        homeSet = true;
                    }
                }
                if (imgKitColor2 && !awayGenericKit) {
                    WideChar const *awayTeamKitPath = raw_ptr<WideChar const>(kitsData, 0x220 + 0x65C + 0x454);
                    if (awayTeamKitPath[0]) {
                        String kitPath = awayTeamKitPath;
                        auto dotPos = kitPath.find(L'.');
                        if (dotPos != String::npos) {
                            kitPath = kitPath.substr(0, dotPos);
                            if (kitPath.size() >= 2 && kitPath[kitPath.size() - 2] == L'_') {
                                if (kitPath[kitPath.size() - 1] == L'h')
                                    awayTeamKitId = 0;
                                else if (kitPath[kitPath.size() - 1] == L'a')
                                    awayTeamKitId = 1;
                                else if (kitPath[kitPath.size() - 1] == L't')
                                    awayTeamKitId = 2;
                            }
                        }
                    }
                    String awayTeamColorPath = GetPathForTeamKitColor(colorsData.paramStr, awayTeamUId, awayTeamKitId);
                    if (!awayTeamColorPath.empty() && FileExists(awayTeamColorPath)) {
                        SetImageFilename(imgKitColor2, awayTeamColorPath);
                        awaySet = true;
                    }
                }
                if (imgKitColor1 && !homeSet) {
                    String homeTeamColorPath = colorsData.paramStr + GetGenericKitColorName(homeTeamKit, homeTeamKitId == 0) + L".png";
                    if (FileExists(homeTeamColorPath))
                        SetImageFilename(imgKitColor1, homeTeamColorPath);
                }
                if (imgKitColor2 && !awaySet) {
                    String awayTeamColorPath = colorsData.paramStr + GetGenericKitColorName(awayTeamKit, awayTeamKitId == 0) + L".png";
                    if (FileExists(awayTeamColorPath))
                        SetImageFilename(imgKitColor2, awayTeamColorPath);
                }
            }
        }
    }
    // custom badges
    if (homeTeam) {
        if (homeBadgeNode) {
            //SafeLog::WriteToFile("popups.log", Format(L"compId: %08X", compId));
            auto badgesData = GetPopupDataForCompetition(GetPopupCustomBadge(), compId);
            //SafeLog::WriteToFile("popups.log", L"badgesPathStr: " + badgesData.paramStr);
            if (!badgesData.paramStr.empty() && badgesData.paramStr != L"-") {
                String homeBadgePath;
                String awayBadgePath;
                void *homeBadge = GetOptionalComponent(screen, homeBadgeNode);
                if (homeBadge) {
                    //SafeLog::WriteToFile("popups.log", L"Home badge available");
                    String uid = Format(L"%08X", homeTeam->GetTeamUniqueID());
                    if (reserveHome) {
                        homeBadgePath = badgesData.paramStr + Format(L"%04d_", GetCurrentYear()) + uid + L"_2.png";
                        if (!FileExists(homeBadgePath)) {
                            homeBadgePath = badgesData.paramStr + uid + L"_2.png";
                            if (!FileExists(homeBadgePath))
                                homeBadgePath.clear();
                        }
                    }
                    if (homeBadgePath.empty()) {
                        homeBadgePath = badgesData.paramStr + Format(L"%04d_", GetCurrentYear()) + uid + L".png";
                        if (!FileExists(homeBadgePath)) {
                            homeBadgePath = badgesData.paramStr + uid + L".png";
                            if (!FileExists(homeBadgePath))
                                homeBadgePath.clear();
                        }
                    }
                    //SafeLog::WriteToFile("popups.log", L"homeBadgePath: " + homeBadgePath);
                }
                void *awayBadge = nullptr;
                if (!oneTeam && awayBadgeNode && awayTeam) {
                    awayBadge = GetOptionalComponent(screen, awayBadgeNode);
                    if (awayBadge) {
                        //SafeLog::WriteToFile("popups.log", L"Away badge available");
                        String uid = Format(L"%08X", awayTeam->GetTeamUniqueID());
                        if (reserveAway) {
                            awayBadgePath = badgesData.paramStr + Format(L"%04d_", GetCurrentYear()) + uid + L"_2.png";
                            if (!FileExists(awayBadgePath)) {
                                awayBadgePath = badgesData.paramStr + uid + L"_2.png";
                                if (!FileExists(awayBadgePath))
                                    awayBadgePath.clear();
                            }
                        }
                        if (awayBadgePath.empty()) {
                            awayBadgePath = badgesData.paramStr + Format(L"%04d_", GetCurrentYear()) + uid + L".png";
                            if (!FileExists(awayBadgePath)) {
                                awayBadgePath = badgesData.paramStr + uid + L".png";
                                if (!FileExists(awayBadgePath))
                                    awayBadgePath.clear();
                            }
                        }
                        //SafeLog::WriteToFile("popups.log", L"awayBadgePath: " + awayBadgePath);
                    }
                }
                if (badgesData.paramInt1 == 0) {
                    if (homeBadge && !homeBadgePath.empty())
                        SetImageFilename(homeBadge, homeBadgePath);
                    if (awayBadge && !awayBadgePath.empty())
                        SetImageFilename(awayBadge, awayBadgePath);
                }
                else if (badgesData.paramInt1 == 1) {
                    if (homeBadge && !homeBadgePath.empty() && (!awayBadge || !awayBadgePath.empty())) {
                        SetImageFilename(homeBadge, homeBadgePath);
                        SetImageFilename(awayBadge, awayBadgePath);
                    }
                }
            }
        }
    }
    // recoloring
    if (homeTeam || (!oneTeam && awayTeam)) {
        //SafeLog::WriteToFile("popups.log", L"performing recolor");
        auto recolorData = GetPopupDataForCompetition(GetPopupRecolor(), compId);
        //SafeLog::WriteToFile("popups.log", L"recolorPathStr: " + recolorData.paramStr);
        if (!recolorData.paramStr.empty()) {
            void *homeTeamName = nullptr;
            void *awayTeamName = nullptr;
            void *homeTeamColor = nullptr;
            void *awayTeamColor = nullptr;
            void *homeScore = nullptr;
            void *awayScore = nullptr;
            if (homeTeam) {
                homeTeamColor = GetOptionalComponent(screen, "ImgTeamColor1");
                if (homeTeamColor) {
                    if (recolorData.paramInt1 != 0 && homeTeamNode)
                        homeTeamName = GetOptionalComponent(screen, homeTeamNode);
                    if (recolorData.paramInt2 != 0 && homeScoreNode)
                        homeScore = GetOptionalComponent(screen, homeScoreNode);
                }
            }
            if (!oneTeam && awayTeam) {
                awayTeamColor = GetOptionalComponent(screen, "ImgTeamColor2");
                if (awayTeamColor) {
                    if (recolorData.paramInt1 != 0 && awayTeamNode)
                        awayTeamName = GetOptionalComponent(screen, awayTeamNode);
                    if (recolorData.paramInt2 != 0 && awayScoreNode)
                        awayScore = GetOptionalComponent(screen, awayScoreNode);
                }
            }
            if (homeTeamColor || awayTeamColor) {
                //SafeLog::WriteToFile("popups.log", L"can perform recolor");
                UInt homeBackgroundColor = 0, homeTextColor = 0, awayBackgroundColor = 0, awayTextColor = 0;
                Bool foundHome = false, foundAway = false;
                if (recolorData.paramStr != L"-") {
                    FifamReader reader(recolorData.paramStr);
                    if (reader.Available()) {
                        while (!reader.IsEof()) {
                            if (!reader.EmptyLine()) {
                                UInt teamId = 0, backgroundColor = 0, textColor = 0;
                                reader.ReadLine(Hexadecimal(teamId), Hexadecimal(backgroundColor), Hexadecimal(textColor));
                                if (!foundHome && homeTeamColor && teamId == homeTeam->GetTeamUniqueID()) {
                                    homeBackgroundColor = backgroundColor;
                                    homeTextColor = textColor;
                                    foundHome = true;
                                    if (!awayTeamColor || foundAway)
                                        break;
                                }
                                if (!foundAway && awayTeamColor && teamId == awayTeam->GetTeamUniqueID()) {
                                    awayBackgroundColor = backgroundColor;
                                    awayTextColor = textColor;
                                    foundAway = true;
                                    if (!homeTeamColor || foundHome)
                                        break;
                                }
                            }
                            else
                                reader.SkipLine();
                        }
                    }
                }
                if (homeTeamColor) {
                    if (!foundHome) {
                        UChar teamColor = homeTeam->GetColor(4);
                        //SafeLog::WriteToFile("popups.log", L"home team recolor from db: " + Format(L"%d", teamColor));
                        if (teamColor > 63)
                            homeBackgroundColor = 0xFF000000;
                        else
                            homeBackgroundColor = 0xFF000000 | (*(UInt *)(0x3125508 + teamColor * 4 + 4 * 4 * 64));
                        if (homeTeamName || homeScore) {
                            if (teamColor > 63)
                                homeTextColor = 0xFFFFFFFF;
                            else {
                                GenColor genBbackgroundColor = GenColor((homeBackgroundColor >> 16) & 0xFF, (homeBackgroundColor >> 8) & 0xFF, homeBackgroundColor & 0xFF);
                                auto white = GenColor::Distance(genBbackgroundColor, GenColor(255, 255, 255));
                                auto black = GenColor::Distance(genBbackgroundColor, GenColor(0, 0, 0));
                                if (white < black)
                                    homeTextColor = 0xFF000000;
                                else
                                    homeTextColor = 0xFFFFFFFF;
                            }
                        }
                    }
                    SetImageColorRGBA(homeTeamColor, homeBackgroundColor);
                    if (homeTeamName)
                        SetTextBoxColorRGBA(homeTeamName, homeTextColor);
                    if (homeScore)
                        SetTextBoxColorRGBA(homeScore, homeTextColor);
                }
                if (awayTeamColor) {
                    if (!foundAway) {
                        UChar teamColor = awayTeam->GetColor(4);
                        //SafeLog::WriteToFile("popups.log", L"away team recolor from db: " + Format(L"%d", teamColor));
                        if (teamColor > 63)
                            awayBackgroundColor = 0xFF000000;
                        else
                            awayBackgroundColor = 0xFF000000 | (*(UInt *)(0x3125508 + teamColor * 4 + 4 * 4 * 64));
                        if (awayTeamName || awayScore) {
                            if (teamColor > 63)
                                awayTextColor = 0xFFFFFFFF;
                            else {
                                GenColor genBbackgroundColor = GenColor((awayBackgroundColor >> 16) & 0xFF, (awayBackgroundColor >> 8) & 0xFF, awayBackgroundColor & 0xFF);
                                auto white = GenColor::Distance(genBbackgroundColor, GenColor(255, 255, 255));
                                auto black = GenColor::Distance(genBbackgroundColor, GenColor(0, 0, 0));
                                if (white < black)
                                    awayTextColor = 0xFF000000;
                                else
                                    awayTextColor = 0xFFFFFFFF;
                            }
                        }
                    }
                    SetImageColorRGBA(awayTeamColor, awayBackgroundColor);
                    if (awayTeamName)
                        SetTextBoxColorRGBA(awayTeamName, awayTextColor);
                    if (awayScore)
                        SetTextBoxColorRGBA(awayScore, awayTextColor);
                }
            }
        }
    }
}

void METHOD OnSetupMatch3DTeamPresentation(void *screen, DUMMY_ARG, CDBOneMatch *match) {
    CallMethod<0xB55900>(screen, match);
    Process3dMatchScreenExtensions(screen, "TrfmTeam1|TbBadge", "TrfmTeam2|TbBadge", "TrfmTeam1|TbName", "TrfmTeam2|TbName", nullptr, nullptr, CTeamIndex::null(), CTeamIndex::null(), 0, false);
}

void METHOD OnSetupMatch3DStatisticsOverlay(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB3BFD0>(screen, data);
    Process3dMatchScreenExtensions(screen, "TbBadgeHome", "TbBadgeAway", "TbClubNameHome", "TbClubNameAway", nullptr, nullptr, CTeamIndex::null(), CTeamIndex::null(), 0, false);
}

void METHOD OnSetupMatch3DCardOverlay(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB14D40>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 6) {
        void *matchController = *raw_ptr<void *>(screen, 0x48C);
        if (matchController) {
            CTeamIndex teamID = CTeamIndex::null();
            CallVirtualMethod<57>(matchController, &teamID, *raw_ptr<UInt>(data, 0x14));
            Process3dMatchScreenExtensions(screen, "TbBadge", nullptr, "TbClubName", nullptr, nullptr, nullptr, teamID, CTeamIndex::null(), 0, true);
        }
    }
}

void METHOD OnSetupMatch3DGoalOverlay(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB15B80>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 1) {
        void *matchController = *raw_ptr<void *>(screen, 0x48C);
        if (matchController) {
            CTeamIndex teamID = CTeamIndex::null();
            CallVirtualMethod<57>(matchController, &teamID, *raw_ptr<UInt>(data, 0x14));
            Process3dMatchScreenExtensions(screen, "TbBadge", nullptr, "TbClubName", nullptr, nullptr, nullptr, teamID, CTeamIndex::null(), 0, true);
        }
    }
}

void METHOD OnSetupMatch3DPitchOverlayStandings(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB719C0>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 0x2000)
        Process3dMatchScreenExtensions(screen, "ImgHomeBadge", "ImgAwayBadge", "TbHomeTeam", "TbAwayTeam", "TbStandingHome", "TbStandingAway", CTeamIndex::null(), CTeamIndex::null(), 0, false);
}

void METHOD OnSetupMatch3DPenaltyShootOut(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB38CE0>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) == 0x80)
        Process3dMatchScreenExtensions(screen, "ImgBadgeHome", "ImgBadgeAway", "TbTeamHome", "TbTeamAway", nullptr, nullptr, CTeamIndex::null(), CTeamIndex::null(), 0, false);
}

void METHOD OnSetupMatch3DSubstitution(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB3D1F0>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 8) {
        void *matchController = *raw_ptr<void *>(screen, 0x48C);
        if (matchController) {
            CTeamIndex teamID = CTeamIndex::null();
            CallVirtualMethod<57>(matchController, &teamID, *raw_ptr<UInt>(data, 0x18));
            if (*raw_ptr<UInt>(data, 0x14) == 1)
                Process3dMatchScreenExtensions(screen, "TbBadge", nullptr, "TbClubName", nullptr, nullptr, nullptr, teamID, CTeamIndex::null(), 0, true);
            else if (*raw_ptr<UInt>(data, 0x14) > 1)
                Process3dMatchScreenExtensions(screen, "TbBadgeSmall", nullptr, "TbClubName", nullptr, nullptr, nullptr, teamID, CTeamIndex::null(), 0, true);
        }
    }
}

void METHOD OnSetupMatch3DManagerSentOff(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xBFF020>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 0x1000) {
        Int employeeId = *raw_ptr<Int>(data, 0x14);
        UInt compId = *raw_ptr<UInt>(data, 0xC);
        CDBEmployee *employee = CallAndReturn<CDBEmployee *, 0xEA2A00>(employeeId);
        if (employee) {
            CTeamIndex teamID = CTeamIndex::null();
            if (CallMethodAndReturn<Bool, 0x12D4AA0>(&compId) && (UChar)CallMethodAndReturn<Int, 0xEA7800>(employee))
                CallMethod<0xEA77C0>(employee, &teamID);
            else
                CallMethod<0xE7EC90>(raw_ptr<void>(employee, 0x10), &teamID);
            Process3dMatchScreenExtensions(screen, "TbBadge", nullptr, "TbClubName", nullptr, nullptr, nullptr, teamID, CTeamIndex::null(), 0, true);
        }
    }
}

void METHOD OnSetuMatch3DHalfTimeStats(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xBF4A80>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 0x10000) {
        Bool process = true;
        Char const *screenName = *raw_ptr<Char const *>(screen, 0x28);
        if (screenName) {
            //Error(screenName);
            StringA screenNameStr = screenName;
            if (screenNameStr.ends_with("_00000000.xml"))
                process = false;
        }
        if (process)
            Process3dMatchScreenExtensions(screen, "ImgHomeBadge", "ImgAwayBadge", "TbHomeTeam", "TbAwayTeam", nullptr, nullptr, CTeamIndex::null(), CTeamIndex::null(), 0, false);
    }
}

void METHOD OnCreateStandingsUI(void *standingsInterface) {
    // create original UI
    CallMethod<0xB3AEE0>(standingsInterface);
    // get additional elements
    auto additionalData = GetStandingsAdditionalData(standingsInterface);
    additionalData->mImgAddedTime = GetOptionalComponent(standingsInterface, "ImgAddedTime");
    additionalData->mTbAddedTime = GetOptionalComponent(standingsInterface, "TbAddedTime");
    additionalData->mImgAggregate = GetOptionalComponent(standingsInterface, "ImgAggregate");
    additionalData->mTbAggregate = GetOptionalComponent(standingsInterface, "TbAggregate");
    // setup added time and aggregate result
    SetVisible(additionalData->mImgAddedTime, false);
    SetVisible(additionalData->mTbAddedTime, false);
    if (IsSecondLegMatch()) {
        SetVisible(additionalData->mImgAggregate, true);
        SetVisible(additionalData->mTbAggregate, true);
        SetText(additionalData->mTbAggregate, L"0-0");
    }
    else {
        SetVisible(additionalData->mImgAggregate, false);
        SetVisible(additionalData->mTbAggregate, false);
    }
    Process3dMatchScreenExtensions(standingsInterface, "ImgHomeBadge", "ImgAwayBadge", "TbHomeTeam", "TbAwayTeam", "TbScoreHome", "TbScoreAway", CTeamIndex::null(), CTeamIndex::null(), 0, false);
}

void UpdateTime(void *standingsInterface) {
    void *tbTime = *raw_ptr<void *>(standingsInterface, 0x4F4);
    auto additionalData = GetStandingsAdditionalData(standingsInterface);
    void *match3dcontroller = *raw_ptr<void *>(standingsInterface, 0x4C8);
    WideChar *time = CallMethodAndReturn<WideChar *, 0xBE5830>(match3dcontroller);
    //SafeLog::Write(time);
    auto timeParts = Utils::Split(time, L'(', true, false);
    if (timeParts.size() == 2) {
        static WideChar timeText[32];
        static WideChar addedTimeText[32];
        wcsncpy(timeText, timeParts[0].c_str(), 31);
        timeText[31] = L'\0';
        SetText(tbTime, timeText);
        if (Utils::EndsWith(timeParts[1], L")"))
            timeParts[1].pop_back();
        wcsncpy(addedTimeText, timeParts[1].c_str(), 31);
        addedTimeText[31] = L'\0';
        SetText(additionalData->mTbAddedTime, addedTimeText);
        SetVisible(additionalData->mImgAddedTime, true);
        SetVisible(additionalData->mTbAddedTime, true);
    }
    else {
        SetVisible(additionalData->mImgAddedTime, false);
        SetVisible(additionalData->mTbAddedTime, false);
        SetText(tbTime, time);
    }
}

void UpdateAggregateScore(void *standingsInterface, WideChar const *goals1, WideChar const *goals2) {
    void *match = *reinterpret_cast<void **>(0x3124748);
    if (match && *raw_ptr<Bool>(match, 0xBB8)) {
        auto additionalData = GetStandingsAdditionalData(standingsInterface);
        UChar result1 = *raw_ptr<UChar>(match, 0x20);
        UChar result2 = *raw_ptr<UChar>(match, 0x21);
        static WideChar aggregateText[32];
        String aggregateStr = Utils::Format(L"(%u-%u)", Utils::SafeConvertInt<UChar>(goals1) + result1, Utils::SafeConvertInt<UChar>(goals2) + result2);
        wcsncpy(aggregateText, aggregateStr.c_str(), 31);
        aggregateText[31] = L'\0';
        SetText(additionalData->mTbAggregate, aggregateText);
    }
}

void UpdateScore(void *standingsInterface) {
    auto additionalData = GetStandingsAdditionalData(standingsInterface);
    void *match3dcontroller = *raw_ptr<void *>(standingsInterface, 0x4C8);
    void *tbScoreHome = *raw_ptr<void *>(standingsInterface, 0x4F0);
    void *tbScoreAway = *raw_ptr<void *>(standingsInterface, 0x4EC);
    WideChar *scoreHome = CallMethodAndReturn<WideChar *, 0xBDE060>(match3dcontroller);
    WideChar *scoreAway = CallMethodAndReturn<WideChar *, 0xBDE1A0>(match3dcontroller);
    SetText(tbScoreHome, scoreHome);
    SetText(tbScoreAway, scoreAway);
    UpdateAggregateScore(standingsInterface, scoreHome, scoreAway);
}

void METHOD OnUpdateTime(void *standingsInterface) {
    UpdateTime(standingsInterface);
    //CallMethod<0xB3A810>(standingsInterface);
    void *match3dcontroller = *raw_ptr<void *>(standingsInterface, 0x4C8);
    if (CallMethodAndReturn<Int, 0xBDE8B0>(match3dcontroller) == 3)
        UpdateScore(standingsInterface);
    else {
        Int evnt = *raw_ptr<Int>(match3dcontroller, 0x1074);
        if (evnt == 4 || evnt == 3)
            UpdateScore(standingsInterface);
    }
}

void METHOD OnUpdateTimeAndScore(void *standingsInterface) {
    UpdateScore(standingsInterface);
    UpdateTime(standingsInterface);
}

void METHOD OnUpdateScore1(void *standingsInterface) {
    UpdateScore(standingsInterface);
}

void METHOD OnUpdateScore2(void *standingsInterface, DUMMY_ARG, Int) {
    UpdateScore(standingsInterface);
}

void METHOD OnRead3dMatchOverlaysConfig(void *t, DUMMY_ARG, Int unk, WideChar const *filename) {
    void *match = *(void **)0x3124748;
    if (match) {
        UInt compId = 0;
        CallMethod<0xE80190>(match, &compId);
        if (compId != 0) {
            //Message(Format(L"compId: %08X", compId));
            UShort year = GetCurrentYear();
            String newfilename = Format(L"fmdata\\popups\\overlay\\%4d_%08X.cfg", year, compId);
            if (!FileExists(newfilename)) {
                if (IsCompetitionLeagueSplit_UInt(compId)) {
                    UInt mainCompId = GetCompetitionLeagueSplitMainLeague(compId);
                    newfilename = Format(L"fmdata\\popups\\overlay\\%4d_%08X.cfg", year, mainCompId);
                    if (!FileExists(newfilename))
                        newfilename.clear();
                }
                else
                    newfilename.clear();
            }
            if (newfilename.empty()) {
                newfilename = Format(L"fmdata\\popups\\overlay\\%4d_%04X.cfg", year, (compId >> 16) & 0xFFFF);
                if (!FileExists(newfilename))
                    newfilename.clear();
            }
            if (newfilename.empty()) {
                newfilename = Format(L"fmdata\\popups\\overlay\\%08X.cfg", compId);
                if (!FileExists(newfilename)) {
                    if (IsCompetitionLeagueSplit_UInt(compId)) {
                        UInt mainCompId = GetCompetitionLeagueSplitMainLeague(compId);
                        newfilename = Format(L"fmdata\\popups\\overlay\\%08X.cfg", mainCompId);
                        if (!FileExists(newfilename))
                            newfilename.clear();
                    }
                    else
                        newfilename.clear();
                }
                if (newfilename.empty()) {
                    newfilename = Format(L"fmdata\\popups\\overlay\\%04X.cfg", (compId >> 16) & 0xFFFF);
                    if (!FileExists(newfilename))
                        newfilename.clear();
                }
            }
            if (!newfilename.empty()) {
                CallMethod<0xB37680>(t, unk, newfilename.c_str());
                //Message(L"OnRead3dMatchOverlaysConfig: %d %s", unk, newfilename.c_str());
                return;
            }
        }
    }
    CallMethod<0xB37680>(t, unk, filename);
    //Message(L"OnRead3dMatchOverlaysConfig: %d %s", unk, filename);
}

void METHOD OnCreateMatch3DPlayerIndicatorUI(void *t) {
    CallMethod<0xC01780>(t);
    if (GetIsSnowy()) {
        SetTextBoxColorRGB(*raw_ptr<void *>(t, 0x52C), 0, 102, 0);
        SetTextBoxColorRGB(*raw_ptr<void *>(t, 0x530), 0, 102, 0);
    }
}

WideChar const *gNew3DScreenNames[] = {
    L"",
    L"12Match3DStandingsOverlay",
    L"12Match3DGoalCardOverlay",
    L"12Match3DLiveTableOverlay",
    L"12Match3DPenaltyShootOutOverlay",
    L"12Match3DStatsOverlay",
    L"12Match3DSubstitutionOverlay",
    L"12Match3DTickerOverlay",
    L"12Match3DTeamPresentation",
    L"12Match3DAfterMatchHighlightsOverlay",
    L"12Match3DHalftimeStats",
    L"12Match3DPitchStandingsOverlay",
};

void *METHOD sub_B24950(void *t, DUMMY_ARG, const char *a2, int a3, int a4, int a5, int a6, char a7) {
    Message(a2);
    return CallMethodAndReturn<void *, 0xB24950>(t, a2, a3, a4, a5, a6, a7);
}

void Patch3dMatchStandings(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadPopupDataSettingsFile(GetPopupKitColors(), "plugins\\ucp\\popup_kitcolors.dat", true);
        ReadPopupDataSettingsFile(GetPopupCustomBadge(), "plugins\\ucp\\popup_badges.dat", true);
        ReadPopupDataSettingsFile(GetPopupRecolor(), "plugins\\ucp\\popup_teamcolors.dat", false);
        // expand struct size
        const UInt newStructSize = STANDINGS3D_ORIGINAL_STRUCT_SIZE + sizeof(Standings3dAdditionalData);
        patch::SetUInt(0xBF68D4 + 1, newStructSize);
        patch::SetUInt(0xBF68DB + 1, newStructSize);
        // install hooks
        patch::SetPointer(0x245D6EC, OnCreateStandingsUI); // UNSAFE HOOK
        patch::SetPointer(0x245D834, OnUpdateTime); // UNSAFE HOOK
        //patch::Nop(0xB3A820, 22);
        patch::RedirectCall(0xB3A791, OnUpdateTimeAndScore); // UNSAFE HOOK
        patch::Nop(0xB3A796, 34);
        //
        patch::RedirectCall(0xB3A787, OnUpdateScore1);
        patch::SetPointer(0x245D868, OnUpdateScore2);

        patch::RedirectCall(0xBFACCB, OnRead3dMatchOverlaysConfig);



        patch::SetPointer(0xB33519 + 1, "Pitca");
        patch::SetPointer(0x4E0260 + 3, gNew3DScreenNames);
        patch::SetUChar(0xB2B92B + 1, 11);
        //injector::WriteMemoryRaw(0x245C4D8, (void *)"Standings", 10, true);
        //patch::SetPointer(0xB719B0 + 1, "screens/12Match3dOverlayKickOffStanding.xml")

        patch::SetPointer(0x246992C, OnCreateMatch3DPlayerIndicatorUI);

        patch::SetPointer(0x246013C, OnSetupMatch3DTeamPresentation);
        patch::SetPointer(0x245DC3C, OnSetupMatch3DStatisticsOverlay);
        patch::SetPointer(0x245A394, OnSetupMatch3DCardOverlay);
        patch::SetPointer(0x245A6A4, OnSetupMatch3DGoalOverlay);
        patch::SetPointer(0x24644AC, OnSetupMatch3DPitchOverlayStandings);
        patch::SetPointer(0x245D08C, OnSetupMatch3DPenaltyShootOut);
        patch::SetPointer(0x245E0AC, OnSetupMatch3DSubstitution);
        patch::SetPointer(0x2468984, OnSetupMatch3DManagerSentOff);
        patch::SetPointer(0x246831C, OnSetuMatch3DHalfTimeStats);

        // remove 1024x1024
        patch::Nop(0xB7274B, 7);

        // fonts
        patch::SetUInt(0x4E77E7 + 1, NEW_MAX_FONTS);
        patch::SetPointer(0x4E77FC + 1, gNewFonts320);
        patch::SetPointer(0x4E77EC + 1, gNewFonts8);
        patch::SetPointer(0x4E7820 + 1, gNewFonts8);

        // ' | ' > ' '
        patch::SetUChar(0x245A4C4 + 2, 0);

        // player name style for popups
        patch::SetUChar(0xD32ED5 + 1, 1);

        // LiveTable - Germany, remove first column white text color
        patch::SetUChar(0xB1F4F5, 0xEB);
        // LiveTable - remove home team color
        patch::SetUChar(0xB1F4B5, 0xEB);
        // LiveTable - remove away team color
        patch::SetUChar(0xB1F4D5, 0xEB);
    }
}
