#include "EuropeanGoldenShoe.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "FifamReadWrite.h"
#include "Utils.h"
#include "DebugPrint.h"

using namespace plugin;

struct GoldenShoeWinner {
    UShort year;
    NameDesc name;
    UChar countryId;
    UChar goals;
    UShort points;
    CJDate birthdate;
    UInt empicsId;
    UInt clubId;
};

static_assert(sizeof(GoldenShoeWinner) == 0x18, "Failed");

Vector<GoldenShoeWinner> &GoldenShoeWinners() {
    static Vector<GoldenShoeWinner> vec;
    return vec;
}

void ClearGoldenShoeWinners() {
    GoldenShoeWinners().clear();
}

void METHOD OnReadFifaWorldPlayers(CDBGame *game) {
    CallMethod<0xF67230>(game);
    ClearGoldenShoeWinners();
    FifamReader r(L"fmdata\\historic\\EuropeanGoldenShoe.txt");
    if (r.Available()) {
        r.SkipLine();
        while (!r.IsEof()) {
            if (!r.EmptyLine()) {
                String firstName, lastName, clubName, pointsStr;
                UInt clubId;
                UShort year, goals;
                UChar countryId;
                Date birthdate;
                r.ReadLineWithSeparator(L'\t', year, lastName, firstName, birthdate, countryId, Hexadecimal(clubId), clubName, goals, pointsStr);
                if (year > 0 && year <= GetStartingYear()) {
                    GoldenShoeWinner winner;
                    winner.year = year;
                    if (goals > 256)
                        goals = 256;
                    winner.goals = goals;
                    winner.points = 0;
                    if (!pointsStr.empty() && pointsStr != L"-") {
                        Float points = Utils::SafeConvertFloat(pointsStr);
                        if (points > 300.0f)
                            points = 300.0f;
                        winner.points = (UShort)(points * 100.0f);
                    }
                    winner.countryId = countryId;
                    winner.birthdate.Set(birthdate.year, birthdate.month, birthdate.day);
                    winner.name.languageId = GetCountryFirstLanguage(countryId);
                    CDBPlayer *player = FindPlayerByStringID(winner.name.ToPlayerStringID(winner.birthdate, 0).c_str());
                    if (player) {
                        winner.name = player->GetNameDesc();
                        winner.empicsId = player->GetEmpicsID();
                    }
                    else {
                        if (firstName.empty()) {
                            winner.name.firstName = GetNamePools()->AddName(winner.name.languageId, 2, lastName.c_str());
                            winner.name.lastName = 0;
                            winner.name.flags = 2;
                        }
                        else {
                            winner.name.firstName = GetNamePools()->AddName(winner.name.languageId, 1, firstName.c_str());
                            winner.name.lastName = GetNamePools()->AddName(winner.name.languageId, 4, lastName.c_str());
                            winner.name.flags = 0;
                        }
                        winner.empicsId = 0;
                    }
                    winner.clubId = clubId;
                    GoldenShoeWinners().push_back(winner);
                }
            }
            else
                r.SkipLine();
        }
    }
}

UInt METHOD OnLoadGameData_LoadGoldenShoeWinners(void *save) {
    ClearGoldenShoeWinners();
    if (SaveGameLoadGetVersion(save) >= 47) {
        UInt count = SaveGameReadSize(save);
        GoldenShoeWinners().resize(count);
        for (UInt i = 0; i < count; i++)
            SaveGameReadData(save, &GoldenShoeWinners()[i], sizeof(GoldenShoeWinner));
    }
    return SaveGameReadSize(save);
}

void METHOD OnSaveGameData_SaveGoldenShoeWinners(void *save, DUMMY_ARG, UInt size) {
    SaveGameWriteSize(save, GoldenShoeWinners().size());
    for (UInt i = 0; i < GoldenShoeWinners().size(); i++)
        SaveGameWriteData(save, &GoldenShoeWinners()[i], sizeof(GoldenShoeWinner));
    SaveGameWriteSize(save, size);
}

void METHOD OnInitGame(CDBGame *game) {
    ClearGoldenShoeWinners();
    CallMethod<0xF598E0>(game);
}

void METHOD OnDestroyGame(CDBGame *game) {
    ClearGoldenShoeWinners();
    CallMethod<0xF5BD40>(game);
}

class StatsEuropeanGoldenShoe {
public:
    void *vtable;
    UChar data[0x4CC - 4];
    UChar listBox[0x704];
    void *tbScreenDesc;
    void *tbScreenName;

    static void METHOD CreateUI(StatsEuropeanGoldenShoe *t) {
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        CallMethod<0x6E2340>(t, 1);
        CallMethod<0xD4F110>(t); // CXgFMPanel::CreateUI()
        listBox->Create((CXgFMPanel *)t, "Lb_ForeignPlayers");
        t->tbScreenDesc = CreateTextBox(t, "Tb_ForeignPlayers");
        t->tbScreenName = CreateTextBox(t, "Tb_ScreenName");
        CFMListBox::InitColumnTypes(listBox, 0, 2, 6, 4, 37, 9, 12, 58, 63);
        CFMListBox::InitColumnFormatting(listBox, 210, 210, 204, 204, 204, 210, 210, 210, 228);
        FillTable(t);
    }

    static void METHOD FillTable(StatsEuropeanGoldenShoe *t) {
        auto winners = GoldenShoeWinners();
        Utils::Sort(winners, [](GoldenShoeWinner const &a, GoldenShoeWinner const &b) {
            return a.year <= b.year;
        });
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        listBox->Clear();
        UInt color = CallMethodAndReturn<UInt, 0x6E22E0>(t); // CStatsBaseScr::GetTextColor()
        Map<String, UInt> totalWins;
        for (UInt i = 0; i < winners.size(); i++) {
            auto &w = winners[i];
            Int lastRowIndex = listBox->GetMaxRows() - 1;
            if (listBox->GetNumRows() >= lastRowIndex)
                break;
            String season = Utils::Format(L"%04d/", w.year - 1) + Utils::Format(L"%04d", w.year).substr(2);
            listBox->AddColumnString(season.c_str(), color, 0);
            listBox->AddCountryFlag(w.countryId, 0);
            CDBPlayer *player = FindPlayerByStringID(w.name.ToPlayerStringID(w.birthdate, 0).c_str());
            if (player)
                listBox->AddColumnInt(player->GetID(), color, 0);
            else
                listBox->AddColumnString(w.name.ToName(), color, 0);
            CDBTeam *team = GetTeamByUniqueID(w.clubId);
            if (team)
                listBox->AddTeamName(team->GetTeamID(), color, 0);
            else {
                auto teamName = GetTranslationIfPresent(Utils::Format("IDS_TEAMNAME_%08X", w.clubId).c_str());
                if (teamName)
                    listBox->AddColumnString(teamName, color, 0);
                else
                    listBox->AddColumnString(GetTranslation("DLG_NA"), color, 0);
            }
            CTeamIndex teamUID = CTeamIndex::make(w.clubId);
            if (!teamUID.isNull())
                listBox->AddCompetitionName(CCompID::Make(teamUID.countryId, COMP_LEAGUE, 0), color, 0);
            else
                listBox->AddColumnString(GetTranslation("DLG_NA"), color, 0);
            listBox->AddColumnInt(w.goals, color, 0);
            listBox->AddColumnFloat((Float)w.points / 100.0f, color, 0);
            listBox->NextRow(0);
            if (w.points == 0)
                listBox->GetCellTextBox(i, 6)->SetText(L"-");
            String playerStringID = w.name.ToPlayerStringID(w.birthdate, w.empicsId);
            if (Utils::Contains(totalWins, playerStringID))
                totalWins[playerStringID]++;
            else
                totalWins[playerStringID] = 1;
            String playerName = listBox->GetCellTextBox(i, 2)->GetText();
            if (Utils::EndsWith(playerName, L" (C)"))
                playerName = playerName.substr(0, playerName.size() - 4);
            if (totalWins[playerStringID] > 1)
                playerName += Utils::Format(L" (x%d)", totalWins[playerStringID]);
            listBox->GetCellTextBox(i, 2)->SetText(playerName.c_str());
        }
        listBox->Sort(0, true);
    }

    StatsEuropeanGoldenShoe(void *guiInstance);
};

UInt VtableStatsEuropeanGoldenShoe[] = {
    0x02532AD4,
    0x006E4750,0x00D378B0,0x004F2030,0x006E49B0,
    (UInt)&StatsEuropeanGoldenShoe::CreateUI, // 0x006E4680,
    0x004F2090,0x00D2AE00,0x00D2AE20,0x004F20A0,0x004060B0,0x004F23A0,0x004F2130,0x004F23D0,0x004F23C0,0x004F23B0,0x004F3370,0x004F2460,0x004F1FE0,0x004F2020,0x004060C0,0x00D2AC60,0x006E4760,0x00D2ADC0,0x00D4A7A0,0x00D2AFB0,0x00D2AFD0,0x00D2AFC0,0x00D2AFE0,0x00D2AFF0,0x00D2B000,0x00D2B010,0x00D2B020,0x00D2B030,0x00D2B040,0x00D2B060,0x00D2B050,0x00D2B070,0x00D2B080,0x00D2B090,0x00D2B0A0,0x00D2B0B0,0x00D2B0C0,0x00D2B0D0,0x00D2B0E0,0x00D2B0F0,0x00D2B100,0x006D8000,0x00D2B110,0x00D2B120,0x00D2B130,0x00D2B140,0x00D38B40,0x00D38BD0,0x00D38C60,0x00D38CF0,0x004060D0,0x00D2B150,0x00D2B160,0x00D2B170,0x00D2ADD0,0x00D2ADE0,0x00D2ADF0,0x00D37A70,0x00D32790,0x00D327D0,0x00D42CB0,0x00D32230,0x00D32390,0x00D2AF60,0x00D2AF90,0x00D39F10,0x00D3A6A0,0x00D39FA0,0x00D3A030,0x00D3A0C0,0x00D3A150,0x00D3A300,0x00D3A4E0,0x00D3A750,0x00D3AB20,0x00D3B3D0,0x00D3B5D0,0x00D3ADF0,0x00D3B140,0x004060E0,0x004060F0,0x006E4770,0x006E47D0,0x006E2300,0x006E4140,0x006E4830,0x006E4860,0x006E25E0,
    (UInt)&StatsEuropeanGoldenShoe::FillTable // 0x006E4B50
};

StatsEuropeanGoldenShoe::StatsEuropeanGoldenShoe(void *guiInstance) {
    CallMethod<0x6E3EC0>(this, guiInstance, 0); // CStatsBaseScr::CStatsBaseScr
    vtable = &VtableStatsEuropeanGoldenShoe[1];
    CallMethod<0xD1AC00>(listBox); // CFMListBox::CFMListBox()
}

class StatsEuropeanGoldenShoeWrapper : public CStatsBaseScrWrapper {
public:
    StatsEuropeanGoldenShoeWrapper(WideChar const *name) {
        CallMethod<0x14978B3>(m_name, name);
        m_nType = 1;
    }

    virtual void *CreateScreen(void *parentInstance) override {
        Char const *screenName = "Screens/10StatsEuropeanGoldenBoot.xml";
        WideChar screenNameW[260];
        void *pDummyPool = *(void **)0x309BBEC;
        void *pGuiFrame = *(void **)0x30C8940;
        void *pGuiStringDb = *(void **)0x3121C04;
        Call<0x1493F12>(screenNameW, 260, screenName); // MultibyteToWide()
        void *instance = CallMethodAndReturn<void *, 0x4EDF40>(pGuiFrame, screenNameW, pGuiStringDb, parentInstance, 0, 0); // CGuiFrame::LoadScreen()
        void *scr = CallVirtualMethodAndReturn<void *, 1>(pDummyPool, sizeof(StatsEuropeanGoldenShoe)); // pDummyPool->Allocate()
        Call<0x1573400>(scr, 0, sizeof(StatsEuropeanGoldenShoe)); // CMemoryMgr::Fill()
        StatsEuropeanGoldenShoe *s = new (scr) StatsEuropeanGoldenShoe(instance);
        CallMethod<0x4F20E0>(s, instance); // CXgPanel::SetName()
        CallVirtualMethod<0x11>(instance, s); // screenGui->SetMessageProc()
        CallVirtualMethod<4>(s); // s->CreateUI()
        return s;
    }
};

void METHOD CreateStatsEuropeanGoldenShoeWrapper(void *vec, DUMMY_ARG, void *data) {
    CallMethod<0x736690>(vec, data);
    void *mem = CallAndReturn<void *, 0x15738F3>(sizeof(StatsEuropeanGoldenShoeWrapper));
    StatsEuropeanGoldenShoeWrapper *w = new (mem) StatsEuropeanGoldenShoeWrapper(GetTranslation("IDS_EUROPEAN_GOLDEN_BOOT"));
    CallMethod<0x736690>(vec, &w);
}

void PatchEuropeanGoldenShoe(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x108F56C, OnReadFifaWorldPlayers); // CDBGame::ReadFifaWorldPlayers
        patch::RedirectCall(0xF5E178, OnLoadGameData_LoadGoldenShoeWinners);
        patch::RedirectCall(0xF5705B, OnSaveGameData_SaveGoldenShoeWinners);
        patch::RedirectCall(0xF5B106, OnInitGame);
        patch::RedirectCall(0x108D840, OnInitGame);
        patch::RedirectCall(0xF5FED2, OnDestroyGame);
        patch::RedirectCall(0x73865A, CreateStatsEuropeanGoldenShoeWrapper);
    }
}
