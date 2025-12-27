#pragma once
#include "plugin.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"

String GetPathForTeamKitColor(String const &dir, UInt teamUId, UInt kitType);
Pair<UInt, UInt> GetGenericKitColorIDs(CDBTeamKit *kit, Bool home);
void InstallKits_FM13();
void PatchKits(FM::Version v);
