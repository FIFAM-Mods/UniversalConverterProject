#pragma once
#include "ScriptsShared.h"
#include "FifamCompID.h"

const UInt Oceania_ParticipantsCountries[] = {
    FifamCompRegion::New_Zealand,
    FifamCompRegion::Fiji,
    FifamCompRegion::Papua_New_Guinea,
    FifamCompRegion::Solomon_Islands,
    FifamCompRegion::Tahiti,
    FifamCompRegion::Vanuatu,
    FifamCompRegion::American_Samoa,
    FifamCompRegion::Cook_Islands,
    FifamCompRegion::Samoa,
    FifamCompRegion::Tonga
};

Bool Oceania_OnGetParticipants(CDBCompetition *comp) {
    if (comp->GetCompID().countryId == FifamCompRegion::Oceania && comp->GetCompID().type == COMP_CHAMPIONSLEAGUE && comp->GetCompID().index == 0) {
        comp->AddTeamsFromCountry(FifamCompRegion::New_Zealand, 1);
        if (!AddCupWinner(comp, FifamCompRegion::New_Zealand, COMP_FA_CUP))
            comp->AddTeamsFromCountry(FifamCompRegion::New_Zealand, 1);
        AddRandomTeams(comp, FifamCompRegion::Fiji,             2, { 4, 2, 2, 2, 1 });
        AddRandomTeams(comp, FifamCompRegion::Papua_New_Guinea, 2, { 4, 2, 2, 2, 1 });
        AddRandomTeams(comp, FifamCompRegion::Solomon_Islands,  2, { 5, 2, 2, 1, 1 });
        AddRandomTeams(comp, FifamCompRegion::Tahiti,           2, { 4, 2, 2, 2, 1 });
        AddRandomTeams(comp, FifamCompRegion::Vanuatu,          2, { 3, 3, 2, 2, 1 });
        AddRandomTeams(comp, FifamCompRegion::American_Samoa,   1, { 4, 2, 1 });
        AddRandomTeams(comp, FifamCompRegion::Cook_Islands,     1, { 5, 2 });
        AddRandomTeams(comp, FifamCompRegion::Samoa,            1, { 4, 3 });
        AddRandomTeams(comp, FifamCompRegion::Tonga,            1, { 4, 3 });
        return true;
    }
    return false;
}

Bool Oceania_OnSortPool(CDBPool *pool) {

    return false;
}

Int Oceania_GetPoolTeamsFromCountry(CDBPool *pool, Int countryId) {
    if (pool->GetCompID().countryId == FifamCompRegion::Oceania && pool->GetCompID().type == COMP_CHAMPIONSLEAGUE) {
        // NOTE: this part of code must be updated in case if OFC continental script is changed
        switch (countryId) {
        case FifamCompRegion::Fiji:
        case FifamCompRegion::New_Zealand:
        case FifamCompRegion::Papua_New_Guinea:
        case FifamCompRegion::Solomon_Islands:
        case FifamCompRegion::Tahiti:
        case FifamCompRegion::Vanuatu:
            return 2;
        case FifamCompRegion::American_Samoa:
        case FifamCompRegion::Cook_Islands:
        case FifamCompRegion::Samoa:
        case FifamCompRegion::Tonga:
            return 1;
        }
        return 0;
    }
    return -1;
}
