#include "Scouting.h"
#include "Utils.h"
#include "GameInterfaces.h"
#include "AbilityColors.h"

using namespace plugin;

bool METHOD IsCountryAvailableForStaffSearch(void *game, DUMMY_ARG, unsigned char countryId) {
    return countryId != 0;
}

struct StaffScoutingData {
    void *pFlags[3];
    void *pFlagBackgrounds[3];
};

const unsigned int STAFF_INFO_ORIGINAL_SIZE = 0x678;
const unsigned int STAFF_NEGOTIATION_ORIGINAL_SIZE = 0xD28;
void *gStaffInfoScreen = nullptr;
void *gStaffNegotiationScreen = nullptr;

void * METHOD OnCreateStaffInfoUI(void *screen, DUMMY_ARG, char const *elementName) {
    StaffScoutingData *data = raw_ptr<StaffScoutingData>(screen, STAFF_INFO_ORIGINAL_SIZE);
    for (unsigned int i = 0; i < 3; i++) {
        char name[256];
        snprintf(name, 256, "TbPrefCountry%d", i + 1);
        data->pFlags[i] = CallMethodAndReturn<void *, 0xD44240>(screen, name);
        snprintf(name, 256, "TbBgPrefCountry%d", i + 1);
        data->pFlagBackgrounds[i] = CallMethodAndReturn<void *, 0xD44240>(screen, name);
    }
    return CallMethodAndReturn<void *, 0xD44240>(screen, elementName);
}

void METHOD OnUpdateStaffInfo(void *screen) {
    gStaffInfoScreen = screen;
    StaffScoutingData *data = raw_ptr<StaffScoutingData>(screen, STAFF_INFO_ORIGINAL_SIZE);
    for (unsigned int i = 0; i < 3; i++) {
        CallVirtualMethod<11>(data->pFlagBackgrounds[i], 0);
        CallVirtualMethod<11>(data->pFlags[i], 0);
    }
    CallMethod<0x69A970>(screen);
    UpdateStaffAbilityColors(screen);
    gStaffInfoScreen = nullptr;
}

unsigned char METHOD OnGetScoutJobPreferredCountry(void *job, DUMMY_ARG, unsigned char index) {
    unsigned char countryId = CallMethodAndReturn<unsigned char, 0x126B150>(job, index);
    if (gStaffInfoScreen) {
        StaffScoutingData *data = raw_ptr<StaffScoutingData>(gStaffInfoScreen, STAFF_INFO_ORIGINAL_SIZE);
        CallVirtualMethod<11>(data->pFlagBackgrounds[index], 1);
        CallVirtualMethod<11>(data->pFlags[index], 1);
        Call<0xD32860>(data->pFlags[index], CallAndReturn<wchar_t const *, 0xD3D800>(0x310B678, countryId, 0), 4, 4);
    }
    return countryId;
}


void *METHOD OnCreateStaffNegotiationPopupUI(void *screen, DUMMY_ARG, char const *elementName) {
    StaffScoutingData *data = raw_ptr<StaffScoutingData>(screen, STAFF_NEGOTIATION_ORIGINAL_SIZE);
    for (unsigned int i = 0; i < 3; i++) {
        char name[256];
        snprintf(name, 256, "TbPrefCountry%d", i + 1);
        data->pFlags[i] = CallMethodAndReturn<void *, 0xD44240>(screen, name);
        snprintf(name, 256, "TbBgPrefCountry%d", i + 1);
        data->pFlagBackgrounds[i] = CallMethodAndReturn<void *, 0xD44240>(screen, name);
    }
    return CallMethodAndReturn<void *, 0xD44240>(screen, elementName);
}

void METHOD OnUpdateStaffNegotiation(void *screen) {
    gStaffNegotiationScreen = screen;
    StaffScoutingData *data = raw_ptr<StaffScoutingData>(screen, STAFF_NEGOTIATION_ORIGINAL_SIZE);
    for (unsigned int i = 0; i < 3; i++) {
        CallVirtualMethod<11>(data->pFlagBackgrounds[i], 0);
        CallVirtualMethod<11>(data->pFlags[i], 0);
    }
    CallMethod<0x695D50>(screen);
    gStaffNegotiationScreen = nullptr;
}

unsigned char METHOD OnGetScoutJobPreferredCountry_Negotiation(void *job, DUMMY_ARG, unsigned char index) {
    unsigned char countryId = CallMethodAndReturn<unsigned char, 0x126B150>(job, index);
    if (gStaffNegotiationScreen) {
        StaffScoutingData *data = raw_ptr<StaffScoutingData>(gStaffNegotiationScreen, STAFF_NEGOTIATION_ORIGINAL_SIZE);
        CallVirtualMethod<11>(data->pFlagBackgrounds[index], 1);
        CallVirtualMethod<11>(data->pFlags[index], 1);
        Call<0xD32860>(data->pFlags[index], CallAndReturn<wchar_t const *, 0xD3D800>(0x310B678, countryId, 0), 4, 4);
    }
    return countryId;
}

// scouting players
bool gCurrentPlayerGk_ScoutingPlayers = false;
unsigned char gCurrentPlayerCountry_ScoutingPlayers = 0;
int gCurrentScout_ScoutingPlayers = -1;
void *gCurrentStaff_ScoutingPlayers = nullptr;

int METHOD StoreCurrentScout1_ScoutingPlayers(void *lb, DUMMY_ARG, int row, int column) {
    gCurrentScout_ScoutingPlayers = CallMethodAndReturn<int, 0xD189A0>(lb, row, column);
    return gCurrentScout_ScoutingPlayers;
}

void *METHOD StoreCurrentScout2_ScoutingPlayers(void *team) {
    gCurrentStaff_ScoutingPlayers = nullptr;
    void *scouts = CallMethodAndReturn<void *, 0xED27E0>(team);
    if (scouts && gCurrentScout_ScoutingPlayers >= 0) {
        void *scout = CallVirtualMethodAndReturn<void *, 2>(scouts, gCurrentScout_ScoutingPlayers);
        if (scout)
            gCurrentStaff_ScoutingPlayers = *raw_ptr<void *>(scout, 4);
    }
    return scouts;
}

CTeamIndex *METHOD StoreCurrentPlayerCountry_ScoutingPlayers(CDBPlayer *player, DUMMY_ARG, CTeamIndex *out) {
    *out = player->GetCurrentTeam();
    if (!GetTeam(*out))
        *out = player->GetNationalTeam();
    gCurrentPlayerCountry_ScoutingPlayers = out->countryId;
    gCurrentPlayerGk_ScoutingPlayers = player->GetMainPosition() == 1;
    return out;
}

CTeamIndex* METHOD GetCurrentPlayerTeam_ScoutingPlayers(CDBPlayer* player, DUMMY_ARG, CTeamIndex* out) {
    *out = player->GetCurrentTeam();
    if (!GetTeam(*out))
        *out = player->GetNationalTeam();
    return out;
}

Bool ScoutKnowsCountry(void *staff, UChar countryId) {
    if (countryId != 0) {
        void *worker = *raw_ptr<void *>(staff, 0xC);
        if (worker) {
            void *job = CallMethodAndReturn<void *, 0x1102D10>(staff, 6);
            if (job) {
                for (unsigned char i = 0; i < CallMethodAndReturn<unsigned char, 0x126B160>(job); i++) {
                    if (CallMethodAndReturn<unsigned char, 0x126B150>(job, i) == countryId)
                        return true;
                }
            }
        }
    }
    return false;
}

UChar GetPlayerScoutingDurationInDays(UChar defaultDuration, Bool scoutKnowsCountry) {
    UChar duration = 3;
    switch (defaultDuration) {
    case 3:
        duration = scoutKnowsCountry ? 1 : 3;
        break;
    case 5:
        duration = scoutKnowsCountry ? 2 : 5;
        break;
    case 10:
        duration = scoutKnowsCountry ? 3 : 9;
        break;
    }
    return duration;
}

void METHOD SetScoutingTaskDuration_ScoutingPlayers(void *task, DUMMY_ARG, unsigned int duration) {
    if (gCurrentStaff_ScoutingPlayers)
        duration = GetPlayerScoutingDurationInDays(duration, ScoutKnowsCountry(gCurrentStaff_ScoutingPlayers, gCurrentPlayerCountry_ScoutingPlayers));
    CallMethod<0x150A5B4>(task, duration);
}

CDBEmployee *gPlayerResearchEmployee = nullptr;
void *gPlayerResearchScout = nullptr;
CDBPlayer *gPlayerResearchPlayer = nullptr;

Char METHOD OnGetEmployeePlayerKnowledge(CDBEmployee *employee, DUMMY_ARG, CDBPlayer *player) {
    gPlayerResearchEmployee = employee;
    gPlayerResearchPlayer = player;
    gPlayerResearchScout = nullptr;
    return CallMethodAndReturn<Char, 0xEB9BA0>(employee, player);
}

Float METHOD OnGetScoutSkill(void *staff, DUMMY_ARG, Int skill) {
    gPlayerResearchScout = staff;
    return CallMethodAndReturn<Float, 0x1102BA0>(staff, skill);
}

UChar GetPlayerResearchSpeed(UInt speed) {
    if (gPlayerResearchEmployee && gPlayerResearchScout && gPlayerResearchPlayer) {
        CTeamIndex staffTeam = CTeamIndex::make(0, 0, 0);
        CallMethod<0xEA2C10>(gPlayerResearchEmployee, &staffTeam);
        if (staffTeam.countryId >= 1 && staffTeam.countryId <= 207) {
            CTeamIndex playerTeam = gPlayerResearchPlayer->GetCurrentTeam();
            if (!GetTeam(playerTeam))
                playerTeam = gPlayerResearchPlayer->GetNationalTeam();
            CallMethod<0xFB5240>(gPlayerResearchPlayer, &playerTeam);
            if (playerTeam.countryId >= 1 && playerTeam.countryId <= 207 && ScoutKnowsCountry(gPlayerResearchScout, playerTeam.countryId)) {
                if (staffTeam.countryId == playerTeam.countryId) {
                    if (speed < 9)
                        speed = 9;
                }
                else {
                    CDBCountry *staffCountry = &GetCountryStore()->m_aCountries[staffTeam.countryId];
                    CDBCountry *playerCountry = &GetCountryStore()->m_aCountries[playerTeam.countryId];
                    if (staffCountry->GetContinent() == playerCountry->GetContinent()) {
                        if (speed < 6)
                            speed = 6;
                    }
                    else {
                        if (speed < 4)
                            speed = 4;
                    }
                }
            }
        }
    }
    gPlayerResearchEmployee = nullptr;
    gPlayerResearchScout = nullptr;
    gPlayerResearchPlayer = nullptr;
    return (UChar)speed;
}

void __declspec(naked) ProcessPlayerResearch() {
    __asm {
        movzx eax, al
        push eax
        call GetPlayerResearchSpeed
        add esp, 4
        mov cl, byte ptr[esp+0xC]
        add cl, al
        mov eax, 0x13F8EE8
        jmp eax
    }
}

void *gPlayerScoutingScout2 = nullptr;

UChar METHOD GetPlayerScoutingDuration(void *t, DUMMY_ARG, UInt countryId1, UInt countryId2) {
    UChar duration = 3;
    if (countryId1 == countryId2)
        duration = 3;
    if (GetCountryStore()->m_aCountries[countryId2].GetContinent() != GetCountryStore()->m_aCountries[countryId1].GetContinent())
        duration = 10;
    else
        duration = 5;
    if (gPlayerScoutingScout2) {
        duration = GetPlayerScoutingDurationInDays(duration, ScoutKnowsCountry(gPlayerScoutingScout2, countryId1));
        gPlayerScoutingScout2 = nullptr;
    }
    return duration;
}

Bool METHOD StoreScoutForScoutingList(void *t, DUMMY_ARG, void *scout, UInt employeeId, Int *playerList, Int *a5, UInt teamID) {
    if (scout)
        gPlayerScoutingScout2 = *raw_ptr<void *>(scout, 4);
    else
        gPlayerScoutingScout2 = nullptr;
    return CallMethodAndReturn<Bool, 0x134E150>(t, scout, employeeId, playerList, a5, teamID);
}

//

void PatchScouting(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // countries
        patch::RedirectCall(0x69020D, IsCountryAvailableForStaffSearch);
        
        patch::SetUInt(0x69AFB4 + 1, STAFF_INFO_ORIGINAL_SIZE + sizeof(StaffScoutingData));
        patch::SetUInt(0x69AFBB + 1, STAFF_INFO_ORIGINAL_SIZE + sizeof(StaffScoutingData));

        patch::RedirectCall(0x69A23D, OnCreateStaffInfoUI);

        patch::RedirectCall(0x69AE48, OnUpdateStaffInfo);
        patch::RedirectCall(0x69B930, OnUpdateStaffInfo);

        patch::RedirectCall(0x69AAEF, OnGetScoutJobPreferredCountry);


        patch::SetUInt(0x697554 + 1, STAFF_NEGOTIATION_ORIGINAL_SIZE + sizeof(StaffScoutingData));
        patch::SetUInt(0x69755B + 1, STAFF_NEGOTIATION_ORIGINAL_SIZE + sizeof(StaffScoutingData));

        patch::RedirectCall(0x694828, OnCreateStaffNegotiationPopupUI);

        patch::RedirectCall(0x697617, OnUpdateStaffNegotiation);
        patch::RedirectCall(0x697E8B, OnUpdateStaffNegotiation);
        patch::RedirectCall(0x697B2A, OnUpdateStaffNegotiation);

        patch::RedirectCall(0x695E8F, OnGetScoutJobPreferredCountry_Negotiation);

        //
        patch::RedirectCall(0x60E429, StoreCurrentScout1_ScoutingPlayers);
        patch::RedirectCall(0x60E475, StoreCurrentScout2_ScoutingPlayers);

        patch::RedirectCall(0x60E734, StoreCurrentPlayerCountry_ScoutingPlayers);
        patch::RedirectCall(0x60E748, GetCurrentPlayerTeam_ScoutingPlayers);
        patch::RedirectCall(0x60E76A, GetCurrentPlayerTeam_ScoutingPlayers);
        patch::RedirectCall(0x134E280, GetCurrentPlayerTeam_ScoutingPlayers);

        patch::RedirectCall(0x60E7C3, SetScoutingTaskDuration_ScoutingPlayers);

        patch::RedirectJump(0x13F8EE2, ProcessPlayerResearch);
        patch::RedirectCall(0x13F8DAC, OnGetEmployeePlayerKnowledge);
        patch::RedirectCall(0x13F8E05, OnGetScoutSkill);

        patch::SetPointer(0x24DE004, GetPlayerScoutingDuration);
        patch::SetPointer(0x24DDFFC, StoreScoutForScoutingList);
    }
}
