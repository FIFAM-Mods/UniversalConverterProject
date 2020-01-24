#pragma once
#include "GameInterfaces.h"
#include "FifamTypes.h"
#include "Random.h"

Bool AddTeamFromCountry(CDBCompetition *comp, UInt countryId, UInt teamIndex, Bool keepAddingIfFailed = false) {
    void *countryStore = plugin::CallAndReturn<void *, 0x415100>();
    UInt numClubs = plugin::CallMethodAndReturn<UInt, 0xFD6910>((UChar *)countryStore + 4296 * countryId + 8);
    for (UInt i = teamIndex; i < numClubs; i++) {
        CTeamIndex teamIndex;
        teamIndex.countryId = countryId;
        teamIndex.index = i + 1;
        teamIndex.type = 0;
        if (!comp->IsTeamPresent(teamIndex)) {
            if (comp->AddTeam(teamIndex))
                return true;
        }
        if (!keepAddingIfFailed)
            break;
    }
    return false;
}

Bool AddCupWinner(CDBCompetition *comp, UInt countryId, UInt cupType, UInt cupIndex = 0) {
    auto cup = GetCompetition(countryId, cupType, cupIndex);
    if (!cup)
        return false;
    return comp->AddTeam(cup->GetChampion(true));
}

void AddRandomTeams(CDBCompetition *comp, UInt countryId, UInt numTeams, Vector<UInt> const &chances) {
    if (numTeams > 0) {
        if (chances.size() <= numTeams) {
            comp->AddTeamsFromCountry(countryId, numTeams);
            return;
        }
        UInt total = 0;
        for (auto i : chances)
            total += i;
        Vector<UInt> c(total);
        total = 0;
        for (UInt i = 0; i < chances.size(); i++) {
            for (UInt j = 0; j < chances[i]; j++)
                c[total + j] = i;
            total += chances[i];
        }
        std::shuffle(c.begin(), c.end(), Random::GetEngine());
        for (UInt i = 0; i < numTeams; i++)
            AddTeamFromCountry(comp, countryId, c[i], true);
    }
}

bool AddFinalWinner(CDBCompetition *comp, UInt regionId, UInt compType) {
    auto round = GetRoundByRoundType(regionId, compType, 15);
    if (round)
        return comp->AddTeam(round->GetChampion());
    return false;
}

UInt AddEuropeanTeams(CDBCompetition *comp, UInt countryId, UInt startPos, UInt numTeams) {
    UInt numAddedTeams = 0;
    auto league = GetLeague(countryId, COMP_LEAGUE, 0);
    if (league) {
        if (startPos == 0)
            startPos = 1;
        UInt numTeamsInLeague = league->GetNumOfRegisteredTeams();
        for (UInt i = startPos - 1; i < numTeamsInLeague; i++) {
            if (!numTeams)
                break;
            CTeamIndex teamId = league->GetTeamAtPosition(i);
            if (teamId.countryId > 0) {
                if (comp->IsTeamPresent(teamId))
                    numTeams--;
                else if (comp->AddTeam(teamId)) {
                    numAddedTeams++;
                    numTeams--;
                }
            }
        }
    }
    return numAddedTeams;
}
