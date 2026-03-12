#pragma once
#include "FifamTypes.h"
#include "plugin.h"

using namespace plugin;

struct DBCity {
    Int id = -1;
    UChar countryId = 0;
    UChar population = 0;
    Float latitude = 0.0f;
    Float longitude = 0.0f;
    Int regionId = -1;
    WideChar *name = nullptr;
};

struct DBRegion {
    Int id = -1;
    UChar countryId = 0;
    Float latitude = 0.0f;
    Float longitude = 0.0f;
    WideChar *name = nullptr;
};

Map<Int, DBCity> &DBCities();
Map<Int, DBRegion> &DBRegions();
DBCity *GetCity(Int id);
DBRegion *GetRegion(Int id);
Bool IsCatalanCity(Int cityID);
class CDBTeam;
Bool IsTeamCatalan(CDBTeam *team);

void PatchCitiesAndRegions(FM::Version v);
