#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"

extern CDBRound *gMyDBRound_RegisterMatch_Round;

void PatchCompetitions(FM::Version v);
Bool IsCompetitionLeagueSplit(CCompID const &compId);
Bool IsCompetitionLeagueSplit_UInt(UInt compIdUInt);
UInt GetCompetitionLeagueSplitMainLeague(UInt compIdUInt);
void Assessment_AddPointsOnRoundFinish(CDBRound *round);
Bool Europe_ChampionsLeagueRoundSorter(CDBTeam *teamA, CDBTeam *teamB);
CTeamIndex GetCompFinalist(UChar region, UChar type, UInt year, Bool runnerUp);
