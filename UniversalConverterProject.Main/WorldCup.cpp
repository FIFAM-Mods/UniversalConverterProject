#include "WorldCup.h"
#include "GameInterfaces.h"
#include "FifamCompRegion.h"
#include "UcpSettings.h"

using namespace plugin;

Bool gChangedStartDateForWC = false;
const UChar NUM_PLAYERS_IN_WC_SQUAD = 26;
Int gSquadPlayerIDs[NUM_PLAYERS_IN_WC_SQUAD];
const UChar NUM_WC_TEAMS = 48;
UInt gWCCountryIDs[NUM_WC_TEAMS];
UInt gWCVenueIDs[20][3][16];

void METHOD SetSeasonStartDateForWorldCup(CDBGame *g, DUMMY_ARG, CJDate d) {
    gChangedStartDateForWC = true;
    g->SetCurrentDate(d);
    //Message(L"SetSeasonStartDateForWorldCup");
}

void METHOD OnLoadGameStartDatabase(void *t, DUMMY_ARG, Int e) {
    if (gChangedStartDateForWC) {
        gChangedStartDateForWC = false;
        CDBGame *g = CDBGame::GetInstance();
        if (g) {
            CJDate date;
            date.Set(patch::GetUInt(0xF5B121 + 1), patch::GetUChar(0xF5B11F + 1), patch::GetUChar(0xF5B11D + 1));
            g->SetCurrentDate(date);
            //Message(L"OnLoadGameStartDatabase");
        }
    }
    CallMethod<0x524A90>(t, e);
}

void __declspec(naked) WcSquadSize1() {
    __asm {
        mov ebp, gSquadPlayerIDs[edi * 4]
        mov esi, 0x108C5C7
        jmp esi
    }
}

void __declspec(naked) WcSquadSize2() {
    __asm {
        mov gSquadPlayerIDs[ecx * 4], eax
        mov edx, 0x108C4B8
        jmp edx
    }
}

void GetNationalTeamNominationSize_WC(UInt *outTotalPlayers, UInt *outGk) {
    if (CDBGame::GetInstance() && CDBGame::GetInstance()->GetIsWorldCupMode()) {
        *outTotalPlayers = 26;
        *outGk = 3;
    }
    else {
        *outTotalPlayers = 23;
        *outGk = 3;
    }
}

void METHOD NationalTeamNominationSize1(void *t, DUMMY_ARG, UShort a, UShort b, UShort c, UShort d, UShort e, UShort f) {
    if (CDBGame::GetInstance() && CDBGame::GetInstance()->GetIsWorldCupMode()) {
        CallMethod<0x106EBA0>(t, 9, 9, 10, 10, 4, 4);
        patch::SetUChar(0x308A0A4, 5);
        patch::SetUChar(0x308A0A8, 2);
    }
    else {
        CallMethod<0x106EBA0>(t, a, b, c, d, e, f);
        patch::SetUChar(0x308A0A4, 4);
        patch::SetUChar(0x308A0A8, 0);
    }
}

UInt METHOD OnLoadCompetitionFile(void *t, DUMMY_ARG, WideChar const *filePath) {
    if (CDBGame::GetInstance() && CDBGame::GetInstance()->GetIsWorldCupMode()) {
        Path p = filePath;
        if (p.filename().string() == "WorldCup.txt") {
            Path newp = p;
            newp.replace_filename("WorldCup_WCMode.txt");
            if (exists(newp)) {
                //::Warning("Loading WC Mode script");
                return CallMethodAndReturn<UInt, 0x14B2C35>(t, newp.c_str());
            }
        }
    }
    return CallMethodAndReturn<UInt, 0x14B2C35>(t, filePath);
}

void *METHOD OnDateWCMode(void *t, DUMMY_ARG, UInt year, UInt month, UInt day) {
    return CallMethodAndReturn<void *, 0x1494A2A>(t, Settings::GetInstance().WCMode_Year, Settings::GetInstance().WCMode_Month, Settings::GetInstance().WCMode_Day);
}

void *METHOD OnDateWCModeSeason(void *t, DUMMY_ARG, UInt year, UInt month, UInt day) {
    return CallMethodAndReturn<void *, 0x1494A2A>(t, Settings::GetInstance().WCModeSeason_Year, Settings::GetInstance().WCModeSeason_Month, Settings::GetInstance().WCModeSeason_Day);
}

CTeamIndex WCMode_GetVenueForMatch(UInt compIndex, UInt matchdayId, UInt pairId) {
    if (compIndex < 20 && matchdayId < 3 && pairId < 16) {
        CDBTeam *team = GetTeamByUniqueID(gWCVenueIDs[compIndex][matchdayId][pairId]);
        if (team)
            return team->GetTeamID();
    }
    return CTeamIndex::null();
}

UInt gWCMode_VenueId = 0;
UInt gWCMode_CompIndex = 0;
UInt gWCMode_MatchdayId = 0;

void WCMode_GetVenueId(WideChar const *buf, WideChar const *format, UInt *outId) {
    swscanf(buf, format, outId);
    gWCMode_VenueId = *outId;
}

UInt METHOD WCMode_GetCompIndex(void *t) {
    gWCMode_CompIndex = CallMethodAndReturn<UInt, 0x4AFEB0>(t);
    return gWCMode_CompIndex;
}

UInt METHOD WCMode_GetMatchdayId(void *t) {
    gWCMode_MatchdayId = CallMethodAndReturn<UInt, 0x4AFEB0>(t);
    return gWCMode_MatchdayId;
}

void METHOD WCMode_GetPairId(void *t) {
    UInt pairId = CallMethodAndReturn<UInt, 0x4AFEB0>(t);
    gWCVenueIDs[gWCMode_CompIndex][gWCMode_MatchdayId][pairId] = gWCMode_VenueId;
}

Bool WCMode_RetTrue() { return true; }
Bool WCMode_RetFalse() { return false; }

void PatchWorldCup(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x108A8F8, OnDateWCMode);
        patch::RedirectCall(0x108C807, OnDateWCModeSeason);
        //patch::SetUInt(0x108A8E6 + 1, 2022); // Start year
        //patch::SetUInt(0x108C7FC + 1, 2021); // Quali start year

        patch::RedirectCall(0x108C81D, SetSeasonStartDateForWorldCup);
        patch::RedirectCall(0x47F7D7, OnLoadGameStartDatabase);

        patch::SetUInt(0x308DC88, FifamCompRegion::United_States); // Hosting team

        using CdeclRetInt = int(__cdecl *)();
        //patch::RedirectCall(0x108C6F0, (CdeclRetInt)([] {
        //    return 8;
        //}));
        patch::Nop(0x108A9C3, 2);
        patch::Nop(0x108A9A1, 2);
        patch::Nop(0x108A9B7, 5);
        patch::RedirectCall(0x108C733, (CdeclRetInt)([] {
            return 10;
        }));
        using Thiscall = void(__fastcall *)(void *);
        patch::RedirectCall(0x108B67E, (Thiscall)([](void *t) {
            CallMethod<0x1010500>(t);
            int date = 0;
            Call<0x108A8B0>(&date);
            void *game = CallAndReturn<void *, 0xF61410>();
            CallMethod<0xF49960>(game, date);
        }));

        patch::SetUChar(0x108C59F + 2, NUM_PLAYERS_IN_WC_SQUAD);
        patch::SetUChar(0x108C5AC + 4, NUM_PLAYERS_IN_WC_SQUAD);
        patch::SetUChar(0x108C5F3 + 2, NUM_PLAYERS_IN_WC_SQUAD);
        patch::RedirectJump(0x108C5C0, WcSquadSize1);
        patch::RedirectJump(0x108C4B1, WcSquadSize2);

        patch::RedirectJump(0xF823E0, GetNationalTeamNominationSize_WC);
        patch::RedirectCall(0xF37F3A, NationalTeamNominationSize1);

        //patch::RedirectCall(0xF929AD, OnLoadCompetitionFile);

        patch::SetPointer(0x108A820 + 1, gWCCountryIDs);
        patch::SetPointer(0x108B080 + 3, gWCCountryIDs);
        patch::SetPointer(0x108B5B6 + 1, gWCCountryIDs);
        patch::SetPointer(0x108BBA8 + 3, gWCCountryIDs);
        patch::SetPointer(0x108B5C7 + 1, &gWCCountryIDs[NUM_WC_TEAMS]);
        patch::SetPointer(0x108B5EF + 1, &gWCCountryIDs[NUM_WC_TEAMS]);
        patch::SetUInt(0x108B0A9 + 2, NUM_WC_TEAMS * 4);
        patch::SetUChar(0x108BC06 + 2, NUM_WC_TEAMS);
        
        patch::SetPointer(0x108BCED + 1, gWCVenueIDs);
        patch::SetUInt(0x108BCE7 + 1, sizeof(gWCVenueIDs));
        patch::SetPointer(0x108BD91, "VENUE%03d");
        patch::RedirectJump(0x108A830, WCMode_GetVenueForMatch);
        patch::RedirectCall(0x108BEBD, WCMode_GetVenueId);
        patch::RedirectCall(0x108BF05, WCMode_GetCompIndex);
        patch::RedirectCall(0x108BF33, WCMode_GetMatchdayId);
        patch::RedirectCall(0x108BF61, WCMode_GetPairId);
        patch::Nop(0x108BF72, 7);

        patch::RedirectCall(0x54F3B1, WCMode_RetTrue);
        patch::RedirectCall(0x54F3C4, WCMode_RetFalse);
    }
}
