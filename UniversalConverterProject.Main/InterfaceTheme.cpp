#include "InterfaceTheme.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"
#include "Utils.h"

using namespace plugin;

String &GetCustomInterfaceFolderW() {
    static String custom1;
    return custom1;
}

StringA &GetCustomInterfaceFolderA() {
    static StringA custom2;
    return custom2;
}

String GetUIScreenFilenameW(String const &filename) {
    if (!GetCustomInterfaceFolderW().empty()) {
        String screenNameW = ToLower(filename);
        if (Utils::StartsWith(screenNameW, L"screens/") || Utils::StartsWith(screenNameW, L"screens\\")) {
            screenNameW = GetCustomInterfaceFolderW() + L"/" + filename;
            if (FmFileExists(screenNameW))
                return screenNameW;

        }
    }
    return filename;
}

StringA GetUIScreenFilenameA(StringA const &filename) {
    if (!GetCustomInterfaceFolderA().empty()) {
        StringA screenNameA = ToLower(filename);
        if (Utils::StartsWith(screenNameA, "screens/") || Utils::StartsWith(screenNameA, "screens\\")) {
            screenNameA = GetCustomInterfaceFolderA() + "/" + filename;
            if (FmFileExists(screenNameA))
                return screenNameA;
        }
    }
    return filename;
}

void METHOD OnLoadScreenResource(void *t, DUMMY_ARG, const WideChar *screenName, Int unk1, void *xmlInstance, Int unk2) {
    if (screenName[0] == 0) {
        CallMethod<0x4EDD40>(t, L"FMEmptyScreen.xml", unk1, xmlInstance, unk2);
        return;
    }
    CallMethod<0x4EDD40>(t, GetUIScreenFilenameW(screenName).c_str(), unk1, xmlInstance, unk2);
}

void *METHOD GetXmlInstance(void *t, DUMMY_ARG, const WideChar *screenName, Int a3, Int baseId, Int style1, Int style2) {
    return CallMethodAndReturn<void *, 0x4EDF40>(t, GetUIScreenFilenameW(screenName).c_str(), a3, baseId, style1, style2);
}

void *METHOD OnGetXmlInstance(void *t, DUMMY_ARG, const WideChar *screenName, Int a3, Int baseId, Int style1, Int style2) {
    return CallMethodAndReturn<void *, 0x4EDF40>(t, GetUIScreenFilenameW(screenName).c_str(), a3, baseId, style1, style2);
}

void METHOD OnSetXmlScreenName(void *t, DUMMY_ARG, const Char *screenName) {
    CallMethod<0x4F20E0>(t, GetUIScreenFilenameA(screenName).c_str());
}

WideChar *OnCopyScreensDirName(WideChar *a1, const WideChar *a2) {
    if (!GetCustomInterfaceFolderW().empty())
        return CallAndReturn<WideChar *, 0x1580246>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str());
    return CallAndReturn<WideChar *, 0x1580246>(a1, a2);
}

WideChar *OnCopyDeskBackgroundsDirName(WideChar *a1, const WideChar *a2) {
    if (!GetCustomInterfaceFolderW().empty())
        return CallAndReturn<WideChar *, 0x1580246>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str());
    return CallAndReturn<WideChar *, 0x1580246>(a1, a2);
}

WideChar *OnCopyArtFmDirName(WideChar *a1, const WideChar *a2) {
    if (!GetCustomInterfaceFolderW().empty())
        return CallAndReturn<WideChar *, 0x1580246>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str());
    return CallAndReturn<WideChar *, 0x1580246>(a1, a2);
}

void OnConcatenateAlternativeScreenName(WideChar *dst, WideChar const *screenName) {
    String scrw = screenName;
    auto slashPos = scrw.find_last_of(L"/\\");
    if (slashPos != String::npos)
        scrw = scrw.substr(slashPos + 1);
    if (scrw.size() >= 2) {
        if (
            (scrw[0] == L'1' && (scrw[1] == L'0' || scrw[1] == L'1' || scrw[1] == L'2' || scrw[1] == L'3')) ||
            (scrw[0] == L'0' && (scrw[1] == L'7' || scrw[1] == L'8' || scrw[1] == L'9'))
        )
        {
            scrw[0] = L'1';
            static WideChar chars[] = { L'3', L'2', L'1', L'0' };
            for (auto c : chars) {
                scrw[1] = c;
                if (FmFileExists(dst + scrw)) {
                    wcscat(dst, scrw.c_str());
                    return;
                }
            }
        }
    }
    wcscat(dst, screenName);
}

Int METHOD OnLoadGuiColors(void *t, DUMMY_ARG, WideChar const *filename) {
    if (!GetCustomInterfaceFolderW().empty()) {
        String themeGuiColors = GetCustomInterfaceFolderW() + L"\\" + filename;
        if (exists(themeGuiColors))
            return CallMethodAndReturn<Int, 0x14B2C35>(t, themeGuiColors.c_str());
    }
    return CallMethodAndReturn<Int, 0x14B2C35>(t, filename);
}

void MyGetCalendarIconPathPositive(WideChar const *dst, WideChar const *format, WideChar const *folderName, WideChar const *fileName) {
    if (!GetCustomInterfaceFolderW().empty()) {
        Call<0x1494136>(dst, format, (GetCustomInterfaceFolderW() + L"\\" + folderName).c_str(), fileName);
        return;
    }
    Call<0x1494136>(dst, format, folderName, fileName);
}

void MyGetCalendarIconPathNegative(WideChar const *dst, WideChar const *format, WideChar const *folderName, WideChar const *fileName) {
    if (!GetCustomInterfaceFolderW().empty()) {
        Call<0x1494136>(dst, format, (GetCustomInterfaceFolderW() + L"\\" + folderName).c_str(), fileName);
        return;
    }
    Call<0x1494136>(dst, format, folderName, fileName);
}

WideChar *OnCopyExperienceIconDirName(WideChar *a1, const WideChar *a2) {
    if (!GetCustomInterfaceFolderW().empty())
        return CallAndReturn<WideChar *, 0x1493F2F>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str());
    return CallAndReturn<WideChar *, 0x1493F2F>(a1, a2);
}

WideChar *OnCopyCharacterIconDirName(WideChar *a1, const WideChar *a2) {
    if (!GetCustomInterfaceFolderW().empty())
        return CallAndReturn<WideChar *, 0x1493F2F>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str());
    return CallAndReturn<WideChar *, 0x1493F2F>(a1, a2);
}

void OnFormatTalentStarPath(WideChar const *dst, WideChar const *format, UInt talent) {
    if (!GetCustomInterfaceFolderW().empty()) {
        Call<0x1494136>(dst, (GetCustomInterfaceFolderW() + L"\\" + format).c_str(), talent);
        return;
    }
    Call<0x1494136>(dst, format, talent);
}

UChar OnSetWidgetTalentStarPath(WideChar const *dst, WideChar const *filepath, UInt scaleX, UInt scaleY) {
    if (!GetCustomInterfaceFolderW().empty())
        return CallAndReturn<UChar, 0xD32860>(dst, (GetCustomInterfaceFolderW() + L"\\" + filepath).c_str(), scaleX, scaleY);
    return CallAndReturn<UChar, 0xD32860>(dst, filepath, scaleX, scaleY);
}

WideChar * METHOD OnResolveScreenPath(void *t, DUMMY_ARG, WideChar *out, const WideChar *in, UChar flag) {
    WideChar *result = CallMethodAndReturn<WideChar *, 0x4D8540>(t, out, in, flag);
    if (!GetCustomInterfaceFolderW().empty()) {
        auto themeScreenFilename = GetUIScreenFilenameW(out);
        wcscpy(out, themeScreenFilename.c_str());
    }
    return result;
}

void OnCopyMailIconFileName(WideChar *a1, const WideChar *a2, UInt a3) {
    if (!GetCustomInterfaceFolderW().empty()) {
        Call<0x1493F41>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str(), a3);
        return;
    }
    Call<0x1493F41>(a1, a2, a3);
}

struct CMenuTopFillCalendarEventsTextBoxes {
    void *m_pTbSmallIcon; // calendar icon
    void *m_pTbBadgeDay; // team badge
    void *m_pTbTinyFlag; // country flag
    void *m_pTbEventTitle; // 
    void *m_pTbEventName; //
    void *m_pTbResult; // result
    void *m_pTbCompetition; // trophy image
};

void METHOD OnSetupTopMenuCalendarDays_DarkTheme(void *t) {
    const UInt iconForegroundColor = 0xff161616; // 0xff0e2535;
    CallMethod<0xA23F00>(t);
    auto currDate = CDBGame::GetInstance()->GetCurrentDate();
    auto dayOfWeek = currDate.GetDayOfWeek();
    for (UInt i = 0; i < 7; i++) {
        UInt color = (i == dayOfWeek) ? iconForegroundColor : 0xfff1f1f1;
        SetTextBoxColors(*raw_ptr<void *>(t, 0x4B8 + i * 4), color);
        SetTextBoxColors(*raw_ptr<void *>(t, 0x4D4 + i * 4), color);
        SetTextBoxColors(*raw_ptr<void *>(t, 0x4F4 + i * 4), color);
        SetTextBoxColors(*raw_ptr<void *>(t, 0x510 + i * 4), color);
        SetTextBoxColors(*raw_ptr<void *>(t, 0x548 + i * 4), color);
        bool imageColorSet = false;
        if (i == dayOfWeek) {
            void *img = *raw_ptr<void *>(t, 0x564 + i * 4);
            void *baseImage = *raw_ptr<void *>(img, 0x60);
            if (baseImage && baseImage != CallAndReturn<void *, 0x1440F4D>()) {
                String filename = ToLower(*raw_ptr<WideChar *>(baseImage, 0x24));
                if (filename.find(L"icons\\calendar") != String::npos) {
                    SetImageColorRGBA(*raw_ptr<void *>(t, 0x564 + i * 4), iconForegroundColor);
                    imageColorSet = true;
                }
            }
        }
        if (!imageColorSet)
            SetImageColorRGBA(*raw_ptr<void *>(t, 0x564 + i * 4), 0xFFFFFFFF);
    }
}

void METHOD OnSetupWeeklyProgressCalendarDays_DarkTheme(void *t) {
    const UInt iconForegroundColor = 0xff161616; // 0xff0e2535;
    CallMethod<0x866D30>(t);
    auto currDate = CDBGame::GetInstance()->GetCurrentDate();
    auto dayOfWeek = currDate.GetDayOfWeek();
    for (UInt i = 0; i < 7; i++) {
        UInt color = (i == dayOfWeek) ? iconForegroundColor : 0xfff1f1f1;
        SetTextBoxColors(*raw_ptr<void *>(t, 0x4C8 + i * 4), color);
        SetTextBoxColors(*raw_ptr<void *>(t, 0x4E4 + i * 4), color);
        SetTextBoxColors(*raw_ptr<void *>(t, 0x500 + i * 4), color);
        SetTextBoxColors(*raw_ptr<void *>(t, 0x51C + i * 4), color);
        bool imageColorSet = false;
        if (i == dayOfWeek) {
            void *img = *raw_ptr<void *>(t, 0x564 + i * 4);
            void *baseImage = *raw_ptr<void *>(img, 0x60);
            if (baseImage && baseImage != CallAndReturn<void *, 0x1440F4D>()) {
                String filename = ToLower(*raw_ptr<WideChar *>(baseImage, 0x24));
                if (filename.find(L"icons\\calendar") != String::npos) {
                    SetImageColorRGBA(*raw_ptr<void *>(t, 0x4AC + i * 4), iconForegroundColor);
                    imageColorSet = true;
                }
            }
        }
        if (!imageColorSet)
            SetImageColorRGBA(*raw_ptr<void *>(t, 0x4AC + i * 4), 0xFFFFFFFF);
    }
}

UChar OnSetWidgetCalendarIconPath(WideChar const *dst, WideChar const *filepath, UInt scaleX, UInt scaleY) {
    if (!GetCustomInterfaceFolderW().empty()) {
        String filepathStr = filepath;
        if (!Utils::StartsWith(filepathStr, GetCustomInterfaceFolderW() + L"\\"))
            return CallAndReturn<UChar, 0xD32860>(dst, (GetCustomInterfaceFolderW() + L"\\" + filepath).c_str(), scaleX, scaleY);
    }
    return CallAndReturn<UChar, 0xD32860>(dst, filepath, scaleX, scaleY);
}

void OnCopyCalendarIconPath(WideChar *a1, const WideChar *a2) {
    if (!GetCustomInterfaceFolderW().empty()) {
        Call<0x1493F2F>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str());
        return;
    }
    Call<0x1493F2F>(a1, a2);
}

UInt FixBlackColorForDarkTheme(UInt color) {
    UChar r = (color >> 16) & 0xFF;
    UChar g = (color >> 8) & 0xFF;
    UChar b = (color) & 0xFF;
    if (r <= 0x30 && g <= 0x30 && b <= 0x30)
        return (color & 0xFF000000) | 0x6E6E6E;
    else if (r == 0x6E && g == 0x6E && b == 0x6E)
        return (color & 0xFF000000) | 0x919191;
    return color;
}

UInt OnGetTeamInterfaceColor(UInt tableId, UInt colorId) {
    UInt color = CallAndReturn<UInt, 0xED2EF0>(tableId, colorId);
    return FixBlackColorForDarkTheme(color);
}

UInt METHOD OnGetTeamHeaderColorForMatchdayComments(CDBTeam *team, DUMMY_ARG, UChar alpha) {
    UInt color = CallMethodAndReturn<UInt, 0xED3480>(team, alpha);
    return FixBlackColorForDarkTheme(color);
}

UInt OnGetHalftimeStatsEventsTextColor(UInt colorId) {
    if (colorId == 1)
        return 0xFF1A1A1A;
    return CallAndReturn<UInt, 0x4E8C10>(colorId);
}

UInt OnGetNewspaperRankingOfPlayersTextBgColor(UInt colorId) {
    return 0xFF1F1F1F;
}

void METHOD OnSetWidgetBrowseArrow(void *t, DUMMY_ARG, Int a2, Int a3, const WideChar *a4) {
    if (!GetCustomInterfaceFolderW().empty()) {
        CallMethod<0xD833B0>(t, a2, a3, (GetCustomInterfaceFolderW() + L"\\" + a4).c_str());
        return;
    }
    CallMethod<0xD833B0>(t, a2, a3, a4);
}

WideChar const *OnGetPlayerIconPath1(UChar id, UChar type) {
    if (type != 0 && type != 1)
        type = 0;
    static Array<Pair<String, Bool>, 21> ary1[2];
    if (!ary1[type][id].second) {
        ary1[type][id].first = CallAndReturn<WideChar const *, 0xD2B810>(id, type);
        if (!ary1[type][id].first.empty())
            ary1[type][id].first = GetCustomInterfaceFolderW() + L"\\" + ary1[type][id].first;
        ary1[type][id].second = true;
    }
    return ary1[type][id].first.c_str();
}

void OnGetPlayerIconPath2(WideChar const *dst, WideChar const *src) {
    if (!GetCustomInterfaceFolderW().empty()) {
        String strSrc = src;
        if (!strSrc.empty()) {
            strSrc = GetCustomInterfaceFolderW() + L"\\" + strSrc;
            Call<0x1493F2F>(dst, strSrc.c_str());
            return;
        }
    }
    Call<0x1493F2F>(dst, src);
}

void OnCopyConfidenceIconPath(WideChar const *a1, WideChar const *a2) {
    if (!GetCustomInterfaceFolderW().empty())
        return Call<0x1493F2F>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str());
    return Call<0x1493F2F>(a1, a2);
}

void OnCopyMoraleIconPath(WideChar const *a1, WideChar const *a2) {
    if (!GetCustomInterfaceFolderW().empty())
        return Call<0x1493F2F>(a1, (GetCustomInterfaceFolderW() + L"\\" + a2).c_str());
    return Call<0x1493F2F>(a1, a2);
}

void OnCopyUpcomingEventsIconPath(WideChar const *a1, WideChar const *a2) {
    if (!GetCustomInterfaceFolderW().empty())
        Call<0x1494136>(a1, (GetCustomInterfaceFolderW() + L"/" + a2).c_str());
    else
        Call<0x1494136>(a1, a2);
}

UChar OnSetWidgetIcoPyramid(WideChar const *dst, WideChar const *filepath, UInt scaleX, UInt scaleY) {
    if (!GetCustomInterfaceFolderW().empty() && wcslen(filepath) != 0)
        return CallAndReturn<UChar, 0xD32860>(dst, (GetCustomInterfaceFolderW() + L"\\" + filepath).c_str(), scaleX, scaleY);
    return CallAndReturn<UChar, 0xD32860>(dst, filepath, scaleX, scaleY);
}

UChar OnSetWidgetProgressBar08(WideChar const *dst, WideChar const *filepath, UInt scaleX, UInt scaleY) {
    if (!GetCustomInterfaceFolderW().empty())
        return CallAndReturn<UChar, 0xD32860>(dst, (GetCustomInterfaceFolderW() + L"\\" + filepath).c_str(), scaleX, scaleY);
    return CallAndReturn<UChar, 0xD32860>(dst, filepath, scaleX, scaleY);
}

void METHOD MyCreateUI(void *t) {
    CallMethod<0xC11A20>(t);
    void *lb = raw_ptr<void>(t, 0x58C);
    void *unk = *raw_ptr<void *>(lb, 8);
    Error("%X", *raw_ptr<void *>(unk, 0));
}

void MyGetTrainingIconPath(WideChar const *dst, WideChar const *format, WideChar const *fileName) {
    if (!GetCustomInterfaceFolderW().empty())
        Call<0x1494136>(dst, (GetCustomInterfaceFolderW() + L"\\" + format).c_str(), fileName);
    else
        Call<0x1494136>(dst, format, fileName);
}

void METHOD MySetLbItemYouthTeamSquad(void *t, DUMMY_ARG, Int a2, Int a3, const WideChar *fileName, Int a5, Int a6) {
    if (!GetCustomInterfaceFolderW().empty())
        CallMethod<0xD199A0>(t, a2, a3, (GetCustomInterfaceFolderW() + L"\\" + fileName).c_str(), a5, a6);
    else
        CallMethod<0xD199A0>(t, a2, a3, fileName, a5, a6);
}

void METHOD MyConstructArtFmLibMediaString(void *t, DUMMY_ARG, const WideChar *fileName) {
    if (!GetCustomInterfaceFolderW().empty())
        CallMethod<0x14978B3>(t, (GetCustomInterfaceFolderW() + L"\\" + fileName).c_str());
    else
        CallMethod<0x14978B3>(t, fileName);
}

void METHOD MyAddHalfTimeSpeechEffectIcon(void *t, DUMMY_ARG, const WideChar *fileName) {
	if (!GetCustomInterfaceFolderW().empty())
		CallMethod<0xD1E4C0>(t, (GetCustomInterfaceFolderW() + L"\\" + fileName).c_str());
	else
		CallMethod<0xD1E4C0>(t, fileName);
}

void * METHOD OnConstructEmployeeOffice(void *office, DUMMY_ARG, CDBEmployee *employee) {
    void *result = CallMethodAndReturn<void *, 0x12F22B0>(office, employee);
    *raw_ptr<UInt>(office, 4) = 100000;
    *raw_ptr<UInt>(office, 12) = 0;
    *raw_ptr<UInt>(office, 16) = 0;
    return result;
}

void OnFormatLoadAnimPath(WideChar const *a1, WideChar const *a2, Int a3) {
    if (!GetCustomInterfaceFolderW().empty())
        Call<0x1494136>(a1, (GetCustomInterfaceFolderW() + L"/" + a2).c_str(), a3);
    else
        Call<0x1494136>(a1, a2, a3);
}

void OnGetTeamLogoPathForSaveGame(WideChar *out, void *loadInfo, Int managerIndex) {
    Call<0x1A198ED>(out, loadInfo, managerIndex);
    if (*out != 0) {
        String newPath = out;
        if (Utils::EndsWith(newPath, L".tga")) {
            newPath = newPath.substr(0, newPath.length() - 4) + L"_1.tga";
            if (FmFileExists(newPath))
                wcscpy(out, newPath.c_str());
        }
    }
}

RoundPair *gCalendarFixtures_RoundPair = nullptr;
UChar gClubFixtures_Score1 = 0;
UChar gClubFixtures_Score2 = 0;

void FixtureAddColumnResultIcon(void *listBox, UChar result1, UChar result2) {
    if (result1 > result2)
        CallMethod<0xD1E4C0>(listBox, L"art/FM08_GFX/Screens/Main/08MainNextOpponentColour1.tga");
    else if (result1 < result2)
        CallMethod<0xD1E4C0>(listBox, L"art/FM08_GFX/Screens/Main/08MainNextOpponentColour3.tga");
    else
        CallMethod<0xD1E4C0>(listBox, L"art/FM08_GFX/Screens/Main/08MainNextOpponentColour2.tga");
}

void CalendarFixtures_InitColumnTypes(void *listBox, UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6, UInt a7, UInt a8, UInt a9) {
    Call<0xD19660>(listBox, a1, a2, a3, a4, a5, 58, a6, a7, a8, a9);
}

void CalendarFixtures_InitColumnFormatting(void *listBox, UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6, UInt a7, UInt a8, UInt a9) {
    Call<0xD196A0>(listBox, a1, a2, a3, a4, a5, 210, a6, a7, a8, a9);
}

void * METHOD CalendarFixtures_GetResultString(RoundPair *t, DUMMY_ARG, void *str, UChar flags, const WideChar *team1name, const WideChar *team2name) {
    gCalendarFixtures_RoundPair = t;
    return CallMethodAndReturn<void *, 0x10ED930>(t, str, flags, team1name, team2name);
}

Bool METHOD CalendarFixtures_AddColumnResult(void *t, DUMMY_ARG, const WideChar *str, UInt color, Int unk) {
    UChar leg = (gCalendarFixtures_RoundPair->m_nFlags & 0x200) ? 1 : 0;
    FixtureAddColumnResultIcon(t, gCalendarFixtures_RoundPair->result1[leg], gCalendarFixtures_RoundPair->result2[leg]);
    CallMethod<0xD1EF40>(t, str, color, unk);
    return true;
}

Bool METHOD CalendarFixtures_AddColumnResultEmpty(void *t, DUMMY_ARG, const WideChar *str, UInt color, Int unk) {
    CallMethod<0xD1EF40>(t, str, color, unk);
    CallMethod<0xD1EF40>(t, str, color, unk);
    return true;
}

void ClubFixtures_InitColumnTypes(void *listBox, UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6, UInt a7, UInt a8) {
    Call<0xD19660>(listBox, a1, a2, a3, a4, a5, 58, a6, a7, a8);
}

void ClubFixtures_InitColumnFormatting(void *listBox, UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6, UInt a7, UInt a8) {
    Call<0xD196A0>(listBox, a1, a2, a3, a4, a5, 210, a6, a7, a8);
}

void *ClubFixtures_StoreScore(void *str, UChar a1, UChar a2, UChar a3, UChar a4, UChar a5, UChar a6, UShort *a7, UShort *a8, Int a9, Int a10, Int a11) {
    gClubFixtures_Score1 = a1;
    gClubFixtures_Score2 = a2;
    return CallAndReturn<void *, 0x14AD9F0>(str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

Bool METHOD ClubFixtures_AddColumnResult(void *t, DUMMY_ARG, const WideChar *str, UInt color, Int unk) {
    FixtureAddColumnResultIcon(t, gClubFixtures_Score1, gClubFixtures_Score2);
    CallMethod<0xD1EF40>(t, str, color, unk);
    return true;
}

Bool METHOD ClubFixtures_AddColumnResultEmpty(void *t, DUMMY_ARG, const WideChar *str, UInt color, Int unk) {
    CallMethod<0xD1EF40>(t, str, color, unk);
    CallMethod<0xD1EF40>(t, str, color, unk);
    return true;
}

void FormatLeagueBadgeName1(UInt *data, UInt length, WideChar const *format, UInt year, UInt type) {
	data[0] = type;
	data[1] = year;
}

void FormatLeagueBadgeName2(WideChar *out, UInt length, WideChar const *format, UInt resX, UInt resY, UInt *data, WideChar const *strCompId) {
	switch (data[0]) {
	case 0:
		_snwprintf(out, length, L"Leagues\\%dx%d\\%04d_%s_1", resX, resY, data[1], strCompId);
		break;
	case 1:
		_snwprintf(out, length, L"Leagues\\%dx%d\\%04d_%s", resX, resY, data[1], strCompId);
		break;
	case 2:
		_snwprintf(out, length, L"Leagues\\%dx%d\\%s_1", resX, resY, strCompId);
		break;
	case 3:
		_snwprintf(out, length, L"Leagues\\%dx%d\\%s", resX, resY, strCompId);
		break;
	}
}

void PatchInterfaceTheme(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

        //patch::SetPointer(0x246CC5C, MyCreateUI);

        GetCustomInterfaceFolderA() = Settings::GetInstance().Theme;
        if (!GetCustomInterfaceFolderA().empty()) {
            GetCustomInterfaceFolderW() = SettingsHelper::ToUTF16(GetCustomInterfaceFolderA());

            enum Theme { LIGHT, DARK, CUSTOM } theme = CUSTOM;
            if (GetCustomInterfaceFolderW() == L"dark")
                theme = DARK;
            else if (GetCustomInterfaceFolderW() == L"light")
                theme = LIGHT;

            //patch::RedirectCall(0x4EDF96, OnLoadScreenResource);
            //patch::RedirectCall(0x4EF7F3, OnLoadScreenResource);
            //patch::RedirectCall(0x4DA50F, OnCopyScreensDirName);
            //patch::RedirectCall(0x4E97ED, OnConcatenateAlternativeScreenName);
            patch::SetPointer(0x23AB3C4, OnResolveScreenPath);
            patch::RedirectCall(0x5880B9, OnFormatLoadAnimPath);
            patch::RedirectCall(0x7B52BF, OnFormatLoadAnimPath);
            patch::RedirectCall(0x8845AB, OnFormatLoadAnimPath);

            if (theme != LIGHT) {
                patch::RedirectCall(0x4E8B1C, OnLoadGuiColors);
                patch::RedirectCall(0x4DA76B, OnCopyDeskBackgroundsDirName);
                patch::RedirectCall(0x4DA53D, OnCopyArtFmDirName);
                patch::RedirectCall(0xD2B73B, OnCopyExperienceIconDirName);
                String toolTipFilename = GetCustomInterfaceFolderW() + L"\\Tooltipp.tga";
                if (toolTipFilename.size() <= 37)
                    injector::WriteMemoryRaw(0x23C25F8, (void *)toolTipFilename.c_str(), toolTipFilename.size() * 2 + 2, true);
            }

            if (theme == DARK) {
                patch::RedirectCall(0xA240DA, OnSetupTopMenuCalendarDays_DarkTheme);
                patch::RedirectCall(0xA24110, OnSetupTopMenuCalendarDays_DarkTheme);
                patch::RedirectCall(0xA2413A, OnSetupTopMenuCalendarDays_DarkTheme);
                patch::RedirectCall(0xA244FD, OnSetupTopMenuCalendarDays_DarkTheme);
                patch::RedirectCall(0x86880E, OnSetupWeeklyProgressCalendarDays_DarkTheme);
                patch::RedirectCall(0x43109F, OnGetTeamInterfaceColor);
                patch::RedirectCall(0x4310A9, OnGetTeamInterfaceColor);
                UInt tableRowColor = 0x20FFFFFF;
                patch::SetUInt(0x89B417 + 1, tableRowColor);
                patch::SetUInt(0xAD05EC + 1, tableRowColor);
                patch::SetUInt(0xAD2F39 + 1, tableRowColor);
                patch::SetUInt(0xC92017 + 1, tableRowColor);
                patch::SetUInt(0xC92167 + 1, tableRowColor);
                patch::SetUInt(0xC922B7 + 1, tableRowColor);
                patch::SetUInt(0xC940A7 + 1, tableRowColor);
                patch::SetUInt(0xC95817 + 1, tableRowColor);
                patch::SetUInt(0xCA08B1 + 1, tableRowColor);
                patch::SetUInt(0xCA584B + 1, tableRowColor);
                patch::RedirectCall(0xD26340, OnGetHalftimeStatsEventsTextColor);
                patch::SetUInt(0x9D9D8E + 1, 0xFFF1F1F1);
                patch::SetUInt(0x9D9E2E + 1, 0xFFF1F1F1);
                patch::SetPointer(0x50BF22 + 1, L"<CREF 0xFF8ab4f8>");
                patch::SetPointer(0x50BF2E + 1, L"<CREF 0xFF6ae46a>");
                patch::SetPointer(0x50BF38 + 1, L"<CREF 0xFFff7769>");
                patch::SetPointer(0x50BF3F + 1, L"<CREF 0xffff7769>");
                patch::SetPointer(0x50BF16 + 1, L"<CREF 0xFFf1f1f1>");
                patch::RedirectCall(0xA605CE, OnGetNewspaperRankingOfPlayersTextBgColor);
                patch::RedirectCall(0xA60716, OnGetNewspaperRankingOfPlayersTextBgColor);
                patch::RedirectCall(0xA60866, OnGetNewspaperRankingOfPlayersTextBgColor);
                patch::RedirectCall(0xA609B6, OnGetNewspaperRankingOfPlayersTextBgColor);
                patch::RedirectCall(0xA60AC6, OnGetNewspaperRankingOfPlayersTextBgColor);
                patch::RedirectCall(0xA60BED, OnGetNewspaperRankingOfPlayersTextBgColor);
                patch::SetUInt(0x8EE320 + 1, 0xFF144B25);
                patch::SetUInt(0x8EE4E0 + 1, 0xFF144B25);
                // merchandise graph
                patch::SetUInt(0x6CA3A8 + 2, 1240);
                patch::SetUInt(0x6CA3C6 + 2, 10352);
                patch::SetUChar(0x6CA3C4 + 1, 22);
                patch::SetUInt(0x6CA3D5 + 2, 1244);
                patch::SetUInt(0x6CA3EE + 2, 10380);
                patch::SetUChar(0x6CA3EC + 1, 27);
                // badge dark mode
                patch::Nop(0xD2B5B0, 3);
                patch::Nop(0xD2B5A6, 1);
                patch::SetUChar(0xD2B5A6 + 1, 0xBB);
                patch::SetUInt(0xD2B5A6 + 2, 1);
                patch::SetUChar(0xF006BC + 1, 1); // CDBTeam::GetFirstTeamLogoPath
                patch::SetUChar(0xF006D5 + 1, 1); // CDBTeam::GetFirstTeamLogoPath
                patch::SetUChar(0xF006EE + 1, 1); // CDBTeam::GetFirstTeamLogoPath
                patch::SetUChar(0xF00707 + 1, 1); // CDBTeam::GetFirstTeamLogoPath
                patch::SetUChar(0xD4D6F5 + 1, 1); // WkCupRendering
                patch::Nop(0xE174E0, 3);             // team logo style
                patch::Nop(0xE174D6, 1);             // team logo style
                patch::SetUChar(0xE174D6 + 1, 0xB9); // team logo style
                patch::SetUInt(0xE174D6 + 2, 1);     // team logo style
                patch::RedirectJump(0xD2D800, OnGetTeamLogoPathForSaveGame); // savegame
				// badge league dark mode
				patch::SetUChar(0x4DCBB5 + 2, 4);
				patch::SetUInt(0x4DC750, 0x6C24548B); // mov edx, [esp+0x6C]
				patch::SetUChar(0x4DC750 + 4, 0x52); // push edx
				patch::Nop(0x4DC755, 2);
				patch::RedirectCall(0x4DC76E, FormatLeagueBadgeName1);
				patch::SetUChar(0x4DC773 + 2, 0x14);
				patch::SetUInt(0x4DC757 + 3, 0x100C + 4);
				patch::SetUInt(0x4DC764 + 3, 0x94 + 4);
				patch::RedirectCall(0x4DC7CD, FormatLeagueBadgeName2);
				patch::RedirectCall(0x4DC86B, FormatLeagueBadgeName2);
				patch::RedirectCall(0x4DC8E1, FormatLeagueBadgeName2);
				patch::RedirectCall(0x4DC9E4, FormatLeagueBadgeName2);
				patch::RedirectCall(0x4DCA5F, FormatLeagueBadgeName2);
				patch::RedirectCall(0x4DCAF0, FormatLeagueBadgeName2);
				patch::RedirectCall(0x4DCB5E, FormatLeagueBadgeName2);
                // fix player attribute
                patch::SetUChar(0x4E8D4C + 1, 8); // 22
                // WC mode
                patch::SetUInt(0x54D5B6 + 6, 0xFF33CC33);
                patch::SetUInt(0x54D5C0 + 6, 0xFFFFFFFF);
                // Rewards shop
                patch::SetUInt(0xE4E2B5 + 1, 0x80FFFFFF);
                // Matchday comments - fix text color
                patch::RedirectCall(0xA8A497, OnGetTeamHeaderColorForMatchdayComments);
                patch::RedirectCall(0xA8A663, OnGetTeamHeaderColorForMatchdayComments);
                patch::RedirectCall(0xA8B914, OnGetTeamHeaderColorForMatchdayComments);
                // text color in CTeamDynamicsAnalyser1PList > m_LbFrustration
                patch::SetUInt(0x5B26D3 + 1, 0xFFF1F1F1);
                patch::SetUInt(0x5B26F4 + 1, 0xFFF1F1F1);
				// line colors in CWeekInfoTraining
				patch::SetUInt(0x8623B7 + 1, 0xFFff7769);
				patch::SetUInt(0x86248B + 1, 0xFFff7769);
				patch::SetUInt(0x86241E + 1, 0xFF8ab4f8);
				patch::SetUInt(0x8624F5 + 1, 0xFF8ab4f8);
            }

            if (theme != LIGHT) {
                patch::RedirectCall(0x1126190 + 0x47, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x67, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x88, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0xA8, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0xC8, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0xE9, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x109, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x129, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x14A, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x16A, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x18A, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x1AB, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x1CB, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x1EB, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x20C, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x22C, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x24C, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x26D, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x28D, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x2AD, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x2CE, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x2EE, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x30E, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x32F, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x34F, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x36F, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x390, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x3E1, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x401, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x422, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x442, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x462, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x483, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x4A3, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x4C3, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x4E4, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x504, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x524, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x545, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x565, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x585, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x5A6, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x5C6, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x5E6, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x607, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x627, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x647, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x668, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x688, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x6A8, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x6C6, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x6E3, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x700, MyGetCalendarIconPathPositive);
                patch::RedirectCall(0x1126190 + 0x717, MyGetCalendarIconPathPositive);

                patch::RedirectCall(0x1126A00 + 0x47, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x67, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x88, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0xA8, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0xC8, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0xE9, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x109, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x129, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x14A, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x16A, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x18A, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x1AB, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x1CB, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x1EB, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x20C, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x22C, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x24C, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x26D, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x28D, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x2AD, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x2CE, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x2EE, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x30E, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x32F, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x34F, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x36F, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x390, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x3DE, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x3FE, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x41E, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x43F, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x45F, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x47F, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x4A0, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x4C0, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x4E0, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x501, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x521, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x541, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x562, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x582, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x5A2, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x5C3, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x5E3, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x603, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x624, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x644, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x664, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x685, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x6A2, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x6BF, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x6DD, MyGetCalendarIconPathNegative);
                patch::RedirectCall(0x1126A00 + 0x6F3, MyGetCalendarIconPathNegative);

                patch::RedirectCall(0xD2BB80 + 0x1F, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x35, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x4B, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x61, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x77, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x8D, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0xA3, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0xB9, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0xCF, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0xE5, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0xFB, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x111, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x127, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x13D, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x153, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x169, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x17F, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x195, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x1AB, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x1C1, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x1D7, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x1ED, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x203, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x219, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x22F, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x245, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x25B, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x271, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x287, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x29D, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x2B3, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x2C9, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x2DF, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x2F5, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x30B, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x321, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x337, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x34D, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x363, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x379, OnCopyCharacterIconDirName);
                patch::RedirectCall(0xD2BB80 + 0x38F, OnCopyCharacterIconDirName);

                patch::RedirectCall(0x9588D8, OnFormatTalentStarPath);
                patch::RedirectCall(0xA58E31, OnFormatTalentStarPath);
                patch::RedirectCall(0xA5A1FD, OnFormatTalentStarPath);
                patch::RedirectCall(0xD2B5FB, OnFormatTalentStarPath);
                patch::RedirectCall(0xD2B672, OnFormatTalentStarPath);
                patch::RedirectCall(0xD2B77B, OnFormatTalentStarPath);
                patch::RedirectCall(0xD34402, OnFormatTalentStarPath);

                patch::RedirectCall(0x558B7C, OnSetWidgetTalentStarPath);
                patch::RedirectCall(0x55CEE5, OnSetWidgetTalentStarPath);
                patch::RedirectCall(0x56BF0C, OnSetWidgetTalentStarPath);
                patch::RedirectCall(0x56DF10, OnSetWidgetTalentStarPath);

                patch::RedirectCall(0x100E800 + 0x2E, OnCopyMailIconFileName);
                patch::RedirectCall(0x100E800 + 0x42, OnCopyMailIconFileName);
                patch::RedirectCall(0x100E800 + 0x56, OnCopyMailIconFileName);
                patch::RedirectCall(0x100E800 + 0x6A, OnCopyMailIconFileName);
                patch::RedirectCall(0x100E800 + 0x7E, OnCopyMailIconFileName);
                patch::RedirectCall(0x100E800 + 0x92, OnCopyMailIconFileName);
                patch::RedirectCall(0x100E800 + 0xA6, OnCopyMailIconFileName);
                patch::RedirectCall(0x100E800 + 0xBA, OnCopyMailIconFileName);

                patch::RedirectCall(0x9635CC, OnSetWidgetCalendarIconPath);
                patch::RedirectCall(0x96515C, OnSetWidgetCalendarIconPath);
                patch::RedirectCall(0x964BE7, OnSetWidgetCalendarIconPath);
                patch::RedirectCall(0x964E4A, OnSetWidgetCalendarIconPath);
                patch::RedirectCall(0x964E78, OnSetWidgetCalendarIconPath);
                patch::RedirectCall(0x964AA1, OnCopyCalendarIconPath);

                patch::RedirectCall(0x9DEE3D, OnSetWidgetBrowseArrow);
                patch::RedirectCall(0x9DEE4D, OnSetWidgetBrowseArrow);

                patch::RedirectCall(0xD634EA, OnGetPlayerIconPath1);
                patch::RedirectCall(0xD63620, OnGetPlayerIconPath1);
                patch::RedirectCall(0xD2B85A, OnGetPlayerIconPath2);
                patch::RedirectCall(0xD2B879, OnGetPlayerIconPath2);

                patch::RedirectCall(0xD2B70C, OnCopyConfidenceIconPath);
                patch::RedirectCall(0xD2B6B2, OnCopyMoraleIconPath);
                patch::RedirectCall(0xD2B6CA, OnCopyMoraleIconPath);
                patch::RedirectCall(0x96D911, OnCopyUpcomingEventsIconPath);
                patch::RedirectCall(0x96E111, OnCopyUpcomingEventsIconPath);

                patch::RedirectCall(0x5DBDDD, OnSetWidgetIcoPyramid);

                patch::Nop(0x5A9E00, 8); // 13TeamLineUpAssistancePop hardcoded color
                patch::Nop(0x5A9E50, 8); // 13TeamLineUpAssistancePop hardcoded color

                patch::RedirectCall(0x129FAC1, MyGetTrainingIconPath);

                patch::RedirectCall(0xC3FD95, MySetLbItemYouthTeamSquad);
                patch::RedirectCall(0xC3FE04, MySetLbItemYouthTeamSquad);
                patch::RedirectCall(0xC3FE5D, MySetLbItemYouthTeamSquad);
                patch::RedirectCall(0xC3FD42, MySetLbItemYouthTeamSquad);
                patch::RedirectCall(0xC3FF13, MySetLbItemYouthTeamSquad);
                patch::RedirectCall(0xC3FF48, MySetLbItemYouthTeamSquad);

                patch::RedirectCall(0xA32512, MyConstructArtFmLibMediaString);

                patch::RedirectCall(0xD19C75, OnSetWidgetProgressBar08);

				patch::RedirectCall(0xC131F9, MyAddHalfTimeSpeechEffectIcon);
				patch::RedirectCall(0xC134A5, MyAddHalfTimeSpeechEffectIcon);
            }

            if (theme == DARK || theme == LIGHT) {
                patch::RedirectCall(0xEB4781, OnConstructEmployeeOffice);
                patch::RedirectCall(0xEB6125, OnConstructEmployeeOffice);
                patch::RedirectCall(0xEB82A6, OnConstructEmployeeOffice);
            }
        }

        patch::RedirectJump(0x4E8D2D, (void *)0x4E8D5E);

        patch::RedirectCall(0x96B3C2, CalendarFixtures_InitColumnTypes);
        patch::RedirectCall(0x96B3F5, CalendarFixtures_InitColumnFormatting);
        patch::RedirectCall(0x96A8E2, CalendarFixtures_GetResultString);
        patch::RedirectCall(0x96AD29, CalendarFixtures_AddColumnResultEmpty);
        patch::RedirectCall(0x96A8FC, CalendarFixtures_AddColumnResult);

        patch::RedirectCall(0x64D6CE, ClubFixtures_InitColumnTypes);
        patch::RedirectCall(0x64D6FC, ClubFixtures_InitColumnFormatting);
        patch::RedirectCall(0x651B46, ClubFixtures_StoreScore);
        patch::RedirectCall(0x651F5E, ClubFixtures_AddColumnResultEmpty);
        patch::RedirectCall(0x651B63, ClubFixtures_AddColumnResult);
    }
}
