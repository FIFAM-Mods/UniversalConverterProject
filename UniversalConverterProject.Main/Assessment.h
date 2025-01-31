#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"

using namespace plugin;

enum class AsianRegion {
    None,
    West,
    East
};

void PatchAssessment(FM::Version v);
UChar GetAfricanCountryAssessmentPosition(UChar countryId);
UChar GetAsianCountryAssessmentPosition(UChar countryId);
UChar GetAsianCountryAssessmentRegionalPosition(UChar countryId);
UChar GetAfricanAssessmentCountryAtPosition(UChar position);
UChar GetAsianAssessmentCountryAtPosition(UChar position);
UChar GetAsianAssessmentCountryAtRegionalPosition(UChar position);
Set<UChar> &GetAsianWestCountries();
Set<UChar> &GetAsianEastCountries();
Bool IsAsianWestCountry(UChar countryId);
Bool IsAsianEastCountry(UChar countryId);
AsianRegion GetAsianCountryRegion(UChar countryId);
UInt GetAfricanAssessmentNumCountries();
UInt GetAsianAssessmentNumCountries();
void AddAfricanAssessmentCountryPoints(UChar countryId, Float points);
void AddAsianAssessmentCountryPoints(UChar countryId, Float points, Bool prelimStage = false);
