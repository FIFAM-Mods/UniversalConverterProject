#pragma once
#include "plugin-std.h"
#include "GameInterfaces.h"

void PatchExtendedCountry(FM::Version v);
bool SortCountriesByFifaRanking(CDBCountry *c1, CDBCountry *c2);
bool SortCountryIDsByFifaRanking(UShort countryId1, UShort countryId2);
bool SortTeamsByCountryFifaRanking(CTeamIndex const &t1, CTeamIndex const &t2);
