#pragma once
#include "ScriptsShared.h"
#include "shared.h"

Bool Europe_ChampionsLeagueRoundSorter(CDBTeam *teamA, CDBTeam *teamB) {
    if (!teamB)
        return true;
    if (!teamA)
        return false;
    UChar prestigeA = teamA->GetInternationalPrestige();
    UChar prestigeB = teamB->GetInternationalPrestige();
    if (prestigeA > prestigeB)
        return true;
    if (prestigeB > prestigeA)
        return false;
    Int posA = GetAssesmentTable()->GetCountryPosition(teamA->GetTeamID().countryId);
    Int posB = GetAssesmentTable()->GetCountryPosition(teamB->GetTeamID().countryId);
    return posA <= posB;
}

Bool Europe_YouthChampionsLeagueRoundSorter(CDBTeam *teamA, CDBTeam *teamB) {
    if (!teamB)
        return true;
    if (!teamA)
        return false;
    Int posA = GetAssesmentTable()->GetCountryPosition(teamA->GetTeamID().countryId);
    Int posB = GetAssesmentTable()->GetCountryPosition(teamB->GetTeamID().countryId);
    return posA <= posB;
}

void Europe_Continental_Callback(CDBCompetition *comp) {
    /*
    auto id = comp->GetCompID();
    if (comp->GetDbType() == DB_POOL) {
        if (id.index == 0) {
            if (id.type == FifamCompType::ChampionsLeague) {
                // group stage
                AddFinalWinner(comp, FifamCompRegion::Europe, FifamCompType::ChampionsLeague);
                AddFinalWinner(comp, FifamCompRegion::Europe, FifamCompType::UefaCup);

                Array<UChar, 55> assessmentTable;

                UInt countryCounter = 1;
                for (UInt i = 1; i <= 55; i++) {
                    auto countryId = GetAssesmentTable()->GetCountryIdAtPosition(i);
                    if (countryId != FifamCompRegion::Liechtenstein)
                        assessmentTable[countryCounter++] = countryId;
                }

                AddEuropeanTeams(comp, assessmentTable[1], 1, 4);
                AddEuropeanTeams(comp, assessmentTable[2], 1, 4);
                AddEuropeanTeams(comp, assessmentTable[3], 1, 4);
                AddEuropeanTeams(comp, assessmentTable[4], 1, 4);
                AddEuropeanTeams(comp, assessmentTable[5], 1, 2);
                AddEuropeanTeams(comp, assessmentTable[6], 1, 2);
                AddEuropeanTeams(comp, assessmentTable[7], 1, 1);
                AddEuropeanTeams(comp, assessmentTable[8], 1, 1);
                AddEuropeanTeams(comp, assessmentTable[9], 1, 1);
                AddEuropeanTeams(comp, assessmentTable[10], 1, 1);

                Bool addedChamp11 = false;
                Bool addedThird5 = false;

                if (comp->GetNumOfRegisteredTeams() == 24) {
                    AddEuropeanTeams(comp, assessmentTable[11], 1, 1);
                    AddEuropeanTeams(comp, assessmentTable[5], 3, 1);
                }
                else if (comp->GetNumOfRegisteredTeams() == 25) {
                    AddEuropeanTeams(comp, assessmentTable[11], 1, 1);
                    if (comp->GetNumOfRegisteredTeams() == 25)
                        AddEuropeanTeams(comp, assessmentTable[5], 3, 1);
                }
            }
            else if (id.type == FifamCompType::UefaCup) {

            }
        }
    }
    else */
    if (comp->GetDbType() == DB_ROUND) {
        auto WriteTeams = [&]() {
            for (UInt i = 0; i < comp->GetNumOfTeams(); i++) {
                auto team = GetTeam(comp->GetTeamID(i));
                if (team) {
                    SafeLog::Write(Utils::Format(L"%d. %08X %s - %d. / %d", i + 1, team->GetTeamID().ToInt(), team->GetName(),
                        GetAssesmentTable()->GetCountryPosition(team->GetTeamID().countryId), team->GetInternationalPrestige()));
                }
                else
                    SafeLog::Write(Utils::Format(L"%d. (null)", i + 1));
            }
        };
        //SafeLog::Write(Utils::Format(L"Sorting competition: %s (%s)\nTeams before sorting:", comp->GetName(), comp->GetCompID().ToStr()));
        //WriteTeams();
        if (comp->GetCompID().type == 38)
            comp->SortTeams(Europe_YouthChampionsLeagueRoundSorter);
        else
            comp->SortTeams(Europe_ChampionsLeagueRoundSorter);
        //SafeLog::Write(L"Teams after reputation/country sorting:");
        //WriteTeams();
        comp->RandomlySortTeams(0, comp->GetNumOfRegisteredTeams() / 2);
        comp->RandomlySortTeams(comp->GetNumOfRegisteredTeams() / 2);
        //SafeLog::Write(L"Teams after random sorting:");
        //WriteTeams();
        //SafeLog::Write(L"Sorted");
    }
}
