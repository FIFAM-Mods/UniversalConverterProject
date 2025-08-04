#include "3dMatchStandings.h"
#include "FifamReadWrite.h"
#include "shared.h"
#include "license_check/license_check.h"
#include "GameInterfaces.h"
#include "Competitions.h"
#include "Kits.h"
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

struct Standings3dCard {
    UChar mStatus; // 0 - not enabled, 1 - enabled, 2 - shown
    UInt mEnableTime;
    void *mpCard;
};

struct Standings3dAdditionalData {
    void *mImgAddedTime;
    void *mTbAddedTime;
    void *mImgAggregate;
    void *mTbAggregate;
    Bool mHasRedCards;
    Standings3dCard mCards[2][3];
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

Bool IsSecondLegMatch() {
    void *match = *reinterpret_cast<void **>(0x3124748);
    return match && *raw_ptr<Bool>(match, 0xBB8);
}

String GetGenericKitColorName(CDBTeamKit *kit, Bool home) {
    Pair<UInt, UInt> color = GetGenericKitColorIDs(kit, home);
    return Utils::Format(L"clr_%d_%d", color.first, color.second);
}

class PopupRecolorProcessor {
public:
    struct ColorsData {
        Array<UInt, 10> mTeamColor = {};
        Bool mHasTetamColors = false;
        Array<UInt, 2> mKitColor = {};
        Bool mHasKitColors = false;
    };
    ColorsData mColors[2];
    UInt mTeamIDs[2] = {};
    Bool mReserve[2] = {};
    Bool mOneTeam = false;

    virtual UInt Process(CGuiNode *guiNode) {
        void *guiObjectNode = CallVirtualMethodAndReturn<void *, 16>(guiNode); // guiNode->AsGuiObjectNode()
        if (guiObjectNode) {
            void *metadata = CallVirtualMethodAndReturn<void *, 11>(guiObjectNode); // guiObjectNode->Metadata()
            if (metadata) {
                void *popupColorEntry = CallVirtualMethodAndReturn<void *, 7>(metadata, "PopupColor"); // metadata->GetEntryByKey()
                if (popupColorEntry) {
                    char const *popupColorType = CallMethodAndReturn<char const *, 0x511630>(popupColorEntry); // popupColorEntry->AsString()
                    if (popupColorType) {
                        void *control = CallVirtualMethodAndReturn<void *, 29>(guiObjectNode); // guiObjectNode->GetControl()
                        if (control) {
                            void *image = CallAndReturn<void *, 0x1448819>(control); // CastToImage()
                            void *textBox = CallAndReturn<void *, 0x1442B60>(control); // CastToTextBox()
                            if (image || textBox) {
                                StringA typeStr = ToLower(popupColorType);
                                Int teamIndex = -1;
                                if (Utils::StartsWith(typeStr, "home"))
                                    teamIndex = 0;
                                else if (Utils::StartsWith(typeStr, "away"))
                                    teamIndex = 1;
                                if (mOneTeam) {
                                    if (teamIndex != -1)
                                        return 0;
                                    teamIndex = 0;
                                }
                                else if (teamIndex == -1)
                                    return 0;
                                if (mColors[teamIndex].mHasTetamColors) {
                                    for (UInt colorId = 1; colorId <= 10; colorId++) {
                                        if (Utils::EndsWith(typeStr, "teamcolor" + std::to_string(colorId))) {
                                            if (image)
                                                SetImageColorRGBA(image, mColors[teamIndex].mTeamColor[colorId - 1]);
                                            else if (textBox)
                                                SetTextBoxColorRGBA(textBox, mColors[teamIndex].mTeamColor[colorId - 1]);
                                            //::Message("Set team color %s %X", typeStr.c_str(), mColors[teamIndex].mTeamColor[colorId - 1]);
                                            return 0;
                                        }
                                    }
                                }
                                if (mColors[teamIndex].mHasKitColors) {
                                    for (UInt colorId = 1; colorId <= 2; colorId++) {
                                        if (Utils::EndsWith(typeStr, "kitcolor" + std::to_string(colorId))) {
                                            if (image)
                                                SetImageColorRGBA(image, mColors[teamIndex].mKitColor[colorId - 1]);
                                            else if (textBox)
                                                SetTextBoxColorRGBA(textBox, mColors[teamIndex].mKitColor[colorId - 1]);
                                            return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                UChar teamIndex = 0;
                if (!mOneTeam) {
                    void *dataEntry = CallVirtualMethodAndReturn<void *, 7>(metadata, "Data"); // metadata->GetEntryByKey()
                    if (dataEntry) {
                        char const *dataType = CallMethodAndReturn<char const *, 0x511630>(dataEntry); // popupColorEntry->AsString()
                        if (dataType && ToLower(dataType) == "awayteam")
                            teamIndex = 1;
                    }
                    if (guiNode->GetUid() && strstr(guiNode->GetUid(), "Away"))
                        teamIndex = 1;
                }
                void *customBadgeDirEntry = CallVirtualMethodAndReturn<void *, 7>(metadata, "CustomBadgeDir"); // metadata->GetEntryByKey()
                if (customBadgeDirEntry) {
                    char const *customBadgeDir = CallMethodAndReturn<char const *, 0x511630>(customBadgeDirEntry); // popupColorEntry->AsString()
                    if (customBadgeDir) {
                        void *control = CallVirtualMethodAndReturn<void *, 29>(guiObjectNode); // guiObjectNode->GetControl()
                        if (control) {
                            void *image = CallAndReturn<void *, 0x1448819>(control); // CastToImage()
                            void *textBox = CallAndReturn<void *, 0x1442B60>(control); // CastToTextBox()
                            control = image ? image : textBox;
                            if (control) {
                                Bool badgeSet = false;
                                if (mTeamIDs[teamIndex]) {
                                    String uid = Format(L"%08X", mTeamIDs[teamIndex]);
                                    if (mReserve[teamIndex]) {
                                        String badgePath = Utils::AtoW(customBadgeDir) + L"\\" + uid + L"_2.png";
                                        if (FmFileExists(badgePath)) {
                                            SetImageFilename(control, badgePath);
                                            badgeSet = true;
                                        }
                                    }
                                    if (!badgeSet) {
                                        String badgePath = Utils::AtoW(customBadgeDir) + L"\\" + uid + L".png";
                                        if (FmFileExists(badgePath)) {
                                            SetImageFilename(control, badgePath);
                                            badgeSet = true;
                                        }
                                    }
                                }
                                if (!badgeSet) {
                                    String badgePath = Utils::AtoW(customBadgeDir) + L"\\00000000.png";
                                    if (FmFileExists(badgePath)) {
                                        SetImageFilename(control, badgePath);
                                        badgeSet = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
};

Map<UInt, Map<UInt, Array<StringA, 10>>> ReadTeamColors(Path const &colorsFilePath) {
    Map<UInt, Map<UInt, Array<StringA, 10>>> result;
    FifamReader reader(colorsFilePath);
    if (reader.Available()) {
        reader.SkipLine();
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                UInt teamId = 0;
                StringA typeStr;
                Array<StringA, 10> colors;
                reader.ReadLine(Hexadecimal(teamId), typeStr, colors[0], colors[1], colors[2], colors[3], colors[4], colors[5],
                    colors[6], colors[7], colors[8], colors[9]);
                typeStr = ToLower(typeStr);
                Int type = -1;
                if (typeStr == "home")
                    type = 0;
                else if (typeStr == "away")
                    type = 1;
                else if (typeStr == "third")
                    type = 3;
                else if (typeStr == "all")
                    type = 4;
                if (type != -1)
                    result[teamId][type] = colors;
            }
            else
                reader.SkipLine();
        }
    }
    return result;
}

GenColor GenColorFromInt(UInt clrValue) {
    return GenColor((clrValue >> 16) & 0xFF, (clrValue >> 8) & 0xFF, clrValue & 0xFF);
}

UInt GenColorToInt(GenColor &clr) {
    return 0xFF000000 | (clr.r << 16) | (clr.g << 8) | clr.b;
}

UInt GetTextColor(UInt clr, Bool grey = false) {
    auto genColor = GenColorFromInt(clr);
    auto white = GenColor::Distance(genColor, GenColor(255, 255, 255));
    auto black = GenColor::Distance(genColor, GenColor(0, 0, 0));
    if (white < black)
        return grey ? 0xFF202020 : 0xFF000000;
    return grey ? 0xFFF0F0F0 : 0xFFFFFFFF;
}

UInt GetAltColor(UInt clr) {
    auto genClr = GenColorFromInt(clr);
    if (genClr.r < 20 && genClr.g < 20 && genClr.b < 20) {
        genClr.r = (UChar)(roundf((Float)genClr.r * 0.85f)) + 38;
        genClr.g = (UChar)(roundf((Float)genClr.g * 0.85f)) + 38;
        genClr.b = (UChar)(roundf((Float)genClr.b * 0.85f)) + 38;
    }
    else {
        genClr.r = (UChar)(roundf((Float)genClr.r * 0.7f));
        genClr.g = (UChar)(roundf((Float)genClr.g * 0.7f));
        genClr.b = (UChar)(roundf((Float)genClr.b * 0.7f));
    }
    return GenColorToInt(genClr);
}

void Process3dMatchScreenExtensions(void *screen, char const *homeBadgeNode, char const *awayBadgeNode,
    CTeamIndex _homeTeamID, CTeamIndex _awayTeamID, UInt _compID, bool oneTeam)
{
    Bool isReserve[2] = { false, false };
    CDBTeam *team[2] = { nullptr, nullptr };
    UInt compId = 0;
    if (_compID != 0)
        compId = _compID;
    if (!_homeTeamID.isNull()) {
        team[0] = GetTeam(_homeTeamID);
        isReserve[0] = _homeTeamID.type == 1;
    }
    if (!oneTeam && !_awayTeamID.isNull()) {
        team[1] = GetTeam(_awayTeamID);
        isReserve[1] = _awayTeamID.type == 1;
    }
    if (_compID == 0 || _homeTeamID.isNull()) {
        CDBOneMatch *match = GetCurrentMatch();
        if (match) {
            if (_homeTeamID.isNull()) {
                CTeamIndex homeTeamIndex = match->GetHomeTeamID();
                if (!homeTeamIndex.isNull()) {
                    team[0] = GetTeam(homeTeamIndex);
                    isReserve[0] = homeTeamIndex.type == 1;
                }
            }
            if (!oneTeam && _awayTeamID.isNull()) {
                CTeamIndex awayTeamIndex = match->GetAwayTeamID();
                if (!awayTeamIndex.isNull()) {
                    team[1] = GetTeam(awayTeamIndex);
                    isReserve[1] = awayTeamIndex.type == 1;
                }
            }
            if (_compID == 0)
                compId = match->GetCompIDInt();
        }
    }
    if (!team[0] && !team[1])
        return;
    // get home/away team kit type (home/away/third)
    UInt kitType[2] = { 0, 0 };
    CDBTeam *teamForKitType[2] = { team[0], team[1] };
    Bool teamsSwapped = false;
    CDBOneMatch *match = GetCurrentMatch();
    if (match) {
        if (!_homeTeamID.isNull()) {
            CTeamIndex awayTeamIndex = match->GetAwayTeamID();
            if (_homeTeamID == awayTeamIndex) {
                std::swap(teamForKitType[0], teamForKitType[1]);
                teamsSwapped = true;
            }
        }
    }
    void *gfxCoreInterface = *reinterpret_cast<void **>(0x30ABBD0);
    void *aiInterface = CallVirtualMethodAndReturn<void *, 7>(gfxCoreInterface);
    void *matchData = CallVirtualMethodAndReturn<void *, 65>(aiInterface);
    void *kitsData = raw_ptr<void *>(matchData, 0x30);
    for (UInt t = 0; t < 2; t++) {
        if (teamForKitType[t]) {
            kitType[t] = *raw_ptr<UInt>(kitsData, (t == 0) ? 0xED8 : 0xEDC);
            CDBTeamKit *kit = teamForKitType[t]->GetKit();
            Bool isGeneric = (*raw_ptr<UInt>(kit, 4) & 0xC0000000) == 0;
            if (!isGeneric) {
                WideChar const *teamKitPath = raw_ptr<WideChar const>(kitsData, (t == 0) ? (0x220 + 0x454) : (0x220 + 0x65C + 0x454));
                if (teamKitPath[0]) {
                    String kitPath = teamKitPath;
                    auto dotPos = kitPath.find(L'.');
                    if (dotPos != String::npos) {
                        kitPath = kitPath.substr(0, dotPos);
                        if (kitPath.size() >= 2 && kitPath[kitPath.size() - 2] == L'_') {
                            if (kitPath[kitPath.size() - 1] == L'h')
                                kitType[t] = 0;
                            else if (kitPath[kitPath.size() - 1] == L'a')
                                kitType[t] = 1;
                            else if (kitPath[kitPath.size() - 1] == L't')
                                kitType[t] = 3;
                        }
                    }
                }
            }
        }
    }
    if (teamsSwapped)
        std::swap(kitType[0], kitType[1]);
    // kit colors
    auto colorsData = GetPopupDataForCompetition(GetPopupKitColors(), compId);
    for (UInt t = 0; t < 2; t++) {
        Vector<void *> imgKit;
        Vector<void *> imgGenKit;
        StringA imgNamePrefix;
        if (!oneTeam)
            imgNamePrefix = (t == 0) ? "Home" : "Away";
        for (UInt i = 0; i < 2; i++) {
            for (UInt k = 0; k <= 3; k++) {
                StringA imgName = imgNamePrefix + ((i == 0) ? "Kit" : "GenKit");
                if (k != 0)
                    imgName += std::to_string(k);
                void *img = GetOptionalComponent(screen, imgName.c_str());
                if (img) {
                    if (i == 0)
                        imgKit.push_back(img);
                    else
                        imgGenKit.push_back(img);
                }
            }
        }
        Bool customKit = false;
        if (!colorsData.paramStr.empty() && !imgKit.empty()) {
            String teamColorPath = GetPathForTeamKitColor(colorsData.paramStr, team[t]->GetTeamUniqueID(), kitType[t]);
            if (!teamColorPath.empty() && FmFileExists(teamColorPath)) {
                for (auto k : imgKit)
                    SetImageFilename(k, teamColorPath);
                customKit = true;
            }
            else {
                String teamColorPath = colorsData.paramStr + GetGenericKitColorName(team[t]->GetKit(), kitType[t] == 0) + L".png";
                if (FmFileExists(teamColorPath)) {
                    for (auto k : imgKit)
                        SetImageFilename(k, teamColorPath);
                    customKit = true;
                }
            }
            for (auto k : imgKit)
                SetVisible(k, customKit);
        }
        for (auto k : imgGenKit)
            SetVisible(k, !customKit);
    }
    // recoloring and custom badges
    PopupRecolorProcessor popupProcessor;
    popupProcessor.mOneTeam = oneTeam;
    popupProcessor.mReserve[0] = isReserve[0];
    popupProcessor.mReserve[1] = isReserve[1];
    popupProcessor.mTeamIDs[0] = team[0] ? team[0]->GetTeamUniqueID() : 0;
    popupProcessor.mTeamIDs[1] = team[1] ? team[1]->GetTeamUniqueID() : 0;
    Map<UInt, Map<UInt, Array<StringA, 10>>> teamColors;
    auto recolorData = GetPopupDataForCompetition(GetPopupRecolor(), compId);
    if (!recolorData.paramStr.empty() && recolorData.paramStr != L"-")
        teamColors = ReadTeamColors(recolorData.paramStr);
    for (UInt t = 0; t < 2; t++) {
        if (team[t]) {
            Pair<UInt, UInt> kitColorId = GetGenericKitColorIDs(team[t]->GetKit(), kitType[t] != 1);
            Pair<UInt, UInt> kitColor = {
                0xFF000000 | *(UInt *)(0x3080410 + kitColorId.first * 4),
                0xFF000000 | *(UInt *)(0x3080410 + kitColorId.second * 4)
            };
            popupProcessor.mColors[t].mHasKitColors = true;
            popupProcessor.mColors[t].mKitColor[0] = kitColor.first;
            popupProcessor.mColors[t].mKitColor[1] = kitColor.second;
            Bool hasColors = false;
            Array<StringA, 10> colors;
            for (UInt k = 0; k < 2; k++) {
                if (!hasColors) {
                    UInt teamId = (k == 0) ? team[t]->GetTeamUniqueID() : 0;
                    if (Utils::Contains(teamColors, teamId)) {
                        Int teamKitType = -1;
                        if (Utils::Contains(teamColors[teamId], kitType[t]))
                            teamKitType = kitType[t];
                        else if (Utils::Contains(teamColors[teamId], 4u))
                            teamKitType = 4;
                        if (teamKitType != -1) {
                            colors = teamColors[teamId][teamKitType];
                            hasColors = true;
                        }
                    }
                }
            }
            if (hasColors) {
                auto WHITE = GenColor(255, 255, 255);
                auto BLACK = GenColor(0, 0, 0);
                UInt clrTeam1 = 0xFF000000 | team[t]->GetColorRGBA(0);
                UInt clrTeam2 = 0xFF000000 | team[t]->GetColorRGBA(1);
                auto kitDiff = Color::Distance(GenColorFromInt(kitColor.first), GenColorFromInt(kitColor.second));
                UInt eplColor = kitColor.second;
                if (kitDiff < 100) {
                    eplColor = GetAltColor(kitColor.first);
                    if (Color::Distance(GenColorFromInt(kitColor.first), GenColorFromInt(eplColor)) < 100)
                        eplColor = GetTextColor(kitColor.first, true);
                }
                UInt epl_1 = eplColor;
                UInt epl_2 = 0;
                if (GenColor::Distance(GenColorFromInt(eplColor), WHITE) < 200) {
                    if (GenColor::Distance(GenColorFromInt(kitColor.first), WHITE) < 200) {
                        epl_1 = 0xFF202020;
                        epl_2 = 0;
                    }
                    else {
                        epl_1 = 0;
                        epl_2 = eplColor;
                    }
                }
                UInt eplPresentation = kitColor.first;
                if (GenColor::Distance(GenColorFromInt(eplPresentation), WHITE) < 200) {
                    eplPresentation = kitColor.second;
                    if (GenColor::Distance(GenColorFromInt(eplPresentation), WHITE) < 200)
                        eplPresentation = 0xFF202020;
                }
                popupProcessor.mColors[t].mHasTetamColors = true;
                for (UInt c = 0; c < 10; c++) {
                    auto clr = ToLower(colors[c]);
                    UInt clrValue = 0;
                    if (Utils::StartsWith(clr, "team_interface_color"))
                        clrValue = 0xFF000000 | team[t]->GetColorRGBA(4);
                    else if (Utils::StartsWith(clr, "team_background"))
                        clrValue = clrTeam1;
                    else if (Utils::StartsWith(clr, "team_foreground")) {
                        clrValue = clrTeam2;
                        if (Color::Distance(GenColorFromInt(clrTeam1), GenColorFromInt(clrTeam2)) < 100)
                            clrValue = GetTextColor(clrTeam1);
                    }
                    else if (Utils::StartsWith(clr, "team_color_1"))
                        clrValue = clrTeam1;
                    else if (Utils::StartsWith(clr, "team_color_2"))
                        clrValue = clrTeam2;
                    else if (Utils::StartsWith(clr, "kit_background"))
                        clrValue = kitColor.first;
                    else if (Utils::StartsWith(clr, "kit_foreground")) {
                        clrValue = kitColor.second;
                        if (kitDiff < 100)
                            clrValue = GetTextColor(kitColor.first);
                    }
                    else if (Utils::StartsWith(clr, "kit_color_1"))
                        clrValue = kitColor.first;
                    else if (Utils::StartsWith(clr, "kit_color_2"))
                        clrValue = kitColor.second;
                    else if (Utils::StartsWith(clr, "epl_color_1"))
                        clrValue = epl_1;
                    else if (Utils::StartsWith(clr, "epl_color_2"))
                        clrValue = epl_2;
                    else if (Utils::StartsWith(clr, "epl_presentation_color"))
                        clrValue = eplPresentation;
                    else
                        clrValue = Utils::SafeConvertInt<UInt>(clr, true);
                    if (Utils::EndsWith(clr, "_text") || clr.find("_text_") != std::string::npos)
                        clrValue = GetTextColor(clrValue);
                    if (Utils::EndsWith(clr, "_alt") || clr.find("_alt_") != std::string::npos)
                        clrValue = GetAltColor(clrValue);
                    if (Utils::EndsWith(clr, "_onblack") || clr.find("_onblack_") != std::string::npos) {
                        if (GenColor::Distance(GenColorFromInt(clrValue), BLACK) < 200)
                            clrValue = 0xFFFFFFFF;
                    }
                    else if (Utils::EndsWith(clr, "_onwhite") || clr.find("_onwhite_") != std::string::npos) {
                        if (GenColor::Distance(GenColorFromInt(clrValue), WHITE) < 200)
                            clrValue = 0xFF000000;
                    }
                    popupProcessor.mColors[t].mTeamColor[c] = clrValue;
                }
            }
        }
    }
    void *guiInstance = *raw_ptr<void *>(screen, 0x20);
    CallVirtualMethod<26>(guiInstance, &popupProcessor, 0);
}

void METHOD OnSetupMatch3DTeamPresentation(void *screen, DUMMY_ARG, CDBOneMatch *match) {
    CallMethod<0xB55900>(screen, match);
    Process3dMatchScreenExtensions(screen, "TrfmTeam1|TbBadge", "TrfmTeam2|TbBadge", CTeamIndex::null(), CTeamIndex::null(), 0, false);
    for (UInt t = 0; t < 2; t++) {
        CDBEmployee *employee = *raw_ptr<CDBEmployee *>(screen, 0x4FC + 0x1C + 0x3C * t);
        if (employee) {
            void *tb = GetOptionalComponent(screen, (t == 0) ? "TrfmTeam1|TbManagerPhoto" : "TrfmTeam2|TbManagerPhoto");
            if (tb)
                Call<0xD4F8E0>(tb, employee, 3, 1);
        }
    }
}

void *gCurrentTeamPresentationScreen = nullptr;

void METHOD OnSetupTeam3DTeamPresentation(void *screen, DUMMY_ARG, void *teamData) {
    gCurrentTeamPresentationScreen = screen;
    CallMethod<0xB570A0>(screen, teamData);
    Process3dMatchScreenExtensions(screen, nullptr, nullptr, *raw_ptr<CTeamIndex>(teamData, 0x20), CTeamIndex::null(), 0, true);
}

void OnSetStatusWidgetImage(void *widget, const WideChar *imagePath, UInt a, UInt b) {
    if (gCurrentTeamPresentationScreen) {
        void *buf[3] = { 0, 0, gCurrentTeamPresentationScreen };
        UInt dummy = 0;
        CallMethod<0xD5FE50>(buf, Utils::SafeConvertInt<UInt>(imagePath), widget, &dummy);
    }
}

void METHOD OnSetPresentationScreenPlayerId(void *str, DUMMY_ARG, const WideChar *, UInt playerId) {
    String playerIdStr = std::to_wstring(playerId);
    CallMethod<0x424060>(str, playerIdStr.c_str(), playerIdStr.size());
}

Float *METHOD OnCalcPlayerLabelPosition(void *t, DUMMY_ARG, Float *out, Float *in) {
    for (UInt i = 0; i < 4; i++)
        out[i] = in[i];
    return out;
}

void METHOD OnSetupMatch3DStatisticsOverlay(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB3BFD0>(screen, data);
    Process3dMatchScreenExtensions(screen, "TbBadgeHome", "TbBadgeAway", CTeamIndex::null(), CTeamIndex::null(), 0, false);
}

void METHOD OnSetupMatch3DCardOverlay(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB14D40>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 6) {
        void *matchController = *raw_ptr<void *>(screen, 0x48C);
        if (matchController) {
            CTeamIndex teamID = CTeamIndex::null();
            CallVirtualMethod<57>(matchController, &teamID, *raw_ptr<UInt>(data, 0x14));
            Process3dMatchScreenExtensions(screen, "TbBadge", nullptr, teamID, CTeamIndex::null(), 0, true);
            Vector<StringA> compsToDisable = { "ImgGoal", "TbGoal" };
            switch (*raw_ptr<UInt>(data, 0x18)) {
            case 0:
                compsToDisable.push_back("Tb2ndYellowCard");
                compsToDisable.push_back("TbRedCard");
                break;
            case 1:
                compsToDisable.push_back("TbYellowCard");
                compsToDisable.push_back("TbRedCard");
                break;
            case 2:
                compsToDisable.push_back("TbYellowCard");
                compsToDisable.push_back("Tb2ndYellowCard");
                break;
            }
            for (auto const &s : compsToDisable) {
                auto comp = GetOptionalComponent(screen, s.c_str());
                if (comp)
                    SetVisible(comp, false);
            }
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
            Process3dMatchScreenExtensions(screen, "TbBadge", nullptr, teamID, CTeamIndex::null(), 0, true);
            Vector<StringA> compsToDisable = { "TbYellowCard", "Tb2ndYellowCard", "TbRedCard" };
            for (auto const &s : compsToDisable) {
                auto comp = GetOptionalComponent(screen, s.c_str());
                if (comp)
                    SetVisible(comp, false);
            }
        }
    }
}

void METHOD OnSetupMatch3DPitchOverlayStandings(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB719C0>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 0x2000)
        Process3dMatchScreenExtensions(screen, "ImgHomeBadge", "ImgAwayBadge", CTeamIndex::null(), CTeamIndex::null(), 0, false);
}

void METHOD OnSetupMatch3DPenaltyShootOut(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB38CE0>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) == 0x80)
        Process3dMatchScreenExtensions(screen, "ImgBadgeHome", "ImgBadgeAway", CTeamIndex::null(), CTeamIndex::null(), 0, false);
}

void METHOD OnSetupMatch3DSubstitution(void *screen, DUMMY_ARG, void *data) {
    CallMethod<0xB3D1F0>(screen, data);
    if (*raw_ptr<UInt>(data, 0x10) & 8) {
        void *matchController = *raw_ptr<void *>(screen, 0x48C);
        if (matchController) {
            CTeamIndex teamID = CTeamIndex::null();
            CallVirtualMethod<57>(matchController, &teamID, *raw_ptr<UInt>(data, 0x18));
            if (*raw_ptr<UInt>(data, 0x14) == 1)
                Process3dMatchScreenExtensions(screen, "TbBadge", nullptr, teamID, CTeamIndex::null(), 0, true);
            else if (*raw_ptr<UInt>(data, 0x14) > 1)
                Process3dMatchScreenExtensions(screen, "TbBadgeSmall", nullptr, teamID, CTeamIndex::null(), 0, true);
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
            Process3dMatchScreenExtensions(screen, "TbBadge", nullptr, teamID, CTeamIndex::null(), 0, true);
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
            if (Utils::EndsWith(screenNameStr, "_00000000.xml"))
                process = false;
        }
        if (process)
            Process3dMatchScreenExtensions(screen, "ImgHomeBadge", "ImgAwayBadge", CTeamIndex::null(), CTeamIndex::null(), 0, false);
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
    additionalData->mHasRedCards = false;
    for (UInt t = 0; t < 2; t++) {
        for (UInt i = 0; i < 3; i++) {
            additionalData->mCards[t][i].mStatus = 0;
            additionalData->mCards[t][i].mEnableTime = 0;
            StringA cardName = (t == 0) ? Utils::Format("HomeRedCard%d", i + 1) :  Utils::Format("AwayRedCard%d", i + 1);
            additionalData->mCards[t][i].mpCard = GetOptionalComponent(standingsInterface, cardName.c_str());
            if (additionalData->mCards[t][i].mpCard) {
                SetVisible(additionalData->mCards[t][i].mpCard, false);
                if (!additionalData->mHasRedCards)
                    additionalData->mHasRedCards = true;
            }
        }
    }
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
    Process3dMatchScreenExtensions(standingsInterface, "ImgHomeBadge", "ImgAwayBadge", CTeamIndex::null(), CTeamIndex::null(), 0, false);
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
    if (additionalData->mHasRedCards) {
        UInt time = GetTickCount();
        // show cards which were enabled
        for (UInt t = 0; t < 2; t++) {
            for (UInt c = 0; c < 3; c++) {
                if (additionalData->mCards[t][c].mStatus == 1 && (time - additionalData->mCards[t][c].mEnableTime) > 1000) {
                    additionalData->mCards[t][c].mStatus = 2;
                    SetVisible(additionalData->mCards[t][c].mpCard, true);
                }
            }
        }
        // enable cards which are not yet enabled
        auto match = GetCurrentMatch();
        if (match) {
            for (UInt t = 0; t < 2; t++) {
                UInt numRedCards = 0;
                UInt *playerFlags = raw_ptr<UInt>(CallMethodAndReturn<void *, 0xE802D0>(match, t == 0), 0x4E0);
                for (UInt p = 0; p < 18; p++) {
                    if (playerFlags[p] & 6) {
                        numRedCards++;
                        if (numRedCards == 3)
                            break;
                    }
                }
                for (UInt c = 0; c < numRedCards; c++) {
                    if (additionalData->mCards[t][c].mStatus == 0) {
                        additionalData->mCards[t][c].mStatus = 1;
                        additionalData->mCards[t][c].mEnableTime = time;
                    }
                }
            }
        }
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
            if (!FmFileExists(newfilename)) {
                if (IsCompetitionLeagueSplit_UInt(compId)) {
                    UInt mainCompId = GetCompetitionLeagueSplitMainLeague(compId);
                    newfilename = Format(L"fmdata\\popups\\overlay\\%4d_%08X.cfg", year, mainCompId);
                    if (!FmFileExists(newfilename))
                        newfilename.clear();
                }
                else
                    newfilename.clear();
            }
            if (newfilename.empty()) {
                newfilename = Format(L"fmdata\\popups\\overlay\\%4d_%04X.cfg", year, (compId >> 16) & 0xFFFF);
                if (!FmFileExists(newfilename))
                    newfilename.clear();
            }
            if (newfilename.empty()) {
                newfilename = Format(L"fmdata\\popups\\overlay\\%08X.cfg", compId);
                if (!FmFileExists(newfilename)) {
                    if (IsCompetitionLeagueSplit_UInt(compId)) {
                        UInt mainCompId = GetCompetitionLeagueSplitMainLeague(compId);
                        newfilename = Format(L"fmdata\\popups\\overlay\\%08X.cfg", mainCompId);
                        if (!FmFileExists(newfilename))
                            newfilename.clear();
                    }
                    else
                        newfilename.clear();
                }
                if (newfilename.empty()) {
                    newfilename = Format(L"fmdata\\popups\\overlay\\%04X.cfg", (compId >> 16) & 0xFFFF);
                    if (!FmFileExists(newfilename))
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

void METHOD OnSetCardFlag(void *t, DUMMY_ARG, UInt playerId, UInt flag, Bool bEnable) {
    ::Warning("OnSetCardFlag %d %d", playerId, bEnable);
    CallMethod<0xE857C0>(t, playerId, flag, bEnable);
}

void METHOD ReadPresentationConfig(void *t, DUMMY_ARG, WideChar const *filePath) {
    void *match = *(void **)0x3124748;
    if (match) {
        UInt compId = 0;
        CallMethod<0xE80190>(match, &compId);
        if (compId != 0) {
            //Message(Format(L"compId: %08X", compId));
            UShort year = GetCurrentYear();
            String newfilename = Format(L"fmdata\\popups\\presentation\\%4d_%08X.txt", year, compId);
            if (!FmFileExists(newfilename)) {
                if (IsCompetitionLeagueSplit_UInt(compId)) {
                    UInt mainCompId = GetCompetitionLeagueSplitMainLeague(compId);
                    newfilename = Format(L"fmdata\\popups\\presentation\\%4d_%08X.txt", year, mainCompId);
                    if (!FmFileExists(newfilename))
                        newfilename.clear();
                }
                else
                    newfilename.clear();
            }
            if (newfilename.empty()) {
                newfilename = Format(L"fmdata\\popups\\presentation\\%4d_%04X.txt", year, (compId >> 16) & 0xFFFF);
                if (!FmFileExists(newfilename))
                    newfilename.clear();
            }
            if (newfilename.empty()) {
                newfilename = Format(L"fmdata\\popups\\presentation\\%08X.txt", compId);
                if (!FmFileExists(newfilename)) {
                    if (IsCompetitionLeagueSplit_UInt(compId)) {
                        UInt mainCompId = GetCompetitionLeagueSplitMainLeague(compId);
                        newfilename = Format(L"fmdata\\popups\\presentation\\%08X.txt", mainCompId);
                        if (!FmFileExists(newfilename))
                            newfilename.clear();
                    }
                    else
                        newfilename.clear();
                }
                if (newfilename.empty()) {
                    newfilename = Format(L"fmdata\\popups\\presentation\\%04X.txt", (compId >> 16) & 0xFFFF);
                    if (!FmFileExists(newfilename))
                        newfilename.clear();
                }
            }
            if (!newfilename.empty()) {
                CallMethod<0xB4D240>(t, newfilename.c_str());
                return;
            }
        }
    }
    CallMethod<0xB4D240>(t, L"fmdata\\ParameterFiles\\Team Presentation.txt");
}

void METHOD OnPresentationInvertPlayersY(void *t) {
    if (false)
        CallMethod<0xB47DA0>(t);
}

void METHOD OnPresentationInvertPlayersX(void *t) {
    if (false)
        CallMethod<0xB47E60>(t);
}

void METHOD SetOverlayMinuteText(void *t, DUMMY_ARG, void *control, UInt minute) {
    SetText(control, Format(L"%d'", minute).c_str());
}

void METHOD PitchDummyAnimationCallback(void *t, DUMMY_ARG, float factor) {

}

Short *CalcBoundingTeamLabel(Short *outRect, void *control1, void *control2, void *control3, void *) {
    outRect[0] = 0;
    outRect[1] = 0;
    outRect[2] = 1280;
    outRect[3] = 1024;
    //Call<0xB46C30>(outRect, control1, control2, control3, 0);
    //::Message("%d %d %d %d", outRect[0], outRect[1], outRect[2], outRect[3]);
    return outRect;
}

void Patch3dMatchStandings(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadPopupDataSettingsFile(GetPopupKitColors(), FM::GameDirPath(L"plugins\\ucp\\popup_kitcolors.dat"), true);
        ReadPopupDataSettingsFile(GetPopupRecolor(), FM::GameDirPath(L"plugins\\ucp\\popup_teamcolors.dat"), false);
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
        patch::RedirectCall(0xB58033, OnSetupTeam3DTeamPresentation);
        patch::RedirectCall(0xB58BEA, OnSetupTeam3DTeamPresentation);
        patch::SetPointer(0xB57ED8 + 4, OnSetupTeam3DTeamPresentation);
        patch::SetPointer(0xB588E9 + 4, OnSetupTeam3DTeamPresentation);
        patch::SetPointer(0x245DC3C, OnSetupMatch3DStatisticsOverlay);
        patch::SetPointer(0x245A394, OnSetupMatch3DCardOverlay);
        patch::SetPointer(0x245A6A4, OnSetupMatch3DGoalOverlay);
        patch::SetPointer(0x24644AC, OnSetupMatch3DPitchOverlayStandings);
        patch::SetPointer(0x245D08C, OnSetupMatch3DPenaltyShootOut);
        patch::SetPointer(0x245E0AC, OnSetupMatch3DSubstitution);
        patch::SetPointer(0x2468984, OnSetupMatch3DManagerSentOff);
        patch::SetPointer(0x246831C, OnSetuMatch3DHalfTimeStats);

        patch::RedirectCall(0xB57469, OnSetStatusWidgetImage);

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

        //patch::RedirectCall(0x438210, OnSetCardFlag);
        //patch::RedirectCall(0x438126, OnSetCardFlag);
        //patch::RedirectCall(0x43810A, OnSetCardFlag);

        // disable TbStatus special tweak
        patch::SetUChar(0xB449D0, 0xC3);
        // replace player status by player id
        patch::Nop(0xB56C2F, 11);
        patch::RedirectCall(0xB56C4B, OnSetPresentationScreenPlayerId);
        // remove pitch label translation
       // patch::RedirectCall(0xB47BBA, OnCalcPlayerLabelPosition);
       // patch::Nop(0xB47BBA + 5, 3);
        patch::SetUChar(0xB4783C, 0x83);
        patch::SetUChar(0xB4783C + 1, 0xC4);
        patch::SetUChar(0xB4783C + 2, 0x04);
        patch::Nop(0xB4783C + 3, 7);
        patch::SetUChar(0xB47866, 0x83);
        patch::SetUChar(0xB47866 + 1, 0xC4);
        patch::SetUChar(0xB47866 + 2, 0x04);
        patch::Nop(0xB47866 + 3, 7);
        patch::Nop(0xB47A60, 3);
        //patch::Nop(0xB47FCA, 2);
        patch::SetPointer(0xB58976 + 4, OnPresentationInvertPlayersY);
        patch::SetPointer(0xB58A03 + 4, OnPresentationInvertPlayersX);
        patch::SetPointer(0xB582F4 + 4, PitchDummyAnimationCallback);
        //patch::SetPointer(0xB57D35 + 4, PitchDummyAnimationCallback);
        //patch::SetPointer(0xB5920D + 4, PitchDummyAnimationCallback);
        //patch::RedirectCall(0xB47399, CalcBoundingTeamLabel);

        patch::RedirectCall(0x20794E8, ReadPresentationConfig);
        patch::RedirectCall(0x209F5D3, ReadPresentationConfig);

        // change minute text to X'
        patch::RedirectCall(0xB14A5A, SetOverlayMinuteText);
        patch::RedirectCall(0xB158E1, SetOverlayMinuteText);
        patch::RedirectCall(0xBFEE72, SetOverlayMinuteText);
    }
}
