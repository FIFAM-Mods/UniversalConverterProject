#include "InterfaceTheme.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"

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
        if (screenNameW.starts_with(L"screens/") || screenNameW.starts_with(L"screens\\")) {
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
        if (screenNameA.starts_with("screens/") || screenNameA.starts_with("screens\\")) {
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
    CallMethod<0xA23F00>(t);
    auto currDate = CDBGame::GetInstance()->GetCurrentDate();
    auto dayOfWeek = currDate.GetDayOfWeek();
    for (UInt i = 0; i < 7; i++) {
        UInt color = (i == dayOfWeek) ? 0xff21252e : 0xfff1f1f1;
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
                    SetImageColorRGBA(*raw_ptr<void *>(t, 0x564 + i * 4), 0xff21252e);
                    imageColorSet = true;
                }
            }
        }
        if (!imageColorSet)
            SetImageColorRGBA(*raw_ptr<void *>(t, 0x564 + i * 4), 0xFFFFFFFF);
    }
}

void METHOD OnSetupWeeklyProgressCalendarDays_DarkTheme(void *t) {
    CallMethod<0x866D30>(t);
    auto currDate = CDBGame::GetInstance()->GetCurrentDate();
    auto dayOfWeek = currDate.GetDayOfWeek();
    for (UInt i = 0; i < 7; i++) {
        UInt color = (i == dayOfWeek) ? 0xff21252e : 0xfff1f1f1;
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
                    SetImageColorRGBA(*raw_ptr<void *>(t, 0x4AC + i * 4), 0xff21252e);
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
        if (!filepathStr.starts_with(GetCustomInterfaceFolderW() + L"\\"))
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

UInt OnGetTeamInterfaceColor(UInt tableId, UInt colorId) {
    UInt color = CallAndReturn<UInt, 0xED2EF0>(tableId, colorId);
    UChar r = (color >> 16) & 0xFF;
    UChar g = (color >> 8) & 0xFF;
    UChar b = (color) & 0xFF;
    if (r <= 0x30 && g <= 0x30 && b <= 0x30)
        return (color & 0xFF000000) | 0x6E6E6E;
    else if (r == 0x6E && g == 0x6E && b == 0x6E)
        return (color & 0xFF000000) | 0x919191;
    return color;
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

void METHOD MyCreateUI(void *t) {
    CallMethod<0xC11A20>(t);
    void *lb = raw_ptr<void>(t, 0x58C);
    void *unk = *raw_ptr<void *>(lb, 8);
    Error("%X", *raw_ptr<void *>(unk, 0));
}

void PatchInterfaceTheme(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

        //patch::SetPointer(0x246CC5C, MyCreateUI);

        GetCustomInterfaceFolderA() = Settings::GetInstance().Theme;
        if (!GetCustomInterfaceFolderA().empty()) {
            GetCustomInterfaceFolderW() = SettingsHelper::ToUTF16(GetCustomInterfaceFolderA());

            //patch::RedirectCall(0x4EDF96, OnLoadScreenResource);
            //patch::RedirectCall(0x4EF7F3, OnLoadScreenResource);
            //patch::RedirectCall(0x4DA50F, OnCopyScreensDirName);
            //patch::RedirectCall(0x4E97ED, OnConcatenateAlternativeScreenName);
            patch::SetPointer(0x23AB3C4, OnResolveScreenPath);
            patch::RedirectCall(0x4DA76B, OnCopyDeskBackgroundsDirName);
            patch::RedirectCall(0x4DA53D, OnCopyArtFmDirName);
            patch::RedirectCall(0x4E8B1C, OnLoadGuiColors);
            patch::RedirectCall(0xD2B73B, OnCopyExperienceIconDirName);

            String toolTipFilename = GetCustomInterfaceFolderW() + L"\\Tooltipp.tga";
            if (toolTipFilename.size() <= 37)
                injector::WriteMemoryRaw(0x23C25F8, (void *)toolTipFilename.c_str(), toolTipFilename.size() * 2 + 2, true);

            if (GetCustomInterfaceFolderW() == L"dark") {
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
            }

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

            patch::RedirectCall(0x9588D2, OnFormatTalentStarPath);
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
        }
    }
}