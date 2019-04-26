#include "Competitions.h"

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
     { L"EURO_NATIONSLEAGUE", 36 }, // Nations League
     { L"YOUTH_CHAMPIONSLEAGUE", 37 }, // Youth Champions League

     { L"", 0 }
};

const unsigned char gNumCompetitionTypes = std::extent<decltype(gNewCompTypeNames)>::value + 10;

unsigned int gContinentalCompetitionTypes[] = {
    9, 10, 12, 11, 13, 35, 37
};

unsigned int gNewIntlComps[] = { 36 };

void *GetCompetition(unsigned char region, unsigned char type, unsigned short index) {
    return CallAndReturn<void *, 0xF8AFC0>(region, type, index);
}

void *GetCompetition(unsigned int *id) {
    return CallAndReturn<void *, 0xF8AF50>(id);
}

void *GetCompetition(unsigned int id) {
    return CallAndReturn<void *, 0xF8AF50>(&id);
}

// TODO: replace with translated strings
void OnSetCompetitionName(int compRegion, int compType, wchar_t const *name) {
    Call<0xF904D0>(compRegion, compType, name);
    Call<0xF904D0>(249, 35, L"International Champions Cup");
    Call<0xF904D0>(255, 36, L"UEFA Nations League");
    Call<0xF904D0>(255, 37, L"UEFA Youth League");
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
    ReadCompetitionFile(L"EuroNationsLeague.txt", L"EURO_NATIONSLEAGUE", root);
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
    case 36: // EURO_NATIONSLEAGUE
        if (year % 2 == 0)
            return true;
        break;
    default:
        return true;
    }
    return false;
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

    if (year % 2 == 0) LaunchCompetition(255, 36, 0); // EURO_NATIONSLEAGUE
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
    if ((gLastCompType >= 15 && gLastCompType <= 18) || gLastCompType == 36)
        return GetCompetition(pCompId);
    return nullptr;
}

unsigned char gCompTypesClub[] =         { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 8, 19, 20, 21, 35, 23, 27, 35 };
unsigned char gCompTypesClubYouth[] =    { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 8, 19, 20, 21, 35, 23, 35, 37 };
unsigned char gCompTypesNationalTeam[] = { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 8, 19, 20, 21, 15, 16, 17, 18, 32, 33, 29, 36 };
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
    movzx eax, cl
    cmp eax, gCurrFixturesCompTypes[edx]
    mov eax, 0x96A6BD
    jmp eax
}}

void __declspec(naked) CheckFixtureList_2_Exec() { __asm {
    mov edx, [esp+0x14]
    movzx eax, cl
    cmp eax, gCurrFixturesCompTypes[edx]
    mov eax, 0x96AABD
    jmp eax
}}


//unsigned char gCompTypesForInternationalCalendar[] = {};

void PatchCompetitions(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

        patch::SetUChar(0x23D4A84, 36);

        patch::SetPointer(0xF815C8 + 1, gNewCompTypeNames);
        patch::SetPointer(0xF87462 + 1, gNewCompTypeNames);
        // comp name
        patch::RedirectCall(0xF9793F, OnSetCompetitionName);
        patch::RedirectCall(0xF97033, OnSetupRoot);
        // comp types
        patch::SetPointer(0x11F2CFE + 2, gContinentalCompetitionTypes);
        patch::SetPointer(0x11F2D4F + 2, gContinentalCompetitionTypes);
        patch::SetUChar(0x11F2D6E + 2, 6 * 4);
        // stats browser 1 - CStatsCupFixturesResults (Cup overview) - continental
        patch::SetUChar(0x703659 + 2, gNumCompetitionTypes - 1);
        patch::SetUChar(0x704F59 + 2, gNumCompetitionTypes - 1);
        patch::SetUChar(0x7061CE + 2, gNumCompetitionTypes - 1);
        // stats browser 1 - CStatsCupFixturesResults (Cup overview) - international
        patch::SetUChar(0x703592, 0xEB);
        patch::SetUChar(0x7035D6, 0xEB);
        patch::SetUChar(0x703515, 0xEB);
        patch::SetUChar(0x703564 + 2, gNumCompetitionTypes - 1);

        // standings
        patch::SetUChar(0xE3EBC1 + 2, gNumCompetitionTypes - 1);

        // unknown - weekly progress?
        patch::SetUChar(0x9776D1 + 2, gNumCompetitionTypes - 1);

        /*patch::RedirectCall(0x977635, Comps1);
        patch::Nop(0x97764C, 5);
        patch::Nop(0x977624, 2);*/

        // matchday cup results
        patch::SetUChar(0xAA115E + 2, gNumCompetitionTypes - 1);

        /*patch::SetUChar(0xAA1008, 0xEB);
        patch::SetUChar(0xAA1077, 0xEB);
        patch::SetUChar(0xAA0FD7 + 2, gNumCompetitionTypes - 1);*/

        // next matchday info?
        patch::SetUChar(0xACE9CE + 2, gNumCompetitionTypes - 1);

        patch::RedirectJump(0x11F4ADC, LaunchCompetitions_Exec);
        patch::RedirectJump(0xF83180, TakesPlaceInThisYear);

        // fixture list
        /*patch::RedirectCall(0x96A3EF, SetupFixturesList);
        patch::RedirectJump(0x96A6B6, CheckFixtureList_1_Exec);
        patch::RedirectJump(0x96AAB5, CheckFixtureList_2_Exec);
        patch::SetUChar(0x96A4B6 + 4, std::extent<decltype(gCompTypesClubYouth)>::value); // Youth clubs
        patch::SetUChar(0x96A4D1 + 4, std::extent<decltype(gCompTypesClub)>::value); // Clubs
        patch::SetUChar(0x96A431 + 1, std::extent<decltype(gCompTypesNationalTeam)>::value); // National teams*/
    }
}
