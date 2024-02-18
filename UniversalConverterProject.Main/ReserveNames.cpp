#include "ReserveNames.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"

using namespace plugin;

WideChar const *METHOD GetReserveTeamName_Manager(CDBTeam *team, DUMMY_ARG, WideChar const *teamName) {
    const WideChar *teamResName = GetTranslationIfPresent(Format("RESERVE_TEAM_%08X", team->GetTeamUniqueID()).c_str());
    if (teamResName)
        return teamResName;
    UInt counter = (*(UInt *)0x3124E9C - 1) & 1;
    *(UInt *)0x3124E9C = counter;
    WideChar *out = (WideChar *)(0x3125208 + 128 * counter);
    const WideChar *countryResFormat = GetTranslationIfPresent(Format("RESERVE_COUNTRY_%u", team->GetCountryId()).c_str());
    if (!countryResFormat) {
        countryResFormat = GetTranslationIfPresent("RESERVE_COUNTRY_0");
        if (!countryResFormat)
            countryResFormat = L"%s Res";
    }
    swprintf(out, countryResFormat, teamName);
    return out;
}

void PatchReserveNames(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD)
        patch::RedirectJump(0xEDBED0, GetReserveTeamName_Manager);
}
