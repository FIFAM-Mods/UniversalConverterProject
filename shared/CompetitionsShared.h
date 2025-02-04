#pragma once
#include "FifamTypes.h"

#define COMP_BASE_ID(region, type) ((UInt)(((region) << 24) | ((type) << 16)))

enum eCompetitionType : unsigned char {
    COMP_ROOT = 0,
    COMP_LEAGUE = 1,
    COMP_LEAGUE_SPARE = 2,
    COMP_FA_CUP = 3,
    COMP_LE_CUP = 4,
    COMP_CHALLENGE_SHIELD = 5,
    COMP_CONFERENCE_CUP = 6,
    COMP_SUPERCUP = 7,
    COMP_RELEGATION = 8,
    COMP_CHAMPIONSLEAGUE = 9,
    COMP_UEFA_CUP = 10,
    COMP_TOYOTA = 11,
    COMP_EURO_SUPERCUP = 12,
    COMP_WORLD_CLUB_CHAMP = 13,
    COMP_UIC = 14,
    COMP_QUALI_WC = 15,
    COMP_QUALI_EC = 16,
    COMP_WORLD_CUP = 17,
    COMP_EURO_CUP = 18,
    COMP_REL1 = 19,
    COMP_REL2 = 20,
    COMP_REL3 = 21,
    COMP_INDOOR = 22,
    COMP_FRIENDLY = 23,
    COMP_POOL = 24,
    COMP_UNKNOWN_25 = 25,
    COMP_UNKNOWN_26 = 26,
    COMP_UNKNOWN_27 = 27,
    COMP_RESERVE = 28,
    COMP_INTERNATIONAL_FRIENDLY = 29,
    COMP_UNKNOWN_30 = 30,
    COMP_U20_WORLD_CUP = 31,
    COMP_CONFED_CUP = 32,
    COMP_COPA_AMERICA = 33,
    // Added competitions
    COMP_ICC = 35, // International Champions Cup
    COMP_EURO_NL_Q = 36, // Nations League Quali
    COMP_EURO_NL = 37, // Nations League Finals
    COMP_YOUTH_CHAMPIONSLEAGUE = 38, // Youth Champions League
    COMP_CONTINENTAL_1 = 39, // Continental 1
    COMP_CONTINENTAL_2 = 40, // Continental 2
    COMP_NAM_NL_Q = 41, // North America Nations League Quali
    COMP_NAM_NL = 42, // North America Nations League
    COMP_NAM_CUP = 43, // North America Cup
    COMP_AFRICA_CUP_Q = 44, // Africa Cup Quali
    COMP_AFRICA_CUP = 45, // Africa Cup
    COMP_ASIA_CUP_Q = 46, // Asia Cup Quali
    COMP_ASIA_CUP = 47, // Asia Cup
    COMP_OFC_CUP_Q = 48, // OFC Cup Quali
    COMP_OFC_CUP = 49, // OFC Cup
    COMP_U20_WC_Q = 50, // U20 WC Quali
    COMP_CONFERENCE_LEAGUE = 51, // Conference League
    COMP_FINALISSIMA = 52, // Finalissima
    COMP_U17_WC_Q = 53, // U17 WC Quali
    COMP_U17_WORLD_CUP = 54, // U17 WC
    COMP_U21_EC_Q = 55, // U21 EC Quali
    COMP_U21_EC = 56, // U21 EC
    COMP_U19_EC_Q = 57, // U19 EC Quali
    COMP_U19_EC = 58, // U19 EC
    COMP_U17_EC_Q = 59, // U17 EC Quali
    COMP_U17_EC = 60, // U17 EC
    COMP_OLYMPIC_Q = 61, // Olympic Games Quali
    COMP_OLYMPIC = 62, // Olympic Games
};

enum CompetitionLaunchSeasonPart : UChar {
    SEASON_START,
    SEASON_END
};

struct CompetitionLaunchInfo {
    UShort year;
    CompetitionLaunchSeasonPart seasonPart;
    UChar period;
    UInt qualiCompId;
};

Map<UInt, CompetitionLaunchInfo> &GetCompetitionLaunchInfos();
CompetitionLaunchInfo &GetCompetitionLaunchInfo(UChar region, UChar type);
UChar GetCompetitionLaunchPeriod(UChar region, UChar type);
Bool LaunchesInYear(UChar countryId, UChar type, UShort year);
Bool LaunchesInSeason(UChar countryId, UChar type, UShort seasonStartYear);
