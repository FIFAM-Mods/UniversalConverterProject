#pragma once
#include "plugin-std.h"
#include "GameInterfaces.h"

Bool IsCompetitionLeagueSplit(CCompID const &compId);
Bool IsCompetitionLeagueSplit_UInt(UInt compIdUInt);
UInt GetCompetitionLeagueSplitMainLeague(UInt compIdUInt);
Bool IsCompetitionLeagueWithSplit(CCompID const &compId);
void PatchLeagueSplit(FM::Version v);
