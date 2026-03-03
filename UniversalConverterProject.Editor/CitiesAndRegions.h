#pragma once
#include "FifamTypes.h"
#include "TranslationShared.h"
#include "plugin.h"

using namespace plugin;

struct DBCity {
    Int id = -1;
    UChar countryId = 0;
    UChar population = 0;
    Float latitude = 0.0f;
    Float longitude = 0.0f;
    Int regionId = -1;
    String names[NUM_TRANSLATION_LANGUAGES];
};

struct DBRegion {
    Int id = -1;
    UChar countryId = 0;
    Float latitude = 0.0f;
    Float longitude = 0.0f;
    String names[NUM_TRANSLATION_LANGUAGES];
};

Map<Int, DBCity> &DBCities();
Map<Int, DBRegion> &DBRegions();
DBCity *GetCity(Int id);
DBRegion *GetRegion(Int id);
void ReadCities(Path const &dbFolder);
void ReadRegions(Path const &dbFolder);
void ReadCitiesAndRegions(Path const &dbFolder);

void PatchCitiesAndRegions(FM::Version v);
