#pragma once
#include "plugin.h"

class CDBTeam;

void PatchClubSponsors(FM::Version v);
void SetupClubSponsorBeforeDBRead(CDBTeam *team);
void SetupClubSponsorAfterDBRead(CDBTeam *team);
