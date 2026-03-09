#pragma once
#include "plugin.h"
#include "FifamTypes.h"

class CDBTeam;

struct TeamExtension {
    UInt bestFormationID[5];
    Int cityId;
};

TeamExtension *GetTeamExtension(CDBTeam *team);
void PatchExtendedTeam(FM::Version v);
void ClearTeamFormationInfo(CDBTeam *team);
