#pragma once
#include "FifamTypes.h"
#include "TranslationShared.h"
#include "plugin.h"

using namespace plugin;

struct DBCity {
    Int id = -1;
    UChar countryId = 0;
    UChar regionIndex = 255;
    UChar population = 0;
    UShort index = 0;
    UShort weight = 1;
    Int regionId = -1;
    Float latitude = 0.0f;
    Float longitude = 0.0f;
    String names[NUM_TRANSLATION_LANGUAGES];
};

struct DBRegion {
    Int id = -1;
    UChar countryId = 0;
    UChar index = 0;
    Float latitude = 0.0f;
    Float longitude = 0.0f;
    String names[NUM_TRANSLATION_LANGUAGES];
};

Array<Vector<DBCity>, 208> &CountryCities();
Array<Vector<DBRegion>, 208> &CountryRegions();
DBCity *GetCity(UChar countryId, UShort index);
DBRegion *GetRegion(UChar countryId, UChar index);
DBCity *GetCity(Int id);
DBRegion *GetRegion(Int id);
void ReadCities(Path const &dbFolder);
void ReadRegions(Path const &dbFolder);
void ReadCitiesAndRegions(Path const &dbFolder);

void PatchCitiesAndRegions(FM::Version v);
