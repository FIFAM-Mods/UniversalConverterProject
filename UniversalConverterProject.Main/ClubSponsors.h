#pragma once
#include "plugin-std.h"

class CDBTeam;

void PatchClubSponsors(FM::Version v);
void SetupClubSponsorBeforeDBRead(CDBTeam *team);
void SetupClubSponsorAfterDBRead(CDBTeam *team);
