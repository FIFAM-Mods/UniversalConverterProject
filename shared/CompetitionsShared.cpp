#include "CompetitionsShared.h"
#include "FifamCompRegion.h"

Map<UInt, CompetitionLaunchInfo> &GetCompetitionLaunchInfos() {
    static Map<UInt, CompetitionLaunchInfo> infos = {
        { COMP_BASE_ID(255, COMP_WORLD_CUP),     { 2026, SEASON_END, 4, COMP_BASE_ID(255, COMP_QUALI_WC) } },
        { COMP_BASE_ID(255, COMP_FINALISSIMA),   { 2026, SEASON_END, 4, 0 } },
        { COMP_BASE_ID(255, COMP_QUALI_EC),      { 2026, SEASON_START, 4, 0 } },
        { COMP_BASE_ID(255, COMP_ASIA_CUP_Q),    { 2026, SEASON_START, 4, 0 } },
        { COMP_BASE_ID(255, COMP_OFC_CUP_Q),     { 2027, SEASON_START, 4, 0 } },
        { COMP_BASE_ID(255, COMP_ASIA_CUP),      { 2027, SEASON_END, 4, COMP_BASE_ID(255, COMP_ASIA_CUP_Q) } },
        { COMP_BASE_ID(255, COMP_EURO_CUP),      { 2028, SEASON_END, 4, COMP_BASE_ID(255, COMP_QUALI_EC) } },
        { COMP_BASE_ID(255, COMP_COPA_AMERICA),  { 2028, SEASON_END, 4, 0 } },
        { COMP_BASE_ID(255, COMP_OFC_CUP),       { 2028, SEASON_END, 4, COMP_BASE_ID(255, COMP_OFC_CUP_Q) } },
        { COMP_BASE_ID(255, COMP_QUALI_WC),      { 2028, SEASON_START, 4, 0 } },
        { COMP_BASE_ID(255, COMP_CONFED_CUP),    { 2029, SEASON_END, 4, 0 } },

        { COMP_BASE_ID(255, COMP_EURO_NL_Q),     { 2026, SEASON_START, 2, 0 } },
        { COMP_BASE_ID(255, COMP_AFRICA_CUP_Q),  { 2026, SEASON_START, 2, 0 } },
        { COMP_BASE_ID(255, COMP_U20_WC_Q),      { 2026, SEASON_START, 2, 0 } },
        { COMP_BASE_ID(255, COMP_NAM_NL_Q),      { 2026, SEASON_START, 2, 0 } },
        { COMP_BASE_ID(255, COMP_EURO_NL),       { 2027, SEASON_END, 2, COMP_BASE_ID(255, COMP_EURO_NL_Q) } },
        { COMP_BASE_ID(255, COMP_AFRICA_CUP),    { 2027, SEASON_END, 2, COMP_BASE_ID(255, COMP_AFRICA_CUP_Q) } },
        { COMP_BASE_ID(255, COMP_NAM_CUP),       { 2027, SEASON_END, 2, COMP_BASE_ID(255, COMP_NAM_CUP) } }, // add quali so it doesn't launch by default
        { COMP_BASE_ID(255, COMP_U20_WORLD_CUP), { 2027, SEASON_END, 2, COMP_BASE_ID(255, COMP_U20_WC_Q) } },
        { COMP_BASE_ID(255, COMP_NAM_NL),        { 2027, SEASON_END, 2, COMP_BASE_ID(255, COMP_NAM_NL_Q) } },

        { COMP_BASE_ID(FifamCompRegion::Europe, COMP_WORLD_CLUB_CHAMP), { 2029, SEASON_END, 4, 0 } },
        { COMP_BASE_ID(FifamCompRegion::SouthAmerica, COMP_YOUTH_CHAMPIONSLEAGUE), { 2026, SEASON_START, 2, 0 } },
        { COMP_BASE_ID(FifamCompRegion::Africa, COMP_CONTINENTAL_1), { 2026, SEASON_START, 4, 0 } }
    };
    return infos;
}

CompetitionLaunchInfo &GetCompetitionLaunchInfo(UChar region, UChar type) {
    UInt baseId = COMP_BASE_ID(region, type);
    if (Utils::Contains(GetCompetitionLaunchInfos(), baseId))
        return GetCompetitionLaunchInfos()[baseId];
    static CompetitionLaunchInfo dummyInfo = { 0, SEASON_START, 0, 0 };
    return dummyInfo;
}

UChar GetCompetitionLaunchPeriod(UChar region, UChar type) {
    auto const &info = GetCompetitionLaunchInfo(region, type);
    if (info.period > 0)
        return info.period;
    return 1;
}

Bool LaunchesInYear(UChar countryId, UChar type, UShort year) {
    auto &info = GetCompetitionLaunchInfo(countryId, type);
    if (info.period == 0 || info.period == 1)
        return true;
    return (year % info.period) == (info.year % info.period);
}

Bool LaunchesInSeason(UChar countryId, UChar type, UShort seasonStartYear) {
    auto &info = GetCompetitionLaunchInfo(countryId, type);
    if (info.period == 0 || info.period == 1)
        return true;
    UInt year = seasonStartYear;
    if (info.seasonPart == SEASON_END)
        year += 1;
    return (year % info.period) == (info.year % info.period);
}
