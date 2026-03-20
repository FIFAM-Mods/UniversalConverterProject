#pragma once
#include "FifamTypes.h"
#include "plugin.h"

using namespace plugin;

struct CityID {
    UChar countryId;
    UChar regionIndex;
    UShort index;

    CityID();
    CityID(UChar CountryId, UShort Index, UChar regionIndex);
    void Clear();
    Bool IsValid();
};

struct RegionID {
    UChar countryId;
    UChar index;

    RegionID();
    RegionID(UChar CountryId, UChar Index);
    void Clear();
    Bool IsValid();
};

struct DBCity {
    Int EditorID = -1;
    CityID ID;
    UChar population = 0;
    UShort weight;
    Float latitude = 0.0f;
    Float longitude = 0.0f;
    WideChar *name = nullptr;
};

struct DBRegion {
    Int EditorID = -1;
    RegionID ID;
    Float latitude = 0.0f;
    Float longitude = 0.0f;
    WideChar *name = nullptr;
};

DBCity *GetCity(CityID ID);
DBRegion *GetRegion(RegionID ID);
Bool IsCatalanCity(CityID cityID);
class CDBTeam;
Bool IsTeamCatalan(CDBTeam *team);

void PatchCitiesAndRegions(FM::Version v);
