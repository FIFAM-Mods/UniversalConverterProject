#pragma once
#include "plugin.h"
#include "FifamTypes.h"
#include "CitiesAndRegions.h"

class CDBTeam;

struct TeamExtension {
    UInt bestFormationID[5];
    CityID cityID;
};

TeamExtension *GetTeamExtension(CDBTeam *team);
void PatchExtendedTeam(FM::Version v);
void ClearTeamFormationInfo(CDBTeam *team);
