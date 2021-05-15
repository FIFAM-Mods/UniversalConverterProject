#include "Competitions.h"
#include "Utils.h"
#include "FifamTypes.h"

using namespace plugin;

struct CompTypeNameDesc { const wchar_t *name; unsigned int unk; unsigned int id; };

CompTypeNameDesc gNewCompTypeNames[] = {
     { L"ROOT", 0, 0 }, // Root competition
     { L"LEAGUE", 0, 1 }, // League
     { L"FA_CUP", 0, 3 }, // FA Cup
     { L"LE_CUP", 0, 4 }, // League Cup
     { L"SUPERCUP", 0, 7 }, // Supercup
     { L"CHALLENGE_SHIELD", 0, 5 }, // Challenge Shield
     { L"CONFERENCE_CUP", 0, 6 }, // Conference Cup
     { L"RELEGATION", 0, 8 }, // Relegation
     { L"CHAMPIONSLEAGUE", 0, 9 }, // Champions League
     { L"UEFA_CUP", 0, 10 }, // UEFA Cup/Europa League
     { L"TOYOTA", 0, 11 }, // TOYOTA Cup
     { L"EURO_SUPERCUP", 0, 12 }, // European Supercup
     { L"WORLD_CLUB_CHAMP", 0, 13 }, // World Club Championship
     { L"UIC", 0, 14 }, // Intertoto Cup
     { L"QUALI_WC", 0, 15 }, // World Cup Qualification
     { L"QUALI_EC", 0, 16 }, // Euro Qualification
     { L"WORLD_CUP", 0, 17 }, // World Cup - Final Stage
     { L"EURO_CUP", 0, 18 }, // Euro - Final Stage
     { L"REL1", 0, 19 }, // Rel1
     { L"REL2", 0, 20 }, // Rel1
     { L"REL3", 0, 21 }, // Rel1
     { L"INDOOR", 0, 22 }, // Indoor competition
     { L"FRIENDLY", 0, 23 }, // Friendly
     { L"POOL", 0, 24 }, // Pool
     { L"CONFED_CUP", 0, 32 }, // Confederations Cup
     { L"COPA_AMERICA", 0, 33 }, // Copa America

     { L"ICC", 0, 35 }, // International Champions Cup
     { L"Q_EURO_NL", 0, 36 }, // Nations League Quali
     { L"EURO_NL", 0, 37 }, // Nations League Finals
     { L"CONTINENTAL_1", 0, 39 }, // Continental 1
     { L"CONTINENTAL_2", 0, 40 }, // Continental 2
     { L"Q_NAM_NL", 0, 41 }, // North America Nations League Quali
     { L"NAM_NL", 0, 42 }, // North America Nations League
     { L"NAM_CUP", 0, 43 }, // North America Cup
     { L"Q_AFRICA_CUP", 0, 44 }, // Africa Cup Quali
     { L"AFRICA_CUP", 0, 45 }, // Africa Cup
     { L"Q_ASIA_CUP", 0, 46 }, // Asia Cup Quali
     { L"ASIA_CUP", 0, 47 }, // Asia Cup
     { L"Q_OFC_CUP", 0, 48 }, // OFC Cup Quali
     { L"OFC_CUP", 0, 49 }, // OFC Cup

     { nullptr, 0, 0 }
};

bool METHOD TakesPlaceInThisYear(void *comp, DUMMY_ARG, int year) {
    switch (*raw_ptr<unsigned char>(comp, 10)) {
    case COMP_QUALI_WC: // QUALI_WC
        if (!(year % 4))
            return true;
        break;
    case COMP_QUALI_EC: // QUALI_EC
    case COMP_ASIA_CUP:
    case COMP_ASIA_CUP_Q:
        if (year % 4 == 2)
            return true;
        break;
    case COMP_WORLD_CUP: // WORLD_CUP
        if (year % 4 == 1)
            return true;
        break;
    case COMP_EURO_CUP: // EURO_CUP
    case COMP_COPA_AMERICA: // COPA_AMERICA
    case COMP_OFC_CUP:
        if (year % 4 == 3)
            return true;
        break;
    case COMP_U20_WORLD_CUP: // U20_WORLD_CUP
    case COMP_NAM_NL:
    case COMP_NAM_NL_Q:
        if (year % 2 == 1)
            return true;
        break;
    case COMP_CONFED_CUP: // CONFED_CUP
        if (year % 4 == 0)
            return true;
        break;
    case COMP_EURO_NL_Q: // EURO_QUALI_NATIONSLEAGUE
    case COMP_EURO_NL: // EURO_NATIONSLEAGUE
    case COMP_AFRICA_CUP:
    case COMP_AFRICA_CUP_Q:
    case COMP_NAM_CUP:
        if (year % 2 == 0)
            return true;
        break;
    case COMP_YOUTH_CHAMPIONSLEAGUE:
        return false;
    default:
        return true;
    }
    return false;
}

struct ExternalScriptDesc {
    wchar_t const *filename;
    unsigned int rootId;
    wchar_t const *blockName;
    unsigned int flags;
};

ExternalScriptDesc gExternalScripts[] = {
    {L"Continental - Europe.txt", 249, L"EURO", 3},
    {L"Continental - South America.txt", 250, L"SOUTHAM", 3},
    {L"Continental - North America.txt", 251, L"INTAMERICA", 3},
    {L"Continental - Africa.txt", 252, L"INTAFRICA", 3},
    {L"Continental - Asia.txt", 253, L"INTASIA", 3},
    {L"Continental - Oceania.txt", 254, L"INTOCEANIA", 3},
    {L"WorldCup.txt", 255, L"WORLD_CUP", 2},
    {L"WorldCupQualification.txt", 255, L"QUALI_WC", 3},
    {L"ConfedCup.txt", 255, L"CONFED_CUP", 1},
    {L"EuropeanChampionship.txt", 255, L"EURO_CUP", 2},
    {L"EuropeanChampionshipQualification.txt", 255, L"QUALI_EC", 3},
    {L"EuropeanChampionship.txt", 255, L"EURO_NL", 1},
    {L"EuropeanChampionship.txt", 255, L"Q_EURO_NL", 1},
    {L"CopaAmerica.txt", 255, L"COPA_AMERICA", 2},
    {L"NorthAmericaCup.txt", 255, L"NAM_CUP", 1},
    {L"NorthAmericaCup.txt", 255, L"NAM_NL", 2},
    {L"NorthAmericaCup.txt", 255, L"Q_NAM_NL", 2},
    {L"AsiaCup.txt", 255, L"ASIA_CUP", 1},
    {L"AsiaCup.txt", 255, L"Q_ASIA_CUP", 1},
    {L"AfricaCup.txt", 255, L"AFRICA_CUP", 1},
    {L"AfricaCup.txt", 255, L"Q_AFRICA_CUP", 1},
    {L"OFCCup.txt", 255, L"OFC_CUP", 2},
    {L"OFCCup.txt", 255, L"Q_OFC_CUP", 2} // not implemented
};

wchar_t const *GetTranslation(char const *key) {
    void *locale = CallAndReturn<void *, 0x575200>();
    return CallMethodAndReturn<wchar_t *, 0x5756F0>(locale, key, *raw_ptr<unsigned int>(locale, 0x18));
}

void *GetLocale() {
    return CallAndReturn<void *, 0x575200>();
}

unsigned int gCalendarContinent = 0;
void SetupCalendarForContinent(unsigned int continent, unsigned int season, void *calendar) {
    gCalendarContinent = continent;
    Call<0x4FFBD0>(continent, season, calendar);
}

void METHOD RemoveCompetitionsFromCalendar(void *obj, DUMMY_ARG, unsigned char type) {
    if (gCalendarContinent != 0) {
        CallMethod<0x4FC310>(obj, COMP_EURO_NL);
        CallMethod<0x4FC310>(obj, COMP_EURO_NL_Q);
    }
    if (gCalendarContinent != 1)
        CallMethod<0x4FC310>(obj, COMP_COPA_AMERICA);
    if (gCalendarContinent != 2) {
        CallMethod<0x4FC310>(obj, COMP_NAM_CUP);
        CallMethod<0x4FC310>(obj, COMP_NAM_NL);
        CallMethod<0x4FC310>(obj, COMP_NAM_NL_Q);
    }
    if (gCalendarContinent != 3) {
        CallMethod<0x4FC310>(obj, COMP_AFRICA_CUP);
        CallMethod<0x4FC310>(obj, COMP_AFRICA_CUP_Q);
    }
    if (gCalendarContinent != 4) {
        CallMethod<0x4FC310>(obj, COMP_ASIA_CUP);
        CallMethod<0x4FC310>(obj, COMP_ASIA_CUP_Q);
    }
    if (gCalendarContinent != 5) {
        CallMethod<0x4FC310>(obj, COMP_OFC_CUP);
        CallMethod<0x4FC310>(obj, COMP_OFC_CUP_Q);
    }
}

wchar_t const *gCompBlockName = nullptr;
void OnFormatCompetitionBlockName(wchar_t *dst, wchar_t const *format, wchar_t const *name, unsigned int number) {
    gCompBlockName = name;
    swprintf(dst, format, name, number);
}

wchar_t const *METHOD OnFindFileBlock(void *file, DUMMY_ARG, wchar_t const *index, wchar_t *pc) {
    if (gCompBlockName) {
        String newBlockName = index;
        newBlockName += gCompBlockName;
        wchar_t const *result = CallMethodAndReturn<wchar_t const *, 0x511FF0>(file, newBlockName.c_str(), pc);
        gCompBlockName = nullptr;
        return result;
    }
    return CallMethodAndReturn<wchar_t const *, 0x511FF0>(file, index, pc);
}

void *gStoredRound = nullptr;

void METHOD StoreRound(void *round, DUMMY_ARG, unsigned int n) {
    gStoredRound = round;
    CallMethod<0x4FC8D0>(round, n);
}

template<bool S, bool L>
void METHOD ReadRoundDate(void *file, DUMMY_ARG, int *n) {
    CallMethod<0x5132E0>(file, n);
    if ((CallMethodAndReturn<bool, 0x4FD380>(gStoredRound, 2012 + S) || CallMethodAndReturn<bool, 0x4FD380>(gStoredRound, 2014 + S)) && *n && *n > 365)
        *raw_ptr<unsigned char>(gStoredRound, 0x316 + 365 * !S + (*n - 365)) = L + 1;
}

void *gCountryForComplexLeagueSystemCheck = nullptr;

Bool METHOD OnGetCountryForComplexLeagueSystemCheck(void *country) {
    gCountryForComplexLeagueSystemCheck = country;
    return CallMethodAndReturn<Bool, 0x4DE560>(gCountryForComplexLeagueSystemCheck);
}

void METHOD OnEnableLeagueCalendarButton(void *t, DUMMY_ARG, Bool32 enable) {
    Bool32 finalEnable = enable;
    if (finalEnable && gCountryForComplexLeagueSystemCheck) {
        finalEnable = CallMethodAndReturn<Bool, 0x4DD450>(gCountryForComplexLeagueSystemCheck);
        gCountryForComplexLeagueSystemCheck = nullptr;
    }
    CallMethod<0x5B9DAC>(t, finalEnable);
}

void PatchCompetitions(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::SetPointer(0x501DC5 + 1, gNewCompTypeNames);
        patch::SetPointer(0x501EF3 + 2, gNewCompTypeNames);
        patch::SetPointer(0x501F00 + 1, gNewCompTypeNames);
        patch::SetPointer(0x501F19 + 3, gNewCompTypeNames);
        patch::SetPointer(0x501F21 + 3, gNewCompTypeNames);
        patch::SetPointer(0x5031CF + 1, gNewCompTypeNames);
        patch::SetPointer(0x50326B + 1, gNewCompTypeNames);
        patch::SetPointer(0x50329F + 1, gNewCompTypeNames);
        patch::SetPointer(0x50331C + 1, gNewCompTypeNames);
        patch::SetPointer(0x503388 + 1, gNewCompTypeNames);
        patch::SetPointer(0x5033C8 + 1, gNewCompTypeNames);
        patch::SetPointer(0x503400 + 1, gNewCompTypeNames);
        patch::SetPointer(0x50350F + 2, gNewCompTypeNames);
        patch::SetPointer(0x50351B + 1, gNewCompTypeNames);
        patch::SetPointer(0x50352D + 3, gNewCompTypeNames);
        patch::SetPointer(0x503535 + 3, gNewCompTypeNames);
        patch::SetPointer(0x50351B + 1, gNewCompTypeNames);
        patch::SetPointer(0x50352D + 3, gNewCompTypeNames);
        patch::SetPointer(0x503535 + 3, gNewCompTypeNames);
        patch::SetPointer(0x50386C + 2, gNewCompTypeNames);
        patch::SetPointer(0x503879 + 1, gNewCompTypeNames);
        patch::SetPointer(0x503891 + 3, gNewCompTypeNames);
        patch::SetPointer(0x503899 + 3, gNewCompTypeNames);
        patch::SetPointer(0x50479A + 1, gNewCompTypeNames);
        patch::SetPointer(0x5048BE + 1, gNewCompTypeNames);
        patch::SetPointer(0x504AA1 + 1, gNewCompTypeNames);

        patch::RedirectJump(0x4FD380, TakesPlaceInThisYear);

        static unsigned char gInternationalCompsCalendars[93440 * std::size(gExternalScripts)];

        patch::SetUInt(0x501061 + 1, (unsigned int)gExternalScripts + 4);

        patch::SetPointer(0x4FFBFB + 2, gInternationalCompsCalendars);
        patch::SetPointer(0x4FFC34 + 2, gInternationalCompsCalendars);
        patch::SetPointer(0x501046 + 1, gInternationalCompsCalendars);
        patch::SetPointer(0x501071 + 4, gInternationalCompsCalendars);
        patch::SetPointer(0x5011B2 + 2, gInternationalCompsCalendars);
        patch::SetPointer(0x649817 + 1, gInternationalCompsCalendars);
        patch::SetInt(0x649810 + 1, std::size(gInternationalCompsCalendars));
        patch::SetPointer(0x4FFC3C + 2, gInternationalCompsCalendars + 2);
        patch::SetUInt(0x50107D + 4, std::size(gExternalScripts));

        patch::RedirectCall(0x4DE716, SetupCalendarForContinent);
        patch::Nop(0x4FFC5F, 2);
        patch::RedirectCall(0x4FFC65, RemoveCompetitionsFromCalendar);

        patch::RedirectCall(0x502337, OnFormatCompetitionBlockName);
        patch::RedirectCall(0x512100, OnFindFileBlock);

        //patch::SetUChar(0x4E3675 + 2, 2);

        // fix international rounds dates
        patch::RedirectCall(0x50549E, StoreRound);
        patch::RedirectCall(0x505622, ReadRoundDate<0, 0>);
        patch::RedirectCall(0x505649, ReadRoundDate<1, 0>);
        patch::RedirectCall(0x505698, ReadRoundDate<0, 1>);
        patch::RedirectCall(0x5056C0, ReadRoundDate<1, 1>);

        // remove fixture files
        patch::RedirectJump(0x4EA3C7, (void *)0x4EA416);
        patch::RedirectJump(0x4E6B98, (void *)0x4E6BD2);
        patch::Nop(0x4E6B06, 9);

        patch::SetUChar(0x50362C, 6); // GET_CC_SPARE fix

        // league levels fix

        static Char const *divNames[] = {
            "Database.Division.01",
            "Database.Division.02",
            "Database.Division.03",
            "Database.Division.04",
            "Database.Division.05",
            "Database.Division.06",
            "Database.Division.07",
            "Database.Division.08",
            "Database.Division.09",
            "Database.Division.10",
            "Database.Division.11",
            "Database.Division.12",
            "Database.Division.13",
            "Database.Division.14",
            "Database.Division.15",
            "Database.Division.16",
        };

        patch::SetPointer(0x4E3B23 + 2, divNames);

        // Editor - disable calendar editing for countries with complex league system
        Int bForceCalendar = GetPrivateProfileIntW(L"MAIN", L"FORCE_CALENDAR_EDITOR", 0, L".\\ucp.ini");
        if (!bForceCalendar) {
            patch::RedirectCall(0x44921D, OnGetCountryForComplexLeagueSystemCheck);
            patch::RedirectCall(0x449249, OnEnableLeagueCalendarButton);
        }
    }
}
