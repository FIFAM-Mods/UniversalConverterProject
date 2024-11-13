#pragma once
#include "FifamTypes.h"
#include "plugin-std.h"
#include "GameInterfaces.h"

void PatchUEFALeaguePhase(FM::Version v);

struct TeamLeaguePhaseInfo {
    CTeamIndex teamId;
    Int matches = 0, wins = 0, draws = 0, losses = 0, goalsFor = 0, goalsAgainst = 0, points = 0;
};

bool DrawUEFALeaguePhase(CDBCompetition *poolTeams, CDBCompetition *poolFixtures, UInt numPots, UInt numMatchdays);
Bool IsUEFALeaguePhaseCompID(CCompID const &compID);
UInt *GetUEFALeaguePhaseMatchdaysCompIDs(UInt compId, UInt &numCompIds);
Bool IsUEFALeaguePhaseMatchdayCompID(CCompID const &compID);
Vector<TeamLeaguePhaseInfo> SortUEFALeaguePhaseTable(UInt compId, CDBCompetition *comp);
void UEFALeaguePhaseMatchdayProcessBonuses(CDBRound *, RoundPair const &pair);
