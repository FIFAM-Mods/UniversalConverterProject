#include "3dMatchStandings.h"
#include "FifamReadWrite.h"
#include "shared.h"
#include "license_check/license_check.h"

const UInt STANDINGS3D_ORIGINAL_STRUCT_SIZE = 0x4F8;

Vector<Pair<UInt, String>> &GetPopupColors() {
    static Vector<Pair<UInt, String>> popupColors;
    return popupColors;
}

Vector<Pair<UInt, String>>& GetPopupBadges() {
	static Vector<Pair<UInt, String>> popupBadges;
	return popupBadges;
}

String GetPopupColorsPathForCompetition(UInt compId) {
    if (compId != 0) {
        for (auto &[id, p] : GetPopupColors()) {
            if (id <= 0xFFFF) {
                if (id == ((compId >> 16) & 0xFFFF))
                    return p;
            }
            else if (id == compId)
                return p;
        }
    }

    return Magic<'u','c','p','_','p','o','p','u','p','s','\\','c','o','l','o','r','s','\\','v','e','r','t','\\'>(3038749816);
}

String GetPopupBadgesPathForCompetition(UInt compId) {
	for (auto& [id, p] : GetPopupBadges()) {
		if (id <= 0xFFFF) {
			if (id == ((compId >> 16) & 0xFFFF))
				return p;
		}
		else if (id == compId)
			return p;
	}
	return String();
}

void ReadColorsSettingsFile() {
    // compId, pathToColorsFolder
    FifamReader reader(Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','p','o','p','u','p','_','c','o','l','o','r','s','.','d','a','t'>(3144975254), 14);
    if (reader.Available()) {
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                UInt compId;
                String pathToColors;
                reader.ReadLine(Hexadecimal(compId), pathToColors);
                Utils::Replace(pathToColors, L"/", L"\\");
                if (!Utils::EndsWith(pathToColors, L"\\"))
                    pathToColors += L"\\";
                GetPopupColors().emplace_back(compId, pathToColors);
            }
            else
                reader.SkipLine();
        }
    }
}

void ReadBadgesSettingsFile() {
	// compId, pathToColorsFolder
	FifamReader reader(Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','p','o','p','u','p','_','b','a','d','g','e','s','.','d','a','t'>(2287039899), 14);
	if (reader.Available()) {
		while (!reader.IsEof()) {
			if (!reader.EmptyLine()) {
				UInt compId;
				String pathToColors;
				reader.ReadLine(Hexadecimal(compId), pathToColors);
				Utils::Replace(pathToColors, L"/", L"\\");
				if (!Utils::EndsWith(pathToColors, L"\\"))
					pathToColors += L"\\";
				GetPopupBadges().emplace_back(compId, pathToColors);
			}
			else
				reader.SkipLine();
		}
	}
}

struct Standings3dAdditionalData {
    void *mImgAddedTime;
    void *mTbAddedTime;
    void *mImgAggregate;
    void *mTbAggregate;
    void *mImgColor1;
    void *mImgColor2;
};

struct AdditionalDataOneCustomBadge {
	void* mImgCustomBadge;
	void* mImgColor1;
	void* mImgColor2;
};

struct AdditionalDataCustomBadges {
	void* mImgCustomBadge1;
	void* mImgCustomBadge2;
	void* mImgColor1;
	void* mImgColor2;
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

void SetVisible(void *widget, bool visible) {
    CallVirtualMethod<11>(widget, visible);
}

void SetText(void *widget, WideChar const *text) {
    CallVirtualMethod<78>(widget, text);
}

unsigned char SetImageFilename(void *widget, std::wstring const &path) {
    return CallAndReturn<unsigned char, 0xD32860>(widget, path.c_str(), 0, 0);
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

void METHOD OnCreateStandingsUI(void *standingsInterface) {
    // get additional elements
    auto additionalData = GetStandingsAdditionalData(standingsInterface);
    additionalData->mImgAddedTime = GetImage(standingsInterface, "ImgAddedTime");
    additionalData->mTbAddedTime = GetTextBox(standingsInterface, "TbAddedTime");
    additionalData->mImgAggregate = GetImage(standingsInterface, "ImgAggregate");
    additionalData->mTbAggregate = GetTextBox(standingsInterface, "TbAggregate");
    additionalData->mImgColor1 = GetImage(standingsInterface, "ImgColor1");
    additionalData->mImgColor2 = GetImage(standingsInterface, "ImgColor2");
    // create original UI
    CallMethod<0xD108D0>(standingsInterface);
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
    // setup team colors
    void *homeTeam = *raw_ptr<void *>(standingsInterface, 0x4CC);
    void *awayTeam = *raw_ptr<void *>(standingsInterface, 0x4D0);
    if (homeTeam && awayTeam) {
        UInt compId = 0;
        void *currentCompetition = *raw_ptr<void *>(standingsInterface, 0x4D4);
        if (currentCompetition)
            compId = *raw_ptr<UInt>(currentCompetition, 0x18);
        String colorsPathStr = GetPopupColorsPathForCompetition(compId);
        if (!colorsPathStr.empty()) {
            UInt homeTeamUId = *raw_ptr<UInt>(homeTeam, 0xF0);
            UInt awayTeamUId = *raw_ptr<UInt>(awayTeam, 0xF0);
        
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
            WideChar const *homeTeamKitPath = raw_ptr<WideChar const>(kitsData, 0x220 + 0x454);
            WideChar const *awayTeamKitPath = raw_ptr<WideChar const>(kitsData, 0x220 + 0x65C + 0x454);
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
            //SafeLog::Write(Utils::Format(L"%08X (%d) - %08X (%d)", homeTeamUId, homeTeamKitId, awayTeamUId, awayTeamKitId));
            //
            String homeTeamColorPath = GetPathForTeamKitColor(colorsPathStr, homeTeamUId, homeTeamKitId);
            if (!homeTeamColorPath.empty() && FileExists(homeTeamColorPath)) {
                SetImageFilename(additionalData->mImgColor1, homeTeamColorPath);
                //SafeLog::Write(L"home team color set");
            }
            String awayTeamColorPath = GetPathForTeamKitColor(colorsPathStr, awayTeamUId, awayTeamKitId);
            if (!awayTeamColorPath.empty() && FileExists(awayTeamColorPath)) {
                SetImageFilename(additionalData->mImgColor2, awayTeamColorPath);
                //SafeLog::Write(L"away team color set");
            }
        }
    }
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


void Patch3dMatchStandings(FM::Version v) {
    ReadColorsSettingsFile();
    if (v.id() == ID_FM_13_1030_RLD) {
        // expand struct size
        const UInt newStructSize = STANDINGS3D_ORIGINAL_STRUCT_SIZE + sizeof(Standings3dAdditionalData);
        patch::SetUInt(0xBF68D4 + 1, newStructSize);
        patch::SetUInt(0xBF68DB + 1, newStructSize);
        // install hooks
        patch::RedirectCall(0xB3AF4F, OnCreateStandingsUI); // UNSAFE HOOK
        patch::SetPointer(0x245D834, OnUpdateTime); // UNSAFE HOOK
        //patch::Nop(0xB3A820, 22);
        patch::RedirectCall(0xB3A791, OnUpdateTimeAndScore); // UNSAFE HOOK
        patch::Nop(0xB3A796, 34);
        //
        patch::RedirectCall(0xB3A787, OnUpdateScore1);
        patch::SetPointer(0x245D868, OnUpdateScore2);
    }
}
