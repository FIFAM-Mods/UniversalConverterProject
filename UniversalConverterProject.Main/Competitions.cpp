#include "Competitions.h"
#include "GameInterfaces.h"
#include "ContinentalOceania.h"
#include <map>

using namespace plugin;

struct CompTypeNameDesc { const wchar_t *name; unsigned int id; };

CompTypeNameDesc gNewCompTypeNames[] = {
     { L"ROOT", 0 }, // Root competition
     { L"LEAGUE", 1 }, // League
     { L"FA_CUP", 3 }, // FA Cup
     { L"LE_CUP", 4 }, // League Cup
     { L"SUPERCUP", 7 }, // Supercup
     { L"CHALLENGE_SHIELD", 5 }, // Challenge Shield
     { L"CONFERENCE_CUP", 6 }, // Conference Cup
     { L"RELEGATION", 8 }, // Relegation
     { L"CHAMPIONSLEAGUE", 9 }, // Champions League
     { L"UEFA_CUP", 10 }, // UEFA Cup/Europa League
     { L"TOYOTA", 11 }, // TOYOTA Cup
     { L"EURO_SUPERCUP", 12 }, // European Supercup
     { L"WORLD_CLUB_CHAMP", 13 }, // World Club Championship
     { L"UIC", 14 }, // Intertoto Cup
     { L"QUALI_WC", 15 }, // World Cup Qualification
     { L"QUALI_EC", 16 }, // Euro Qualification
     { L"WORLD_CUP", 17 }, // World Cup - Final Stage
     { L"EURO_CUP", 18 }, // Euro - Final Stage
     { L"INDOOR", 22 }, // Indoor competition
     { L"RESERVE", 28 }, // Reserve competition
     { L"POOL", 24 }, // Pool
     { L"U20_WORLD_CUP", 31 }, // U-20 World Cup
     { L"CONFED_CUP", 32 }, // Confederations Cup
     { L"COPA_AMERICA", 33 }, // Copa America
     { L"ICC", 35 }, // International Champions Cup
     { L"EURO_QUALI_NL", 36 }, // Nations League Quali
     { L"EURO_NL", 37 }, // Nations League Finals
     { L"YOUTH_CHAMPIONSLEAGUE", 38 }, // Youth Champions League

     { L"", 0 }
};

const unsigned char gNumCompetitionTypes = std::extent<decltype(gNewCompTypeNames)>::value + 10;

unsigned int gContinentalCompetitionTypes[] = {
    9, 10, 12, 11, 13, 35
};

unsigned int gNewIntlComps[] = { 36, 37 };

unsigned int gInternationalCompsTypes[] = { 36, 15, 16, 37, 17, 18, 32, 33 }; // for NT matches info screen

// TODO: replace with translated strings
void OnSetCompetitionName(int compRegion, int compType, wchar_t const *name) {
    Call<0xF904D0>(compRegion, compType, name);
    Call<0xF904D0>(249, 35, GetTranslation("IDS_NAME_ICC"));
    Call<0xF904D0>(255, 36, GetTranslation("IDS_NAME_EURO_NL_QUALIFICATION"));
    Call<0xF904D0>(255, 37, GetTranslation("IDS_NAME_EURO_NL"));
    Call<0xF904D0>(249, 38, GetTranslation("IDS_NAME_EURO_YOUTH_LEAGUE"));
}

void ReadCompetitionFile(wchar_t const *filename, wchar_t const *typeName, void *root) {
    static wchar_t buf[MAX_PATH];
    void *resolver = CallAndReturn<void *, 0x40BF10>();
    wchar_t *filepath = CallVirtualMethodAndReturn<wchar_t *, 5>(resolver, buf, 73, filename, 0, 0, 0, 0);
    unsigned int file[4];
    CallMethod<0x14B2BEA>(file);
    if (CallMethodAndReturn<bool, 0x14B2C35>(file, filepath)) {
        unsigned int rootCompId = 0;
        if (root)
            rootCompId = *raw_ptr<unsigned int>(root, 0x18);
        Call<0xF92660>(file, typeName, &rootCompId, -1, 0, 0, 0);
        CallMethod<0x14B3160>(file);
    }
    CallMethod<0x14B2C24>(file);
}

void METHOD OnSetupRoot(void *root, DUMMY_ARG, int id) {
    //ReadCompetitionFile(L"EuroNationsLeague.txt", L"EURO_NATIONSLEAGUE", root);
    CallMethod<0x11F21B0>(root, id);
}

bool METHOD TakesPlaceInThisYear(void *comp, DUMMY_ARG, int) {
    void *game = CallAndReturn<void *, 0xF61410>();
    unsigned int date;
    CallMethod<0xF499A0>(game, &date);
    unsigned short year = CallMethodAndReturn<unsigned short, 0x1494CC9>(&date);
    switch (*raw_ptr<unsigned char>(comp, 0x1A)) {
    case 15: // QUALI_WC
        if (!(year % 4))
            return true;
        break;
    case 16: // QUALI_EC
        if (year % 4 == 2)
            return true;
        break;
    case 17: // WORLD_CUP
        if (year % 4 == 1)
            return true;
        break;
    case 18: // EURO_CUP
        if (year % 4 == 3)
            return true;
        break;
    case 31: // U20_WORLD_CUP
        if (year % 2 == 1)
            return true;
        break;
    case 32: // CONFED_CUP
        if (year % 4 == 0)
            return true;
        break;
    case 33: // COPA_AMERICA
        if (year % 4 == 2)
            return true;
        break;
    case 36: // EURO_QUALI_NATIONSLEAGUE
    case 37: // EURO_NATIONSLEAGUE
        if (year % 2 == 0)
            return true;
        break;
    default:
        return true;
    }
    return false;
}

WideChar const * METHOD GetCompetitionShortName(CDBCompetition *comp) {
    switch (comp->GetCompID().type) {
    case COMP_WORLD_CLUB_CHAMP:
        return GetTranslation("IDS_FIFA_CLUB_WORLD_CUP_SHORTNAMES");
    case COMP_QUALI_WC:
        return GetTranslation("IDS_FIFA_WORLD_CUP_QUALIFICATION_SHORTNAMES");
    case COMP_WORLD_CUP:
        return GetTranslation("IDS_FIFA_SHORTNAMES");
    case COMP_CONFED_CUP:
        return GetTranslation("IDS_FIFA_CONFED_CUP_SHORTNAMES");
    case COMP_EURO_CUP:
        return GetTranslation("IDS_EURO_CUP_SHORTNAMES");
    case COMP_QUALI_EC:
        return GetTranslation("IDS_EURO_CUP_QUALIFICATION_SHORTNAMES");
    case COMP_COPA_AMERICA:
        return GetTranslation("IDS_COPA_AMERICA_SHORTNAMES");
    case 36: // EURO_QUALI_NL
        return GetTranslation("IDS_EURO_NL_QUALIFICATION_SHORTNAMES");
    case 37: // EURO_NL
        return GetTranslation("IDS_EURO_NL_SHORTNAMES");
    }
    if (CallMethodAndReturn<Bool, 0x12D4AA0>(&comp->GetCompID()) || CallMethodAndReturn<Bool, 0x12D4A40>(&comp->GetCompID())) {
        CDBCompetition *baseComp = GetCompetition(comp->GetCompID().ToInt() & 0xFFFF0000);
        if (baseComp)
            return raw_ptr<WideChar>(baseComp, 0x26);
    }
    return raw_ptr<WideChar>(comp, 0x26);
}

void LaunchCompetition(unsigned char region, unsigned char type, unsigned short index) {
    void *comp = GetCompetition(region, type, index);
    if (comp)
        CallVirtualMethod<4>(comp);
}

void LaunchCompetitions() {
    void *game = CallAndReturn<void *, 0xF61410>();
    unsigned short year = CallMethodAndReturn<unsigned short, 0xF498C0>(game);

    if (year % 2 == 1) LaunchCompetition(255, 31, 0); // U20_WORLD_CUP
    if (year % 4 == 0) LaunchCompetition(255, 32, 0); // CONFED_CUP
    if (year % 4 == 2) LaunchCompetition(255, 33, 0); // COPA_AMERICA

    if (year % 2 == 0) LaunchCompetition(255, 36, 0); // EURO_QUALI_NATIONSLEAGUE
}

void __declspec(naked) LaunchCompetitions_Exec() {
    __asm call LaunchCompetitions
    __asm mov eax, 0x11F5AF8
    __asm jmp eax
}

void Comps1(unsigned int compId, void *employee, void *vec) {
    unsigned int copaAmericaId = 0xFF210000;
    if (GetCompetition(copaAmericaId)) {
        Call<0x9767C0>(copaAmericaId, employee, vec);
        Call<0x976CC0>(copaAmericaId, employee, vec);
    }
    for (auto id : gNewIntlComps) {
        unsigned int newCompId = 0xFF000000 | (id << 16);
        if (GetCompetition(newCompId)) {
            Call<0x9767C0>(newCompId, employee, vec);
            Call<0x976CC0>(newCompId, employee, vec);
        }
    }
}

unsigned char gLastCompType = 0;

bool METHOD MatchdayScreen_TestGameFlag(void *game, DUMMY_ARG, unsigned int flag) {
    if (gLastCompType >= 15 && gLastCompType <= 18)
        return CallMethodAndReturn<bool, 0xF49CA0>(game, flag);
    return false;
}

void *MatchdayScreen_GetCompById(unsigned int *pCompId) {
    gLastCompType = (*pCompId >> 16) & 0xFF;
    if ((gLastCompType >= 15 && gLastCompType <= 18) || gLastCompType == 36 || gLastCompType == 37)
        return GetCompetition(pCompId);
    return nullptr;
}

unsigned char gCompTypesClub[] =         { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 8, 19, 20, 21, 35, 23, 27, 35 };
unsigned char gCompTypesClubYouth[] =    { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 8, 19, 20, 21, 35, 23, 35, 38 };
unsigned char gCompTypesNationalTeam[] = { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 8, 19, 20, 21, 15, 16, 17, 18, 32, 33, 29, 36, 37 };
unsigned char *gCurrFixturesCompTypes = gCompTypesClub;

void *SetupFixturesList(unsigned int teamId) {
    void *team = CallAndReturn<void *, 0xEC8F70>(teamId);
    if (team) {
        unsigned short teamIndex = teamId & 0xFFFF;
        if (teamIndex == 0xFFFF)
            gCurrFixturesCompTypes = gCompTypesNationalTeam;
        else {
            unsigned char teamType = (teamId >> 24) & 0xFF;
            if (teamType == 2 || teamType == 4)
                gCurrFixturesCompTypes = gCompTypesClubYouth;
            else
                gCurrFixturesCompTypes = gCompTypesClub;
        }
    }
    return team;
}

void __declspec(naked) CheckFixtureList_1_Exec() { __asm {
    shr ecx, 0x10
    mov eax, gCurrFixturesCompTypes
    add eax, edx
    mov al, byte ptr[eax]
    cmp cl, al
    mov eax, 0x96A6BD
    jmp eax
}}

void __declspec(naked) CheckFixtureList_2_Exec() { __asm {
    mov edx, [esp+0x14]
    mov eax, gCurrFixturesCompTypes
    add eax, edx
    mov al, byte ptr[eax]
    cmp cl, al
    mov eax, 0x96AABD
    jmp eax
}}

void METHOD ChampionsLeagueUniversalSort(CDBPool *comp, DUMMY_ARG, int numGroups, int numTeamsInGroup) {
    if (Oceania_OnSortPool(comp))
        return;
    switch (comp->GetNumOfTeams()) {
    case 48:
        numGroups = 12;
        numTeamsInGroup = 4;
        break;
    case 40:
        numGroups = 10;
        numTeamsInGroup = 4;
        break;
    case 32:
        numGroups = 8;
        numTeamsInGroup = 4;
        break;
    case 30:
        numGroups = 10;
        numTeamsInGroup = 3;
        break;
    case 24:
        numGroups = 6;
        numTeamsInGroup = 4;
        break;
    case 20:
        numGroups = 5;
        numTeamsInGroup = 4;
        break;
    case 16:
        numGroups = 4;
        numTeamsInGroup = 4;
        break;
    case 15:
        numGroups = 5;
        numTeamsInGroup = 3;
        break;
    case 12:
        numGroups = 4;
        numTeamsInGroup = 3;
        break;
    case 10:
        numGroups = 2;
        numTeamsInGroup = 5;
        break;
    case 9:
        numGroups = 3;
        numTeamsInGroup = 3;
        break;
    case 8:
        numGroups = 2;
        numTeamsInGroup = 4;
        break;
    case 6:
        numGroups = 2;
        numTeamsInGroup = 3;
        break;
    }
    CallMethod<0x10F2340>(comp, numGroups, numTeamsInGroup);
}

unsigned char METHOD GetPoolNumberOfTeamsFromCountry(CDBPool *pool, DUMMY_ARG, int countryId) {
    int numTeams = Oceania_GetPoolTeamsFromCountry(pool, countryId);
    if (numTeams >= 0)
        return numTeams;
    numTeams = 0;
    CDBCompetition *comp = nullptr;
    for (int i = 0; i < pool->GetNumOfScriptCommands(); i++) {
        auto command = pool->GetScriptCommand(i);
        switch (command->m_nCommandId) {
        case 2: // RESERVE_ASSESSMENT_TEAMS
            if (countryId == GetAssesmentTable()->GetCountryIdAtPosition(LOWORD(command->m_params)))
                numTeams += HIWORD(command->m_params);
            break;
        case 4: // GET_EUROPEAN_ASSESSMENT_TEAMS
            if (countryId == GetAssesmentTable()->GetCountryIdAtPosition(LOWORD(command->m_params)))
                numTeams += HIBYTE(command->m_params);
            break;
        case 7: // GET_TAB_X_TO_Y
            if (countryId == command->m_competitionId.countryId)
                numTeams += HIBYTE(command->m_params);
            break;
        case 12: // GET_EUROPEAN_ASSESSMENT_CUPWINNER
            if (countryId == GetAssesmentTable()->GetCountryIdAtPosition(LOWORD(command->m_params)))
                --numTeams;
            break;
        case 28: // GET_INTERNATIONAL_TEAMS
            if (countryId == command->m_competitionId.countryId)
                numTeams += command->m_params;
            break;
        case 31: // GET_CHAMP_COUNTRY_TEAM
            comp = GetCompetition(command->m_competitionId);
            if (comp && countryId == pool->GetChampion().countryId)
                ++numTeams;
            break;
        default:
            continue;
        }
    }
    if (pool->GetCompID().index == 0 && pool->GetCompID().countryId == 253) { // asia continental
        comp = GetCompetition((pool->GetCompID().ToInt() & 0xFFFF0001) | 1);
        if (comp && comp->GetDbType() == 5) // DB_POOL
            numTeams += comp->GetNumTeamsFromCountry(countryId);
    }
    else if (pool->GetCompID().countryId == 250 && pool->GetCompID().type == COMP_UEFA_CUP) { // south america continental
        auto copaLibertadoresPool = GetCompetition(250, COMP_CHAMPIONSLEAGUE, 0);
        if (copaLibertadoresPool) {
            numTeams -= copaLibertadoresPool->GetNumTeamsFromCountry(countryId);
            auto copaLibertadoresFinal = GetCompetition(250, COMP_CHAMPIONSLEAGUE, 22);
            if (copaLibertadoresFinal && copaLibertadoresFinal->GetChampion().countryId == countryId)
                ++numTeams;
        }
    }
    return (numTeams > 0) ? numTeams : 0;
}

unsigned int gParticipantsRegion = 0;
unsigned int gParticipantsCompType = 0;
unsigned int gParticipantsListId = 0;

void METHOD OnFillEuropeanCompsParticipants(void *obj, DUMMY_ARG, unsigned int region, unsigned int compType, unsigned int listId) {
    gParticipantsRegion = region;
    gParticipantsCompType = compType;
    gParticipantsListId = listId;
    for (unsigned int i = 0; i <= 5; i++) {
        if (plugin::CallAndReturn<bool, 0xFF7F60>(i)) {
            gParticipantsRegion = 249 + i;
            break;
        }
    }
    // TODO: implement all continents
    CallMethod<0x88F670>(obj, gParticipantsRegion, gParticipantsCompType, gParticipantsListId);
}

int METHOD EuropeanCompsParticipants_GetNumEntries(void *obj) {
    if (gParticipantsRegion == 254 && gParticipantsCompType == COMP_CHAMPIONSLEAGUE)
        return std::size(Oceania_ParticipantsCountries);
    return plugin::CallAndReturn<int, 0x121D1C0>(obj);
}

unsigned char METHOD EuropeanCompsParticipants_GetCountryAtPosition(void *obj, DUMMY_ARG, int position) {
    if (gParticipantsRegion == 254 && gParticipantsCompType == COMP_CHAMPIONSLEAGUE) {
        if (position > 0 && position <= (int)std::size(Oceania_ParticipantsCountries))
            return Oceania_ParticipantsCountries[position - 1];
        return 0;
    }
    return plugin::CallAndReturn<unsigned char, 0x121CFF0>(obj, position);
}

CDBPool *EuropeanCompsParticipants_GetPool(unsigned int countryId, unsigned int compType, unsigned short index) {
    //if (countryId == 254 && compType == COMP_CHAMPIONSLEAGUE)
    //    index = 4;
    return plugin::CallAndReturn<CDBPool *, 0xF8C5C0>(countryId, compType, index);
}

void OnGetSpare(CDBCompetition **ppComp) {
    if (*ppComp) {
        if (Oceania_OnGetParticipants(*ppComp))
            return;
        Call<0x139D9A0>(ppComp);
    }
}

//unsigned char gCompTypesForInternationalCalendar[] = {};

void METHOD OnAddMyCareerTrophyNationalTeam(void *vec, DUMMY_ARG, void *pValue) {
    CallMethod<0x416950>(vec, pValue);
    static UInt compType = 37;
    CallMethod<0x416950>(vec, &compType);
}

UChar GetCompetitionGroupByCompetitionId(UInt competitionId) {
    CDBLeague *league = nullptr;
    switch ((competitionId >> 16) & 0xFF) {
    case COMP_LEAGUE:
        return 1;
    case COMP_FA_CUP:
    case COMP_LE_CUP:
        return 2;
    case COMP_RELEGATION:
        league = GetLeague(competitionId);
        if (league && league->GetNumMatchdays() >= 4)
            return 1;
        break;
    case COMP_CHAMPIONSLEAGUE:
    case COMP_UEFA_CUP:
    case COMP_TOYOTA:
    case COMP_EURO_SUPERCUP:
    case COMP_WORLD_CLUB_CHAMP:
        return 3;
    case COMP_QUALI_WC:
    case COMP_QUALI_EC:
    case COMP_WORLD_CUP:
    case COMP_EURO_CUP:
    case COMP_INTERNATIONAL_FRIENDLY:
    case COMP_U20_WORLD_CUP:
    case COMP_CONFED_CUP:
    case COMP_COPA_AMERICA:
    case 36:
    case 37:
        return 4;
    case COMP_FRIENDLY:
    case COMP_UNKNOWN_27:
    case COMP_RESERVE:
        return 5;
    }
    return 0;
}

UChar METHOD GetCompetitionTypeForNationalTeamHost(CDBCompetition *comp) {
    UChar compType = *raw_ptr<UChar>(comp, 0x1A);
    if (compType == 37)
        return COMP_WORLD_CUP;
    return compType;
}

Char const *GetNationalTeamECQualifiedTeamsScreenName() {
    if (GetCurrentYear() == 2020)
        return "Screens/10NationalTeamECQualifiedTeams_2020_FF12.xml";
    return "Screens/10NationalTeamECQualifiedTeams.xml";
}

Char const *GetNationalTeamTournamentWelcomeScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010TournamentWelcome.xml";
    if (game->TestFlag(2)) {
        return "Screens/10NationalTeamWCTournamentWelcome.xml";
    }
    else {
        //if (game->TestFlag(4);
        if (game->GetCurrentYear() == 2020)
            return "Screens/10NationalTeamECTournamentWelcome_2020_FF12.xml";
        return "Screens/10NationalTeamECTournamentWelcome.xml";
    }
    return "Screens/10NationalTeamECTournamentWelcome.xml";
}

Char const *GetNationalTeamTournamentGroupsScreenName() {
    if (*(UChar *)0x3111C74 == 2) {
        auto game = CDBGame::GetInstance();
        if (game->GetCurrentYear() == 2020)
            return "Screens/11NationalTeamECTournamentGroups_2020_FF12.xml";
        return "Screens/11NationalTeamECTournamentGroups.xml";
    }
    return "Screens/11NationalTeamWCTournamentGroups.xml";
}

Char const *GetNationalTeamECTournamentFinalsScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetCurrentYear() == 2020)
        return "Screens/10NationalTeamECTournamentFinals_2020_FF12.xml";
    return "Screens/10NationalTeamECTournamentFinals.xml";
}

Char const *GetNationalTeamTournamentReportScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010TournamentReport.xml";
    if (game->TestFlag(4) || !game->TestFlag(2)) {
        if (game->GetCurrentYear() == 2020)
            return "Screens/10NationalTeamECTournamentReport_2020_FF12.xml";
        return "Screens/10NationalTeamECTournamentReport.xml";
    }
    return "Screens/10NationalTeamWCTournamentReport.xml";
}

Char const *GetNationalTeamTournamentDreamTeamScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010DreamTeam.xml";
    if (game->TestFlag(4) || !game->TestFlag(2)) {
        if (game->GetCurrentYear() == 2020)
            return "Screens/10NationalTeamECTournamentDreamTeam_2020_FF12.xml";
        return "Screens/10NationalTeamECTournamentDreamTeam.xml";
    }
    return "Screens/10NationalTeamWCTournamentDreamTeam.xml";
}

Char const *GetNationalTeamTournamentECUpsAndDownsScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetCurrentYear() == 2020)
        return "Screens/13NationalTeamECUpsAndDowns_2020_FF12.xml";
    return "Screens/13NationalTeamECUpsAndDowns.xml";
}

UInt gTmpAddress = 0;
wchar_t gTmpMessage[256];

void __declspec(naked) GetNumOfTeamsInComp() {
    __asm {
        mov eax, dword ptr[esp]
        mov gTmpAddress, eax
        test ecx, ecx
        jz SHOW_ERR
        mov eax, [ecx + 0xA8]
        //cdq
        //sub eax, edx
        //sar eax, 1
        retn
        SHOW_ERR :
    }
        swprintf(gTmpMessage, L"0x%X", gTmpAddress);
        MessageBoxW(NULL, gTmpMessage, L"Season 2019 (v.1.3) - Exception", MB_ICONERROR);
    __asm {
        xor eax, eax
        retn
    }
}

CDBRound *GetECLast16Round(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_EURO_CUP, 12); // Last16
    if (!round) {
        round = GetRound(255, COMP_EURO_CUP, 15);
        if (!round) {
            round = GetRoundByRoundType(255, COMP_EURO_CUP, 13); // Quarterfinal
            if (!round)
                round = GetRound(255, COMP_EURO_CUP, 10);
        }
    }
    return round;
}

CDBRound *GetECQuarterfinal(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_EURO_CUP, 13);
    if (!round) {
        round = GetRound(255, COMP_EURO_CUP, 16);
        if (!round)
            round = GetRound(255, COMP_EURO_CUP, 10);
    }
    return round;
}

CDBRound *GetECSemifinal(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_EURO_CUP, 14);
    if (!round) {
        round = GetRound(255, COMP_EURO_CUP, 17);
        if (!round)
            round = GetRound(255, COMP_EURO_CUP, 11);
    }
    return round;
}

CDBRound *GetECFinal(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_EURO_CUP, 15);
    if (!round) {
        round = GetRound(255, COMP_EURO_CUP, 18);
        if (!round)
            round = GetRound(255, COMP_EURO_CUP, 12);
    }
    return round;
}

void METHOD OnSetupTeamsForECQuarterfinal(void *screen, DUMMY_ARG, UChar roundType, CDBCompetition *comp) {
    CDBRound *last16 = GetECLast16Round(0, 0, 0);
    if (last16)
        CallMethod<0x8304A0>(screen, 0, last16);
    CallMethod<0x8304A0>(screen, 1, comp);
}

struct ECFinalsAdditionalData {
    CTeamIndex mTeamIDs[30];
    void *mFlags[30];
    void *mTextBoxes[60];
};

ECFinalsAdditionalData *GetECFinalsAdditionalData(void *screen) {
    return raw_ptr<ECFinalsAdditionalData>(screen, 0x594);
}

void * METHOD OnCreateNationalTeamTournamentFinalsEC(void *screen, DUMMY_ARG, Int a) {
    CallMethod<0x830390>(screen, a);
    auto data = GetECFinalsAdditionalData(screen);
    for (UInt i = 0; i < 30; i++) {
        data->mTeamIDs[i].countryId = 0;
        data->mTeamIDs[i].type = 0;
        data->mTeamIDs[i].index = 0;
        data->mFlags[i] = nullptr;
        data->mTextBoxes[i] = nullptr;
        data->mTextBoxes[i + 30] = nullptr;
    }
    return screen;
}

void __declspec(naked) ECSetupTeamsForFinal() {
    __asm {
        cmp al, 2
        jnz CHECK_FOR_FINAL
        lea esi, [ebx + 0x744]
        lea edi, [ebx + 0x66C]
        mov[esp + 0x10], 24
        jmp RETN_BACK
    CHECK_FOR_FINAL:
        cmp al, 3
        jnz RETN_BACK
        lea esi, [ebx + 0x764]
        lea edi, [ebx + 0x67C]
        mov [esp + 0x10], 28
     RETN_BACK:
        mov eax, 0x830510
        jmp eax
    }
}

CDBLeague *OnGetLeagueTournamentWelcome(UChar region, UChar type, UShort index) {
    if (type == COMP_EURO_CUP) {
        CDBLeague *league = GetLeague(region, type, index + 6);
        if (league)
            return league;
    }
    return GetLeague(region, type, index);
}

void PatchCompetitions(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

        ///patch::RedirectCall(0x10F47CB, ChampionsLeagueUniversalSort);

        ///patch::SetPointer(0x24B1874, GetPoolNumberOfTeamsFromCountry);

        ///patch::RedirectCall(0x88F9DD, OnFillEuropeanCompsParticipants);
        ///patch::RedirectCall(0x88F9ED, OnFillEuropeanCompsParticipants);

        ///patch::RedirectCall(0x88F73C, EuropeanCompsParticipants_GetNumEntries);
        ///patch::RedirectCall(0x88F85D, EuropeanCompsParticipants_GetNumEntries);
        ///patch::RedirectCall(0x88F75D, EuropeanCompsParticipants_GetCountryAtPosition);
        ///patch::RedirectCall(0x88F6F6, EuropeanCompsParticipants_GetPool);

        ///patch::RedirectCall(0x139EFAF, OnGetSpare);

        
        //patch::SetUChar(0x23D4A84, 36);
        patch::SetPointer(0x5EF7D0 + 2, gInternationalCompsTypes);
        patch::SetUChar(0x5F03ED + 2, (UChar)(std::size(gInternationalCompsTypes) * 4));
        // B8 64 00 00 00
        patch::SetUInt(0x5EF7FC, 0x000064B8);
        patch::SetUChar(0x5EF7FC + 4, 0);
        patch::Nop(0x5EF7FC + 5, 2);

        patch::SetPointer(0xF815C8 + 1, gNewCompTypeNames);
        patch::SetPointer(0xF87462 + 1, gNewCompTypeNames);
        // comp name
        patch::RedirectCall(0xF9793F, OnSetCompetitionName);
        //patch::RedirectCall(0xF97033, OnSetupRoot);
        // comp types
        patch::SetPointer(0x11F2CFE + 2, gContinentalCompetitionTypes);
        patch::SetPointer(0x11F2D4F + 2, gContinentalCompetitionTypes);
        patch::SetUChar(0x11F2D6E + 2, 6 * 4);

        // CStatsCupFixturesResults
        patch::SetUChar(0x703659 + 2, gNumCompetitionTypes - 1);
        patch::SetUChar(0x703592, 0xEB);
        patch::SetUChar(0x7035D6, 0xEB);
        patch::SetUChar(0x703515, 0xEB);
        patch::SetUChar(0x703564 + 2, gNumCompetitionTypes - 1);

        // CStatsCupResults
        patch::SetUChar(0x704F59 + 2, gNumCompetitionTypes - 1);
        patch::SetUChar(0x704E4E, 0xEB);
        patch::SetUChar(0x704E92, 0xEB);
        patch::SetUChar(0x704ED6, 0xEB);
        patch::Nop(0x704DFB, 2);
        patch::Nop(0x704E00, 2);
        patch::SetUChar(0x704E25 + 2, gNumCompetitionTypes - 1);
        
        // CStatsCupTablesFinals
        patch::SetUChar(0x7061CE + 2, gNumCompetitionTypes - 1);

        // standings
        patch::SetUChar(0xE3EBC1 + 2, gNumCompetitionTypes - 1);

        // unknown - weekly progress?
        patch::SetUChar(0x9776D1 + 2, gNumCompetitionTypes - 1);

        //patch::RedirectCall(0x977635, Comps1);
        //patch::Nop(0x97764C, 5);
        //patch::Nop(0x977624, 2);

        // matchday cup results
        patch::SetUChar(0xAA115E + 2, gNumCompetitionTypes - 1);

        //patch::SetUChar(0xAA1008, 0xEB);
        //patch::SetUChar(0xAA1077, 0xEB);
        //patch::SetUChar(0xAA0FD7 + 2, gNumCompetitionTypes - 1);

        // next matchday info?
        patch::SetUChar(0xACE9CE + 2, gNumCompetitionTypes - 1);

        patch::RedirectJump(0x11F4ADC, LaunchCompetitions_Exec);
        patch::RedirectJump(0xF83180, TakesPlaceInThisYear);

        // fixture list
        patch::RedirectCall(0x96A3EF, SetupFixturesList);
        patch::RedirectJump(0x96A6B6, CheckFixtureList_1_Exec);
        patch::RedirectJump(0x96AAB5, CheckFixtureList_2_Exec);
        patch::SetUInt(0x96A4B6 + 4, std::size(gCompTypesClubYouth)); // Youth clubs
        patch::SetUInt(0x96A4D1 + 4, std::size(gCompTypesClub)); // Clubs
        patch::SetUInt(0x96A431 + 1, std::size(gCompTypesNationalTeam)); // National teams
        
        // competition short names
        patch::RedirectJump(0xF8B3E0, GetCompetitionShortName);

        patch::SetUChar(0xEFBCAF + 2, gNumCompetitionTypes - 4);
        static UChar gCompWinYearsJumpTable[] = { 0, 1, 11, 11, 2, 11, 3, 4, 5, 6, 7, 11, 11, 11, 8, 9, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 10, 11 };
        patch::SetPointer(0xEFBCB8 + 3, gCompWinYearsJumpTable);
        patch::RedirectCall(0x7DA52D, OnAddMyCareerTrophyNationalTeam);

        // international goals
        patch::RedirectJump(0xF8DF90, GetCompetitionGroupByCompetitionId);

        // tournament status (groups)
        static UInt gTournamentStatusGroupsComps[] = { 15, 16, 17, 18, 32, 33, 36, 37 };
        patch::SetPointer(0x844334 + 2, gTournamentStatusGroupsComps);
        patch::SetUChar(0x8443A0 + 2, (UChar)std::size(gTournamentStatusGroupsComps) * 4);

        // tournament status (fixtures and tables)
        static UInt gTournamentStatusFixturesAndTablesComps[] = { 15, 16, 17, 18, 32, 33, 36, 37 };
        patch::SetPointer(0x837760 + 3, gTournamentStatusFixturesAndTablesComps);
        patch::SetPointer(0x8377A8 + 3, gTournamentStatusFixturesAndTablesComps);
        patch::SetUChar(0x83776A + 2, (UChar)std::size(gTournamentStatusFixturesAndTablesComps));
        patch::SetUChar(0x837785 + 2, (UChar)std::size(gTournamentStatusFixturesAndTablesComps));
        patch::SetUInt(0x83778E, 0x3C24448B);
        patch::SetUInt(0x837738 + 4, 100);

        // international hosts - national team
        patch::RedirectCall(0x1044950, GetCompetitionTypeForNationalTeamHost);

        // Euro
        const UInt NumEuroTeams = 24;
        const UInt NumEuroGroups = 6;

        // Euro - welcome
        patch::SetUChar(0x836409 + 2, NumEuroTeams - 1);
        patch::SetUInt(0x835E8F + 4, NumEuroGroups);
        patch::SetChar(0x8359D1 + 2, NumEuroTeams - 18);

        // Euro - screens
        patch::RedirectJump(0x829880, GetNationalTeamECQualifiedTeamsScreenName);
        patch::RedirectJump(0x835590, GetNationalTeamTournamentWelcomeScreenName);
        patch::RedirectJump(0x831DE0, GetNationalTeamTournamentGroupsScreenName);
        patch::RedirectJump(0x8300E0, GetNationalTeamECTournamentFinalsScreenName);
        patch::RedirectJump(0x832940, GetNationalTeamTournamentReportScreenName);
        patch::RedirectJump(0x82E9E0, GetNationalTeamTournamentDreamTeamScreenName);
        patch::RedirectJump(0xE5F100, GetNationalTeamTournamentECUpsAndDownsScreenName);

        patch::RedirectCall(0x106D2F3, GetECLast16Round);
        patch::RedirectCall(0x106D571, GetECLast16Round);

        patch::RedirectCall(0x49B747, OnCreateNationalTeamTournamentFinalsEC);
        patch::SetUInt(0x49B714 + 1, 0x594 + sizeof(ECFinalsAdditionalData));
        patch::SetUInt(0x49B71B + 1, 0x594 + sizeof(ECFinalsAdditionalData));
        patch::SetUInt(0x830190 + 1, 1);
        patch::SetUInt(0x830184 + 2, 0x594 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x8301CA + 1, 1);
        patch::SetUInt(0x8301CF + 2, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUInt(0x830201 + 1, 30);
        patch::SetUInt(0x83021F + 2, 0x594 + 3 * 4 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x830225 + 1, 15);
        patch::SetUInt(0x830249 + 2, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUInt(0x83024F + 1, 30);

        patch::SetUInt(0x830AEC + 2, 0x594 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUChar(0x830B04 + 2, 60);
        patch::SetUInt(0x830B0B + 2, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUChar(0x830B23 + 2, 30);
        patch::SetUInt(0x830B3A + 3, 0x594 + offsetof(ECFinalsAdditionalData, mTeamIDs) + 2);
        patch::SetUInt(0x830B5F + 3, 0x594 + offsetof(ECFinalsAdditionalData, mTeamIDs) + 2);
        patch::SetUInt(0x830B44 + 3, 0x594 + offsetof(ECFinalsAdditionalData, mTeamIDs));
        patch::SetUInt(0x830B69 + 3, 0x594 + offsetof(ECFinalsAdditionalData, mTeamIDs));
        
        patch::RedirectCall(0x8309BF, GetECQuarterfinal);
        patch::RedirectCall(0x8309DE, GetECSemifinal);
        patch::RedirectCall(0x8309FD, GetECFinal);
        patch::RedirectCall(0x8309D0, OnSetupTeamsForECQuarterfinal);
        patch::SetUChar(0x8309CC + 1, 1);
        patch::SetUChar(0x8309EB + 1, 2);
        patch::SetUChar(0x830A0C + 1, 3);
        
        patch::SetUInt(0x8304CA + 2, 0x594 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x8304D0 + 2, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUInt(0x83052C + 3, 0x594 + 4 + offsetof(ECFinalsAdditionalData, mTeamIDs));
        patch::SetUInt(0x8304E2 + 2, 0x594 + 32 * 4 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x8304E8 + 2, 0x594 + 16 * 4 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUInt(0x8304EE + 4, 16);
        patch::RedirectJump(0x8304F8, ECSetupTeamsForFinal);

        patch::SetUInt(0x830455 + 3, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));

        patch::RedirectCall(0x835EC9, OnGetLeagueTournamentWelcome);
    }
}
