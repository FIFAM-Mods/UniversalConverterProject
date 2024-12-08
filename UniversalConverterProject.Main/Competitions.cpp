#include "Competitions.h"
#include "GameInterfaces.h"
#include "FifamCompID.h"
#include "FifamRoundID.h"
#include "shared.h"
#include "Log.h"
#include <map>
#include "license_check/license_check.h"
#include "InternationalCupsFix.h"
#include "Random.h"
#include "FifamContinent.h"
#include "FifamClubTeamType.h"
#include "FifamNation.h"
#include "FifamBeg.h"
#include "UEFALeaguePhase.h"

using namespace plugin;

void ClearID(CTeamIndex &teamIndex) {
    teamIndex.countryId = 0;
    teamIndex.type = 0;
    teamIndex.index = 0;
};

Bool CompareIDs(CTeamIndex const &a, CTeamIndex const &b) {
    return a.countryId == b.countryId && a.type == b.type && a.index == b.index;
};

Bool Europe_ChampionsLeagueRoundSorter(CDBTeam *teamA, CDBTeam *teamB) {
    if (!teamB)
        return true;
    if (!teamA)
        return false;
    UChar prestigeA = teamA->GetInternationalPrestige();
    UChar prestigeB = teamB->GetInternationalPrestige();
    if (prestigeA > prestigeB)
        return true;
    if (prestigeB > prestigeA)
        return false;
    Int posA = GetAssesmentTable()->GetCountryPositionLastYear(teamA->GetTeamID().countryId);
    Int posB = GetAssesmentTable()->GetCountryPositionLastYear(teamB->GetTeamID().countryId);
    return posA <= posB;
}

Bool Europe_YouthChampionsLeagueRoundSorter(CDBTeam *teamA, CDBTeam *teamB) {
    if (!teamB)
        return true;
    if (!teamA)
        return false;
    Int posA = GetAssesmentTable()->GetCountryPositionLastYear(teamA->GetTeamID().countryId);
    Int posB = GetAssesmentTable()->GetCountryPositionLastYear(teamB->GetTeamID().countryId);
    return posA <= posB;
}

Bool ChampionsLeagueRoundSorter(CDBTeam *teamA, CDBTeam *teamB) {
    if (!teamB)
        return true;
    if (!teamA)
        return false;
    UChar prestigeA = teamA->GetInternationalPrestige();
    UChar prestigeB = teamB->GetInternationalPrestige();
    if (prestigeA > prestigeB)
        return true;
    if (prestigeB > prestigeA)
        return false;
    Int posA = (teamA->GetTeamID().countryId < 208) ? GetCountryStore()->m_aCountries[teamA->GetTeamID().countryId].GetLeagueAverageLevel() : 0;
    Int posB = (teamB->GetTeamID().countryId < 208) ? GetCountryStore()->m_aCountries[teamB->GetTeamID().countryId].GetLeagueAverageLevel() : 0;
    return posA <= posB;
}

Bool Europe_CountryLeagueSorter(CTeamIndex const &a, CTeamIndex const &b) {
    if (!b.countryId)
        return true;
    if (!a.countryId)
        return false;
    Int posA = GetAssesmentTable()->GetCountryPosition(a.countryId);
    Int posB = GetAssesmentTable()->GetCountryPosition(b.countryId);
    return posA <= posB;
}

Bool CountryLeagueSorter(CTeamIndex const &a, CTeamIndex const &b) {
    if (!b.countryId)
        return true;
    if (!a.countryId)
        return false;
    Int posA = (a.countryId < 208) ? GetCountryStore()->m_aCountries[a.countryId].GetLeagueAverageLevel() : 0;
    Int posB = (b.countryId < 208) ? GetCountryStore()->m_aCountries[b.countryId].GetLeagueAverageLevel() : 0;
    return posA <= posB;
}

Bool NationalTeamSorter(CTeamIndex const &a, CTeamIndex const &b) {
    if (!b.countryId)
        return true;
    if (!a.countryId)
        return false;
    UInt ranking1 = 0;
    if (a.countryId < 208) {
        CDBCountry *c = &GetCountryStore()->m_aCountries[a.countryId];
        ranking1 = *raw_ptr<UShort>(c, 0x284) + *raw_ptr<UShort>(c, 0x288) + *raw_ptr<UShort>(c, 0x28C) + *raw_ptr<UShort>(c, 0x290);
    }
    UInt ranking2 = 0;
    if (b.countryId < 208) {
        CDBCountry *c = &GetCountryStore()->m_aCountries[b.countryId];
        ranking2 = *raw_ptr<UShort>(c, 0x284) + *raw_ptr<UShort>(c, 0x288) + *raw_ptr<UShort>(c, 0x28C) + *raw_ptr<UShort>(c, 0x290);
    }
    return ranking1 >= ranking2;
}

UInt Oceania_ParticipantsCountries[] = {
    FifamCompRegion::Fiji,
    FifamCompRegion::Anguilla,
    FifamCompRegion::New_Zealand,
    FifamCompRegion::Papua_New_Guinea,
    FifamCompRegion::Solomon_Islands,
    FifamCompRegion::Tahiti,
    FifamCompRegion::Vanuatu,
    FifamCompRegion::American_Samoa,
    FifamCompRegion::Cook_Islands,
    FifamCompRegion::Samoa,
    FifamCompRegion::Tonga
};

UInt SouthAmerica_ParticipantsCountries[] = {
    FifamCompRegion::Brazil,
    FifamCompRegion::Argentina,
    FifamCompRegion::Colombia,
    FifamCompRegion::Chile,
    FifamCompRegion::Uruguay,
    FifamCompRegion::Bolivia,
    FifamCompRegion::Ecuador,
    FifamCompRegion::Paraguay,
    FifamCompRegion::Peru,
    FifamCompRegion::Venezuela
};

UInt NorthAmerica_ParticipantsCountries[] = {
    FifamCompRegion::Mexico,
    FifamCompRegion::United_States,
    FifamCompRegion::Canada,
    FifamCompRegion::Costa_Rica,
    FifamCompRegion::Honduras,
    FifamCompRegion::Panama,
    FifamCompRegion::El_Salvador,
    FifamCompRegion::Guatemala,
    FifamCompRegion::Nicaragua,
    FifamCompRegion::Belize,
    FifamCompRegion::Jamaica,
    FifamCompRegion::Haiti,
    FifamCompRegion::Trinidad_Tobago,
    FifamCompRegion::Dominican_Republic,
    FifamCompRegion::Antigua_and_Barbuda,
    FifamCompRegion::Aruba,
    FifamCompRegion::Bahamas,
    FifamCompRegion::Barbados,
    FifamCompRegion::Bermuda,
    FifamCompRegion::British_Virgin_Is,
    FifamCompRegion::Cayman_Islands,
    FifamCompRegion::Cuba,
    FifamCompRegion::Netherlands_Antil,
    FifamCompRegion::Dominica,
    FifamCompRegion::Grenada,
    FifamCompRegion::Guyana,
    FifamCompRegion::Montserrat,
    FifamCompRegion::Puerto_Rico,
    FifamCompRegion::St_Kitts_Nevis,
    FifamCompRegion::St_Lucia,
    FifamCompRegion::St_Vincent_Gren,
    FifamCompRegion::Surinam,
    FifamCompRegion::Turks_and_Caicos,
    FifamCompRegion::US_Virgin_Islands
};

UInt Africa_ParticipantsCountries[] = {
    FifamCompRegion::Morocco,
    FifamCompRegion::Tunisia,
    FifamCompRegion::Egypt,
    FifamCompRegion::Algeria,
    FifamCompRegion::DR_Congo,
    FifamCompRegion::South_Africa,
    FifamCompRegion::Zambia,
    FifamCompRegion::Sudan,
    FifamCompRegion::Nigeria,
    FifamCompRegion::Guinea,
    FifamCompRegion::Angola,
    FifamCompRegion::Tanzania,
    FifamCompRegion::Cote_d_Ivoire,
    FifamCompRegion::Benin,
    FifamCompRegion::Botswana,
    FifamCompRegion::Burkina_Faso,
    FifamCompRegion::Burundi,
    FifamCompRegion::Cameroon,
    FifamCompRegion::Cape_Verde_Islands,
    FifamCompRegion::Central_African_Rep,
    FifamCompRegion::Chad,
    FifamCompRegion::Congo,
    FifamCompRegion::Djibouti,
    FifamCompRegion::Equatorial_Guinea,
    FifamCompRegion::Eritrea,
    FifamCompRegion::Ethiopia,
    FifamCompRegion::Gabon,
    FifamCompRegion::Gambia,
    FifamCompRegion::Ghana,
    FifamCompRegion::Guinea_Bissau,
    FifamCompRegion::Kenya,
    FifamCompRegion::Lesotho,
    FifamCompRegion::Liberia,
    FifamCompRegion::Libya,
    FifamCompRegion::Madagascar,
    FifamCompRegion::Malawi,
    FifamCompRegion::Mali,
    FifamCompRegion::Mauritania,
    FifamCompRegion::Mauritius,
    FifamCompRegion::Mozambique,
    FifamCompRegion::Namibia,
    FifamCompRegion::Niger,
    FifamCompRegion::Rwanda,
    FifamCompRegion::Sao_Tome_e_Principe,
    FifamCompRegion::Senegal,
    FifamCompRegion::Seychelles,
    FifamCompRegion::Sierra_Leone,
    FifamCompRegion::Somalia,
    FifamCompRegion::Swaziland,
    FifamCompRegion::Togo,
    FifamCompRegion::Uganda,
    FifamCompRegion::Zimbabwe
};

UInt Asia_ParticipantsCountries[] = {
    FifamCompRegion::China_PR,
    FifamCompRegion::Qatar,
    FifamCompRegion::Japan,
    FifamCompRegion::Saudi_Arabia,
    FifamCompRegion::Korea_Republic,
    FifamCompRegion::Iran,
    FifamCompRegion::United_Arab_Emirates,
    FifamCompRegion::Thailand,
    FifamCompRegion::Iraq,
    FifamCompRegion::Uzbekistan,
    FifamCompRegion::Australia,
    FifamCompRegion::Jordan,
    FifamCompRegion::Philippines,
    FifamCompRegion::Korea_DPR,
    FifamCompRegion::India,
    FifamCompRegion::Vietnam,
    FifamCompRegion::Tajikistan,
    FifamCompRegion::Malaysia,
    FifamCompRegion::Singapore,
    FifamCompRegion::Turkmenistan,
    FifamCompRegion::Lebanon,
    FifamCompRegion::Syria,
    FifamCompRegion::Hong_Kong,
    FifamCompRegion::Myanmar,
    FifamCompRegion::Bahrain,
    FifamCompRegion::Bangladesh,
    FifamCompRegion::Maldives,
    FifamCompRegion::Indonesia,
    FifamCompRegion::Oman,
    FifamCompRegion::Palestinian_Authority,
    FifamCompRegion::Cambodia,
    FifamCompRegion::Macao,
    FifamCompRegion::Kyrgyzstan,
    FifamCompRegion::Kuwait,
    FifamCompRegion::Laos,
    FifamCompRegion::Nepal,
    FifamCompRegion::Sri_Lanka,
    FifamCompRegion::Bhutan,
    FifamCompRegion::Taiwan,
    FifamCompRegion::Mongolia,
    FifamCompRegion::Afghanistan,
    FifamCompRegion::Yemen,
    FifamCompRegion::Brunei_Darussalam,
    FifamCompRegion::Pakistan,
    FifamCompRegion::Guam
};

struct CompTypeNameDesc { const wchar_t *name; unsigned int id; };

CompTypeNameDesc gNewCompTypeNames[] = {
     { L"ROOT", 0 }, // Root competition
     { L"LEAGUE", 1 }, // League
     { L"FA_CUP", 3 }, // FA Cup
     { L"LE_CUP", 4 }, // League Cup
     { L"SUPERCUP", 7 }, // Supercup
     { L"CHALLENGE_SHIELD", 5 }, // Challenge Shield
     { L"CONFERENCE_CUP", 6 }, // Conference Cup
     { L"RELEGATION", 8 }, // Relegation
     { L"CHAMPIONSLEAGUE", 9 }, // Champions League
     { L"UEFA_CUP", 10 }, // UEFA Cup/Europa League
     { L"TOYOTA", 11 }, // TOYOTA Cup
     { L"EURO_SUPERCUP", 12 }, // European Supercup
     { L"WORLD_CLUB_CHAMP", 13 }, // World Club Championship
     { L"UIC", 14 }, // Intertoto Cup
     { L"QUALI_WC", 15 }, // World Cup Qualification
     { L"QUALI_EC", 16 }, // Euro Qualification
     { L"WORLD_CUP", 17 }, // World Cup - Final Stage
     { L"EURO_CUP", 18 }, // Euro - Final Stage
     { L"INDOOR", 22 }, // Indoor competition
     { L"RESERVE", 28 }, // Reserve competition
     { L"POOL", 24 }, // Pool
     { L"U20_WORLD_CUP", 31 }, // U-20 World Cup
     { L"CONFED_CUP", 32 }, // Confederations Cup
     { L"COPA_AMERICA", 33 }, // Copa America

     { L"ICC", 35 }, // International Champions Cup
     { L"Q_EURO_NL", 36 }, // Nations League Quali
     { L"EURO_NL", 37 }, // Nations League Finals
     { L"YOUTH_CHAMPIONSLEAGUE", 38 }, // Youth Champions League
     { L"CONTINENTAL_1", 39 }, // Continental 1
     { L"CONTINENTAL_2", 40 }, // Continental 2
     { L"Q_NAM_NL", 41 }, // North America Nations League Quali
     { L"NAM_NL", 42 }, // North America Nations League
     { L"NAM_CUP", 43 }, // North America Cup
     { L"Q_AFRICA_CUP", 44 }, // Africa Cup Quali
     { L"AFRICA_CUP", 45 }, // Africa Cup
     { L"Q_ASIA_CUP", 46 }, // Asia Cup Quali
     { L"ASIA_CUP", 47 }, // Asia Cup
     { L"Q_OFC_CUP", 48 }, // OFC Cup Quali
     { L"OFC_CUP", 49 }, // OFC Cup
     { L"Q_U20_WC", 50 }, // U20 WC Quali
     { L"CONFERENCE_LEAGUE", 51 }, // Conference League
     { L"FINALISSIMA", 52 }, // Finalissima
     { L"Q_U17_WC", 53 }, // U17 WC Quali
     { L"U17_WORLD_CUP", 54 }, // U17 WC
     { L"Q_U21_EC", 55 }, // U21 EC Quali
     { L"U21_EC", 56 }, // U21 EC
     { L"Q_U19_EC", 57 }, // U19 EC Quali
     { L"U19_EC", 58 }, // U19 EC
     { L"Q_U17_EC", 59 }, // U17 EC Quali
     { L"U17_EC", 60 }, // U17 EC
     { L"Q_OLYMPIC", 61 }, // Olympic Games Quali
     { L"OLYMPIC", 62 }, // Olympic Games
     { L"", 0 }
};

const unsigned char gNumCompetitionTypes = 53;

unsigned int gContinentalCompetitionTypes[] = {
    9, 10, COMP_CONFERENCE_LEAGUE, 12, 11, COMP_UIC, COMP_ICC, COMP_CONTINENTAL_1, COMP_CONTINENTAL_2 // TODO: set root for Youth CL and WCC
};

unsigned int gNewIntlComps[] = { 36, 37 };

unsigned int gInternationalCompsTypes[] = { COMP_EURO_NL_Q, 15, 16, COMP_EURO_NL, 17, 18, 32, 33, COMP_NAM_NL_Q, COMP_NAM_NL, COMP_NAM_CUP,
    COMP_AFRICA_CUP_Q, COMP_AFRICA_CUP, COMP_ASIA_CUP_Q, COMP_ASIA_CUP, COMP_OFC_CUP_Q, COMP_OFC_CUP, COMP_FINALISSIMA }; // for NT matches info screen

void OnSetCompetitionName(int compRegion, int compType, wchar_t const *name) {
    Call<0xF904D0>(compRegion, compType, name);

    // international

    Call<0xF904D0>(FifamCompRegion::International, COMP_EURO_NL_Q, GetTranslation("IDS_NAME_EURO_NL_QUALIFICATION"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_EURO_NL, GetTranslation("IDS_NAME_EURO_NL"));

    Call<0xF904D0>(FifamCompRegion::International, COMP_NAM_NL_Q, GetTranslation("IDS_NAME_NAM_NL_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_NAM_NL, GetTranslation("IDS_NAME_NAM_NL"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_NAM_CUP, GetTranslation("IDS_NAME_NAM_CUP"));

    Call<0xF904D0>(FifamCompRegion::International, COMP_ASIA_CUP_Q, GetTranslation("IDS_NAME_ASIA_CUP_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_ASIA_CUP, GetTranslation("IDS_NAME_ASIA_CUP"));

    Call<0xF904D0>(FifamCompRegion::International, COMP_AFRICA_CUP_Q, GetTranslation("IDS_NAME_AFRICA_CUP_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_AFRICA_CUP, GetTranslation("IDS_NAME_AFRICA_CUP"));

    Call<0xF904D0>(FifamCompRegion::International, COMP_OFC_CUP_Q, GetTranslation("IDS_NAME_OFC_CUP_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_OFC_CUP, GetTranslation("IDS_NAME_OFC_CUP"));

    Call<0xF904D0>(FifamCompRegion::International, COMP_FINALISSIMA, GetTranslation("IDS_NAME_FINALISSIMA"));

    // international youth

    Call<0xF904D0>(FifamCompRegion::International, COMP_U20_WC_Q, GetTranslation("IDS_NAME_U20_WC_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_U17_WORLD_CUP, GetTranslation("IDS_NAME_U17_WORLD_CUP"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_U17_WC_Q, GetTranslation("IDS_NAME_U17_WC_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_U21_EC, GetTranslation("IDS_NAME_U21_EC"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_U21_EC_Q, GetTranslation("IDS_NAME_U21_EC_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_U19_EC, GetTranslation("IDS_NAME_U19_EC"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_U19_EC_Q, GetTranslation("IDS_NAME_U19_EC_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_U17_EC, GetTranslation("IDS_NAME_U17_EC"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_U17_EC_Q, GetTranslation("IDS_NAME_U17_EC_Q"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_OLYMPIC, GetTranslation("IDS_NAME_OLYMPIC"));
    Call<0xF904D0>(FifamCompRegion::International, COMP_OLYMPIC_Q, GetTranslation("IDS_NAME_OLYMPIC_Q"));

    // continental

    Call<0xF904D0>(FifamCompRegion::Europe, COMP_ICC, GetTranslation("IDS_NAME_ICC"));
    Call<0xF904D0>(FifamCompRegion::Europe, COMP_UIC, GetTranslation("IDS_NAME_UIC"));
    Call<0xF904D0>(FifamCompRegion::Europe, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_EURO_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::Europe, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_EURO_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::Europe, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_EURO_CONTINENTAL_2"));
    Call<0xF904D0>(FifamCompRegion::Europe, COMP_CONFERENCE_LEAGUE, GetTranslation("IDS_NAME_EURO_CONFERENCE_LEAGUE"));

    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_SOUTHAM_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_SOUTHAM_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_SOUTHAM_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_SOUTHAM_CONTINENTAL_2"));
    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_CONFERENCE_LEAGUE, GetTranslation("IDS_NAME_SOUTHAM_CONFERENCE_LEAGUE"));

    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_NORTHAM_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_NORTHAM_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_NORTHAM_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_NORTHAM_CONTINENTAL_2"));
    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_CONFERENCE_LEAGUE, GetTranslation("IDS_NAME_NORTHAM_CONFERENCE_LEAGUE"));

    Call<0xF904D0>(FifamCompRegion::Africa, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_AFRICA_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::Africa, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_AFRICA_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::Africa, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_AFRICA_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::Africa, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_AFRICA_CONTINENTAL_2"));
    Call<0xF904D0>(FifamCompRegion::Africa, COMP_CONFERENCE_LEAGUE, GetTranslation("IDS_NAME_AFRICA_CONFERENCE_LEAGUE"));
    
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_ASIA_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_ASIA_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_ASIA_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_ASIA_CONTINENTAL_2"));
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_CONFERENCE_LEAGUE, GetTranslation("IDS_NAME_ASIA_CONFERENCE_LEAGUE"));

    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_OCEANIA_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_OCEANIA_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_OCEANIA_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_OCEANIA_CONTINENTAL_2"));
    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_CONFERENCE_LEAGUE, GetTranslation("IDS_NAME_OCEANIA_CONFERENCE_LEAGUE"));

    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_EURO_NL,
        CTeamIndex::make(FifamCompRegion::Spain, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Croatia, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_NAM_CUP,
        CTeamIndex::make(FifamCompRegion::Mexico, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Panama, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_ASIA_CUP,
        CTeamIndex::make(FifamCompRegion::Qatar, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Japan, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_AFRICA_CUP,
        CTeamIndex::make(FifamCompRegion::Senegal, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Egypt, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_OFC_CUP,
        CTeamIndex::make(FifamCompRegion::New_Zealand, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Papua_New_Guinea, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_FINALISSIMA,
        CTeamIndex::make(FifamCompRegion::Argentina, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Italy, 0, 0xFFFF));
    // TODO: Conference League winner in Rules.sav
}

void ReadCompetitionFile(wchar_t const *filename, wchar_t const *typeName, void *root) {
    static wchar_t buf[MAX_PATH];
    void *resolver = CallAndReturn<void *, 0x40BF10>();
    wchar_t *filepath = CallVirtualMethodAndReturn<wchar_t *, 5>(resolver, buf, 73, filename, 0, 0, 0, 0);
    unsigned int file[4];
    CallMethod<0x14B2BEA>(file);
    if (CallMethodAndReturn<bool, 0x14B2C35>(file, filepath)) {
        unsigned int rootCompId = 0;
        if (root)
            rootCompId = *raw_ptr<unsigned int>(root, 0x18);
        Call<0xF92660>(file, typeName, &rootCompId, -1, 0, 0, 0);
        CallMethod<0x14B3160>(file);
    }
    CallMethod<0x14B2C24>(file);
}

void METHOD OnSetupRootContinentalEurope(void *root, DUMMY_ARG, int id) {
    ReadCompetitionFile(L"Continental - Europe.txt", L"YOUTH", root);
    CallMethod<0x11F21B0>(root, id);
}

void METHOD OnSetupRootContinentalSouthAmerica(void *root, DUMMY_ARG, int id) {
    ReadCompetitionFile(L"Continental - South America.txt", L"YOUTH", root);
    CallMethod<0x11F21B0>(root, id);
}

void METHOD OnSetupRootInternational(void *root, DUMMY_ARG, int id) {
    ReadCompetitionFile(L"EuropeanChampionship.txt", L"EURO_NL", root);
    ReadCompetitionFile(L"EuropeanChampionship.txt", L"Q_EURO_NL", root);
    ReadCompetitionFile(L"AsiaCup.txt", L"ASIA_CUP", root);
    ReadCompetitionFile(L"AsiaCup.txt", L"Q_ASIA_CUP", root);
    ReadCompetitionFile(L"OFCCup.txt", L"OFC_CUP", root);
    ReadCompetitionFile(L"OFCCup.txt", L"Q_OFC_CUP", root);
    ReadCompetitionFile(L"NorthAmericaCup.txt", L"NAM_CUP", root);
    ReadCompetitionFile(L"NorthAmericaCup.txt", L"NAM_NL", root);
    ReadCompetitionFile(L"NorthAmericaCup.txt", L"Q_NAM_NL", root);
    ReadCompetitionFile(L"AfricaCup.txt", L"AFRICA_CUP", root);
    ReadCompetitionFile(L"AfricaCup.txt", L"Q_AFRICA_CUP", root);
    ReadCompetitionFile(L"CopaAmerica.txt", L"FINALISSIMA", root);
    //ReadCompetitionFile(L"WorldCupU17.txt", L"U17_WORLD_CUP", root);
    //ReadCompetitionFile(L"EuropeanChampionshipU21.txt", L"U21_EC", root);
    //ReadCompetitionFile(L"EuropeanChampionshipU19.txt", L"U19_EC", root);
    //ReadCompetitionFile(L"EuropeanChampionshipU17.txt", L"U17_EC", root);
    //ReadCompetitionFile(L"OlympicGames.txt", L"OLYMPIC", root);
    CallMethod<0x11F21B0>(root, id);
}

bool METHOD TakesPlaceInThisYear(CDBCompetition *comp, DUMMY_ARG, int) {
    void *game = CallAndReturn<void *, 0xF61410>();
    unsigned int date;
    CallMethod<0xF499A0>(game, &date);
    unsigned short year = CallMethodAndReturn<unsigned short, 0x1494CC9>(&date);
    switch (comp->GetCompID().type) {
    case COMP_QUALI_EC: // QUALI_EC
    case COMP_ASIA_CUP:
    case COMP_ASIA_CUP_Q:
        if (year % 4 == 2)
            return true;
        break;
    case COMP_WORLD_CUP: // WORLD_CUP
        if (year % 4 == 1)
            return true;
        break;
    case COMP_EURO_CUP: // EURO_CUP
    case COMP_COPA_AMERICA: // COPA_AMERICA
    case COMP_OFC_CUP:
        if (year % 4 == 3)
            return true;
        break;
    case COMP_QUALI_WC: // QUALI_WC
    case COMP_CONFED_CUP: // CONFED_CUP
    case COMP_FINALISSIMA: // FINALISSIMA
    case COMP_WORLD_CLUB_CHAMP:
        if (year % 4 == 0)
            return true;
        break;
    case COMP_EURO_NL_Q: // EURO_QUALI_NATIONSLEAGUE
    case COMP_EURO_NL: // EURO_NATIONSLEAGUE
    case COMP_AFRICA_CUP:
    case COMP_AFRICA_CUP_Q:
    case COMP_NAM_CUP:
    case COMP_U20_WC_Q:
    case COMP_U20_WORLD_CUP: // U20_WORLD_CUP
    case COMP_NAM_NL:
    case COMP_NAM_NL_Q:
        if (year % 2 == 0)
            return true;
        break;
    case COMP_YOUTH_CHAMPIONSLEAGUE:
        if (comp->GetCompID().countryId == FifamCompRegion::SouthAmerica) {
            if (year % 2 == 0)
                return true;
            return false;
        }
        return true;
    default:
        return true;
    }
    return false;
}

WideChar const * METHOD GetCompetitionShortName(CDBCompetition *comp) {
    CCompID compId = comp->GetCompID();
    switch (compId.type) {
    case COMP_WORLD_CLUB_CHAMP:
        return GetTranslation("IDS_FIFA_CLUB_WORLD_CUP_SHORTNAMES");
    case COMP_QUALI_WC:
        return GetTranslation("IDS_FIFA_WORLD_CUP_QUALIFICATION_SHORTNAMES");
    case COMP_WORLD_CUP:
        return GetTranslation("IDS_FIFA_SHORTNAMES");
    case COMP_CONFED_CUP:
        return GetTranslation("IDS_FIFA_CONFED_CUP_SHORTNAMES");
    case COMP_EURO_CUP:
        return GetTranslation("IDS_EURO_CUP_SHORTNAMES");
    case COMP_QUALI_EC:
        return GetTranslation("IDS_EURO_CUP_QUALIFICATION_SHORTNAMES");
    case COMP_COPA_AMERICA:
        return GetTranslation("IDS_COPA_AMERICA_SHORTNAMES");
    case COMP_EURO_NL_Q: // EURO_QUALI_NL
        return GetTranslation("IDS_EURO_NL_QUALIFICATION_SHORTNAMES");
    case COMP_EURO_NL: // EURO_NL
        return GetTranslation("IDS_EURO_NL_SHORTNAMES");
    case COMP_NAM_NL_Q:
        return GetTranslation("IDS_SHORTNAME_NAM_NL_Q");
    case COMP_NAM_NL:
        return GetTranslation("IDS_SHORTNAME_NAM_NL");
    case COMP_NAM_CUP:
        return GetTranslation("IDS_SHORTNAME_NAM_CUP");
    case COMP_ASIA_CUP_Q:
        return GetTranslation("IDS_SHORTNAME_ASIA_CUP_Q");
    case COMP_ASIA_CUP:
        return GetTranslation("IDS_SHORTNAME_ASIA_CUP");
    case COMP_AFRICA_CUP_Q:
        return GetTranslation("IDS_SHORTNAME_AFRICA_CUP_Q");
    case COMP_AFRICA_CUP:
        return GetTranslation("IDS_SHORTNAME_AFRICA_CUP");
    case COMP_OFC_CUP:
        return GetTranslation("IDS_SHORTNAME_OFC_CUP");
    case COMP_OFC_CUP_Q:
        return GetTranslation("IDS_SHORTNAME_OFC_CUP_Q");
    case COMP_FINALISSIMA:
        return GetTranslation("IDS_SHORTNAME_FINALISSIMA");
    case COMP_U20_WC_Q:
        return GetTranslation("IDS_SHORTNAME_U20_WC_Q");
    case COMP_U17_WORLD_CUP:
        return GetTranslation("IDS_SHORTNAME_U17_WORLD_CUP");
    case COMP_U17_WC_Q:
        return GetTranslation("IDS_SHORTNAME_U17_WC_Q");
    case COMP_U21_EC:
        return GetTranslation("IDS_SHORTNAME_U21_EC");
    case COMP_U21_EC_Q:
        return GetTranslation("IDS_SHORTNAME_U21_EC_Q");
    case COMP_U19_EC:
        return GetTranslation("IDS_SHORTNAME_U19_EC");
    case COMP_U19_EC_Q:
        return GetTranslation("IDS_SHORTNAME_U19_EC_Q");
    case COMP_U17_EC:
        return GetTranslation("IDS_SHORTNAME_U17_EC");
    case COMP_U17_EC_Q:
        return GetTranslation("IDS_SHORTNAME_U17_EC_Q");
    case COMP_OLYMPIC:
        return GetTranslation("IDS_SHORTNAME_OLYMPIC");
    case COMP_OLYMPIC_Q:
        return GetTranslation("IDS_SHORTNAME_OLYMPIC_Q");
    }
    if (CallMethodAndReturn<Bool, 0x12D4AA0>(&compId) || CallMethodAndReturn<Bool, 0x12D4A40>(&compId)) {
        CDBCompetition *baseComp = GetCompetition(compId.ToInt() & 0xFFFF0000);
        if (baseComp)
            return raw_ptr<WideChar>(baseComp, 0x26);
    }
    return raw_ptr<WideChar>(comp, 0x26);
}

void LaunchCompetition(unsigned char region, unsigned char type, unsigned short index) {
    void *comp = GetCompetition(region, type, index);
    if (comp)
        CallVirtualMethod<4>(comp);
}

void LaunchCompetitions() {
    void *game = CallAndReturn<void *, 0xF61410>();
    unsigned short year = CallMethodAndReturn<unsigned short, 0xF498C0>(game);
    if (year % 2 == 1) {
        //LaunchCompetition(255, COMP_NAM_NL_Q, 0);
        //LaunchCompetition(255, COMP_NAM_NL, 0);
    }
    if (year % 4 == 2) {
        LaunchCompetition(255, COMP_QUALI_EC, 0);
        LaunchCompetition(255, COMP_ASIA_CUP_Q, 0);
        //LaunchCompetition(255, COMP_ASIA_CUP, 0);
    }
    if (year % 4 == 3) {
        //LaunchCompetition(255, COMP_COPA_AMERICA, 0);
        LaunchCompetition(255, COMP_OFC_CUP, 0);
    }
    if (year % 2 == 0) {
        LaunchCompetition(255, COMP_NAM_NL_Q, 0);
        LaunchCompetition(255, COMP_EURO_NL_Q, 0);
        //LaunchCompetition(255, COMP_EURO_NL, 0);
        LaunchCompetition(255, COMP_AFRICA_CUP_Q, 0);
        //LaunchCompetition(255, COMP_AFRICA_CUP, 0);
        LaunchCompetition(255, COMP_NAM_CUP, 0);
        LaunchCompetition(255, COMP_U20_WC_Q, 0);
        LaunchCompetition(FifamCompRegion::SouthAmerica, COMP_YOUTH_CHAMPIONSLEAGUE, 0);
    }
    if (year % 4 == 0) {
        LaunchCompetition(255, COMP_QUALI_WC, 0);
        LaunchCompetition(255, COMP_CONFED_CUP, 0);
        LaunchCompetition(255, COMP_FINALISSIMA, 0);
        LaunchCompetition(249, COMP_WORLD_CLUB_CHAMP, 0);
    }
    if (year % 4 == 1) {
        if (GetCurrentYear() == GetStartingYear())
            LaunchCompetition(255, COMP_QUALI_WC, 0);
    }
    if (year % 4 == 3) {
        if (GetCurrentYear() == GetStartingYear())
            LaunchCompetition(255, COMP_QUALI_EC, 0);
    }

    LaunchCompetition(FifamCompRegion::NorthAmerica, COMP_YOUTH_CHAMPIONSLEAGUE, 0);
    LaunchCompetition(FifamCompRegion::Africa, COMP_YOUTH_CHAMPIONSLEAGUE, 0);
    LaunchCompetition(FifamCompRegion::Asia, COMP_YOUTH_CHAMPIONSLEAGUE, 0);
    LaunchCompetition(FifamCompRegion::Oceania, COMP_YOUTH_CHAMPIONSLEAGUE, 0);
}

void __declspec(naked) LaunchCompetitions_Exec() {
    __asm call LaunchCompetitions
    __asm mov eax, 0x11F5AF8
    __asm jmp eax
}

void Comps1(unsigned int compId, void *employee, void *vec) {
    unsigned int copaAmericaId = 0xFF210000;
    if (GetCompetition(copaAmericaId)) {
        Call<0x9767C0>(copaAmericaId, employee, vec);
        Call<0x976CC0>(copaAmericaId, employee, vec);
    }
    for (auto id : gNewIntlComps) {
        unsigned int newCompId = 0xFF000000 | (id << 16);
        if (GetCompetition(newCompId)) {
            Call<0x9767C0>(newCompId, employee, vec);
            Call<0x976CC0>(newCompId, employee, vec);
        }
    }
}

unsigned char gLastCompType = 0;

bool METHOD MatchdayScreen_TestGameFlag(void *game, DUMMY_ARG, unsigned int flag) {
    if (gLastCompType >= 15 && gLastCompType <= 18)
        return CallMethodAndReturn<bool, 0xF49CA0>(game, flag);
    return false;
}

unsigned char gCompTypesClub[] =         { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 14, 8, 19, 20, 21, 23, 27,
    COMP_ICC, COMP_CONTINENTAL_1, COMP_CONTINENTAL_2, COMP_CONFERENCE_LEAGUE };
unsigned char gCompTypesClubYouth[] =    { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 14, 8, 19, 20, 21, 23,
    COMP_YOUTH_CHAMPIONSLEAGUE, COMP_CONFERENCE_LEAGUE };
unsigned char gCompTypesNationalTeam[] = { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 8, 19, 20, 21, 15, 16, 17, 18, 32, 33, 29,
    COMP_EURO_NL_Q, COMP_EURO_NL, COMP_NAM_NL_Q, COMP_NAM_NL, COMP_NAM_CUP, COMP_AFRICA_CUP, COMP_AFRICA_CUP_Q, COMP_ASIA_CUP,
    COMP_ASIA_CUP_Q, COMP_OFC_CUP_Q, COMP_OFC_CUP, COMP_FINALISSIMA };
unsigned char *gCurrFixturesCompTypes = gCompTypesClub;

void *SetupFixturesList(unsigned int teamId) {
    void *team = CallAndReturn<void *, 0xEC8F70>(teamId);
    if (team) {
        unsigned short teamIndex = teamId & 0xFFFF;
        if (teamIndex == 0xFFFF)
            gCurrFixturesCompTypes = gCompTypesNationalTeam;
        else {
            unsigned char teamType = (teamId >> 24) & 0xFF;
            if (teamType == 2 || teamType == 4)
                gCurrFixturesCompTypes = gCompTypesClubYouth;
            else
                gCurrFixturesCompTypes = gCompTypesClub;
        }
    }
    return team;
}

void __declspec(naked) CheckFixtureList_1_Exec() { __asm {
    shr ecx, 0x10
    mov eax, gCurrFixturesCompTypes
    add eax, edx
    mov al, byte ptr[eax]
    cmp cl, al
    mov eax, 0x96A6BD
    jmp eax
}}

void __declspec(naked) CheckFixtureList_2_Exec() { __asm {
    mov edx, [esp+0x14]
    mov eax, gCurrFixturesCompTypes
    add eax, edx
    mov al, byte ptr[eax]
    cmp cl, al
    mov eax, 0x96AABD
    jmp eax
}}

void METHOD ChampionsLeagueUniversalSort(CDBPool *comp, DUMMY_ARG, int numGroups, int numTeamsInGroup) {
    auto id = comp->GetCompID();
    if (id.countryId == FifamCompRegion::International && id.type == COMP_EURO_NL_Q && id.index == 0) {
        //if (GetStartingYear() != 2023 || GetCurrentYear() != 2024) {
            comp->RandomlySortTeams(0, 4);
            comp->RandomlySortTeams(4, 4);
            comp->RandomlySortTeams(8, 4);
            comp->RandomlySortTeams(12, 4);

            comp->RandomlySortTeams(16, 4);
            comp->RandomlySortTeams(20, 4);
            comp->RandomlySortTeams(24, 4);
            comp->RandomlySortTeams(28, 4);

            comp->RandomlySortTeams(32, 4);
            comp->RandomlySortTeams(36, 4);
            comp->RandomlySortTeams(40, 4);
            comp->RandomlySortTeams(44, 4);

            comp->RandomlySortTeams(48, 2);
            comp->RandomlySortTeams(50, 2);
            comp->RandomlySortTeams(52, 2);
        //}
        return;
    }
    else if (comp->GetCompID().countryId == FifamCompRegion::International && comp->GetCompID().type == COMP_NAM_NL_Q && comp->GetCompID().index == 0) {
        // first 4 slots are reserved for Mexico, UnitedStates, CostaRica, Canada
        // League A
        comp->SortTeamIDs(4, 10, NationalTeamSorter); // League A
        comp->RandomlySortTeams(4, 2); // League A Pot 1
        comp->RandomlySortTeams(6, 2); // League A Pot 2
        comp->RandomlySortTeams(8, 2); // League A Pot 3
        comp->RandomlySortTeams(10, 2); // League A Pot 4
        comp->RandomlySortTeams(12, 2); // League A Pot 5
        // League B
        comp->SortTeamIDs(14, 12, NationalTeamSorter); // League B
        comp->RandomlySortTeams(14, 4); // League B Pot 1
        comp->RandomlySortTeams(18, 4); // League B Pot 2
        comp->RandomlySortTeams(22, 4); // League B Pot 3
        // League C
        comp->SortTeamIDs(26, 8, NationalTeamSorter); // League C
        comp->RandomlySortTeams(26, 2); // League C Pot 1
        comp->RandomlySortTeams(28, 2); // League C Pot 2
        comp->RandomlySortTeams(30, 2); // League C Pot 3
        comp->RandomlySortTeams(32, 2); // League C Pot 4
        return;
    }
    switch (comp->GetNumOfTeams()) {
    case 48:
        numGroups = 12;
        numTeamsInGroup = 4;
        break;
    case 40:
        numGroups = 10;
        numTeamsInGroup = 4;
        break;
    case 32:
        numGroups = 8;
        numTeamsInGroup = 4;
        break;
    case 30:
        numGroups = 10;
        numTeamsInGroup = 3;
        break;
    case 24:
        numGroups = 6;
        numTeamsInGroup = 4;
        break;
    case 20:
        numGroups = 5;
        numTeamsInGroup = 4;
        break;
    case 16:
        numGroups = 4;
        numTeamsInGroup = 4;
        break;
    case 15:
        numGroups = 5;
        numTeamsInGroup = 3;
        break;
    case 12:
        numGroups = 4;
        numTeamsInGroup = 3;
        break;
    case 10:
        numGroups = 2;
        numTeamsInGroup = 5;
        break;
    case 9:
        numGroups = 3;
        numTeamsInGroup = 3;
        break;
    case 8:
        numGroups = 2;
        numTeamsInGroup = 4;
        break;
    case 6:
        numGroups = 2;
        numTeamsInGroup = 3;
        break;
    }
    CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
    UInt numTeamsBefore = 0;
    for (UInt i = 0; i < comp->GetNumOfTeams(); i++) {
        if (pTeamIDs[i].countryId != 0)
            numTeamsBefore++;
    }
    Vector<CTeamIndex> teams(comp->GetNumOfTeams());
    memcpy(teams.data(), pTeamIDs, 4 * comp->GetNumOfTeams());
    Bool southAmericaRemoveBottom4 = false;
    if (id.countryId == FifamCompRegion::SouthAmerica && (id.type == COMP_CHAMPIONSLEAGUE || id.type == COMP_UEFA_CUP) && comp->GetNumOfTeams() == 32 && comp->GetNumOfRegisteredTeams() == 32 && numGroups == 8 && numTeamsInGroup == 4) {
        //SafeLog::WriteToFile("southam_pool.log", L"before sorting");
        //for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
        //    SafeLog::WriteToFile("southam_pool.log", Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamName(pTeamIDs[i])));
        // remove last 4 teams before sorting
        for (UInt i = 28; i < 32; i++)
            ClearID(pTeamIDs[i]);
        //SafeLog::WriteToFile("southam_pool.log", L"removed play-off teams");
        //for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
        //    SafeLog::WriteToFile("southam_pool.log", Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamName(pTeamIDs[i])));
        southAmericaRemoveBottom4 = true;
    }
    CallMethod<0x10F2340>(comp, numGroups, numTeamsInGroup);
    if (southAmericaRemoveBottom4) {
        //SafeLog::WriteToFile("southam_pool.log", L"after sorting");
        //for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
        //    SafeLog::WriteToFile("southam_pool.log", Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamName(pTeamIDs[i])));
        UInt numTeamsAdded = 0;
        for (UInt i = 0; i < 8; i++) {
            if (pTeamIDs[i * 4 + 3].countryId == 0) {
                pTeamIDs[i * 4 + 3] = teams[28 + numTeamsAdded];
                numTeamsAdded++;
                if (numTeamsAdded == 4)
                    break;
            }
        }
        //SafeLog::WriteToFile("southam_pool.log", L"final");
        //for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
        //    SafeLog::WriteToFile("southam_pool.log", Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamName(pTeamIDs[i])));
    }
    if (numTeamsBefore > 0) {
        UInt numTeamsAfter = 0;
        for (UInt i = 0; i < comp->GetNumOfTeams(); i++) {
            if (pTeamIDs[i].countryId != 0)
                numTeamsAfter++;
        }
        if (numTeamsAfter < numTeamsBefore) {
            //SafeLog::WriteToFile("cl_pool_sorting.log", L"Pool with universal ChampionsLeague sorting: wrong pool");
            //for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
            //    SafeLog::WriteToFile("cl_pool_sorting.log", Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamName(pTeamIDs[i])));
            memcpy(pTeamIDs, teams.data(), 4 * comp->GetNumOfTeams());
            if (id.countryId == FifamCompRegion::Europe && id.type == COMP_CHAMPIONSLEAGUE && comp->GetNumOfTeams() == 32 && numGroups == 8 && numTeamsInGroup == 4)
                comp->SortTeams(8, comp->GetNumOfTeams() - 8, Europe_ChampionsLeagueRoundSorter); // fix in FM 23: replaced 7 by 8
            else if (id.countryId == FifamCompRegion::Europe && id.type == COMP_UEFA_CUP && comp->GetNumOfTeams() == 32 && numGroups == 8 && numTeamsInGroup == 4)
                comp->SortTeams(1, comp->GetNumOfTeams() - 1, Europe_ChampionsLeagueRoundSorter);
            else if (id.countryId == FifamCompRegion::Europe && id.type != COMP_WORLD_CLUB_CHAMP && id.type != COMP_TOYOTA && id.type != COMP_ICC)
                comp->SortTeams(Europe_ChampionsLeagueRoundSorter);
            else
                comp->SortTeams(ChampionsLeagueRoundSorter);
            //SafeLog::WriteToFile("cl_pool_sorting.log", L"Pool with universal ChampionsLeague sorting: sorted by rating");
            //for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
            //    SafeLog::WriteToFile("cl_pool_sorting.log", Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamName(pTeamIDs[i])));
            for (Int i = 0; i < numTeamsInGroup; i++)
                comp->RandomlySortTeams(i * numGroups, numGroups);
            //SafeLog::WriteToFile("cl_pool_sorting.log", L"Pool with universal ChampionsLeague sorting: randomized by pots");
            //for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
            //    SafeLog::WriteToFile("cl_pool_sorting.log", Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamName(pTeamIDs[i])));
            memcpy(teams.data(), pTeamIDs, 4 * comp->GetNumOfTeams());
            memset(pTeamIDs, 0, 4 * comp->GetNumOfTeams());
            for (Int g = 0; g < numGroups; g++) {
                for (Int t = 0; t < numTeamsInGroup; t++)
                    pTeamIDs[g * numTeamsInGroup + t] = teams[t * numGroups + g];
            }
            //SafeLog::WriteToFile("cl_pool_sorting.log", L"Pool with universal ChampionsLeague sorting: fixed pool");
            //for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
            //    SafeLog::WriteToFile("cl_pool_sorting.log", Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamName(pTeamIDs[i])));
        }
    }
}

UChar GetCountryAtAssessmentPosition(UInt position) {
    UChar lpos = GetAssesmentTable()->GetCountryPosition(FifamCompRegion::Liechtenstein) + 1;
    UChar rpos = GetAssesmentTable()->GetCountryPosition(FifamCompRegion::Russia) + 1;
    if (position >= lpos)
        position += 1;
    if (position >= rpos)
        position += 1;
    return GetAssesmentTable()->GetCountryIdAtPosition(position);
}

unsigned char METHOD GetPoolNumberOfTeamsFromCountry(CDBPool *pool, DUMMY_ARG, int countryId) {
#define _hibyte(a) ((a>>24)&0xFF)
#define _hiword(a) ((a>>16)&0xFFFF)
#define _loword(a) (a&0xFFFF)
    int numTeams = 0;
    CDBCompetition *comp = nullptr;
    for (int i = 0; i < pool->GetNumOfScriptCommands(); i++) {
        auto command = pool->GetScriptCommand(i);
        switch (command->m_nCommandId) {
        case 2: // RESERVE_ASSESSMENT_TEAMS
            if (countryId == GetCountryAtAssessmentPosition(_loword(command->m_params)))
                numTeams += _hiword(command->m_params);
            break;
        case 4: // GET_EUROPEAN_ASSESSMENT_TEAMS
            if (countryId == GetCountryAtAssessmentPosition(_loword(command->m_params)))
                numTeams += _hibyte(command->m_params);
            break;
        case 7: // GET_TAB_X_TO_Y
            if (countryId == command->m_competitionId.countryId)
                numTeams += _hibyte(command->m_params);
            break;
        case 12: // GET_EUROPEAN_ASSESSMENT_CUPWINNER
            if (countryId == GetCountryAtAssessmentPosition(_loword(command->m_params)))
                --numTeams;
            break;
        case 28: // GET_INTERNATIONAL_TEAMS
            if (countryId == command->m_competitionId.countryId)
                numTeams += command->m_params;
            break;
        case 31: // GET_CHAMP_COUNTRY_TEAM
            comp = GetCompetition(command->m_competitionId);
            if (comp && countryId == pool->GetChampion().countryId)
                ++numTeams;
            break;
        }
    }
    return (numTeams > 0) ? numTeams : 0;
}

unsigned int gParticipantsRegion = 0;

bool GetFirstManagerRegion(unsigned int &outRegion) {
    for (unsigned int i = 0; i <= 5; i++) {
        if (plugin::CallAndReturn<bool, 0xFF7F60>(i)) {
            outRegion = 249 + i;
            return true;
        }
    }
    return false;
}

void METHOD OnFillEuropeanCompsParticipants(void *obj, DUMMY_ARG, unsigned int region, unsigned int compType, unsigned int listId) {
    if (listId == 1)
        return;
    gParticipantsRegion = region;
    GetFirstManagerRegion(gParticipantsRegion);
    UChar continentalComps[3] = {COMP_CHAMPIONSLEAGUE, COMP_UEFA_CUP, COMP_CONFERENCE_LEAGUE};
    for (UInt i = 0; i < 3; i++) {
        Bool enable = false;
        if (GetCompetition(gParticipantsRegion, continentalComps[i], 0)) {
            CallMethod<0x88F670>(obj, gParticipantsRegion, continentalComps[i], i);
            enable = true;
        }
        SetVisible(*raw_ptr<void*>(obj, 0x19C0 + i * 4), enable);
        SetVisible(*raw_ptr<void *>(obj, 0x4B4 + i * 0x704 + 8), enable);
    }
}

int METHOD EuropeanCompsParticipants_GetNumEntries(void *obj) {
    if (gParticipantsRegion == FifamCompRegion::SouthAmerica)
        return std::size(SouthAmerica_ParticipantsCountries);
    else if (gParticipantsRegion == FifamCompRegion::NorthAmerica)
        return std::size(NorthAmerica_ParticipantsCountries);
    else if (gParticipantsRegion == FifamCompRegion::Africa)
        return std::size(Africa_ParticipantsCountries);
    else if (gParticipantsRegion == FifamCompRegion::Asia)
        return std::size(Asia_ParticipantsCountries);
    else if (gParticipantsRegion == FifamCompRegion::Oceania)
        return std::size(Oceania_ParticipantsCountries);
    return plugin::CallMethodAndReturn<int, 0x121D1C0>(obj);
}

unsigned char METHOD EuropeanCompsParticipants_GetCountryAtPosition(void *obj, DUMMY_ARG, int position) {
    if (gParticipantsRegion == FifamCompRegion::SouthAmerica) {
        if (position > 0 && position <= (int)std::size(SouthAmerica_ParticipantsCountries))
            return SouthAmerica_ParticipantsCountries[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::NorthAmerica) {
        if (position > 0 && position <= (int)std::size(NorthAmerica_ParticipantsCountries))
            return NorthAmerica_ParticipantsCountries[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::Africa) {
        if (position > 0 && position <= (int)std::size(Africa_ParticipantsCountries))
            return Africa_ParticipantsCountries[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::Asia) {
        if (position > 0 && position <= (int)std::size(Asia_ParticipantsCountries))
            return Asia_ParticipantsCountries[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::Oceania) {
        if (position > 0 && position <= (int)std::size(Oceania_ParticipantsCountries))
            return Oceania_ParticipantsCountries[position - 1];
        return 0;
    }
    return plugin::CallMethodAndReturn<unsigned char, 0x121CFF0>(obj, position);
}

CDBPool *EuropeanCompsParticipants_GetPool(unsigned int countryId, unsigned int compType, unsigned short index) {
    return plugin::CallAndReturn<CDBPool *, 0xF8C5C0>(countryId, compType, index);
}

struct ScriptCommandCallData {
    CDBCompetition *currComp = nullptr;
    UInt nSpacesReservedForCountry[208] = {};
    UChar bReservedSpaces = 0;
    Char _pad345[3];
} gScriptCommandCallData;

CScriptCommand gScriptCommand;

void __declspec(naked) ScriptGetTabXToY_Exe() {
    __asm {
        lea eax, gScriptCommandCallData
        push eax
        lea eax, gScriptCommand
        mov ecx, 0x139E790
        call ecx
        add esp, 4
        retn
    }
}

void ScriptGetTabXToY(CDBCompetition *dst, CDBCompetition *from, UInt startPos, UInt numTeams) {
    gScriptCommandCallData.currComp = dst;
    gScriptCommand.m_competitionId = from->GetCompID();
    gScriptCommand.m_nCommandId = 7;
    *(UShort *)(&gScriptCommand.m_params) = startPos;
    ((UChar *)(&gScriptCommand.m_params))[3] = (UChar)numTeams;
    ScriptGetTabXToY_Exe();
}

void AddGermanyRegionalligaPromotionTeams(CDBCompetition *dst, Bool promotionRound) {
    Array<UInt, 3> leagueIndices = { 7, 3, 4 }; // Bayern, Nord, Nordost
    UInt seasonIndex = GetCurrentYear() % 3; // 0 in 2022, 1 in 2023, 2 in 2024
    if (promotionRound) {
        for (UInt i = 0; i < 3; i++) {
            if (i != seasonIndex) {
                auto l = GetLeague(FifamCompRegion::Germany, FifamCompType::League, leagueIndices[i]);
                if (l)
                    ScriptGetTabXToY(dst, l, 1, 1);
            }
        }
    }
    else {
        auto l = GetLeague(FifamCompRegion::Germany, FifamCompType::League, leagueIndices[seasonIndex]);
        if (l)
            ScriptGetTabXToY(dst, l, 1, 1);
    }
}

CTeamIndex GetTeamFromEuropeanAssociation(UInt association, Vector<CTeamIndex> const &ct) {
	CDBLeague *league = GetLeague(association, COMP_LEAGUE, 0);
	if (league) {
		UInt pos = 1;
		while (pos < league->GetNumOfRegisteredTeams()) {
			CTeamIndex teamId = league->GetTeamAtPosition(pos - 1);
			if (teamId.countryId != 0 && teamId.type == FifamClubTeamType::First && !Utils::Contains(ct, teamId))
				return teamId;
		}
	}
	return CTeamIndex::null();
}

template<typename T>
Bool VecRemoveMulti(Vector<Vector<T>> &vec, T const &item) {
	for (auto &v : vec) {
		if (Utils::Contains(v, item)) {
			Utils::Remove(v, item);
			return true;
		}
	}
	return false;
}

void GenerateUEFALeaguePhaseMatches(String const &name, CDBCompetition *poolTeams, CDBCompetition *poolFixtures, UInt numPots, UInt numMatchdays) {
    if (!poolTeams || !poolFixtures)
        return;
    DrawUEFALeaguePhase(poolTeams, poolFixtures, numPots, numMatchdays);

}

template<typename T>
Bool VecContainsMulti(Vector<Vector<T>> &vec, T const &item) {
	for (auto &v : vec) {
		if (Utils::Contains(v, item))
			return true;
	}
	return false;
}

CTeamIndex GetCompFinalist(UChar region, UChar type, UInt year, Bool runnerUp) {
    auto compFinal = GetRoundByRoundType(region, type, ROUND_FINAL);
    if (compFinal) {
        CompMatchResult *info = CallMethodAndReturn<CompMatchResult *, 0xF89B90>(compFinal, year, ROUND_FINAL);
        if (info) {
            UChar result1 = info->result1stLeg[0];
            UChar result2 = info->result1stLeg[1];
            if (info->flags & FifamBeg::With2ndLeg) {
                result1 += info->result2ndtLeg[0];
                result2 += info->result2ndtLeg[1];
            }
            if (result2 > result1)
                return runnerUp ? info->team1 : info->team2;
            return runnerUp ? info->team2 : info->team1;
        }
    }
    return CTeamIndex::null();
};

CTeamIndex GetLeagueTeamAtPlace(CDBLeague *league, UInt place) {
    if (league && league->GetNumOfTeams() > place) {
        TeamLeaguePositionData infos[24];
        league->SortTeams(infos, league->GetEqualPointsSorting(), 0, 120, 0, 120);
        if (league->GetCompID().countryId < 208)
            CallMethod<0x1052910>(league, infos);
        return infos[place].m_teamID;
    }
    return CTeamIndex::null();
}

UInt StadiumCapacity(CDBTeam *team) {
    void *stadiumDevelopment = CallMethodAndReturn<void *, 0xECFFC0>(team);
    if (stadiumDevelopment)
        return CallMethodAndReturn<UInt, 0xF74220>(stadiumDevelopment);
    return 0;
}

String StadiumStringId(CDBTeam *team) {
    void *stadiumDevelopment = CallMethodAndReturn<void *, 0xECFFC0>(team);
    if (stadiumDevelopment) {
        UInt stadiumCapacity = CallMethodAndReturn<UInt, 0xF74220>(stadiumDevelopment);
        if (stadiumCapacity > 0) {
            String stadiumName = CallMethodAndReturn<WideChar const *, 0xF73BD0>(stadiumDevelopment, true);
            if (!stadiumName.empty())
                return stadiumName + Utils::Format(L"%u", stadiumCapacity);
        }
    }
    return String();
}

UInt GetMinCapacityForCompetitionFinal(CCompID const &compId) {
    switch (compId.countryId) {
    case FifamCompRegion::Europe:
        if (compId.type == COMP_CHAMPIONSLEAGUE)
            return 50'000;
        return 40'000;
    case FifamCompRegion::SouthAmerica:
        if (compId.type == COMP_CHAMPIONSLEAGUE)
            return 70'000;
        return 50'000;
    case FifamCompRegion::NorthAmerica:
        if (compId.type == COMP_CHAMPIONSLEAGUE)
            return 25'000;
        return 20'000;
    case FifamCompRegion::Africa:
        if (compId.type == COMP_CHAMPIONSLEAGUE)
            return 60'000;
        return 20'000;
    case FifamCompRegion::Asia:
        if (compId.type == COMP_CHAMPIONSLEAGUE)
            return 40'000;
        return 20'000;
    case FifamCompRegion::Oceania:
        return 10'000;
    }
    return 30'000;
}

Set<String> GetExcludedStadiums(Bool isEurope) {
    Set<String> excludedStadiums;
    if (isEurope) {
        void *hosts = CallAndReturn<void *, 0x117C830>();
        UInt clHostId = CallMethodAndReturn<UInt, 0x117A560>(hosts);
        if (clHostId > 0) {
            auto clHostTeam = GetTeam(CTeamIndex::make(clHostId));
            if (clHostTeam) {
                String id = StadiumStringId(clHostTeam);
                if (!id.empty())
                    excludedStadiums.insert(id);
            }
        }
        UInt elHostId = CallMethodAndReturn<UInt, 0x117A570>(hosts);
        if (elHostId > 0) {
            auto elHostTeam = GetTeam(CTeamIndex::make(elHostId));
            if (elHostTeam) {
                String id = StadiumStringId(elHostTeam);
                if (!id.empty())
                    excludedStadiums.insert(id);
            }
        }
    }
    return excludedStadiums;
}

void SelectWCCHostTeam(CDBCompetition *comp) {
    CTeamIndex hostTeam = CTeamIndex::null();
    FifamNation countriesList[] = {
        FifamNation::United_States,
        FifamNation::Saudi_Arabia,
        FifamNation::China_PR,
        FifamNation::Qatar
    };
    CDBCountry *hostCountry = GetCountry(countriesList[CRandom::GetRandomInt(std::size(countriesList))].ToInt());
    if (hostCountry) {
        Vector<Pair<UInt, CDBTeam *>> teams;
        for (Int t = 1; t <= hostCountry->GetLastTeamIndex(); t++) {
            auto team = GetTeam(CTeamIndex::make(hostCountry->GetCountryId(), FifamClubTeamType::First, t));
            if (team) {
                teams.emplace_back(
                    (team->GetInternationalPrestige() << 24) | (team->GetNationalPrestige() << 16) | CRandom::GetRandomInt(32767),
                    team);
            }
        }
        if (!teams.empty()) {
            Utils::Sort(teams, [](Pair<UInt, CDBTeam *> const &a, Pair<UInt, CDBTeam *> const &b) { return a.first >= b.first; });
            hostTeam = teams[CRandom::GetRandomInt(Utils::Min(3u, teams.size()))].second->GetTeamID();
        }
    }
    if (hostTeam.isNull())
        hostTeam = CTeamIndex::make(FifamNation::United_States, FifamClubTeamType::First, 1);
    comp->AddTeam(hostTeam);
}

void SelectWCCHostStadiums(CDBCompetition *comp) {
    CDBPool *hostTeamPool = GetPool(FifamCompRegion::Europe, COMP_WORLD_CLUB_CHAMP, 0);
    if (!hostTeamPool)
        return;
    if (hostTeamPool->GetNumOfRegisteredTeams() == 0) {
        SelectWCCHostTeam(hostTeamPool);
        if (hostTeamPool->GetNumOfRegisteredTeams() == 0)
            return;
    }
    CDBTeam *hostTeam = GetTeam(hostTeamPool->GetTeamID(0));
    if (!hostTeam)
        return;
    CDBCountry *hostCountry = hostTeam->GetCountry();
    if (!hostCountry)
        return;
    Set<String> excludedStadiums = GetExcludedStadiums(hostCountry->GetContinent() == FifamContinent::Europe);
    Map<String, CDBTeam *> uniqueStadiums;
    for (Int t = 1; t <= hostCountry->GetLastTeamIndex(); t++) {
        auto team = GetTeam(CTeamIndex::make(hostCountry->GetCountryId(), FifamClubTeamType::First, t));
        if (team) {
            String id = StadiumStringId(team);
            if (!id.empty() && !Utils::Contains(excludedStadiums, id))
                uniqueStadiums[id] = team;
        }
    }
    if (uniqueStadiums.empty())
        return;
    Vector<CDBTeam *> stadiumsSorted;
    for (auto const &[s, team] : uniqueStadiums)
        stadiumsSorted.push_back(team);
    Utils::Sort(stadiumsSorted, [](CDBTeam *a, CDBTeam *b) {
        return StadiumCapacity(a) > StadiumCapacity(b);
    });
    Utils::Shuffle(stadiumsSorted, 3);
    for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
        comp->AddTeam(stadiumsSorted[i % Utils::Min(12u, stadiumsSorted.size())]->GetTeamID());
}

void OnGetSpare(CDBCompetition **ppComp) {
    const Bool DUMP_TO_LOG = true;
    auto Log = [&DUMP_TO_LOG](String const &message) {
        if (DUMP_TO_LOG)
            SafeLog::Write(message);
    };
    auto DumpPool = [&Log](CDBCompetition *comp, String const &title) {
        Log(title);
        CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
        for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
            Log(Utils::Format(L"%2d. %s", i, TeamTagWithCountry(pTeamIDs[i])));
    };
    if (*ppComp) {
        auto comp = *ppComp;
        auto id = comp->GetCompID();
        auto rt = comp->GetRoundType();
        if (id.countryId == FifamCompRegion::Europe) {
            if (comp->GetDbType() == DB_ROUND) {
                if (id.type == COMP_YOUTH_CHAMPIONSLEAGUE) {
                    if (id.index == 13) { // last32
                        if (comp->GetNumOfTeams() == 32) {
                            comp->RandomlySortTeams(0, 6);
                            comp->RandomlySortTeams(6, 10);
                            comp->RandomlySortTeams(16, 6);
                            comp->RandomlySortTeams(22, 10);
                        }
                    }
                    else {
                        comp->SortTeams(Europe_YouthChampionsLeagueRoundSorter);
                        comp->RandomizePairs();
                    }
                }
                else {
                    comp->SortTeams(Europe_ChampionsLeagueRoundSorter);
                    //comp->RandomizePairs();
                    if (comp->GetNumOfTeams() == comp->GetNumOfRegisteredTeams()) {
                        comp->RandomlySortTeams(0, comp->GetNumOfRegisteredTeams() / 2);
                        comp->RandomlySortTeams(comp->GetNumOfRegisteredTeams() / 2);
                    }
                    else if (comp->GetNumOfRegisteredTeams() >= comp->GetNumOfTeams() / 2) {
                        UInt numTeamsToSort = comp->GetNumOfTeams() / 2 - (comp->GetNumOfTeams() - comp->GetNumOfRegisteredTeams());
                        CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
                        std::reverse(&pTeamIDs[0], &pTeamIDs[comp->GetNumOfTeams() / 2]);
                        if (numTeamsToSort != 0) {
                            comp->RandomlySortTeams(0, numTeamsToSort);
                            comp->RandomlySortTeams(comp->GetNumOfTeams() / 2, numTeamsToSort);
                        }
                    }
                    else
                        comp->RandomlySortTeams(0, comp->GetNumOfRegisteredTeams());
                }
            }
            else if (comp->GetDbType() == DB_POOL) {
                if (id.type == COMP_CHAMPIONSLEAGUE && id.index == 0 && comp->GetNumOfTeams() == 83 /*&& comp->GetNumOfRegisteredTeams() == 79*/) {

                    CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
					Vector<CTeamIndex> leaguePhase;
					Vector<Vector<CTeamIndex>> qualiChamp(3);
					Vector<Vector<CTeamIndex>> qualiLeague(2);

					auto DumpCLRounds = [&Log, &leaguePhase, &qualiChamp, &qualiLeague](String const &title) {
                        Log(title);
                        Log(L"League Phase:");
                        for (UInt i = 0; i < leaguePhase.size(); i++)
                            Log(Utils::Format(L"%2d. %s", i, TeamTagWithCountry(leaguePhase[i])));
                        for (UInt r = 0; r < qualiChamp.size(); r++) {
                            Log(Utils::Format(L"Champ Quali %d:", r + 1));
                            for (UInt i = 0; i < qualiChamp[r].size(); i++)
                                SafeLog::Write(Utils::Format(L"%2d. %s", i, TeamTagWithCountry(qualiChamp[r][i])));
                        }
                        for (UInt r = 0; r < qualiLeague.size(); r++) {
                            SafeLog::Write(Utils::Format(L"League Quali %d:", r + 1));
                            for (UInt i = 0; i < qualiLeague[r].size(); i++)
                                SafeLog::Write(Utils::Format(L"%2d. %s", i, TeamTagWithCountry(qualiLeague[r][i])));
                        }
					};

                    DumpPool(comp, L"Champions League Pool");

					for (UInt i = 0; i < 25; i++) {
						CTeamIndex teamID = pTeamIDs[i];
						if (!teamID.isNull())
							leaguePhase.push_back(teamID);
					}
					for (UInt i = 0; i < 4; i++) {
						CTeamIndex teamID = pTeamIDs[i + 25];
						if (!teamID.isNull())
							qualiChamp[0].push_back(teamID);
					}
					for (UInt i = 0; i < 8; i++) {
						CTeamIndex teamID = pTeamIDs[i + 29];
						if (!teamID.isNull())
							qualiChamp[1].push_back(teamID);
					}
					for (UInt i = 0; i < 31; i++) {
						CTeamIndex teamID = pTeamIDs[i + 37];
						if (!teamID.isNull())
							qualiChamp[2].push_back(teamID);
					}
					for (UInt i = 0; i < 5; i++) {
						CTeamIndex teamID = pTeamIDs[i + 68];
						if (!teamID.isNull())
							qualiLeague[0].push_back(teamID);
					}
					for (UInt i = 0; i < 6; i++) {
						CTeamIndex teamID = pTeamIDs[i + 73];
						if (!teamID.isNull())
							qualiLeague[1].push_back(teamID);
					}

					DumpCLRounds(L"Champions League Rounds");

					if (leaguePhase.empty() || qualiChamp[0].empty() || qualiChamp[1].empty() || qualiChamp[2].empty() || qualiLeague[0].empty() || qualiLeague[1].empty()) {
                        Log(L"Not enough teams in Champions League");
						return;
					}

					CTeamIndex winnerCL = CTeamIndex::null();
					CTeamIndex winnerEL = CTeamIndex::null();

					auto finalCL = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, FifamRoundID::Final);
					auto finalEL = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::UefaCup, FifamRoundID::Final);
					if (finalCL)
						winnerCL = finalCL->GetChampion();
					if (finalEL)
						winnerEL = finalEL->GetChampion();

					Log(Utils::Format(L"Champions League Winner: %s", TeamTagWithCountry(winnerCL)));
					Log(Utils::Format(L"Europa League Winner: %s", TeamTagWithCountry(winnerEL)));

					// Champions League winner

					if (!winnerCL.isNull()) {
						if (Utils::Contains(leaguePhase, winnerCL)) {
							Vector<CTeamIndex> qualiTeams;
							for (auto &v : qualiChamp)
								qualiTeams.insert(qualiTeams.end(), v.begin(), v.end());
							if (!qualiTeams.empty()) {
								Utils::Sort(qualiTeams, [](CTeamIndex const &a, CTeamIndex const &b) {
									return Europe_ChampionsLeagueRoundSorter(GetTeam(a), GetTeam(b));
								});
								CTeamIndex bestTeam = qualiTeams.front();
								leaguePhase.push_back(bestTeam);
								VecRemoveMulti(qualiChamp, bestTeam);
                                Log(Utils::Format(L"%s moved to League Phase because of CL Winner", TeamTagWithCountry(bestTeam)));
							}
						}
						else {
							leaguePhase.push_back(winnerCL);
							if (!VecRemoveMulti(qualiChamp, winnerCL))
								VecRemoveMulti(qualiLeague, winnerCL);
						}
					}

					// Europa League winner

					if (!winnerEL.isNull()) {
						if (Utils::Contains(leaguePhase, winnerEL)) {
							Vector<CTeamIndex> qualiTeams;

							for (auto &qc : qualiChamp) // For all rounds in Champions Quali
								qualiTeams.insert(qualiTeams.end(), qc.begin(), qc.end()); // Add all teams

							for (auto &ql : qualiLeague) { // For all rounds in League Quali
								for (auto &teamFromQualiLeague : ql) { // For all teams in round
									// Do they have higher-ranked domestic team in Champions Quali?
									Bool haveHigherRankedDomesticTeam = false;
									for (auto const &qc : qualiChamp) { // For all rounds in Champions Quali
										for (auto const &teamFromQualiChamp : qc) { // For all teams in round
											if (teamFromQualiChamp.countryId == teamFromQualiLeague.countryId) { // From same country
												haveHigherRankedDomesticTeam = true;
												break;
											}
										}
									}
									if (!haveHigherRankedDomesticTeam)
										qualiTeams.push_back(teamFromQualiLeague);
								}
							}
							if (!qualiTeams.empty()) {
								Utils::Sort(qualiTeams, [](CTeamIndex const &a, CTeamIndex const &b) {
									return Europe_ChampionsLeagueRoundSorter(GetTeam(a), GetTeam(b));
								});
								CTeamIndex bestTeam = qualiTeams.front();
								leaguePhase.push_back(bestTeam);
								if (!VecRemoveMulti(qualiChamp, bestTeam))
									VecRemoveMulti(qualiLeague, bestTeam);
                                Log(Utils::Format(L"%s moved to League Phase because of EL Winner", TeamTagWithCountry(bestTeam)));
							}
						}
						else {
							leaguePhase.push_back(winnerEL);
							if (!VecRemoveMulti(qualiChamp, winnerEL))
								VecRemoveMulti(qualiLeague, winnerEL);
						}
					}

					Vector<Pair<UChar, Float>> assessmentRanking;

					CAssessmentTable *at = GetAssesmentTable();
					for (UInt a = 0; a < at->m_nNumEntries; a++) {
						CAssessmentInfo *info = &at->m_aEntries[a];
						if (info->m_nCountryIndex != FifamCompRegion::Russia && info->m_nCountryIndex != FifamCompRegion::Liechtenstein)
							assessmentRanking.emplace_back(info->m_nCountryIndex, info->m_fYear_2);
					}
					Utils::Sort(assessmentRanking, [](Pair<UChar, Float> const &a, Pair<UChar, Float> const &b) {
						return a.second > b.second;
					});

					UChar bestAssessmentCountry1 = assessmentRanking.size() > 0 ? assessmentRanking[0].first : 0;
					UChar bestAssessmentCountry2 = assessmentRanking.size() > 1 ? assessmentRanking[1].first : 0;

                    Log(Utils::Format(L"Best assessment countries: %s, %s", CountryName(bestAssessmentCountry1), CountryName(bestAssessmentCountry2)));

					auto AddTeamForEPS = [&Log, &leaguePhase, &qualiChamp, &qualiLeague](UChar countryId) {
						Vector<CTeamIndex *> teamsFromThisCountryInQuali;
						// Find all teams from this country in Champions Quali
						for (auto &qc : qualiChamp) { // For all rounds in Champions Quali
							for (auto &team : qc) { // For all teams in round
								if (team.countryId == countryId)
                                    teamsFromThisCountryInQuali.push_back(&team);
							}
						}
						// Find all teams from this country in League Quali
						for (auto &ql : qualiLeague) { // For all rounds in League Quali
							for (auto &team : ql) { // For all teams in round
								if (team.countryId == countryId)
                                    teamsFromThisCountryInQuali.push_back(&team);
							}
						}
						// Get one more team from this association
                        CTeamIndex oneMoreTeam = CTeamIndex::null();
						CDBLeague *league = GetLeague(countryId, COMP_LEAGUE, 0);
						if (league) {
							UInt pos = 1;
							while (pos < league->GetNumOfRegisteredTeams()) {
								CTeamIndex team = league->GetTeamAtPosition(pos - 1);
								if (team.countryId != 0 && team.type == FifamClubTeamType::First
									&& !Utils::Contains(leaguePhase, team)
									&& !VecContainsMulti(qualiChamp, team)
									&& !VecContainsMulti(qualiLeague, team))
								{
                                    oneMoreTeam = team;
                                    break;
								}
                                pos++;
							}
						}
                        if (!oneMoreTeam.isNull()) {
                            if (teamsFromThisCountryInQuali.empty()) { // If there are no teams in quali - add new team to League Phase
                                leaguePhase.push_back(oneMoreTeam);
                                Log(Utils::Format(L"%s moved to League Phase because of EPS", TeamTagWithCountry(oneMoreTeam)));
                            }
                            else {
                                // Move first team in quali to League Phase
                                leaguePhase.push_back(*teamsFromThisCountryInQuali[0]);
                                Log(Utils::Format(L"%s moved to League Phase because of EPS", TeamTagWithCountry(*teamsFromThisCountryInQuali[0])));
                                // Shift all teams in quali
                                for (UInt i = 1; i < teamsFromThisCountryInQuali.size(); i++) {
                                    *teamsFromThisCountryInQuali[i - 1] = *teamsFromThisCountryInQuali[i];
                                    Log(Utils::Format(L"%s moved to next Quali Phase because of EPS", TeamTagWithCountry(*teamsFromThisCountryInQuali[i])));
                                }
                                // Put new team to last quali place
                                *teamsFromThisCountryInQuali[teamsFromThisCountryInQuali.size() - 1] = oneMoreTeam;
                                Log(Utils::Format(L"%s added to Qualification because of EPS", TeamTagWithCountry(oneMoreTeam)));
                            }
                        }
                        else
                            Log(Utils::Format(L"Unable to add one more team from %s", CountryName(countryId)));
					};

					if (bestAssessmentCountry1 != 0)
					    AddTeamForEPS(bestAssessmentCountry1);
					if (bestAssessmentCountry2 != 0)
					    AddTeamForEPS(bestAssessmentCountry2);

					DumpCLRounds(L"Champions League Rounds with all teams");

                    auto FillRound = [&Log](String const &name, Vector<CTeamIndex> &r, UInt teams, Vector<Vector<CTeamIndex> *> const &otherRounds) {
                        if (r.size() > teams) {
                            Log(Utils::Format(L"Wrong team count in round %s (%d/%d)", name, r.size(), teams));
                            return;
                        }
                        if (r.size() < teams) {
                            for (auto &v : otherRounds) {
                                while (r.size() < teams && !v->empty()) {
                                    r.push_back(*(v->begin()));
                                    v->erase(v->begin());
                                }
                                if (r.size() == teams)
                                    break;
                            }
                        }
                    };

                    FillRound(L"League Phase", leaguePhase, 29, { &qualiChamp[0], &qualiChamp[1], &qualiChamp[2], &qualiLeague[0], &qualiLeague[1] });
                    FillRound(L"Champ Quali 1", qualiChamp[0], 4, { &qualiChamp[1], &qualiChamp[2] });
                    FillRound(L"Champ Quali 2", qualiChamp[1], 8, { &qualiChamp[2] });
                    FillRound(L"League Quali 1", qualiLeague[0], 5, { &qualiLeague[1] });
                    
					DumpCLRounds(L"Champions League Rounds with filled rounds");

                    CallMethod<0xF82440>(comp); // CDBCompetition::ClearTeams()

                    auto AddTeams = [](CTeamIndex *dst, Vector<CTeamIndex> const &src, UInt numTeams) {
                        UInt numAdded = 0;
                        for (auto const &t : src) {
                            if (numAdded < numTeams)
                                dst[numAdded++] = t;
                            else
                                break;
                        }
                    };

                    AddTeams(&pTeamIDs[0], leaguePhase, 29);
                    AddTeams(&pTeamIDs[29], qualiChamp[0], 4);
                    AddTeams(&pTeamIDs[33], qualiChamp[1], 8);
                    AddTeams(&pTeamIDs[41], qualiChamp[2], 31);
                    AddTeams(&pTeamIDs[72], qualiLeague[0], 5);
                    AddTeams(&pTeamIDs[77], qualiLeague[1], 6);

                    comp->SetNumOfRegisteredTeams(comp->GetNumOfTeams());

                    DumpPool(comp, L"Modified Champions League Pool");
                }
                else if (id.type == COMP_CHAMPIONSLEAGUE && id.index == 8 && comp->GetNumOfTeams() == 36) {
                    CTeamIndex winnerCL = CTeamIndex::null();
                    auto finalCL = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, FifamRoundID::Final);
                    if (finalCL)
                        winnerCL = finalCL->GetChampion();
                    comp->SortTeamIDs([&winnerCL](CTeamIndex const &a, CTeamIndex const &b) {
                        if (!b.countryId)
                            return true;
                        if (!a.countryId)
                            return false;
                        if (!winnerCL.isNull()) {
                            if (a == winnerCL)
                                return true;
                            else if (b == winnerCL)
                                return false;
                        }
                        return Europe_ChampionsLeagueRoundSorter(GetTeam(a), GetTeam(b));
                    });
                    comp->RandomlySortTeams(0, 9);
                    comp->RandomlySortTeams(9, 9);
                    comp->RandomlySortTeams(18, 9);
                    comp->RandomlySortTeams(27, 9);
                    DumpPool(comp, L"Champions League teams sorted by pots");
                }
                else if (id.type == COMP_CHAMPIONSLEAGUE && id.index == 9 && comp->GetNumOfTeams() == 288) {
                    GenerateUEFALeaguePhaseMatches(L"Champions League", GetPool(FifamCompRegion::Europe, COMP_CHAMPIONSLEAGUE, 8), comp, 4, 8);
                    DumpPool(comp, L"Champions League generated matchdays");
                }
                else if (id.type == COMP_CHAMPIONSLEAGUE && id.index == 18 && comp->GetNumOfTeams() == 36) {
                    SortUEFALeaguePhaseTable(0xF9090000, comp);
                    DumpPool(comp, L"Champions League League Phase sorted table");
                }
                else if (id.type == COMP_UEFA_CUP && id.index == 0 && comp->GetNumOfTeams() == 46 /*&& comp->GetNumOfRegisteredTeams() == 45*/) {
                    auto finalConf = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::ConferenceLeague, FifamRoundID::Final);
                    if (finalConf) {
                        auto confWinner = finalConf->GetChampion();
                        Log(Utils::Format(L"Europa League: Conference League winner is %s", TeamTagWithCountry(confWinner)));
                        if (confWinner.countryId != 0) {
                            auto compCL = GetCompetition(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 0);
                            if (!compCL || !compCL->IsTeamPresent(confWinner)) {
                                CTeamIndex* pTeamIDs = *raw_ptr<CTeamIndex*>(comp, 0xA0);
                                DumpPool(comp, L"Europa League Pool");
                                Int teamPos = comp->GetTeamIndex(confWinner);
                                if (teamPos != 0) {
                                    if (teamPos == -1)
                                        teamPos = comp->GetNumOfRegisteredTeams();
                                    for (UInt i = teamPos; i > 0; i--)
                                        pTeamIDs[i] = pTeamIDs[i - 1];
                                    pTeamIDs[0] = confWinner;
                                    DumpPool(comp, L"Modified Europa League Pool");
                                }
                                else
                                    Log(L"Europa League - Conference League Winner already at position 0.");
                            }
                            else
                                Log(L"Europa League - Conference League Winner already in Champions League pool");
                        }
                        else
                            Log(L"Europa League - Conference League Winner is not set");
                    }
                    else
                        Log(L"Europa League - Conference League Final is not available");
                    comp->SetNumOfRegisteredTeams(comp->GetNumOfTeams());
                }
                else if (id.type == COMP_UEFA_CUP && id.index == 6 && comp->GetNumOfTeams() == 36) {
                    comp->SortTeams(Europe_ChampionsLeagueRoundSorter);
                    comp->RandomlySortTeams(0, 9);
                    comp->RandomlySortTeams(9, 9);
                    comp->RandomlySortTeams(18, 9);
                    comp->RandomlySortTeams(27, 9);
                    DumpPool(comp, L"Europa League teams sorted by pots");
                }
                else if (id.type == COMP_UEFA_CUP && id.index == 7 && comp->GetNumOfTeams() == 288) {
                    GenerateUEFALeaguePhaseMatches(L"Europa League", GetPool(FifamCompRegion::Europe, COMP_UEFA_CUP, 6), comp, 4, 8);
                    DumpPool(comp, L"Europa League generated matchdays");
                }
                else if (id.type == COMP_UEFA_CUP && id.index == 16 && comp->GetNumOfTeams() == 36) {
                    SortUEFALeaguePhaseTable(0xF90A0000, comp);
                    DumpPool(comp, L"Europa League League Phase sorted table");
                }
                else if (id.type == COMP_CONFERENCE_LEAGUE && id.index == 0 && comp->GetNumOfTeams() == 112 /*&& comp->GetNumOfRegisteredTeams() == 111*/) {
                    CTeamIndex liechtensteinCupWinner = CTeamIndex::null();
                    auto liechtensteinCup = GetCompetition(FifamCompRegion::Switzerland, FifamCompType::LeagueCup, 0);
                    if (liechtensteinCup)
                        liechtensteinCupWinner = liechtensteinCup->GetChampion();
                    else
                        Log(L"Conference League - Liechtenstein Cup is not available");
                    if (liechtensteinCupWinner.countryId == 0) { // no cup available - find all Liechtenstein clubs and select random
                        Vector<CTeamIndex> liechtensteinClubs;
                        CDBCountry* country = &GetCountryStore()->m_aCountries[FifamCompRegion::Switzerland];
                        if (country) {
                            for (Int t = 1; t <= country->GetLastTeamIndex(); t++) {
                                CTeamIndex teamIndex = CTeamIndex::make(FifamCompRegion::Switzerland, FifamClubTeamType::First, t);
                                if (IsLiechtensteinClubFromSwitzerland(teamIndex))
                                    liechtensteinClubs.push_back(teamIndex);
                            }
                        }
                        for (UInt i = 0; i < 5; i++) {
                            CTeamIndex teamIndex = CTeamIndex::make(FifamCompRegion::Liechtenstein, FifamClubTeamType::First, i + 1);
                            if (GetTeam(teamIndex))
                                liechtensteinClubs.push_back(teamIndex);
                        }
                        if (!liechtensteinClubs.empty()) {
                            if (liechtensteinClubs.size() == 1)
                                liechtensteinCupWinner = liechtensteinClubs[0];
                            else
                                liechtensteinCupWinner = liechtensteinClubs[Random::Get(0, liechtensteinClubs.size() - 1)];
                        }
                        DumpPool(comp, L"Conference League - Random club from Liechtenstein (list):");
                    }
                    Log(Utils::Format(L"Conference League - Liechtenstein Cup Winner: %s", TeamTagWithCountry(liechtensteinCupWinner)));
                    if (liechtensteinCupWinner.countryId != 0) {
                        if (!comp->IsTeamPresent(liechtensteinCupWinner)) {
                            auto compCL = GetCompetition(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 0);
                            if (!compCL || !compCL->IsTeamPresent(liechtensteinCupWinner)) {
                                auto compEL = GetCompetition(FifamCompRegion::Europe, FifamCompType::UefaCup, 0);
                                if (!compEL || !compEL->IsTeamPresent(liechtensteinCupWinner)) {
                                    Int liechtensteinPos = GetAssesmentTable()->GetCountryPositionLastYear(FifamCompRegion::Liechtenstein) + 1;
                                    UInt poolPosIndex = comp->GetNumOfTeams() - 1;
                                    if (liechtensteinPos >= 1 && liechtensteinPos <= 5)
                                        poolPosIndex = 4;
                                    else if (liechtensteinPos >= 6 && liechtensteinPos <= 38)
                                        poolPosIndex = 55;
                                    Log(Utils::Format(L"Conference League - inserting Liechtenstein Cup Winner at index %d. (AT pos %d)", poolPosIndex, liechtensteinPos));
                                    CTeamIndex* pTeamIDs = *raw_ptr<CTeamIndex*>(comp, 0xA0);
                                    DumpPool(comp, L"Conference League Pool");
                                    for (UInt i = comp->GetNumOfTeams() - 1; i > poolPosIndex; i--)
                                        pTeamIDs[i] = pTeamIDs[i - 1];
                                    pTeamIDs[poolPosIndex] = liechtensteinCupWinner;
                                    DumpPool(comp, L"Modified Conference League Pool");
                                }
                                else
                                    Log(L"Conference League - Liechtenstein Cup Winner already in Europa League pool");
                            }
                            else
                                Log(L"Conference League - Liechtenstein Cup Winner already in Champions League pool");
                        }
                        else
                            Log(L"Conference League - Liechtenstein Cup Winner already in Conference League pool ???");
                    }
                    comp->SetNumOfRegisteredTeams(comp->GetNumOfTeams());
                }
                else if (id.type == COMP_CONFERENCE_LEAGUE && rt == ROUND_QUALI) {
                    comp->SortTeams(Europe_ChampionsLeagueRoundSorter);
                    if (comp->GetNumOfTeams() == comp->GetNumOfRegisteredTeams()) {
                        comp->RandomlySortTeams(0, comp->GetNumOfRegisteredTeams() / 2);
                        comp->RandomlySortTeams(comp->GetNumOfRegisteredTeams() / 2);
                    }
                    else if (comp->GetNumOfRegisteredTeams() >= comp->GetNumOfTeams() / 2) {
                        UInt numTeamsToSort = comp->GetNumOfTeams() / 2 - (comp->GetNumOfTeams() - comp->GetNumOfRegisteredTeams());
                        CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
                        std::reverse(&pTeamIDs[0], &pTeamIDs[comp->GetNumOfTeams() / 2]);
                        if (numTeamsToSort != 0) {
                            comp->RandomlySortTeams(0, numTeamsToSort);
                            comp->RandomlySortTeams(comp->GetNumOfTeams() / 2, numTeamsToSort);
                        }
                    }
                    else
                        comp->RandomlySortTeams(0, comp->GetNumOfRegisteredTeams());
                }
                else if (id.type == COMP_CONFERENCE_LEAGUE && id.index == 10 && comp->GetNumOfTeams() == 36) {
                    comp->SortTeams(Europe_ChampionsLeagueRoundSorter);
                    comp->RandomlySortTeams(0, 12);
                    comp->RandomlySortTeams(12, 12);
                    comp->RandomlySortTeams(24, 12);
                    DumpPool(comp, L"Conference League teams sorted by pots");
                }
                else if (id.type == COMP_CONFERENCE_LEAGUE && id.index == 11 && comp->GetNumOfTeams() == 216) {
                    GenerateUEFALeaguePhaseMatches(L"Conference League", GetPool(FifamCompRegion::Europe, COMP_CONFERENCE_LEAGUE, 10), comp, 3, 6);
                    DumpPool(comp, L"Conference League generated matchdays");
                }
                else if (id.type == COMP_CONFERENCE_LEAGUE && id.index == 18 && comp->GetNumOfTeams() == 36) {
                    SortUEFALeaguePhaseTable(0xF9330000, comp);
                    DumpPool(comp, L"Conference League League Phase sorted table");
                }
                else if (id.type == COMP_YOUTH_CHAMPIONSLEAGUE && id.index == 0 && comp->GetNumOfTeams() == 52 && comp->GetNumOfRegisteredTeams() == 0) {
                    auto compCL = GetCompetition(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 8);
                    UInt teamCounter = 0;
                    CTeamIndex* pTeamIDs = *raw_ptr<CTeamIndex*>(comp, 0xA0);
                    auto AddTeam = [&pTeamIDs, &compCL, &teamCounter](CTeamIndex const &teamID, String const &name) {
                        if (!teamID.isNull()) {
                            CTeamIndex teamIDFirst = teamID.firstTeam();
                            if (!compCL || !compCL->IsTeamPresent(teamIDFirst)) {
                                pTeamIDs[teamCounter++] = teamIDFirst;
                                SafeLog::Write(Utils::Format(L"Youth League: added %s - %s", name, TeamName(teamIDFirst)));
                                return true;
                            }
                        }
                        return false;
                    };
                    for (UInt i = 1; i <= 55; i++) {
                        UChar countryId = GetAssesmentTable()->GetCountryIdAtPositionLastYear(i);
                        if (countryId == FifamCompRegion::Russia || countryId == FifamCompRegion::Liechtenstein)
                            continue;
                        Bool added = false;
                        CDBLeague *league = GetLeague(countryId, FifamCompType::League, 32); // Youth League A
                        if (league) {
                            added = AddTeam(league->GetChampion(), Utils::Format(L"%s youth league champion", CountryName(countryId)));
                            if (!added) {
                                for (UInt p = 0; p < league->GetNumOfTeams(); p++) {
                                    added = AddTeam(GetLeagueTeamAtPlace(league, p), Utils::Format(L"%s youth league place %u", CountryName(countryId), p + 1));
                                    if (added)
                                        break;
                                }
                            }
                        }
                        if (!added) {
                            league = GetLeague(countryId, FifamCompType::League, 0);
                            if (league) {
                                added = AddTeam(league->GetChampion(), Utils::Format(L"%s league champion", CountryName(countryId)));
                                if (!added) {
                                    for (UInt p = 0; p < league->GetNumOfTeams(); p++) {
                                        added = AddTeam(GetLeagueTeamAtPlace(league, p), Utils::Format(L"%s league place %u", CountryName(countryId), p + 1));
                                        if (added)
                                            break;
                                    }
                                }
                            }
                        }
                        if (teamCounter == comp->GetNumOfTeams())
                            break;
                    }
                    comp->SetNumOfRegisteredTeams(comp->GetNumOfTeams());
                    DumpPool(comp, L"Youth Champions League Pool National Leagues Path pool");
                }
                else if (id.type == COMP_YOUTH_CHAMPIONSLEAGUE && id.index == 5 && comp->GetNumOfTeams() == 216) {
                    CDBCompetition *clLeagueMatches = GetCompetition(FifamCompRegion::Europe, COMP_CHAMPIONSLEAGUE, 9);
                    if (clLeagueMatches) {
                        CTeamIndex *pDst = *raw_ptr<CTeamIndex *>(comp, 0xA0);
                        CTeamIndex *pSrc = *raw_ptr<CTeamIndex *>(clLeagueMatches, 0xA0);
                        for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
                            pDst[i] = pSrc[i];
                        comp->SetNumOfRegisteredTeams(comp->GetNumOfTeams());
                    }
                    DumpPool(comp, L"Youth League copied matchdays");
                }
                else if (id.type == COMP_YOUTH_CHAMPIONSLEAGUE && id.index == 12 && comp->GetNumOfTeams() == 36) {
                    SortUEFALeaguePhaseTable(0xF9260000, comp);
                    DumpPool(comp, L"Youth League League Phase sorted table");
                }
                else if (id.type == COMP_WORLD_CLUB_CHAMP && id.index == 0 && comp->GetNumOfTeams() == 1) {
                    SelectWCCHostTeam(comp);
                    DumpPool(comp, L"FIFA Club World Cup host team");
                }
                else if (id.type == COMP_WORLD_CLUB_CHAMP && id.index == 1 && comp->GetNumOfTeams() == 16) {
                    SelectWCCHostStadiums(comp);
                    DumpPool(comp, L"FIFA Club World Cup host stadiums");
                }
                else if (id.type == COMP_WORLD_CLUB_CHAMP && id.index == 2 && comp->GetNumOfTeams() == 32) {
                    CDBPool *hostStadiumsPool = GetPool(FifamCompRegion::Europe, COMP_WORLD_CLUB_CHAMP, 1);
                    if (hostStadiumsPool && hostStadiumsPool->GetNumOfRegisteredTeams() == 0)
                        SelectWCCHostStadiums(hostStadiumsPool);
                    // Call<0x121B350>(L"Tournaments.txt"); // test - read Tournaments.txt before competition is processed
                    auto AddCompFinalist = [&comp](UChar region, UChar type, UInt year, Bool runnerUp) {
                        CTeamIndex winner = GetCompFinalist(region, type, year, runnerUp);
                        if (!winner.isNull() && comp->AddTeam(winner)) {
                            SafeLog::Write(Utils::Format(L"FIFA Club World Cup: Added %s of %s (year %u): %s",
                                runnerUp ? L"runner-up" : L"winner",
                                CompetitionTag(GetRoundByRoundType(region, type, ROUND_FINAL)), year, TeamName(winner)));
                            return true;
                        }
                        return false;
                    };
                    auto AddBestClubsFromContinent = [&comp](UChar continent, UInt count) {
                        if (count == 0)
                            return;
                        Vector<Pair<UInt, CDBTeam *>> teams;
                        for (UInt c = 1; c <= 207; c++) {
                            CDBCountry *country = &GetCountryStore()->m_aCountries[c];
                            if (country->GetContinent() == continent) {
                                for (Int t = 1; t <= country->GetLastTeamIndex(); t++) {
                                    CTeamIndex teamID = CTeamIndex::make(c, FifamClubTeamType::First, t);
                                    auto team = GetTeam(teamID);
                                    if (team) {
                                        teams.emplace_back(
                                            (team->GetInternationalPrestige() << 24) | (team->GetNationalPrestige() << 16) | CRandom::GetRandomInt(32767),
                                            team);
                                    }
                                }
                            }
                        }
                        if (teams.size() > 1) {
                            Utils::Sort(teams, [](Pair<UInt, CDBTeam *> const &a, Pair<UInt, CDBTeam *> const &b) {
                                return a.first >= b.first;
                            });
                        }
                        UInt numAddedTeams = 0;
                        for (UInt t = 0; t < teams.size(); t++) {
                            if (comp->AddTeam(teams[t].second->GetTeamID())) {
                                String continentName;
                                CDBCountry *teamCountry = teams[t].second->GetCountry();
                                if (teamCountry)
                                    continentName = teamCountry->GetContinentName();
                                SafeLog::Write(Utils::Format(L"FIFA Club World Cup: Added best team from %s - %s (IP: %u, NP: %u)",
                                    continentName, TeamName(teams[t].second), teams[t].second->GetInternationalPrestige(),
                                    teams[t].second->GetNationalPrestige()));
                                numAddedTeams++;
                                if (numAddedTeams == count)
                                    return;
                            }
                        }
                    };
                    const UInt MAX_TEAMS_AFC = 4;
                    const UInt MAX_TEAMS_CAF = 4;
                    const UInt MAX_TEAMS_CONCACAF = 4;
                    const UInt MAX_TEAMS_CONMEBOL = 6;
                    const UInt MAX_TEAMS_UEFA = 12;
                    const UInt MAX_TEAMS_OFC = 1;
                    UChar numAFC = 0, numCAF = 0, numCONCACAF = 0, numCONMEBOL = 0, numUEFA = 0, numOFC = 0;
                    for (UInt i = 0; i < 2; i++) {
                        for (UShort y = 0; y < 4; y++) {
                            if (numAFC < MAX_TEAMS_AFC)
                                numAFC += AddCompFinalist(FifamCompRegion::Asia, COMP_CHAMPIONSLEAGUE, GetCurrentYear() - y, i);
                            if (numCAF < MAX_TEAMS_CAF)
                                numCAF += AddCompFinalist(FifamCompRegion::Africa, COMP_CHAMPIONSLEAGUE, GetCurrentYear() - y, i);
                            if (numCONCACAF < MAX_TEAMS_CONCACAF)
                                numCONCACAF += AddCompFinalist(FifamCompRegion::NorthAmerica, COMP_CHAMPIONSLEAGUE, GetCurrentYear() - y, i);
                            if (numCONMEBOL < MAX_TEAMS_CONMEBOL)
                                numCONMEBOL += AddCompFinalist(FifamCompRegion::SouthAmerica, COMP_CHAMPIONSLEAGUE, GetCurrentYear() - y, i);
                            if (numUEFA < MAX_TEAMS_UEFA)
                                numUEFA += AddCompFinalist(FifamCompRegion::Europe, COMP_CHAMPIONSLEAGUE, GetCurrentYear() - y, i);
                        }
                    }
                    {
                        Vector<Pair<UInt, CDBTeam *>> ofcWinners;
                        Set<CDBTeam *> ofcWinnersSet;
                        for (UShort y = 0; y < 4; y++) {
                            CTeamIndex winner = GetCompFinalist(FifamCompRegion::Oceania, COMP_CHAMPIONSLEAGUE, GetCurrentYear() - y, false);
                            if (!winner.isNull()) {
                                auto team = GetTeam(winner);
                                if (team && !Utils::Contains(ofcWinnersSet, team)) {
                                    ofcWinners.emplace_back(
                                        (team->GetInternationalPrestige() << 24) | (team->GetNationalPrestige() << 16) | CRandom::GetRandomInt(32767),
                                        team);
                                    ofcWinnersSet.insert(team);
                                }
                            }
                        }
                        if (!ofcWinners.empty()) {
                            if (ofcWinners.size() > 1) {
                                Utils::Sort(ofcWinners, [](Pair<UInt, CDBTeam *> const &a, Pair<UInt, CDBTeam *> const &b) {
                                    return a.first >= b.first;
                                });
                            }
                            for (UInt o = 0; o < ofcWinners.size(); o++) {
                                if (comp->AddTeam(ofcWinners[o].second->GetTeamID())) {
                                    numOFC += 1;
                                    SafeLog::Write(Utils::Format(L"FIFA Club World Cup: Added best winner of OFC Champions League - %s (IP: %u, NP: %u)",
                                        TeamName(ofcWinners[o].second), ofcWinners[o].second->GetInternationalPrestige(),
                                        ofcWinners[o].second->GetNationalPrestige()));
                                    if (numOFC >= MAX_TEAMS_OFC)
                                        break;
                                }
                            }
                        }
                    }
                    if (numAFC < MAX_TEAMS_AFC)
                        AddBestClubsFromContinent(FifamContinent::Asia, MAX_TEAMS_AFC - numAFC);
                    if (numCAF < MAX_TEAMS_CAF)
                        AddBestClubsFromContinent(FifamContinent::Africa, MAX_TEAMS_CAF - numCAF);
                    if (numCONCACAF < MAX_TEAMS_CONCACAF)
                        AddBestClubsFromContinent(FifamContinent::NorthAmerica, MAX_TEAMS_CONCACAF - numCONCACAF);
                    if (numCONMEBOL < MAX_TEAMS_CONMEBOL)
                        AddBestClubsFromContinent(FifamContinent::SouthAmerica, MAX_TEAMS_CONMEBOL - numCONMEBOL);
                    if (numUEFA < MAX_TEAMS_UEFA)
                        AddBestClubsFromContinent(FifamContinent::Europe, MAX_TEAMS_UEFA - numUEFA);
                    if (numOFC < MAX_TEAMS_OFC)
                        AddBestClubsFromContinent(FifamContinent::Oceania, MAX_TEAMS_OFC - numOFC);
                    DumpPool(comp, L"FIFA Club World Cup participants");
                }
                else if (id.type == COMP_TOYOTA && id.index == 0 && comp->GetNumOfTeams() == 6) {
                    Bool doSwap = true;
                    if (GetCurrentYear() == GetStartingYear()) {
                        if ((GetCurrentYear() & 2) == 1) // do not swap if the game starts in odd year
                            doSwap = false;
                    }
                    if (doSwap) {
                        CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
                        // team 3. is AFC winner
                        // team 4. is CAF winner
                        std::swap(pTeamIDs[3], pTeamIDs[4]);
                    }
                }
            }
            return;
        }
        else if (id.countryId == FifamCompRegion::SouthAmerica
            || id.countryId == FifamCompRegion::NorthAmerica
            || id.countryId == FifamCompRegion::Asia
            || id.countryId == FifamCompRegion::Africa
            || id.countryId == FifamCompRegion::Oceania)
        {
            if (comp->GetDbType() == DB_ROUND) {
                if (id.countryId == FifamCompRegion::SouthAmerica && (id.type == COMP_CHAMPIONSLEAGUE || id.type == COMP_UEFA_CUP) && rt == ROUND_LAST_16)
                    comp->RandomizePairs();
                else if (id.countryId == FifamCompRegion::SouthAmerica && id.type == COMP_UEFA_CUP && rt == ROUND_QUALI && comp->GetNumOfTeams() == 32 && comp->GetNumOfTeams() == comp->GetNumOfRegisteredTeams()) {
                    comp->RandomlySortTeams(0, 4);
                    comp->RandomlySortTeams(4, 4);
                    comp->RandomlySortTeams(8, 4);
                    comp->RandomlySortTeams(12, 4);
                    comp->RandomlySortTeams(16, 4);
                    comp->RandomlySortTeams(20, 4);
                    comp->RandomlySortTeams(24, 4);
                    comp->RandomlySortTeams(28, 4);
                    Array<CTeamIndex, 32> teams;
                    for (UInt i = 0; i < std::size(teams); i++)
                        ClearID(teams[i]);
                    CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
                    for (UInt i = 0; i < 8; i++) {
                        teams[i * 2] = pTeamIDs[i * 4];
                        teams[i * 2 + 1] = pTeamIDs[i * 4 + 1];
                        teams[i * 2 + 16] = pTeamIDs[i * 4 + 2];
                        teams[i * 2 + 1 + 16] = pTeamIDs[i * 4 + 3];
                    }
                    for (UInt i = 0; i < 32; i++)
                        pTeamIDs[i] = teams[i];
                }
                else if (id.countryId == FifamCompRegion::NorthAmerica && id.type == COMP_CONTINENTAL_2 && rt == ROUND_LAST_16) { // leagues cup last16
                    if (!CDBGame::GetInstance()->IsCountryPlayable(FifamCompRegion::Mexico)
                        || !CDBGame::GetInstance()->IsCountryPlayable(FifamCompRegion::United_States))
                    {
                        comp->AddTeamsFromCountry(FifamCompRegion::Mexico, 8);
                        comp->AddTeamsFromCountry(FifamCompRegion::United_States, 8);
                        comp->SortTeams(ChampionsLeagueRoundSorter);
                        comp->RandomizePairs();
                    }
                }
                else if (id.countryId == FifamCompRegion::Africa && id.type == COMP_UEFA_CUP && rt == ROUND_QUALI3) // playoff round
                    comp->RandomizePairs();
                else if (id.countryId == FifamCompRegion::Africa && id.type == COMP_CHAMPIONSLEAGUE && rt == ROUND_QUARTERFINAL) // quarterfinal
                    comp->RandomizePairs();
                else if (id.countryId == FifamCompRegion::Africa && id.type == COMP_UEFA_CUP && rt == ROUND_QUARTERFINAL) // quarterfinal
                    comp->RandomizePairs();
                else {
                    comp->SortTeams(ChampionsLeagueRoundSorter);
                    comp->RandomizePairs();
                }
            }
            else if (comp->GetDbType() == DB_POOL) {
                if (id.countryId == FifamCompRegion::Asia) {
                    if (id.type == COMP_CHAMPIONSLEAGUE && id.index == 0 && comp->GetNumOfTeams() == 58 && comp->GetNumOfRegisteredTeams() == 54) {
                        CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
                        if (DUMP_TO_LOG) {
                            SafeLog::Write(L"AFC Champions League Pool");
                            for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
                                SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamNameWithCountry(pTeamIDs[i])));
                        }
                        for (UInt i = 56; i > 48; i--)
                            pTeamIDs[i - 1] = pTeamIDs[i - 3];
                        if (DUMP_TO_LOG) {
                            SafeLog::Write(L"AFC Champions League Pool - after shifting");
                            for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
                                SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamNameWithCountry(pTeamIDs[i])));
                        }
                        pTeamIDs[47 - 1] = CTeamIndex::make(0, 0, 0);
                        pTeamIDs[48 - 1] = CTeamIndex::make(0, 0, 0);
                        pTeamIDs[57 - 1] = CTeamIndex::make(0, 0, 0);
                        pTeamIDs[58 - 1] = CTeamIndex::make(0, 0, 0);
                        if (DUMP_TO_LOG) {
                            SafeLog::Write(L"AFC Champions League Pool - set null team IDs");
                            for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
                                SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamNameWithCountry(pTeamIDs[i])));
                        }
                        CTeamIndex winnerACL = CTeamIndex::make(0, 0, 0);
                        CTeamIndex winnerACC = CTeamIndex::make(0, 0, 0);
                        auto finalACL = GetRoundByRoundType(FifamCompRegion::Asia, FifamCompType::ChampionsLeague, FifamRoundID::Final);
                        if (finalACL) {
                            auto winner = finalACL->GetChampion();
                            if (winner.countryId != 0 && !comp->IsTeamPresent(winner))
                                winnerACL = winner;
                        }
                        auto finalACC = GetRoundByRoundType(FifamCompRegion::Asia, FifamCompType::UefaCup, FifamRoundID::Final);
                        if (finalACC) {
                            auto winner = finalACC->GetChampion();
                            if (winner.countryId != 0 && !comp->IsTeamPresent(winner))
                                winnerACC = winner;
                        }
                        if (DUMP_TO_LOG) {
                            SafeLog::Write(L"AFC Champions League Pool - ACL winner: " + TeamNameWithCountry(winnerACL));
                            SafeLog::Write(L"AFC Champions League Pool - ACC winner: " + TeamNameWithCountry(winnerACC));
                        }
                        if (winnerACL.countryId != 0 || winnerACC.countryId != 0) {
                            Function<Int(UChar *, UInt, UChar)> GetIndexInArray = [](UChar *arr, UInt maxSize, UChar countryId) -> Int {
                                for (UInt ia = 0; ia < maxSize; ia++) {
                                    if (arr[ia] == countryId)
                                        return ia;
                                }
                                return -1;
                            };
                            auto FindSpotsForCountry = [&](UChar countryId, UInt numSlots) {
                                static UChar countries_W[] = {
                                    FifamCompRegion::Qatar,
                                    FifamCompRegion::Saudi_Arabia,
                                    FifamCompRegion::Iran,
                                    FifamCompRegion::United_Arab_Emirates,
                                    FifamCompRegion::Iraq,
                                    FifamCompRegion::Uzbekistan,
                                    FifamCompRegion::Jordan,
                                    FifamCompRegion::India,
                                    FifamCompRegion::Tajikistan,
                                    FifamCompRegion::Turkmenistan,
                                    FifamCompRegion::Lebanon,
                                    FifamCompRegion::Syria,
                                    FifamCompRegion::Bahrain,
                                    FifamCompRegion::Bangladesh,
                                    FifamCompRegion::Maldives,
                                    FifamCompRegion::Oman,
                                    FifamCompRegion::Palestinian_Authority,
                                    FifamCompRegion::Kyrgyzstan,
                                    FifamCompRegion::Kuwait,
                                    FifamCompRegion::Nepal,
                                    FifamCompRegion::Sri_Lanka,
                                    FifamCompRegion::Bhutan,
                                    FifamCompRegion::Afghanistan,
                                    FifamCompRegion::Yemen,
                                    FifamCompRegion::Pakistan
                                };
                                static UChar countries_E[] = {
                                    FifamCompRegion::China_PR,
                                    FifamCompRegion::Japan,
                                    FifamCompRegion::Korea_Republic,
                                    FifamCompRegion::Thailand,
                                    FifamCompRegion::Australia,
                                    FifamCompRegion::Philippines,
                                    FifamCompRegion::Korea_DPR,
                                    FifamCompRegion::Vietnam,
                                    FifamCompRegion::Malaysia,
                                    FifamCompRegion::Singapore,
                                    FifamCompRegion::Hong_Kong,
                                    FifamCompRegion::Myanmar,
                                    FifamCompRegion::Indonesia,
                                    FifamCompRegion::Cambodia,
                                    FifamCompRegion::Macao,
                                    FifamCompRegion::Laos,
                                    FifamCompRegion::Taiwan,
                                    FifamCompRegion::Mongolia,
                                    FifamCompRegion::Brunei_Darussalam,
                                    FifamCompRegion::Guam
                                };
                                static Pair<UInt, UInt> slotPositionsW[7] = {
                                    { 3, 33 },
                                    { 6, 34 },
                                    { 35, 39 },
                                    { 40, 41 },
                                    { 42, 43 },
                                    { 44, 47 },
                                    { 47, 48 }
                                };
                                static Pair<UInt, UInt> slotPositionsE[7] = {
                                    { 19, 36 },
                                    { 22, 37 },
                                    { 38, 49 },
                                    { 50, 51 },
                                    { 52, 53 },
                                    { 54, 57 },
                                    { 57, 58 }
                                };
                                bool east = false;
                                Int countryIndex = GetIndexInArray(countries_W, std::size(countries_W), countryId);
                                if (countryIndex == -1) {
                                    countryIndex = GetIndexInArray(countries_E, std::size(countries_E), countryId);
                                    if (countryIndex != -1)
                                        east = true;
                                    else
                                        countryIndex = 6;
                                }
                                if (countryIndex > 6)
                                    countryIndex = 6;
                                auto resultSlots = east ? slotPositionsE[countryIndex] : slotPositionsW[countryIndex];
                                if (numSlots == 1) {
                                    if (countryIndex != 6)
                                        resultSlots.first = resultSlots.second;
                                    resultSlots.second = 0;
                                }
                                if (DUMP_TO_LOG) {
                                    SafeLog::Write(Format(L"AFC Champions League Pool - FindSpotsForCountry: country %s numSlots %d",
                                        GetCountryStore()->m_aCountries[countryId].GetName(), numSlots));
                                    for (UInt slot = 0; slot < numSlots; slot++) {
                                        SafeLog::Write(Format(L"AFC Champions League Pool - FindSpotsForCountry: slot %d. region %s index %d",
                                            slot + 1, east? L"East" : L"West", countryIndex));
                                    }
                                }
                                return resultSlots;
                            };
                            if (winnerACL.countryId != 0 && winnerACC.countryId != 0) {
                                Pair<UInt, UInt> slots;
                                if (winnerACL.countryId == winnerACC.countryId) {
                                    slots = FindSpotsForCountry(winnerACL.countryId, 2);
                                    if (DUMP_TO_LOG) {
                                        SafeLog::Write(L"AFC Champions League Pool - 2 team from same country to add:");
                                    }
                                }
                                else {
                                    slots.first = FindSpotsForCountry(winnerACL.countryId, 1).first;
                                    slots.second = FindSpotsForCountry(winnerACC.countryId, 1).first;
                                    if (DUMP_TO_LOG) {
                                        SafeLog::Write(L"AFC Champions League Pool - 2 team to add:");
                                    }
                                }
                                if (DUMP_TO_LOG) {
                                    SafeLog::Write(TeamName(winnerACL) + L") at " + std::to_wstring(slots.first));
                                    SafeLog::Write(TeamName(winnerACC) + L") at " + std::to_wstring(slots.second));
                                }
                                if (slots.first != 0)
                                    pTeamIDs[slots.first - 1] = winnerACL;
                                if (slots.second != 0)
                                    pTeamIDs[slots.first - 1] = winnerACC;
                            }
                            else {
                                CTeamIndex winner = winnerACL;
                                UChar slotCountryId = winnerACL.countryId;
                                if (slotCountryId == 0) {
                                    slotCountryId = winnerACC.countryId;
                                    winner = winnerACC;
                                }
                                auto slots = FindSpotsForCountry(slotCountryId, 1);
                                if (DUMP_TO_LOG) {
                                    SafeLog::Write(L"AFC Champions League Pool - 1 team to add:");
                                    SafeLog::Write(TeamName(winner) + L") at " + std::to_wstring(slots.first));
                                }
                                if (slots.first != 0)
                                    pTeamIDs[slots.first - 1] = winner;
                            }
                        }
                        *raw_ptr<UInt>(comp, 0xA4) = 58;
                        if (DUMP_TO_LOG) {
                            SafeLog::Write(L"AFC Champions League Pool - modified");
                            for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
                                SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), TeamNameWithCountry(pTeamIDs[i])));
                        }
                    }
                }
            }
            return;
        }
        else if (id.countryId == FifamCompRegion::International) {
            if (comp->GetDbType() == DB_ROUND) {
                comp->SortTeamIDs(NationalTeamSorter);
                comp->RandomizePairs();
            }
            return;
        }
        else if (id.countryId == FifamCompRegion::Germany) {
            if (comp->GetDbType() == DB_ROUND && id.type == COMP_RELEGATION)
                AddGermanyRegionalligaPromotionTeams(comp, true);
            else if (comp->GetDbType() == DB_POOL && id.index == 2)
                AddGermanyRegionalligaPromotionTeams(comp, false);
            return;
        }
        else if (id.countryId == FifamCompRegion::Switzerland) {
            if (id.type == COMP_LE_CUP && id.index == 0 && comp->GetNumOfTeams() == 14 && comp->GetNumOfRegisteredTeams() == 0) {
                Vector<CTeamIndex> liechtensteinClubs;
                CDBCountry* country = &GetCountryStore()->m_aCountries[FifamCompRegion::Switzerland];
                if (country) {
                    for (Int t = 1; t <= country->GetLastTeamIndex(); t++) {
                        CTeamIndex teamIndex = CTeamIndex::make(FifamCompRegion::Switzerland, FifamClubTeamType::First, t);
                        if (IsLiechtensteinClubFromSwitzerland(teamIndex))
                            liechtensteinClubs.push_back(teamIndex);
                    }
                }
                for (UInt i = 0; i < 5; i++) {
                    CTeamIndex teamIndex = CTeamIndex::make(FifamCompRegion::Liechtenstein, FifamClubTeamType::First, i + 1);
                    if (GetTeam(teamIndex))
                        liechtensteinClubs.push_back(teamIndex);
                }
                for (CTeamIndex teamIndex : liechtensteinClubs) {
                    teamIndex.type = FifamClubTeamType::Reserve;
                    liechtensteinClubs.push_back(teamIndex);
                }
                if (liechtensteinClubs.size() > comp->GetNumOfTeams())
                    liechtensteinClubs.resize(comp->GetNumOfTeams());
                CTeamIndex* pTeamIDs = *raw_ptr<CTeamIndex*>(comp, 0xA0);
                for (UInt i = 0; i < liechtensteinClubs.size(); i++)
                    pTeamIDs[i] = liechtensteinClubs[i];
                comp->SetNumOfRegisteredTeams(liechtensteinClubs.size());
                if (DUMP_TO_LOG) {
                    SafeLog::Write(L"Liechtenstein Cup Modified");
                    for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
                        SafeLog::Write(Utils::Format(L"%2d. %s", i + 1, TeamTagWithCountry(liechtensteinClubs[i])));
                }
            }
        }
        else if (id.countryId == FifamCompRegion::Canada) {
            if (id.type == COMP_FA_CUP && id.index == 0 && comp->GetNumOfTeams() == 14) {
                Vector<CTeamIndex> canadianClubs;
                if (GetStartingYear() == GetCurrentYear() && GetCurrentYear() == 2023) { // UPDATE: every season
                    static const UInt CanadianChampionshipByeTeams[] = { 0x5F0022, 0x5F000D }; // Toronto FC, Vancouver Whitecaps FC
                    for (UInt clubUID : CanadianChampionshipByeTeams) {
                        CDBTeam *team = GetTeamByUniqueID(clubUID);
                        if (team && !Utils::Contains(canadianClubs, team->GetTeamID()))
                            canadianClubs.push_back(team->GetTeamID());
                    }
                }
                else {
                    CTeamIndex canadaChampion = comp->GetChampion();
                    if (!canadaChampion.isNull())
                        canadianClubs.push_back(canadaChampion);
                    CTeamIndex canadaRunnerUp = comp->GetRunnerUp();
                    if (!canadaRunnerUp.isNull() && !Utils::Contains(canadianClubs, canadaRunnerUp))
                        canadianClubs.push_back(canadaRunnerUp);
                }
                static const UInt MLSCanadianClubs[] = {0x5F0022, 0x5F000D, 0x5F340C}; // Toronto FC, Vancouver Whitecaps FC, CF Montreal
                for (UInt clubUID : MLSCanadianClubs) {
                    CDBTeam *team = GetTeamByUniqueID(clubUID);
                    if (team && !Utils::Contains(canadianClubs, team->GetTeamID()))
                        canadianClubs.push_back(team->GetTeamID());
                }
                for (UInt i = 0; i < comp->GetNumOfRegisteredTeams(); i++) {
                    CTeamIndex teamId = comp->GetTeamID(i);
                    if (!teamId.isNull() && !Utils::Contains(canadianClubs, teamId))
                        canadianClubs.push_back(teamId);
                }
                if (canadianClubs.size() > comp->GetNumOfTeams())
                    canadianClubs.resize(comp->GetNumOfTeams());
                CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
                for (UInt i = 0; i < canadianClubs.size(); i++)
                    pTeamIDs[i] = canadianClubs[i];
                comp->SetNumOfRegisteredTeams(canadianClubs.size());
                if (DUMP_TO_LOG) {
                    SafeLog::Write(L"Canadian Championship Modified");
                    for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
                        SafeLog::Write(Utils::Format(L"%2d. %s", i + 1, TeamTagWithCountry(canadianClubs[i])));
                }
            }
        }
        Call<0x139D9A0>(ppComp);
    }
}

//unsigned char gCompTypesForInternationalCalendar[] = {};

void METHOD OnAddMyCareerTrophyNationalTeam(void *vec, DUMMY_ARG, void *pValue) {
    CallMethod<0x416950>(vec, pValue);
    UChar compType = COMP_EURO_NL;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_NAM_NL;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_NAM_CUP;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_ASIA_CUP;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_AFRICA_CUP;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_OFC_CUP;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_FINALISSIMA;
    CallMethod<0x416950>(vec, &compType);
}

void METHOD OnAddMyCareerTrophyContinental(void *vec, DUMMY_ARG, void *pValue) {
    CallMethod<0x416950>(vec, pValue);
    UChar compType = COMP_CONFERENCE_LEAGUE;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_CONTINENTAL_1;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_CONTINENTAL_2;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_UIC;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_ICC;
    CallMethod<0x416950>(vec, &compType);
}

UChar GetCompetitionGroupByCompetitionId(UInt competitionId) {
    CDBLeague *league = nullptr;
    switch ((competitionId >> 16) & 0xFF) {
    case COMP_LEAGUE:
        return 1;
    case COMP_FA_CUP:
    case COMP_LE_CUP:
        return 2;
    case COMP_RELEGATION:
        league = GetLeague(competitionId);
        if (league && league->GetNumMatchdays() >= 4)
            return 1;
        break;
    case COMP_CHAMPIONSLEAGUE:
    case COMP_UEFA_CUP:
    case COMP_CONFERENCE_LEAGUE:
    case COMP_TOYOTA:
    case COMP_UIC:
    case COMP_EURO_SUPERCUP:
    case COMP_WORLD_CLUB_CHAMP:
    case COMP_CONTINENTAL_1:
    case COMP_CONTINENTAL_2:
        return 3;
    case COMP_QUALI_WC:
    case COMP_QUALI_EC:
    case COMP_WORLD_CUP:
    case COMP_EURO_CUP:
    case COMP_INTERNATIONAL_FRIENDLY:
    case COMP_U20_WORLD_CUP:
    case COMP_U20_WC_Q:
    case COMP_CONFED_CUP:
    case COMP_COPA_AMERICA:
    case COMP_EURO_NL_Q:
    case COMP_EURO_NL:
    case COMP_NAM_NL:
    case COMP_NAM_NL_Q:
    case COMP_NAM_CUP:
    case COMP_ASIA_CUP:
    case COMP_ASIA_CUP_Q:
    case COMP_AFRICA_CUP:
    case COMP_AFRICA_CUP_Q:
    case COMP_OFC_CUP:
    case COMP_OFC_CUP_Q:
    case COMP_FINALISSIMA:
        return 4;
    case COMP_FRIENDLY:
    case COMP_ICC:
    case COMP_UNKNOWN_27:
    case COMP_RESERVE:
        return 5;
    }
    return 0;
}

UChar METHOD GetCompetitionTypeForNationalTeamHost(CDBCompetition *comp) {
    UChar compType = *raw_ptr<UChar>(comp, 0x1A);
    if (compType == COMP_EURO_NL || compType == COMP_NAM_NL || compType == COMP_NAM_CUP || compType == COMP_ASIA_CUP || compType == COMP_AFRICA_CUP
        || compType == COMP_OFC_CUP || compType == COMP_U17_WORLD_CUP || compType == COMP_U21_EC || compType == COMP_U19_EC || compType == COMP_U17_EC
        || compType == COMP_OLYMPIC || compType == COMP_OFC_CUP_Q || compType == COMP_FINALISSIMA)
    {
        return COMP_WORLD_CUP;
    }
    return compType;
}

Char const *GetNationalTeamECQualifiedTeamsScreenName() {
    if (GetCurrentYear() == 2024)
        return "Screens/10NationalTeamECQualifiedTeams_2024_FF12.xml";
    return "Screens/10NationalTeamECQualifiedTeams.xml";
}

Char const *GetNationalTeamWCQualifiedTeams2ScreenName() {
    return "Screens/10NationalTeamWCQualifiedTeams2.xml";
}

Char const *GetNationalTeamWCQualifiedTeams4ScreenName() {
    return "Screens/10NationalTeamWCQualifiedTeams4.xml";
}

Char const *GetNationalTeamTournamentWelcomeScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010TournamentWelcome.xml";
    if (game->TestFlag(2)) {
        return "Screens/10NationalTeamWCTournamentWelcome.xml";
    }
    else {
        //if (game->TestFlag(4);
        if (GetCurrentYear() == 2024)
            return "Screens/10NationalTeamECTournamentWelcome_2024_FF12.xml";
    }
    return "Screens/10NationalTeamECTournamentWelcome.xml";
}

Char const *GetNationalTeamTournamentGroupsScreenName() {
    if (*(UChar *)0x3111C74 == 2) {
        if (GetCurrentYear() == 2024)
            return "Screens/11NationalTeamECTournamentGroups_2024_FF12.xml";
        return "Screens/11NationalTeamECTournamentGroups.xml";
    }
    return "Screens/11NationalTeamWCTournamentGroups.xml";
}

Char const *GetNationalTeamECTournamentFinalsScreenName() {
    if (GetCurrentYear() == 2024)
        return "Screens/10NationalTeamECTournamentFinals_2024_FF12.xml";
    return "Screens/10NationalTeamECTournamentFinals.xml";
}

Char const *GetNationalTeamWCTournamentFinalsScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010TournamentFinals.xml";
    if (GetCurrentYear() & 1)
        return "Screens/10NationalTeamU20TournamentFinals.xml";
    return "Screens/10NationalTeamWCTournamentFinals.xml";
}

Char const *GetNationalTeamTournamentReportScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010TournamentReport.xml";
    if (game->TestFlag(4) || !game->TestFlag(2)) {
        if (GetCurrentYear() == 2024)
            return "Screens/10NationalTeamECTournamentReport_2024_FF12.xml";
        return "Screens/10NationalTeamECTournamentReport.xml";
    }
    return "Screens/10NationalTeamWCTournamentReport.xml";
}

Char const *GetNationalTeamTournamentDreamTeamScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010DreamTeam.xml";
    if (game->TestFlag(4) || !game->TestFlag(2)) {
        if (GetCurrentYear() == 2024)
            return "Screens/10NationalTeamECTournamentDreamTeam_2024_FF12.xml";
        return "Screens/10NationalTeamECTournamentDreamTeam.xml";
    }
    return "Screens/10NationalTeamWCTournamentDreamTeam.xml";
}

Char const *GetNationalTeamTournamentECUpsAndDownsScreenName() {
    if (GetCurrentYear() == 2024)
        return "Screens/13NationalTeamECUpsAndDowns_2024_FF12.xml";
    return "Screens/13NationalTeamECUpsAndDowns.xml";
}

Char const *GetNationalTeamTournamentWCUpsAndDownsScreenName() {
    return "Screens/13NationalTeamWCUpsAndDowns.xml";
}

CDBRound *GetECLast16Round(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_EURO_CUP, 12); // Last16
    if (!round) {
        round = GetRound(255, COMP_EURO_CUP, 15);
        if (!round) {
            round = GetRoundByRoundType(255, COMP_EURO_CUP, 13); // Quarterfinal
            if (!round)
                round = GetRound(255, COMP_EURO_CUP, 10);
        }
    }
    return round;
}

CDBRound *GetECQuarterfinal(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_EURO_CUP, 13);
    if (!round) {
        round = GetRound(255, COMP_EURO_CUP, 16);
        if (!round)
            round = GetRound(255, COMP_EURO_CUP, 10);
    }
    return round;
}

CDBRound *GetECSemifinal(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_EURO_CUP, 14);
    if (!round) {
        round = GetRound(255, COMP_EURO_CUP, 17);
        if (!round)
            round = GetRound(255, COMP_EURO_CUP, 11);
    }
    return round;
}

CDBRound *GetECFinal(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_EURO_CUP, 15);
    if (!round) {
        round = GetRound(255, COMP_EURO_CUP, 18);
        if (!round)
            round = GetRound(255, COMP_EURO_CUP, 12);
    }
    return round;
}

CDBRound *GetECFinalForSimulation(Int, Int, Int) {
    UInt region = FifamCompRegion::Europe;
    GetFirstManagerRegion(region);
    UInt compType = COMP_EURO_CUP;
    if (region == FifamCompRegion::SouthAmerica)
        compType = COMP_COPA_AMERICA;
    else if (region == FifamCompRegion::NorthAmerica)
        compType = COMP_NAM_CUP;
    else if (region == FifamCompRegion::Africa)
        compType = COMP_AFRICA_CUP;
    else if (region == FifamCompRegion::Asia)
        compType = COMP_ASIA_CUP;
    else if (region == FifamCompRegion::Oceania)
        compType = COMP_OFC_CUP;
    return GetRoundByRoundType(255, compType, ROUND_FINAL);
}

CDBCompetition *GetECPoolForSimulation(Int, Int, Int) {
    UInt region = FifamCompRegion::Europe;
    GetFirstManagerRegion(region);
    UInt compType = COMP_EURO_CUP;
    if (region == FifamCompRegion::SouthAmerica)
        compType = COMP_COPA_AMERICA;
    else if (region == FifamCompRegion::NorthAmerica)
        compType = COMP_NAM_CUP;
    else if (region == FifamCompRegion::Africa)
        compType = COMP_AFRICA_CUP;
    else if (region == FifamCompRegion::Asia)
        compType = COMP_ASIA_CUP;
    else if (region == FifamCompRegion::Oceania)
        compType = COMP_OFC_CUP;
    return GetCompetition(255, compType, 0);
}

CDBRound *GetWCLast32Round(Int, Int, Int) {
    CDBRound *round = GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_3);
    if (!round) {
        round = GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_LAST_16);
        if (!round) {
            round = GetRound(255, COMP_WORLD_CUP, 14);
            if (!round)
                round = GetRound(255, COMP_WORLD_CUP, 9);
        }
    }
    return round;
}

CDBRound *GetWCFinal(Int, Int, Int) {
    return GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_FINAL);
}

CDBRound *GetCompetitionFinalForSimulationScreen(UInt region, UInt type, UInt index) {
    GetFirstManagerRegion(region);
    return GetRoundByRoundType(region, type, ROUND_FINAL);
}

CDBCompetition *GetCompetitionPoolForSimulationScreen(UInt region, UInt type, UInt index) {
    GetFirstManagerRegion(region);
    return GetCompetition(region, type, 0);
}

void *gSimulationLbChampions = nullptr;

void METHOD OnStoreSimulationLbChampions(void *t) {
    CallMethod<0xD1AF40>(t);
    gSimulationLbChampions = t;
}

CDBCompetition *GetCompetitionPoolForSimulationScreen_Supercup(UInt region, UInt type, UInt index) {
    GetFirstManagerRegion(region);
    auto confPool = GetCompetition(region, COMP_CONFERENCE_LEAGUE, 0);
    if (confPool) {
        auto confFinal = GetRoundByRoundType(region, COMP_CONFERENCE_LEAGUE, 15);
        if (confFinal) {
            auto championId = confFinal->GetChampion(false);
            if (!championId.isNull()) {
                auto confId = confPool->GetCompID();
                CallMethod<0xD1D8D0>(gSimulationLbChampions, &confId, GetGuiColor(1), 0);
                CallMethod<0xD1E620>(gSimulationLbChampions, &championId);
                CallMethod<0xD1F060>(gSimulationLbChampions, &championId, GetGuiColor(1), 0);
                CallMethod<0xD18920>(gSimulationLbChampions, 0);
            }
        }
    }
    return GetCompetition(region, type, 0);
}

void METHOD OnSetupTeamsForECQuarterfinal(void *screen, DUMMY_ARG, UChar roundType, CDBCompetition *comp) {
    CDBRound *last16 = GetECLast16Round(0, 0, 0);
    if (last16)
        CallMethod<0x8304A0>(screen, 0, last16);
    CallMethod<0x8304A0>(screen, 1, comp);
}

struct ECFinalsAdditionalData {
    CTeamIndex mTeamIDs[30];
    void *mFlags[30];
    void *mTextBoxes[60];
};

ECFinalsAdditionalData *GetECFinalsAdditionalData(void *screen) {
    return raw_ptr<ECFinalsAdditionalData>(screen, 0x594);
}

void * METHOD OnCreateNationalTeamTournamentFinalsEC(void *screen, DUMMY_ARG, Int a) {
    CallMethod<0x830390>(screen, a);
    auto data = GetECFinalsAdditionalData(screen);
    for (UInt i = 0; i < 30; i++) {
        data->mTeamIDs[i].countryId = 0;
        data->mTeamIDs[i].type = 0;
        data->mTeamIDs[i].index = 0;
        data->mFlags[i] = nullptr;
        data->mTextBoxes[i] = nullptr;
        data->mTextBoxes[i + 30] = nullptr;
    }
    return screen;
}

void __declspec(naked) ECSetupTeamsForFinal() {
    __asm {
        cmp al, 2
        jnz CHECK_FOR_FINAL
        lea esi, [ebx + 0x744]
        lea edi, [ebx + 0x66C]
        mov[esp + 0x10], 24
        jmp RETN_BACK
    CHECK_FOR_FINAL:
        cmp al, 3
        jnz RETN_BACK
        lea esi, [ebx + 0x764]
        lea edi, [ebx + 0x67C]
        mov [esp + 0x10], 28
     RETN_BACK:
        mov eax, 0x830510
        jmp eax
    }
}

CDBLeague *OnGetLeagueTournamentWelcome(UChar region, UChar type, UShort index) {
    if (type == COMP_EURO_CUP) {
        CDBLeague *league = GetLeague(region, type, index + 6);
        if (league)
            return league;
    }
    return GetLeague(region, type, index);
}

struct NTWelcomeAdditionalData {
    void *mTextBoxes[48];
    void *mFlags[48];
    UInt mCountryIDs[48];
};

void __declspec(naked) OnNTWelcome1() {
    __asm {
        mov [edi - 0xC0], eax
        mov edx, [eax]
        mov ecx, 0x8364D4
        jmp ecx
    }
}

void __declspec(naked) OnNTWelcome2() {
    __asm {
        mov [edi - 0xC0], eax
        mov edx, [eax]
        mov ecx, 0x836434
        jmp ecx
    }
}

void __declspec(naked) OnNTWelcome3() {
    __asm {
        mov ecx, [esi - 0xC0]
        mov edx, [ecx]
        mov eax, 0x8359FB
        jmp eax
    }
}

void METHOD NationalTeamTournamentGroupsButtonReleased(void *t, DUMMY_ARG, UInt *pId, Int) {
    void *pBtSwitchGroups = *raw_ptr<void *>(t, 0x48C);
    if (*pId == CallVirtualMethodAndReturn<UInt, 23>(pBtSwitchGroups)) { // GetId()
        UInt numGroups = *raw_ptr<UInt>(t, 0x2108); // m_nNumGrpoups
        UInt *pFirstGroupIndexToShow = raw_ptr<UInt>(t, 0x210C); // m_nFirstGroupIndexToShow
        *pFirstGroupIndexToShow += 4;
        if (*pFirstGroupIndexToShow >= numGroups)
            *pFirstGroupIndexToShow = 0;
        Char compType = CDBGame::GetInstance()->TestFlag(2) ? 'W' : 'E';
        UInt textId = *pFirstGroupIndexToShow + 4;
        if (textId >= numGroups)
            textId = 0;
        textId = textId / 4 + 1;
        CallVirtualMethod<88>(pBtSwitchGroups, GetTranslation(FormatStatic("IDS_%cC_GROUPDS_%u", compType, textId))); // SetButtonText
        CallMethod<0x832080>(t); // UpdateGroups
    }
}

struct WCFinalsAdditionalData {
    CTeamIndex mTeamIDs[32 * 2];
    void *mTextBoxes[128];
    void *mFlags[64];
};

WCFinalsAdditionalData *GetWCFinalsAdditionalData(void *screen) {
    return raw_ptr<WCFinalsAdditionalData>(screen, 0x6C0);
}

void *METHOD OnCreateNationalTeamTournamentFinalsWC(void *screen, DUMMY_ARG, void *guiInstance) {
    CallMethod<0x830C50>(screen, guiInstance);
    auto data = GetWCFinalsAdditionalData(screen);
    for (UInt i = 0; i < std::size(data->mTeamIDs); i++) {
        data->mTeamIDs[i].countryId = 0;
        data->mTeamIDs[i].type = 0;
        data->mTeamIDs[i].index = 0;
    }
    return screen;
}

void __declspec(naked) OnWCFinals1() {
    __asm {
        add ebx, 4
        cmp eax, 128
        mov ecx, 0x830F5C
        jmp ecx
    }
}

void __declspec(naked) OnWCFinals2() {
    __asm {
        add ebx, 4
        cmp edi, 128
        mov eax, 0x83101D
        jmp eax
    }
}

void METHOD OnWCFinalsSetupClick(void *screen, DUMMY_ARG, UInt *controlId, Int) {
    auto data = GetWCFinalsAdditionalData(screen);
    for (UInt i = 0; i < 64; i++) {
        if (CallVirtualMethodAndReturn<UInt, 23>(data->mFlags[i]) == *controlId) {
            if (data->mTeamIDs[i].countryId)
                Call<0xD42E60>(&data->mTeamIDs[i], screen, 0, 0);
            return;
        }
    }
    for (UInt i = 0; i < 1; i++) {
        void *imgFlag = *raw_ptr<void *>(screen, 0x51C + i * 4);
        if (CallVirtualMethodAndReturn<UInt, 23>(imgFlag) == *controlId) {
            CTeamIndex teamID = CTeamIndex::make(*raw_ptr<UChar>(screen, 0x514 + i), 0, 0xFFFF);
            Call<0xD42E60>(&data->mTeamIDs[i], screen, 0, 0);
            return;
        }
    }
}

void METHOD OnWCFinalsSetupRound(void *screen, DUMMY_ARG, UChar roundIndex, void *) {
    auto data = GetWCFinalsAdditionalData(screen);
    Bool isU20 = *raw_ptr<UInt>(screen, 0x48C) == 2;
    CDBRound *round = nullptr;
    void **textBoxes = data->mTextBoxes;
    void **flags = data->mFlags;
    UInt pairOffset = 0;
    Bool oldWCMode = false;
    if (!isU20) {
        auto wcPool = GetPool(255, COMP_WORLD_CUP, 0);
        if (wcPool && wcPool->GetNumOfTeams() == 32)
            oldWCMode = true;
    }
    if (isU20 || oldWCMode) {
        UChar compType = isU20 ? COMP_U20_WORLD_CUP : COMP_WORLD_CUP;
        switch (roundIndex) {
        case 0:
            round = GetRoundByRoundType(255, compType, ROUND_LAST_16);
            break;
        case 1:
            round = GetRoundByRoundType(255, compType, ROUND_QUARTERFINAL);
            textBoxes = &data->mTextBoxes[32];
            flags = &data->mFlags[16];
            pairOffset = 16;
            break;
        case 2:
            round = GetRoundByRoundType(255, compType, ROUND_SEMIFINAL);
            textBoxes = &data->mTextBoxes[48];
            flags = &data->mFlags[24];
            pairOffset = 24;
            break;
        case 3:
            round = GetRoundByRoundType(255, compType, ROUND_FINAL_3RD_PLACE);
            textBoxes = &data->mTextBoxes[60];
            flags = &data->mFlags[30];
            pairOffset = 30;
            break;
        case 4:
            round = GetRoundByRoundType(255, compType, ROUND_FINAL);
            textBoxes = &data->mTextBoxes[56];
            flags = &data->mFlags[28];
            pairOffset = 28;
            break;
        }
    }
    else {
        switch (roundIndex) {
        case 0:
            round = GetWCLast32Round(0, 0, 0);
            break;
        case 1:
            round = GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_LAST_16);
            textBoxes = &data->mTextBoxes[64];
            flags = &data->mFlags[32];
            pairOffset = 32;
            break;
        case 2:
            round = GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_QUARTERFINAL);
            textBoxes = &data->mTextBoxes[96];
            flags = &data->mFlags[48];
            pairOffset = 48;
            break;
        case 3:
            round = GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_SEMIFINAL);
            textBoxes = &data->mTextBoxes[112];
            flags = &data->mFlags[56];
            pairOffset = 56;
            break;
        case 4:
            round = GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_FINAL_3RD_PLACE);
            textBoxes = &data->mTextBoxes[124];
            flags = &data->mFlags[62];
            pairOffset = 62;
            break;
        case 5:
            round = GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_FINAL);
            textBoxes = &data->mTextBoxes[120];
            flags = &data->mFlags[60];
            pairOffset = 60;
            break;
        }
    }
    if (!round)
        return;
    wchar_t countryLogoPath[512];
    for (UInt p = 0; p < round->GetNumOfPairs(); p++) {
        RoundPair pair;
        round->GetRoundPair(p, pair);
        if (pair.AreTeamsValid()) {
            data->mTeamIDs[pairOffset] = pair.m_n1stTeam;
            data->mTeamIDs[pairOffset + 1] = pair.m_n2ndTeam;
            CDBTeam *team1 = GetTeam(pair.m_n1stTeam);
            if (team1) {
                SetText(*textBoxes, team1->GetShortName(pair.m_n1stTeam));
                SetImageFilename(*flags, GetCountryLogoPath(countryLogoPath, pair.m_n1stTeam.countryId, 1), 4, 4);
                SetVisible(*flags, true);
            }
            textBoxes++;
            SetText(*textBoxes, GetTranslation("CAL_VERSUS_LABEL"));
            textBoxes++;
            flags++;
            CDBTeam *team2 = GetTeam(pair.m_n2ndTeam);
            if (team2) {
                SetText(*textBoxes, team2->GetShortName(pair.m_n2ndTeam));
                SetImageFilename(*flags, GetCountryLogoPath(countryLogoPath, pair.m_n2ndTeam.countryId, 1), 4, 4);
                SetVisible(*flags, true);
            }
            textBoxes++;
            char strBuf[0x14];
            void *str = pair.GetResultString(strBuf, 0x18, 0, 0);
            SetText(*textBoxes, *raw_ptr<WideChar const *>(str, 8));
            CallMethod<0x40BF80>(strBuf);
            textBoxes++;
            flags++;
        }
        pairOffset += 2;
    }
}

CDBRound *OnWCFinalsGetWCFinal(void *) {
    return GetRoundByRoundType(255, COMP_WORLD_CUP, ROUND_FINAL);
}

void __declspec(naked) OnWCFinals3() {
    __asm {
        add esi, 4
        cmp ebx, 128
        mov eax, 0x83137B
        jmp eax
    }
}

Int METHOD OnWCQualiGetNumOfRegisteredTeams_FirstLastPlace(CDBCompetition *comp) {
    auto pool = GetCompetition(255, COMP_QUALI_WC, 0);
    if (pool) {
        for (Int i = 0; i < pool->GetNumOfScriptCommands(); i++) {
            auto command = pool->GetScriptCommand(i);
            if (command->m_nCommandId == 7 && command->m_competitionId.ToInt() == comp->GetCompID().ToInt()) // GET_TAB_X_TO_Y
                return comp->GetNumOfRegisteredTeams();
        }
    }
    return 0;
}

void METHOD OnSetupRootInternationalComps(void *vec, DUMMY_ARG, UInt *pCompId) {
    CallMethod<0x6E3FD0>(vec, pCompId);
    *pCompId = 0xFF240000;
    CallMethod<0x6E3FD0>(vec, pCompId);
    *pCompId = 0xFF250000;
    CallMethod<0x6E3FD0>(vec, pCompId);
    for (UInt i = 41; i <= 62; i++) {
        *pCompId = 0xFF000000 | (i << 16);
        CallMethod<0x6E3FD0>(vec, pCompId);
    }
}

WideChar *ReadCompetitionName(void *file, WideChar *out) {
    auto result = CallAndReturn<WideChar *, 0xF87370>(file, out);
    if (out[0] == L'$') {
        String key = String(out).substr(1);
        auto value = GetTranslation(WtoA(key).c_str());
        if (value)
            wcsncpy(out, value, 60);
    }
    return result;
}

void METHOD OnSetupAssessmentEntry(CAssessmentTable *table, DUMMY_ARG, UChar countryIndex, UShort y1, UShort y2, UShort y3, UShort y4, UShort y5, UShort y6) {
    if (countryIndex == 207) // Kosovo
        CallMethod<0x121D410>(table, countryIndex, 250, 150, 183, 233, 287, 300); // done for 2024/2025
    else
        CallMethod<0x121D410>(table, countryIndex, y1, y2, y3, y4, y5, y6);
}

CDBCompetition *gCurrentScriptComp = nullptr;

void METHOD OnScriptProcess(CDBCompetition *comp) {
    gCurrentScriptComp = comp;
    auto compId = comp->GetCompID();
    if (compId.countryId == FifamCompRegion::Germany && compId.type == FifamCompType::League) { // Regionalliga
        if (compId.index == 3 || compId.index == 4 || compId.index == 7) {
            Array<UInt, 3> leagueIndices = { 7, 3, 4 }; // Nord, Nordost, Bayern
            UInt seasonIndex = (GetCurrentYear() + 1) % 3;
            if (leagueIndices[seasonIndex] == compId.index)
                CallMethod<0x10503E0>(comp, 1, 0);
            else
                CallMethod<0x10503E0>(comp, 0, 1);
        }
    }
    CallMethod<0x139EE70>(comp);
    if (compId.type == COMP_RELEGATION) {
        auto DateStr = [](UInt d) {
            CJDate fmDate;
            memcpy(&fmDate, &d, 4);
            return Utils::Format(L"%02d.%02d.%04d", fmDate.GetDays(), fmDate.GetMonth(), fmDate.GetYear());
        };
        CDBGame *game = CDBGame::GetInstance();
        if (game) {
            // get current date
            UInt currDate = game->GetCurrentDate().Value();
            // get season end date
            UInt seasonEndDate = 0;
            CallMethod<0xF49A00>(game, &seasonEndDate);
            SafeLog::Write(L"Processing relegation competition dates: " + comp->GetCompID().ToStr() + L" current date: " + DateStr(currDate) + L" season end: " + DateStr(seasonEndDate));
            if (currDate < seasonEndDate) {
                UInt date = 0; // last date of the competition
                if (comp->GetDbType() == DB_LEAGUE) {
                    UInt numMatches = comp->GetNumMatchdays();
                    if (numMatches > 0)
                        date = CallMethodAndReturn<UInt, 0x1050320>(comp, numMatches - 1);
                }
                else if (comp->GetDbType() == DB_ROUND) {
                    UInt legFlags = CallMethodAndReturn<UInt, 0x10423C0>(comp, 0);
                    date = CallMethodAndReturn<UInt, 0x10423F0>(comp, (legFlags >> 1) & 1);
                }
                SafeLog::Write(L"Last competition date: " + DateStr(date));
                if (date != 0 && date > currDate) {
                    for (UInt i = 0; i < comp->GetNumOfTeams(); i++) {
                        CTeamIndex &teamId = comp->GetTeamID(i);
                        if (teamId.countryId) {
                            auto team = GetTeam(teamId);
                            if (team) {
                                SafeLog::Write(String(L"Team: ") + team->GetName());
                                void *cal = raw_ptr<void>(team, 0x2D1C);
                                // find holiday date in team calendar
                                UInt holDate = 0;
                                CallMethod<0x11255E0>(cal, &holDate);
                                SafeLog::Write(L"Old holidays date: " + DateStr(holDate));
                                if (holDate != 0 && holDate > currDate && holDate < date) {
                                    // find next best date for holiday
                                    UInt nextDate = 0;
                                    Bool foundNextDate = false;
                                    for (nextDate = date + 2; nextDate < seasonEndDate; nextDate++) {
                                        if (CallMethodAndReturn<UChar, 0x11256A0>(cal, &nextDate) == 0) {
                                            foundNextDate = true;
                                            break;
                                        }
                                    }
                                    if (!foundNextDate) {
                                        nextDate = date + 1;
                                        if (CallMethodAndReturn<UChar, 0x11256A0>(cal, &nextDate) == 0) {
                                            foundNextDate = true;
                                        }
                                    }
                                    if (foundNextDate) {
                                            // remove holidays from calendar
                                        CallMethod<0x112B940>(cal, &holDate, 0);
                                        // set new holiday date
                                        CallMethod<0x112B940>(cal, &nextDate, 28);

                                        SafeLog::Write(L"New holidays date: " + DateStr(nextDate));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //SafeLog::Write(Utils::Format(L"Competition: %s", comp->GetName()));
    //for (UInt i = 0; i < comp->GetNumOfTeams(); i++) {
    //    CTeamIndex &teamId = comp->GetTeamID(i);
    //    if (teamId.countryId) {
    //        auto team = GetTeam(teamId);
    //        if (team)
    //            SafeLog::Write(Utils::Format(L"%02d. %s", i, team->GetName()));
    //        else
    //            SafeLog::Write(Utils::Format(L"%02d. (n/a)", i));
    //    }
    //    else
    //        SafeLog::Write(Utils::Format(L"%02d. (n/a)", i));
    //}
    gCurrentScriptComp = nullptr;
}

Bool IsContinentalOrInternational(CDBCompetition *comp) {
    auto countryId = comp->GetCompID().countryId;
    return countryId >= 249 && countryId <= 255;
}

CDBCompetition *OnGetCompetition_GET_CHAMP(CCompID const &compId) {
    CDBCompetition *comp = GetCompetition(compId);
    if (compId.countryId > 0 && compId.countryId < 208 && gCurrentScriptComp && IsContinentalOrInternational(gCurrentScriptComp)) {
        if (!comp || !comp->GetChampion().countryId || !GetTeam(comp->GetChampion())) {
            gCurrentScriptComp->AddTeamsFromCountry(compId.countryId, 1);
            return nullptr;
        }
    }
    return comp;
}

CDBCompetition *OnGetCompetition_GET_RUNNER_UP(CCompID const &compId) {
    CDBCompetition *comp = GetCompetition(compId);
    if (compId.countryId > 0 && compId.countryId < 208 && gCurrentScriptComp && IsContinentalOrInternational(gCurrentScriptComp)) {
        if (!comp || !comp->GetRunnerUp().countryId || !GetTeam(comp->GetRunnerUp())) {
            gCurrentScriptComp->AddTeamsFromCountry(compId.countryId, 1);
            return nullptr;
        }
    }
    return comp;
}

CDBCompetition *OnGetCompetition_GET_CHAMP_OR_RUNNER_UP(CCompID const &compId) {
    CDBCompetition *comp = GetCompetition(compId);
    if (compId.countryId > 0 && compId.countryId < 208 && gCurrentScriptComp && IsContinentalOrInternational(gCurrentScriptComp)) {
        if (!comp || !comp->GetChampion().countryId || !GetTeam(comp->GetChampion()) || !comp->GetRunnerUp().countryId || !GetTeam(comp->GetRunnerUp())) {
            gCurrentScriptComp->AddTeamsFromCountry(compId.countryId, 1);
            return nullptr;
        }
    }
    return comp;
}

CTeamIndex * METHOD OnGetChampion_GET_CHAMP(CDBCompetition *comp, DUMMY_ARG, Bool continental) {
    if (comp->GetCompID().index == 32) {
        CTeamIndex *champ = &comp->GetChampion(false);
        if (champ->countryId == 0) {
            auto compId = comp->GetCompID();
            auto compNatLeague = GetCompetition(compId.countryId, compId.type, 0);
            if (compNatLeague)
                champ = &compNatLeague->GetChampion(false);
        }
        return champ;
    }
    return &comp->GetChampion(continental);
}

CTeamIndex *METHOD OnGetRunnerUp_GET_RUNNER_UP(CDBCompetition *comp) {
    if (comp->GetCompID().index == 32) {
        CTeamIndex *champ = &comp->GetRunnerUp();
        if (champ->countryId == 0) {
            auto compId = comp->GetCompID();
            auto compNatLeague = GetCompetition(compId.countryId, compId.type, 0);
            if (compNatLeague)
                champ = &compNatLeague->GetRunnerUp();
        }
        return champ;
    }
    return &comp->GetRunnerUp();
}

template<UChar ResultNoCountry = 0>
UChar METHOD GetCountryAtAssessmentPositionLastYear(CAssessmentTable* table, DUMMY_ARG, UInt position) {
    if (gCurrentScriptComp && gCurrentScriptComp->GetCompID().countryId == FifamCompRegion::Europe &&
        (gCurrentScriptComp->GetCompID().type == COMP_CHAMPIONSLEAGUE ||
        gCurrentScriptComp->GetCompID().type == COMP_UEFA_CUP ||
        gCurrentScriptComp->GetCompID().type == COMP_CONFERENCE_LEAGUE ||
        gCurrentScriptComp->GetCompID().type == COMP_YOUTH_CHAMPIONSLEAGUE)
        )
    {
        UInt positionOriginal = position;
        UChar resultOriginal = table->GetCountryIdAtPositionLastYear(positionOriginal);
        UChar pos1 = table->GetCountryPositionLastYear(FifamCompRegion::Liechtenstein) + 1;
        UChar pos2 = table->GetCountryPositionLastYear(FifamCompRegion::Russia) + 1;
        if (pos1 > pos2)
            std::swap(pos1, pos2);
        if (position >= pos1)
            position += 1;
        if (position >= pos2)
            position += 1;
        UChar result = table->GetCountryIdAtPositionLastYear(position);
        //SafeLog::Write(Utils::Format(L"%s: Position %d translated to %d (from %s to %s)", 
        //    CompetitionTag(gCurrentScriptComp), positionOriginal, position, CountryName(resultOriginal), CountryName(result)));
        return (result == 0) ? ResultNoCountry : result;
    }
    return table->GetCountryIdAtPositionLastYear(position);
}

void METHOD OnSetupRootSetLeagueChampionTeam(CDBLeague *league, DUMMY_ARG, CTeamIndex const &teamId) {
    league->SetChampion(teamId);
    league->SetRunnerUp(league->GetTeamID(1));
    SafeLog::Write(Utils::Format(L"OnSetupRootSetLeagueChampionTeam: League %08X champ %08X runner %08X", league->GetCompID().ToInt(), teamId, league->GetTeamID(1)));
}

UChar METHOD OnProcessRootGetCountryId(CDBRoot *root) {
    UChar countryId = root->GetCompID().countryId;
    CDBLeague *youthLeague = GetLeague(countryId, COMP_LEAGUE, 32);
    if (youthLeague) {
        TeamLeaguePositionData infos[24];
        youthLeague->SortTeams(infos, youthLeague->GetEqualPointsSorting() | 2, 0, 120, 0, 120);
        youthLeague->SetChampion(infos[0].m_teamID);
        youthLeague->SetRunnerUp(infos[1].m_teamID);

    }
    return countryId;
}

void EuropeanCompsParticipantsInitColumns(UInt lb, int, int, int, int, int, int, int, int, int, int, int) {
    Call<0xD19660>(lb, 2, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 63);
    for (UInt i = 0; i < 8; i++)
        * (UChar *)(lb + 24 * (3 + i * 2) + 29) = 204;
}

void *OnGetGameInstanceSetupCompetitionWinners() {
    auto SetDefaultWinners = [](UChar region, Vector<UChar> const &defaultCountries) {
        if (defaultCountries.size() != 3)
            return;
        Array<CTeamIndex, 3> defaultChampions;
        Array<CDBRound *, 3> rounds;
        Array<UChar, 3> roundTypes = { FifamCompType::ChampionsLeague, FifamCompType::UefaCup,  FifamCompType::ConferenceLeague };
        Array<CTeamIndex, 3> champions;
        for (UInt i = 0; i < 3; i++) {
            defaultChampions[i] = CTeamIndex::make(defaultCountries[i], 0, 1);
            rounds[i] = GetRoundByRoundType(region, roundTypes[i], ROUND_FINAL);
            champions[i] = (rounds[i] && rounds[i]->GetChampion().countryId && GetTeam(rounds[i]->GetChampion())) ?
                rounds[i]->GetChampion() : CTeamIndex::null();
        }
        if (region == FifamCompRegion::Europe && rounds[2] && champions[2].isNull()) {
            CDBTeam *olympiacos = GetTeamByUniqueID(0x00160008);
            if (olympiacos && olympiacos->GetTeamID() != champions[0] && olympiacos->GetTeamID() != champions[1]) {
                rounds[2]->SetChampion(olympiacos->GetTeamID());
                champions[2] = olympiacos->GetTeamID();
            }
        }
        Set<UInt> addedChampions;
        for (UInt i = 0; i < 3; i++) {
            if (!champions[i].isNull())
                addedChampions.insert(champions[i].ToInt());
        }
        for (UInt i = 0; i < 3; i++) {
            if (rounds[i] && champions[i].isNull()) {
                for (auto const &t : defaultChampions) {
                    if (!t.isNull() && !Utils::Contains(addedChampions, t.ToInt())) {
                        rounds[i]->SetChampion(t);
                        addedChampions.insert(t.ToInt());
                        SafeLog::Write(Utils::Format(L"Set default champion for %s - %s", rounds[i]->GetName(), TeamTag(t)));
                        break;
                    }
                }
            }
        }
    };
    SetDefaultWinners(FifamCompRegion::Europe, { FifamCompRegion::Spain, FifamCompRegion::England, FifamCompRegion::Italy });
    SetDefaultWinners(FifamCompRegion::SouthAmerica, { FifamCompRegion::Brazil, FifamCompRegion::Argentina, FifamCompRegion::Colombia });
    SetDefaultWinners(FifamCompRegion::NorthAmerica, { FifamCompRegion::Mexico, FifamCompRegion::United_States, FifamCompRegion::Canada });
    SetDefaultWinners(FifamCompRegion::Asia, { FifamCompRegion::United_Arab_Emirates, FifamCompRegion::Japan, FifamCompRegion::Korea_Republic });
    SetDefaultWinners(FifamCompRegion::Africa, { FifamCompRegion::Egypt, FifamCompRegion::Tunisia, FifamCompRegion::DR_Congo });
    SetDefaultWinners(FifamCompRegion::Oceania, { FifamCompRegion::New_Zealand, FifamCompRegion::Fiji, FifamCompRegion::Papua_New_Guinea });

    CDBCompetition *liechtensteinCup = GetCompetition(FifamCompRegion::Switzerland, FifamCompType::LeagueCup, 0);
    if (liechtensteinCup) {
        CDBTeam* vaduz = GetTeamByUniqueID(0x002F0013);
        if (vaduz)
            liechtensteinCup->SetChampion(vaduz->GetTeamID());
    }

    return CDBGame::GetInstance();
}

CDBTeam *GetTeamInitTeamByID(CTeamIndex teamIndex) {
    if (teamIndex.index != 0xFFFF && (teamIndex.type & 0x8))
        return GetTeamByUniqueID(teamIndex.ToInt());
    return GetTeam(teamIndex);
}

wchar_t const *gCompBlockName = nullptr;
void OnFormatCompetitionBlockName(wchar_t *dst, wchar_t const *format, wchar_t const *name, unsigned int number) {
    gCompBlockName = name;
    swprintf(dst, format, name, number);
}

wchar_t const *METHOD OnFindFileBlock(void *file, DUMMY_ARG, wchar_t const *index, wchar_t *pc) {
    if (gCompBlockName) {
        String newBlockName = index;
        newBlockName += gCompBlockName;
        wchar_t const *result = CallMethodAndReturn<wchar_t const *, 0x511FF0>(file, newBlockName.c_str(), pc);
        gCompBlockName = nullptr;
        return result;
    }
    return CallMethodAndReturn<wchar_t const *, 0x511FF0>(file, index, pc);
}

// TODO: remake this
void __declspec(naked) LaunchQualiExec() {
    __asm {
        push    0
        mov     edx, [eax + 0xC]
        mov     ecx, edi
        mov     eax, 0x11F4704
        jmp     eax
    }
}

CDBCompetition *GetPoolForGameStartNationalTeam(CCompID *compId) {
    auto y = GetCurrentYear();
    if ((y % 4) == 0 || (y % 4) == 1)
        return GetCompetition(FifamCompRegion::International, COMP_QUALI_WC, 1);
    if ((y % 4) == 2)
        return GetCompetition(FifamCompRegion::International, COMP_EURO_NL_Q, 0);
    return GetCompetition(FifamCompRegion::International, COMP_QUALI_EC, 0);
}

void METHOD OnClearInternationalCompsEvents(void *vec, DUMMY_ARG, UInt *pId) {
    auto AddComp = [&](UInt type) {
        CallMethod<0x6E3FD0>(vec, &type);
    };
    auto year = GetCurrentYear();
    if (year % 4 == 0) { // 2020/2021
        AddComp(COMP_QUALI_EC); // 2020
        AddComp(COMP_EURO_CUP); // 2020
        AddComp(COMP_COPA_AMERICA); // 2020
        AddComp(COMP_OFC_CUP); // 2020
    }
    if (year % 4 == 1) { // 2017/2018
        AddComp(COMP_CONFED_CUP); // 2017
        AddComp(COMP_FINALISSIMA); // 2017
    }
    if (year % 4 == 2) { // 2018/2019
        AddComp(COMP_QUALI_WC); // 2018
        AddComp(COMP_WORLD_CUP); // 2018
    }
    if (year % 4 == 3) { // 2019/2020
        AddComp(COMP_ASIA_CUP_Q); // 2019
        AddComp(COMP_ASIA_CUP); // 2019
    }
    if (year % 2 == 0) { // 2018/2019, 2020/2021
        AddComp(COMP_NAM_NL); // 2020
        AddComp(COMP_NAM_NL_Q); // 2020
    }
    if (year % 2 == 1) { // 2019/2020, 2021/2022
        AddComp(COMP_U20_WC_Q); // 2019
        AddComp(COMP_U20_WORLD_CUP); // 2019
        AddComp(COMP_EURO_NL_Q); // 2019
        AddComp(COMP_EURO_NL); // 2019
        AddComp(COMP_AFRICA_CUP_Q); // 2019
        AddComp(COMP_AFRICA_CUP); // 2019
        AddComp(COMP_NAM_CUP); // 2019
    }
}

void METHOD OnAddTrophyStickerCompID(void *vec, DUMMY_ARG, UInt *pCompId) {
    pCompId[1] = 0xFF000000 | (COMP_NAM_CUP << 16);
    CallMethod<0x129CB00>(vec, pCompId);
    pCompId[1] = 0xFF000000 | (COMP_AFRICA_CUP << 16);
    CallMethod<0x129CB00>(vec, pCompId);
    pCompId[1] = 0xFF000000 | (COMP_ASIA_CUP << 16);
    CallMethod<0x129CB00>(vec, pCompId);
    pCompId[1] = 0xFF000000 | (COMP_OFC_CUP << 16);
    CallMethod<0x129CB00>(vec, pCompId);
    pCompId[1] = 0xFF000000 | (COMP_EURO_NL << 16);
    CallMethod<0x129CB00>(vec, pCompId);
    pCompId[1] = 0xFF000000 | (COMP_NAM_NL << 16);
    CallMethod<0x129CB00>(vec, pCompId);
    pCompId[1] = 0xFF000000 | (COMP_FINALISSIMA << 16);
    CallMethod<0x129CB00>(vec, pCompId);
}

void METHOD OnAddTrophyClubTrophiesScreen_Continental(void *vec, DUMMY_ARG, UChar *pCompType) {
    CallMethod<0x416950>(vec, pCompType);
    *pCompType = COMP_CONFERENCE_LEAGUE;
    CallMethod<0x416950>(vec, pCompType);
    *pCompType = COMP_UIC;
    CallMethod<0x416950>(vec, pCompType);
    *pCompType = COMP_CONTINENTAL_1;
    CallMethod<0x416950>(vec, pCompType);
    *pCompType = COMP_CONTINENTAL_2;
    CallMethod<0x416950>(vec, pCompType);
}

void METHOD OnAddTrophyClubTrophiesScreen_Other(void *vec, DUMMY_ARG, UChar *pCompType) {
    CallMethod<0x416950>(vec, pCompType);
    *pCompType = COMP_ICC;
    CallMethod<0x416950>(vec, pCompType);
}

CTeamIndex gClubAchievementsScreenTeam;

CDBTeam *OnGetTeamForClubAchievementsScreen(CTeamIndex teamIndex) {
    gClubAchievementsScreenTeam = teamIndex;
    return GetTeam(teamIndex);
}

WideChar *OnGetTrophyPicForClubAchievementsScreen(WideChar *buf, CCompID compId, Int flag, Bool *outResult) {
    if (gClubAchievementsScreenTeam.countryId >= 1 && gClubAchievementsScreenTeam.countryId <= 207) {
        auto countryStore = GetCountryStore();
        if (countryStore) {
            UInt continent = countryStore->m_aCountries[gClubAchievementsScreenTeam.countryId].GetContinent();
            if (continent <= 5)
                compId.countryId = 249 + continent;
        }
    }
    return CallAndReturn<WideChar *, 0xD2B490>(buf, compId, flag, outResult);
}

UInt METHOD GetDateTrophyBook(void *data) {
    CallMethod<0x1494AE2>(data, 2019, 7, 1);
    return 1;
}

UInt METHOD GetIsInternationalCompQuali(CDBCompetition *comp) {
    auto type = comp->GetCompID().type;
    Bool result =
        type == COMP_QUALI_EC ||
        type == COMP_QUALI_WC ||

        type == COMP_AFRICA_CUP_Q ||
        type == COMP_ASIA_CUP_Q ||
        type == COMP_OFC_CUP_Q ||
        type == COMP_NAM_NL_Q ||
        type == COMP_EURO_NL_Q ||

        type == COMP_NAM_NL ||
        type == COMP_EURO_NL ||
        type == COMP_OFC_CUP ||
        type == COMP_ASIA_CUP ||
        type == COMP_AFRICA_CUP ||
        type == COMP_NAM_CUP ||
        type == COMP_FINALISSIMA ||

        type == COMP_INTERNATIONAL_FRIENDLY;
    if (result)
        return COMP_QUALI_WC;
    return 0;
}

UInt METHOD GetIsCompWithNTStadsSelection(CDBCompetition *comp) {
    auto type = comp->GetCompID().type;
    Bool result =
        type == COMP_QUALI_EC ||
        type == COMP_QUALI_WC ||

        type == COMP_AFRICA_CUP_Q ||
        type == COMP_ASIA_CUP_Q ||
        type == COMP_OFC_CUP_Q ||
        type == COMP_NAM_NL_Q ||
        type == COMP_EURO_NL_Q;
    if (result)
        return COMP_QUALI_EC;
    return 0;
}

void __declspec(naked) WatchMatchesVideoTextCmpRegion() {
    __asm {
        cmp dl, 0xF9
        jz WatchMatchesVideoTextCmpRegion_RET1
        cmp dl, 0xFA
        jz WatchMatchesVideoTextCmpRegion_RET1
        cmp dl, 0xFB
        jz WatchMatchesVideoTextCmpRegion_RET1
        cmp dl, 0xFC
        jz WatchMatchesVideoTextCmpRegion_RET1
        cmp dl, 0xFD
        jz WatchMatchesVideoTextCmpRegion_RET1
        cmp dl, 0xFE
        jz WatchMatchesVideoTextCmpRegion_RET1
        mov eax, 0xAC4936
        jmp eax
        WatchMatchesVideoTextCmpRegion_RET1 :
        mov eax, 0xAC492E
        jmp eax
    }
}

Bool GetIsSpecialMatch(void* match, Int, UInt* unk) {
    *unk = 0;
    CCompID compId;
    CallMethod<0xE80190>(match, &compId);
    if (compId.type == COMP_U20_WC_Q || compId.type == COMP_U20_WORLD_CUP)
        return false;
    return CallMethodAndReturn<Bool, 0xE82EC0>(match);
}

CTeamIndex* METHOD OnGetTeamIDMDSelectScreen_H(void* match, DUMMY_ARG, CTeamIndex* ret_teamID) {
    CallMethod<0xE7FCF0>(match, ret_teamID);
    if ((ret_teamID->type == 2 || ret_teamID->type == 4) && ret_teamID->index != 0xFFFF)
        ret_teamID->type = 0;
    return ret_teamID;
}

CTeamIndex* METHOD OnGetTeamIDMDSelectScreen_A(void* match, DUMMY_ARG, CTeamIndex* ret_teamID) {
    CallMethod<0xE7FD00>(match, ret_teamID);
    if ((ret_teamID->type == 2 || ret_teamID->type == 4) && ret_teamID->index != 0xFFFF)
        ret_teamID->type = 0;
    return ret_teamID;
}

void METHOD SetupLeagueMatchImportance(CDBLeague* league, DUMMY_ARG, void* match) {
    if (league->GetCompID().type == COMP_ICC) {
        CallMethod<0x137DB90>(league, match); // friendly
        return;
    }
    CallMethod<0x1053430>(league, match); // league
}

void METHOD SetupRoundMatchImportance(CDBRound* round, DUMMY_ARG, void* match) {
    if (round->GetCompID().type == COMP_ICC) {
        CallMethod<0x137DB90>(round, match); // friendly
        return;
    }
    CallMethod<0x1042570>(round, match); // round
}

CDBCompetition* gInternationalPlayersCompetition = nullptr;

CDBRoot* METHOD GetInternationalPlayersCompetitionRoot(CDBCompetition* comp) {
    gInternationalPlayersCompetition = comp;
    return CallMethodAndReturn<CDBRoot*, 0xF8B680>(comp);
}

bool METHOD InternationalPlayersCheck(CDBGame* game, DUMMY_ARG, Int flag) {
    if (gInternationalPlayersCompetition) {
        CCompID compId = gInternationalPlayersCompetition->GetCompID();
        if (compId.type != COMP_WORLD_CUP
            && compId.type != COMP_QUALI_WC
            && compId.type != COMP_EURO_CUP
            && compId.type != COMP_QUALI_EC)
        {
            return false;
        }
    }
    return CallMethodAndReturn<bool, 0xF49CA0>(game, flag);
}

CTeamIndex& METHOD GetEuropeanAssessmentCupRunnerUp(CDBCompetition* comp) {
    static CTeamIndex nullTeam = CTeamIndex::make(0, 0, 0);
    return nullTeam;
}

CTeamIndex& METHOD GetEuropeanAssessmentCupWinner(CDBCompetition* comp, DUMMY_ARG, Bool checkIntl) {
    CTeamIndex& champ = comp->GetChampion(checkIntl);
    if (IsLiechtensteinClubFromSwitzerland(champ)) {
        static CTeamIndex nullTeam = CTeamIndex::make(0, 0, 0);
        return nullTeam;
    }
    return champ;
}

CTeamIndex& METHOD GetEuropeanAssessmentTeamsGetTeamAtPosition(CDBLeague* league, DUMMY_ARG, Int position) {
    CTeamIndex& teamIndex = league->GetTeamAtPosition(position);
    if (IsLiechtensteinClubFromSwitzerland(teamIndex)) {
        static CTeamIndex nullTeam = CTeamIndex::make(0, 0, 0);
        return nullTeam;
    }
    return teamIndex;
}

CTeamIndex& METHOD FillAssessmentReservesGetTeamAtPosition(CDBLeague* league, DUMMY_ARG, Int position) {
    CTeamIndex& teamIndex = league->GetTeamAtPosition(position);
    if (IsLiechtensteinClubFromSwitzerland(teamIndex)) {
        static CTeamIndex nullTeam = CTeamIndex::make(0, 0, 0);
        return nullTeam;
    }
    return teamIndex;
}

Bool METHOD OnAddCupTeam(CDBCup *cup, DUMMY_ARG, CTeamIndex const &teamIndex, Int putAt) {
    if (cup->GetCompID().ToInt() == 0x2F030000 && IsLiechtensteinClubFromSwitzerland(teamIndex)) // { 47, FA_CUP, 0 }
        return false;
    return CallMethodAndReturn<Bool, 0xF85C80>(cup, &teamIndex, putAt);
}

unsigned char METHOD GetFACompCountryId_SkipReserveTeams(void* comp) {
    return 45;
}

Vector<UChar> &GetCountriesWithLeagueSplit() {
    static Vector<UChar> countriesWithLeagueSplit;
    return countriesWithLeagueSplit;
}

Vector<Pair<UChar, UChar>> &GetDelayedLeagueCups() {
    static Vector<Pair<UChar, UChar>> delayedLeagueCups;
    return delayedLeagueCups;
}

struct LeagueTableInfo {
    Char promotion = -1;
    Char promotionPlayOff = -1;
    Char relegation = -1;
    Char relegationPlayOff = -1;
};

Map<UInt, LeagueTableInfo> &GetLeagueTableInfoMap() {
    static Map<UInt, LeagueTableInfo> leagueTableInfoMap;
    return leagueTableInfoMap;
}

LeagueTableInfo *GetLeagueTableInfo(UInt leagueId) {
    auto it = GetLeagueTableInfoMap().find(leagueId);
    if (it != GetLeagueTableInfoMap().end())
        return &(*it).second;
    return nullptr;
}

Array<UChar, 207> gCountryReserveLevelId = {};

void ReadLeaguesConfig() {
    FifamReader leagueSplitReader(Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','l','e','a','g','u','e','_','s','p','l','i','t','.','c','s','v'>(830049359), 14);
    if (leagueSplitReader.Available()) {
        leagueSplitReader.SkipLine();
        while (!leagueSplitReader.IsEof()) {
            if (!leagueSplitReader.EmptyLine()) {
                UChar countryId = 0;
                leagueSplitReader.ReadLine(countryId);
                if (countryId != 0) {
                    GetCountriesWithLeagueSplit().push_back(countryId);
                }
            }
            else
                leagueSplitReader.SkipLine();
        }
        leagueSplitReader.Close();
    }
    FifamReader specialLeagueCupsReader(Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','s','p','e','c','i','a','l','_','l','e','a','g','u','e','_','c','u','p','s','.','c','s','v'>(3558633990), 14);
    if (specialLeagueCupsReader.Available()) {
        specialLeagueCupsReader.SkipLine();
        while (!specialLeagueCupsReader.IsEof()) {
            if (!specialLeagueCupsReader.EmptyLine()) {
                UChar countryId = 0, cupIndex = 0;
                specialLeagueCupsReader.ReadLine(countryId, cupIndex);
                if (countryId != 0)
                    GetDelayedLeagueCups().emplace_back(countryId, cupIndex);
            }
            else
                specialLeagueCupsReader.SkipLine();
        }
        specialLeagueCupsReader.Close();
    }
    FifamReader leagueTablesReader(Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','l','e','a','g','u','e','_','t','a','b','l','e','s','.','c','s','v'>(137862772), 14);
    if (leagueTablesReader.Available()) {
        leagueTablesReader.SkipLine();
        while (!leagueTablesReader.IsEof()) {
            if (!leagueTablesReader.EmptyLine()) {
                UChar countryId = 0, leagueIndex = 0;
                Int pro = -1, ppo = -1, rel = -1, rpo = -1;
                leagueTablesReader.ReadLine(countryId, leagueIndex, OptionalInt(pro), OptionalInt(ppo), OptionalInt(rel), OptionalInt(rpo));
                if (countryId != 0) {
                    UInt leagueId = leagueIndex | 0x10000 | (countryId << 24);
                    LeagueTableInfo info;
                    info.promotion = pro;
                    info.promotionPlayOff = ppo;
                    info.relegation = rel;
                    info.relegationPlayOff = rpo;
                    GetLeagueTableInfoMap()[leagueId] = info;
                }
            }
            else
                leagueTablesReader.SkipLine();
        }
        leagueTablesReader.Close();
    }
    for (UInt i = 0; i < std::size(gCountryReserveLevelId); i++)
        gCountryReserveLevelId[i] = 0;
    FifamReader leagueReserveReader(L"plugins\\ucp\\reserve_leagues.csv", 14);
    if (leagueReserveReader.Available()) {
        leagueReserveReader.SkipLine();
        while (!leagueReserveReader.IsEof()) {
            if (!leagueReserveReader.EmptyLine()) {
                UChar countryId = 0, levelId = 0;
                leagueReserveReader.ReadLine(countryId, levelId);
                if (countryId >= 1 && countryId <= 207)
                    gCountryReserveLevelId[countryId - 1] = levelId;
            }
            else
                leagueReserveReader.SkipLine();
        }
        leagueReserveReader.Close();
    }
}

const UChar COUNTRY_WITH_LEAGUE_SPLIT_ID = FifamCompRegion::Scotland;
CCompID COMP_LEAGUE_SPLIT_CHAMP = CCompID::Make(FifamCompRegion::Scotland, FifamCompType::Relegation, 0);
CCompID COMP_LEAGUE_SPLIT_RELEGATION = CCompID::Make(FifamCompRegion::Scotland, FifamCompType::Relegation, 1);
CCompID COMP_LEAGUE_WITH_SPLIT = CCompID::Make(FifamCompRegion::Scotland, FifamCompType::League, 0);
CCompID COMP_LEAGUE_SPLIT_NULL = CCompID::Make(FifamCompRegion::None, FifamCompType::Root, 0);

Bool IsCountryWithLeagueSplit(UChar countryId) {
    for (UInt i = 0; i < GetCountriesWithLeagueSplit().size(); i++) {
        if (GetCountriesWithLeagueSplit()[i] == countryId)
            return true;
    }
    return false;
}

Bool IsCountryWithLeaguePlayOffCup(UChar countryId, UChar cupIndex) {
    for (UInt i = 0; i < GetDelayedLeagueCups().size(); i++) {
        if (GetDelayedLeagueCups()[i].first == countryId && GetDelayedLeagueCups()[i].second == cupIndex)
            return true;
    }
    return false;
}

UChar GetCountryWithLeagueSplitDefaultId(UChar countryId) {
    return IsCountryWithLeagueSplit(countryId) ? COUNTRY_WITH_LEAGUE_SPLIT_ID : 0;
}

Bool IsCompetitionLeagueSplitChampionship(CCompID const &compId) {
    return compId.type == FifamCompType::Relegation && compId.index == 0 && IsCountryWithLeagueSplit(compId.countryId);
}

UInt GetCompetitionLeagueSplitChampionship_UInt(UInt compIdUInt) {
    CCompID compId;
    compId.SetFromInt(compIdUInt);
    return IsCompetitionLeagueSplitChampionship(compId) ? compIdUInt : 0;
}

Bool IsCompetitionLeagueSplitRelegation(CCompID const &compId) {
    return compId.type == FifamCompType::Relegation && compId.index == 1 && IsCountryWithLeagueSplit(compId.countryId);
}

UInt GetCompetitionLeagueSplitRelegation_UInt(UInt compIdUInt) {
    CCompID compId;
    compId.SetFromInt(compIdUInt);
    return IsCompetitionLeagueSplitRelegation(compId) ? compIdUInt : 0;
}

Bool IsCompetitionLeagueSplit(CCompID const &compId) {
    return IsCompetitionLeagueSplitChampionship(compId) || IsCompetitionLeagueSplitRelegation(compId);
}

Bool IsCompetitionLeagueSplit_UInt(UInt compIdUInt) {
    CCompID compId;
    compId.SetFromInt(compIdUInt);
    return IsCompetitionLeagueSplit(compId);
}

UInt GetCompetitionLeagueSplitMainLeague(UInt compIdUInt) {
    CCompID compId;
    compId.SetFromInt(compIdUInt);
    compId.index = 0;
    compId.type = COMP_LEAGUE;
    return compId.ToInt();
}

Bool IsCompetitionLeagueWithSplit(CCompID const &compId) {
    return compId.type == FifamCompType::League && compId.index == 0 && IsCountryWithLeagueSplit(compId.countryId);
}

UChar METHOD LeagueSplit_GetCompetitionCountryId(CDBCompetition *comp) {
    return IsCountryWithLeagueSplit(comp->GetCompID().countryId) ? COUNTRY_WITH_LEAGUE_SPLIT_ID : 0;
}

UShort METHOD LeagueSplit_GetTeamCountryId(CDBTeam *team) {
    auto countryId = CallMethodAndReturn<UShort, 0xEC9500>(team);
    return IsCountryWithLeagueSplit((UChar)countryId) ? COUNTRY_WITH_LEAGUE_SPLIT_ID : 0;
}

UChar LeagueSplit_Team_StoredCountryId = 0;

UShort METHOD LeagueSplit_GetTeamCountryId_StoreCountryId(CDBTeam *team) {
    UShort countryId = CallMethodAndReturn<UShort, 0xEC9500>(team);
    if (IsCountryWithLeagueSplit((UChar)countryId)) {
        LeagueSplit_Team_StoredCountryId = (UChar)countryId;
        return COUNTRY_WITH_LEAGUE_SPLIT_ID;
    }
    LeagueSplit_Team_StoredCountryId = 0;
    return 0;
}

CCompID * METHOD LeagueSplit_GetCompId_League(CDBCompetition *comp, DUMMY_ARG, CCompID *out) {
    if (IsCompetitionLeagueWithSplit(comp->GetCompID()))
        *out = COMP_LEAGUE_WITH_SPLIT;
    else
        *out = COMP_LEAGUE_SPLIT_NULL;
    return out;
}

CCompID *METHOD LeagueSplit_GetCompId_Championship(CDBCompetition *comp, DUMMY_ARG, CCompID *out) {
    if (IsCompetitionLeagueSplitChampionship(comp->GetCompID()))
        *out = COMP_LEAGUE_SPLIT_CHAMP;
    else
        *out = COMP_LEAGUE_SPLIT_NULL;
    return out;
}

UChar LeagueSplit_StoredCountryId = 0;

CCompID *METHOD LeagueSplit_GetCompId_Championship_StoreCountryId(CDBCompetition *comp, DUMMY_ARG, CCompID *out) {
    if (IsCompetitionLeagueSplitChampionship(comp->GetCompID())) {
        *out = COMP_LEAGUE_SPLIT_CHAMP;
        LeagueSplit_StoredCountryId = comp->GetCompID().countryId;
    }
    else {
        *out = COMP_LEAGUE_SPLIT_NULL;
        LeagueSplit_StoredCountryId = 0;
    }
    return out;
}

CCompID *METHOD LeagueSplit_GetCompId_Relegation(CDBCompetition *comp, DUMMY_ARG, CCompID *out) {
    if (IsCompetitionLeagueSplitRelegation(comp->GetCompID()))
        *out = COMP_LEAGUE_SPLIT_RELEGATION;
    else
        *out = COMP_LEAGUE_SPLIT_NULL;
    return out;
}

CDBLeague *LeagueSplit_GetLeagueFromStoredCountry(UChar countryId, UChar compType, UShort compIndex) {
    return CallAndReturn<CDBLeague *, 0xF8C620>(LeagueSplit_StoredCountryId, compType, compIndex);
}

CDBLeague *LeagueSplit_GetLeagueFromStoredCountry_Team(UChar countryId, UChar compType, UShort compIndex) {
    return CallAndReturn<CDBLeague *, 0xF8C620>(LeagueSplit_Team_StoredCountryId, compType, compIndex);
}

CCompID *METHOD LeagueSplit_sub_ECCC90_Champ(CDBTeam *team, DUMMY_ARG, CCompID *outCompId, CTeamIndex teamIndex) {
    CallMethodAndReturn<CCompID *, 0xECCC90>(team, outCompId, teamIndex);
    if (IsCompetitionLeagueSplitChampionship(*outCompId))
        *outCompId = COMP_LEAGUE_SPLIT_CHAMP;
    else
        *outCompId = COMP_LEAGUE_SPLIT_NULL;
    return outCompId;
}

CCompID *METHOD LeagueSplit_sub_ECCC90_Relegation(CDBTeam *team, DUMMY_ARG, CCompID *outCompId, CTeamIndex teamIndex) {
    CallMethodAndReturn<CCompID *, 0xECCC90>(team, outCompId, teamIndex);
    if (IsCompetitionLeagueSplitRelegation(*outCompId))
        *outCompId = COMP_LEAGUE_SPLIT_RELEGATION;
    else
        *outCompId = COMP_LEAGUE_SPLIT_NULL;
    return outCompId;
}

void __declspec(naked) LeagueSplit_Cmp_Relegation1() {
    __asm {
        mov eax, [esp + 0x44]
        push eax
        call GetCompetitionLeagueSplitRelegation_UInt
        cmp [esp + 0x44], eax
        mov eax, 0x1068383
        jmp eax
    }
}

UInt LeagueSplit_CountryId_TitleRace = 0;

void __declspec(naked) LeagueSplit_Cmp_CountryId() {
    __asm {
        movzx eax, byte ptr[esi + 0xEA]
        mov LeagueSplit_CountryId_TitleRace, eax
        push eax
        call GetCountryWithLeagueSplitDefaultId
        add esp, 4
        cmp al, 42
        mov eax, 0xEE298A
        jmp eax
    }
}

CDBLeague *LeagueSplit_GetLeague_TitleRace(UChar countryId, UChar compType, UShort compIndex) {
    return CallAndReturn<CDBLeague *, 0xF8C620>(LeagueSplit_CountryId_TitleRace, compType, compIndex);
}

CCompID *METHOD LeagueSplit_GetCompId_Universal(CDBCompetition *comp, DUMMY_ARG, CCompID *outCompId) {
    CCompID compId = comp->GetCompID();
    if (IsCompetitionLeagueSplitChampionship(compId))
        *outCompId = COMP_LEAGUE_SPLIT_CHAMP;
    else if (IsCompetitionLeagueSplitRelegation(compId))
        *outCompId = COMP_LEAGUE_SPLIT_RELEGATION;
    else
        *outCompId = COMP_LEAGUE_SPLIT_NULL;
    return outCompId;
}

Bool METHOD LeagueSplit_GetCompDbType(CDBCompetition *comp) {
    if (IsCountryWithLeagueSplit(comp->GetCompID().countryId))
        return comp->GetDbType() == 1;
    return false;
}

CDBCup *OnGetLeagueCupToLaunch(unsigned int countryId, unsigned int type, unsigned short index) {
    if (IsCountryWithLeaguePlayOffCup(countryId, (UChar)index))
        return nullptr;
    return CallAndReturn<CDBCup *, 0xF8B1E0>(countryId, type, index);
}

void METHOD GetPositionPromotionsRelegationsPlacesInfo(CDBLeague *league, DUMMY_ARG, unsigned char *numGreen, unsigned char *numBlue, unsigned char *numWhite, unsigned char *numPink) {
    CallMethod<0xF8E3E0>(league, numGreen, numBlue, numWhite, numPink);
    auto rel = GetCompetition(league->GetCompID().countryId, COMP_RELEGATION, 0);
    if (rel) {
        auto info = GetLeagueTableInfo(league->GetCompID().ToInt());
        if (info) {
            UInt numTeams = league->GetNumOfTeams();
            if (info->promotion >= 0)
                *numGreen = info->promotion;
            if (info->promotionPlayOff >= 0)
                *numBlue = info->promotionPlayOff;
            if (info->relegationPlayOff >= 0)
                *numPink = info->relegationPlayOff;
            if (info->relegation >= 0 && numTeams >= (UInt)info->relegation)
                *numWhite = numTeams - info->relegation;
        }
    }
    //if (id.type == FifamCompType::League) {
    //    if (id.countryId == FifamCompRegion::Germany) {
    //        if (id.index == 1 || id.index == 2) {
    //            *numGreen = 2;
    //            *numBlue = 1;
    //        }
    //        else if (id.index == 3 || id.index == 4 || id.index == 5) {
    //            *numBlue = 1;
    //        }
    //        else if (id.index == 7) {
    //            *numGreen = 0;
    //            *numBlue = 1;
    //            *numWhite = 16;
    //            *numPink = 2;
    //        }
    //        else if (id.index == 8 || id.index == 9 || id.index == 10) {
    //            *numGreen = 0;
    //            *numBlue = 1;
    //        }
    //        else if (id.index == 11) {
    //            *numGreen = 1;
    //            *numBlue = 1;
    //        }
    //        else if (id.index == 20 || id.index == 21) {
    //            *numPink = 0;
    //        }
    //    }
    //    else if (id.countryId == FifamCompRegion::Romania) {
    //        if (id.index == 0) {
    //            *numGreen = 0;
    //            *numBlue = 0;
    //            *numWhite = 12;
    //            *numPink = 0;
    //        }
    //    }
    //}
}

void SetupCupDraw(CCompID compId, UShort roundType, Bool unk) {
    if (compId.countryId == FifamCompRegion::Europe && roundType != ROUND_SEMIFINAL) {
        Call<0xF4AB00>(compId, roundType, unk);
        //Error(L"SetupCupDraw: %s %d", compId.ToStr().c_str(), roundType);
    }
}

Bool GetCanBePostPoned1(CCompID compId) {
    switch (compId.type) {
    case COMP_LEAGUE:
    case COMP_LEAGUE_SPARE:
    case COMP_LE_CUP:
    case COMP_CHALLENGE_SHIELD:
    case COMP_CONFERENCE_CUP:
    case COMP_FRIENDLY:
        return true;
    }
    return false;
}

Bool GetCanBePostPoned2(CCompID compId) {
    switch (compId.type) {
    case COMP_LEAGUE:
    case COMP_LE_CUP:
    case COMP_CHALLENGE_SHIELD:
    case COMP_CONFERENCE_CUP:
    case COMP_SUPERCUP:
    case COMP_RESERVE:
        return true;
    }
    return false;
}

CDBCompetition *OnGetCompForStatsScreen(CCompID const &compId) {
    CDBCompetition *comp = GetCompetition(compId);
    if (comp && compId.type == COMP_RELEGATION) {
        for (UInt i = 0; i < 32; i++) {
            auto predId = comp->GetPredecessor(i);
            if (predId.type == COMP_RELEGATION) {
                auto pred = GetCompetition(predId);
                if (!pred)
                    return nullptr;
                Bool isPredLeagueSplit = pred->GetDbType() == DB_LEAGUE && IsCountryWithLeagueSplit(predId.countryId) && (predId.index == 0 || predId.index == 1);
                if (!isPredLeagueSplit)
                    return nullptr;
            }
        }
    }
    return comp;
}

void METHOD OnKeepingList(CDBCompetition *comp, DUMMY_ARG, FmVec<CCompID> &vec) {
    CallMethod<0xF93AB0>(comp, &vec);
    SafeLog::Write(L"OnKeepingList: " + comp->GetCompID().ToStr());
    for (UInt i = 0; i < vec.size(); i++)
        SafeLog::Write(vec[i]->ToStr());
}

void METHOD GetLeagueInfo(CDBTeam *team, DUMMY_ARG, UChar *outDivision, UChar *outPlace, CTeamIndex const &teamIndex) {
    //UInt teamIndex = CallAndReturn<UChar, 0x14F6FFB>(teamIndex.type);
}

UChar METHOD GetTeamLeaguePlace(CDBTeam *team, DUMMY_ARG, Bool bFirst) {
    auto result = CallMethodAndReturn<UChar, 0xEE1DF0>(team, bFirst);
    CTeamIndex teamIndex = team->GetTeamID();
    teamIndex.type = bFirst ? 0 : 1;
    if (IsCountryWithLeagueSplit(teamIndex.countryId)) {
        CDBLeague *rel1 = GetLeague(teamIndex.countryId, COMP_RELEGATION, 0);
        if (rel1 && !rel1->IsTeamPresent(teamIndex)) {
            CDBLeague *rel2 = GetLeague(teamIndex.countryId, COMP_RELEGATION, 1);
            if (rel2 && rel2->IsTeamPresent(teamIndex))
                result += rel1->GetNumOfTeams();
        }
    }
    return result;
}

Int gCurrentCLTeamIndex = -1;

CTeamIndex & METHOD OnGetCLPoolTeam(CDBCompetition *comp, DUMMY_ARG, Int index) {
    if (comp->GetCompID().countryId == FifamCompRegion::Europe && comp->GetCompID().type == FifamCompType::ChampionsLeague && comp->GetNumOfTeams() == 32)
        gCurrentCLTeamIndex = index;
    else if (comp->GetCompID().countryId == FifamCompRegion::Europe && comp->GetCompID().type == FifamCompType::UefaCup && comp->GetNumOfTeams() == 32 && index == 0)
        gCurrentCLTeamIndex = index;
    else
        gCurrentCLTeamIndex = -1;
    return comp->GetTeamID(index);
}

UInt METHOD OnGetCLTeamIntlPrestige(CDBTeam *team) {
    UInt p = team->GetInternationalPrestige();
    if (gCurrentCLTeamIndex >= 0 && gCurrentCLTeamIndex < 8)
        p += 10000;
    gCurrentCLTeamIndex = -1;
    return p;
}

void METHOD OnAddYouthAndReserveTeams(CDBCompetition *comp, DUMMY_ARG, int teamType) {
    if (comp->GetCompetitionType() != COMP_RELEGATION)
        CallMethod<0x1066490>(comp, teamType);
}

void METHOD OnSetupLeagueComposition(CDBCompetition *comp) {
    if (comp->GetCompetitionType() != COMP_RELEGATION)
        CallMethod<0x105DD70>(comp);
}

UChar *METHOD OnFillClubInfoFixturesList(void *m, DUMMY_ARG, UChar *arg) {
    UChar compTypes[] = { COMP_CONFERENCE_LEAGUE, COMP_UIC, COMP_ICC, COMP_YOUTH_CHAMPIONSLEAGUE, COMP_CONTINENTAL_1, COMP_CONTINENTAL_2 };
    for (UChar compType : compTypes)
        *CallMethodAndReturn<UChar *, 0x651180>(m, &compType) = 1;
    return CallMethodAndReturn<UChar *, 0x651180>(m, arg);
}

UChar *METHOD OnFillClubInfoFixturesList_NT(void *m, DUMMY_ARG, UChar *arg) {
    UChar compTypes[] = { COMP_EURO_NL_Q, COMP_EURO_NL, COMP_NAM_NL_Q, COMP_NAM_NL, COMP_NAM_CUP, COMP_AFRICA_CUP_Q, COMP_AFRICA_CUP, COMP_ASIA_CUP_Q, COMP_ASIA_CUP, COMP_OFC_CUP_Q, COMP_OFC_CUP, COMP_FINALISSIMA };
    for (UChar compType : compTypes)
        *CallMethodAndReturn<UChar *, 0x651180>(m, &compType) = 1;
    return CallMethodAndReturn<UChar *, 0x651180>(m, arg);
}

Bool METHOD OnRegisterSquadSelectionScreenForWC_EC(CJDate *a, DUMMY_ARG, CJDate *b) {
    if (CDBGame::GetInstance() && CallMethodAndReturn<Bool, 0xF4A340>(CDBGame::GetInstance()))
        return false;
    return plugin::CallMethodAndReturn<Bool, 0x1494F7D>(a, b);
}

unsigned short METHOD IsFinalRound_22(CDBRound *round) {
    return (round->GetRoundType() == 15) ? 22 : 0x7FFF;
}

UInt METHOD OnGetQualifiedForContinentalCompetition_SeasonGoals(CDBTeam *team, DUMMY_ARG, Int *a, CCompID *outCompId) {
    UInt result = CallMethodAndReturn<UInt, 0xEFE330>(team, a, outCompId);
    if (outCompId->countryId >= FifamCompRegion::Europe && outCompId->countryId <= FifamCompRegion::Oceania &&
        (outCompId->type == FifamCompType::ChampionsLeague || outCompId->type == FifamCompType::UefaCup || outCompId->type == FifamCompType::ConferenceLeague))
    {
        outCompId->index = 0;
    }
    return result;
}

// NOTE: QUALI_EC 1 and QUALI_WC 8 are hardcoded now in the plugin

void METHOD OnPoolLaunch(CDBPool* pool) {
    CCompID compID = pool->GetCompID();
    if (compID.countryId == FifamCompRegion::NorthAmerica && compID.type == COMP_CONTINENTAL_2 && compID.index == 0) {
        if (!CDBGame::GetInstance()->IsCountryPlayable(FifamCompRegion::Mexico)
            || !CDBGame::GetInstance()->IsCountryPlayable(FifamCompRegion::United_States))
        {
            CCompID successorID = pool->GetSuccessor(0);
            if (successorID.countryId == FifamCompRegion::NorthAmerica && successorID.type == COMP_CONTINENTAL_2 && successorID.index == 1) {
                CDBRound *leaguesCupLast16 = GetRoundByRoundType(FifamCompRegion::NorthAmerica, COMP_CONTINENTAL_2, ROUND_LAST_16);
                if (leaguesCupLast16) {
                    *raw_ptr<CCompID>(pool, 0x12C) = leaguesCupLast16->GetCompID(); // successor[0]
                    *raw_ptr<CCompID>(leaguesCupLast16, 0xAC) = compID; // predecessor[0]
                }
            }
        }
    }
    if (GetCompetition(FifamCompRegion::International, FifamCompType::EuroNLQ, 0)) { // if Euro NL is present - we need to wait for it
        if (compID.type == COMP_QUALI_EC && compID.index == 1) {
            Bool startSim = GetCurrentYear() == GetStartingYear() && ((GetCurrentYear() % 4) == 3);
            if (!startSim && (GetCurrentYear() % 4) != 2)
                return;
        }
        if (compID.type == COMP_QUALI_WC && compID.index == 8) {
            Bool startSim = GetCurrentYear() == GetStartingYear() && ((GetCurrentYear() % 4) == 1);
            if (!startSim) {
                if ((GetCurrentYear() % 4) != 0)
                    return;
                if (GetCurrentMonth() == 7)
                    return;
            }
        }
    }
    bool clPoolLaunching = compID.type == COMP_CHAMPIONSLEAGUE && compID.countryId == FifamCompRegion::Europe && compID.index == 0
        && !pool->IsLaunched();
    CallMethod<0x10F1A40>(pool);
    if (clPoolLaunching) {
        UChar numTeams = GetPoolNumberOfTeamsFromCountry(pool, 0, FifamCompRegion::Russia);
        if (numTeams == 0) {
            auto info = GetAssesmentTable()->GetInfoForCountry(FifamCompRegion::Russia);
            if (info)
                info->AddPoints(4.33f);
        }
    }
}

CDBLeague* OnGetLeagueForClubTableDevelopment(CCompID const &compID) {
    if (compID.type == COMP_RELEGATION)
        return nullptr;
    return GetLeague(compID);
}

CDBLeague* gCurrentLeagueGetTabXToY = nullptr;

CDBLeague * METHOD OnScriptGetTabXToY(CDBCompetition *comp) {
    gCurrentLeagueGetTabXToY = CallMethodAndReturn<CDBLeague*, 0xF84720>(comp);
    return gCurrentLeagueGetTabXToY;
}

CTeamIndex * METHOD OnGetTabXToYTeamLeaguePositionDataGetTeamID(void *data) {
    CTeamIndex* teamId = CallMethodAndReturn<CTeamIndex*, 0x14E7679>(data);
    if (gCurrentLeagueGetTabXToY) {
        CCompID compId = gCurrentLeagueGetTabXToY->GetCompID();
        if (compId.countryId == FifamCompRegion::United_States && compId.type == FifamCompType::Relegation) {
            if (compId.index == 0) { // East - top 12
                if (GetCurrentYear() == GetStartingYear()) {
                    Set<UInt> eastTeams = {
                        0x5F2149, // Atlanta United FC
                        0x5F3335, // Charlotte Football Club
                        0x5F221E, // FC Cincinnati
                        0x5F0002, // Columbus Crew
                        0x5F214B, // Inter Miami CF
                        0x5F340C, // Club de Foot Montreal
                        0x5F000A, // New England Revolution
                        0x5F1020, // New York City FC
                        0x5F0006, // New York Red Bulls
                        0x5F1001, // Orlando City SC
                        0x5F1003, // Philadelphia Union
                        0x5F0003, // D.C. United
                    };
                    CDBTeam *team = GetTeam(*teamId);
                    if (team && !Utils::Contains(eastTeams, team->GetTeamUniqueID()))
                        teamId = nullptr;
                }
                else {
                    CDBCompetition* pool = GetCompetition(FifamCompRegion::United_States, FifamCompType::Pool, 0);
                    if (pool) {
                        int teamNumber = pool->GetTeamIndex(*teamId);
                        if (teamNumber >= 12 && teamNumber <= 23)
                            teamId = nullptr;
                    }
                }
            }
            else if (compId.index == 1) { // West - bottom 12
                if (GetCurrentYear() == GetStartingYear()) {
                    Set<UInt> westTeams = {
                        0x5F0008, // FC Dallas
                        0x5F0004, // Sporting Kansas City
                        0x5F214A, // Los Angeles Football Club
                        0x5F1004, // Minnesota United FC
                        0x5F0012, // Portland Timbers
                        0x5F000B, // Real Salt Lake
                        0x5F0016, // Seattle Sounders FC
                        0x5F000D, // Vancouver Whitecaps FC
                        0x5F3421, // St. Louis City SC
                        0x5F0007, // Houston Dynamo FC
                        0x5F0024, // San Jose Earthquakes
                        0x5F2073, // Nashville SC
                    };
                    CDBTeam *team = GetTeam(*teamId);
                    if (team && !Utils::Contains(westTeams, team->GetTeamUniqueID()))
                        teamId = nullptr;
                }
                else {
                    CDBCompetition* pool = GetCompetition(FifamCompRegion::United_States, FifamCompType::Pool, 0);
                    if (pool) {
                        int teamNumber = pool->GetTeamIndex(*teamId);
                        if (teamNumber >= 0 && teamNumber <= 11)
                            teamId = nullptr;
                    }
                }
            }
        }
    }
    if (!teamId) {
        static CTeamIndex dummyTeamIndex_getTabXToY = CTeamIndex::make(0, 0, 0);
        return &dummyTeamIndex_getTabXToY;
    }
    return teamId;
}

UChar OnGetLevelWithReserveTeams(Int countryId) {
    if (countryId >= 1 && countryId <= 207 && gCountryReserveLevelId[countryId - 1] != 0)
        return gCountryReserveLevelId[countryId - 1] - 1;
    return 255;
}

unsigned int *METHOD IsCLELQuali(CDBRound *comp, DUMMY_ARG, unsigned int *id) {
    *id = 0;
    if (comp) {
        auto compId = comp->GetCompID();
        if (compId.countryId == FifamCompRegion::Europe) {
            if (compId.type == COMP_CHAMPIONSLEAGUE || compId.type == COMP_UEFA_CUP || compId.type == COMP_CONFERENCE_LEAGUE) {
                if (comp->GetRoundType() == ROUND_QUALI || comp->GetRoundType() == ROUND_QUALI2)
                    *id = 0xF9000000;
            }
        }
    }
    return id;
}

UChar gCupDrawCompType = 0;
UChar gCupDrawConferenceLeagueState[5] = {};
UInt gCupDrawConferenceLeagueRoundIDs[5] = { 10, 9, 12, 13, 14 };

void __declspec(naked) CupDraw_Clear() {
    __asm {
        mov     [esp + 0x20], eax
        lea     ebx, [eax + 4]
        mov     gCupDrawCompType, al
        mov     gCupDrawConferenceLeagueState[0], al
        mov     gCupDrawConferenceLeagueState[1], al
        mov     gCupDrawConferenceLeagueState[2], al
        mov     gCupDrawConferenceLeagueState[3], al
        mov     gCupDrawConferenceLeagueState[4], al
        mov     eax, 0xF909C5
        jmp     eax
    }
}

void __declspec(naked) CupDraw_Compare1() {
    __asm {
        mov     al, [esi + 0x1A]
        mov     gCupDrawCompType, al
        cmp     al, 9
        jz      JMP_F90AA4
        cmp     al, 10
        jz      JMP_F90AA4
        cmp     al, 51
        jz      JMP_F90AA4
        mov     eax, 0xF90C16
        jmp     eax
    JMP_F90AA4:
        mov     eax, 0xF90AA4
        jmp     eax
    }
}

void __declspec(naked) CupDraw_Group() {
    __asm {
        cmp     gCupDrawCompType, 51
        jnz     JMP_F90AA4
        mov     al, 1
        mov     gCupDrawConferenceLeagueState[0], al
    JMP_F90AA4:
        mov     cl, [esi + 0x1B]
        mov     [esp + 0x12], cl
        mov     eax, 0xF90AEC
        jmp     eax
    }
}

void __declspec(naked) CupDraw_Process() {
    __asm {
        cmp     esi, 5
        jb      JMP_F90CA2
        xor     esi, esi
    LOOP0:
        mov     al, 0
        cmp     gCupDrawConferenceLeagueState[esi], al
        jz      NEXT0
        mov     edx, gCupDrawConferenceLeagueRoundIDs[esi * TYPE int]
        push    1
        push    edx
        push    0xF9330000
        call    SetupCupDraw
        add     esp, 0xC
    NEXT0:
        inc     esi
        cmp     esi, 5
        jb      LOOP0
        mov     eax, 0xF90CC6
        jmp     eax
    JMP_F90CA2:
        mov     eax, 0xF90CA2
        jmp     eax
    }
}

void __declspec(naked) CupDraw_Compare2() {
    __asm {
        jz      JMP_F90B8B
        cmp     al, 51
        jnz     JMP_F90C16
    JMP_F90B8B:
        mov     eax, 0xF90B8B
        jmp     eax
    JMP_F90C16 :
        mov     eax, 0xF90C16
        jmp     eax
    }
}

void __declspec(naked) CupDraw_R1() {
    __asm {
        mov     al, 1
        cmp     gCupDrawCompType, 51
        jnz     SET_EL
        mov     gCupDrawConferenceLeagueState[1], al
        jmp     JMP_BACK
    SET_EL:
        mov     [esp + 0x35], al
    JMP_BACK:
        mov     eax, 0xF90BD8
        jmp     eax
    }
}

void __declspec(naked) CupDraw_R2() {
    __asm {
        mov     al, 1
        cmp     gCupDrawCompType, 51
        jnz     SET_EL
        mov     gCupDrawConferenceLeagueState[2], al
        jmp     JMP_BACK
    SET_EL:
        mov     [esp + 0x36], al
    JMP_BACK:
        mov     eax, 0xF90BED
        jmp     eax
    }
}

void __declspec(naked) CupDraw_R3() {
    __asm {
        mov     al, 1
        cmp     gCupDrawCompType, 51
        jnz     SET_EL
        mov     gCupDrawConferenceLeagueState[3], al
        jmp     JMP_BACK
    SET_EL:
        mov     [esp + 0x37], al
    JMP_BACK:
        mov     eax, 0xF90C02
        jmp     eax
    }
}

void __declspec(naked) CupDraw_R4() {
    __asm {
        mov     al, 1
        cmp     gCupDrawCompType, 51
        jnz     SET_EL
        mov     gCupDrawConferenceLeagueState[4], al
        jmp     JMP_BACK
    SET_EL:
        mov     [esp + 0x38], al
    JMP_BACK:
        mov     eax, 0xF90C16
        jmp     eax
    }
}

CCompID *METHOD OnGetIsCompetitionCupType(void *match, DUMMY_ARG, CCompID *out) {
    CallMethod<0xE80190>(match, out);
    if (out->type == COMP_CONFERENCE_LEAGUE || out->type == COMP_CONTINENTAL_1 || out->type == COMP_CONTINENTAL_2
        || out->type == COMP_EURO_NL || out->type == COMP_EURO_NL_Q || out->type == COMP_NAM_CUP || out->type == COMP_NAM_NL
        || out->type == COMP_NAM_NL_Q || out->type == COMP_ASIA_CUP || out->type == COMP_ASIA_CUP_Q || out->type == COMP_AFRICA_CUP
        || out->type == COMP_AFRICA_CUP_Q || out->type == COMP_OFC_CUP || out->type == COMP_OFC_CUP_Q || out->type == COMP_ICC
        || out->type == COMP_FINALISSIMA)
    {
        out->type = COMP_CHAMPIONSLEAGUE;
    }
    return out;
}

void __declspec(naked) AddUefaPointsOnMatchFinished() {
    __asm {
        cmp     al, 0xA // UEFA Europa League
        jz      JMP_121DFFB
        cmp     al, 0x33 // UEFA Conference League
        jz      JMP_121DFFB
        mov     eax, 0x121E19B
        jmp     eax
    JMP_121DFFB:
        mov     eax, 0x121DFFB
        jmp     eax
    }
}

void METHOD AddUEFAPointsOnCompetitionLaunch(CAssessmentTable* table, DUMMY_ARG, CCompID const& compId) {
    if (compId.countryId != FifamCompRegion::Europe)
        return;
    CDBCompetition* comp = GetCompetition(compId);
    if (!comp)
        return;
    if (comp->GetDbType() == DB_ROUND) {
        UChar type = compId.type;
        UInt rt = comp->GetRoundType();
        Float points = 0.0f;
        if (type == COMP_CHAMPIONSLEAGUE) {
            if (compId.ToInt() == 0xF909000A)
                points = 6.0f;
            else if (rt == ROUND_LAST_16 || rt == ROUND_QUARTERFINAL || rt == ROUND_SEMIFINAL || rt == ROUND_FINAL)
                points = 1.5f;
        }
        else if (type == COMP_UEFA_CUP) {
            if (rt == ROUND_LAST_16 || rt == ROUND_QUARTERFINAL || rt == ROUND_SEMIFINAL || rt == ROUND_FINAL)
                points = 1.0f;
        }
        else if (type == COMP_CONFERENCE_LEAGUE) {
            if (rt == ROUND_LAST_16 || rt == ROUND_QUARTERFINAL || rt == ROUND_SEMIFINAL || rt == ROUND_FINAL)
                points = 0.5f;
        }
        if (points > 0.0f) {
            for (UInt i = 0; i < comp->GetNumOfTeams(); i++) {
                UChar countryId = GetTeamCountryId_LiechtensteinCheck(comp->GetTeamID(i));
                if (countryId >= 1 && countryId <= 207)
                    table->GetInfoForCountry(countryId)->AddPoints(points);
            }
        }
    }
}

CTeamIndex& METHOD AddUEFAPoints_Get1stTeam(RoundPair *rp) {
    if (IsLiechtensteinClubFromSwitzerland(rp->m_n1stTeam)) {
        static CTeamIndex liechtensteinDummyTeam = CTeamIndex::make(FifamCompRegion::Liechtenstein, 0, 0);
        return liechtensteinDummyTeam;
    }
    return rp->m_n1stTeam;
}

CTeamIndex& METHOD AddUEFAPoints_Get2ndTeam(RoundPair* rp) {
    if (IsLiechtensteinClubFromSwitzerland(rp->m_n2ndTeam)) {
        static CTeamIndex liechtensteinDummyTeam = CTeamIndex::make(FifamCompRegion::Liechtenstein, 0, 0);
        return liechtensteinDummyTeam;
    }
    return rp->m_n2ndTeam;
}

Bool32 TeamIndexSort_UEFAPoints(CTeamIndex const &a, CTeamIndex const &b) {
    UChar country1 = GetTeamCountryId_LiechtensteinCheck(a);
    UChar country2 = GetTeamCountryId_LiechtensteinCheck(b);
    return GetAssesmentTable()->GetTotalPointsForCountry(country2) < GetAssesmentTable()->GetTotalPointsForCountry(country1);
}

CTeamIndex gParticipantsGetTeamIDTeamIndexForCheck;
CTeamIndex gParticipantsGetTeamIDTeamIndexForList;

CTeamIndex& METHOD ParticipantsGetTeamID_LiechtensteinCheck(CDBCompetition* comp, DUMMY_ARG, Int index) {
    gParticipantsGetTeamIDTeamIndexForList = comp->GetTeamID(index);
    gParticipantsGetTeamIDTeamIndexForCheck = gParticipantsGetTeamIDTeamIndexForList;
    gParticipantsGetTeamIDTeamIndexForCheck.countryId = GetTeamCountryId_LiechtensteinCheck(gParticipantsGetTeamIDTeamIndexForList);
    return gParticipantsGetTeamIDTeamIndexForCheck;
}

void METHOD ParticipantsListBoxAddTeamWidget(void* lb, DUMMY_ARG, CTeamIndex const& teamIndex) {
    CallMethod<0xD1E620>(lb, &gParticipantsGetTeamIDTeamIndexForList);
}

void METHOD ParticipantsListBoxAddTeamName(void* lb, DUMMY_ARG, CTeamIndex const& teamIndex, UInt color, UInt *unk) {
    CallMethod<0xD1F060>(lb, &gParticipantsGetTeamIDTeamIndexForList, color, unk);
}

UChar METHOD GetNumClubsInContinentalEuroCups(CAssessmentInfo *info) {
    UChar compTypes[] = { COMP_CHAMPIONSLEAGUE, COMP_UEFA_CUP, COMP_CONFERENCE_LEAGUE };
    UChar result = 0;
    for (auto type : compTypes) {
        auto pool = GetPool(FifamCompRegion::Europe, type, 0);
        if (pool) {
            for (UInt i = 0; i < pool->GetNumOfRegisteredTeams(); i++) {
                if (GetTeamCountryId_LiechtensteinCheck(pool->GetTeamID(i)) == info->m_nCountryIndex)
                    result++;
            }
        }
    }
    return result;
}

void __declspec(naked) OnCompareNumCompetitionTeams() {
    __asm {
        mov[esi + 0xA4], eax
        cmp[esi + 0xA8], ebp
        mov eax, 0xF91B58
        jmp eax
    }
}

Bool gDontTouchResult = false;

void METHOD OnFormatResult(void *t, DUMMY_ARG, FmVec<MatchGoalInfo> *vec, void *res) {
    gDontTouchResult = vec->empty();
    CallMethod<0xA8CE80>(t, vec, res);
    gDontTouchResult = false;
}

void __declspec(naked) OnClearFormatResult() {
    __asm {
        cmp gDontTouchResult, 0
        jne JMP_BACK
        mov byte ptr[ebp + 8], 0
        mov byte ptr[ebp + 9], 0
        JMP_BACK:
        cmp byte ptr[ebp + 0x10F], 0
            mov eax, 0xA8D126
            jmp eax
    }
}

RoundPair *roundPairForAggregateResult = nullptr;

Bool METHOD StoreRoundPairForAggregateResult(RoundPair *pair, DUMMY_ARG, Int flag) {
    Bool result = CallMethodAndReturn<Bool, 0x10ED420>(pair, flag);
    if (result)
        roundPairForAggregateResult = pair;
    else
        roundPairForAggregateResult = nullptr;
    return result;
}

void OnGetRoundAggregateResult(FmVec<MatchGoalInfo> *vec, Int unk, UChar *result) {
    if (vec->empty() && roundPairForAggregateResult) {
        result[0] = roundPairForAggregateResult->result1[0] + roundPairForAggregateResult->result1[1];
        result[1] = roundPairForAggregateResult->result2[0] + roundPairForAggregateResult->result2[1];
        if (result[1] > result[0])
            std::swap(result[0], result[1]);
        return;
    }
    Call<0x6E7700>(vec, unk, result);
}

void __declspec(naked) OnWidgetAllClubs() {
    __asm {
        cmp eax, 51
        mov[esp + 0x20], eax
        mov ecx, 0x9DD6C8
        jmp ecx
    }
}

UInt GetStadiumForContinentalCompetition(FifamContinent const &continent) {
    //SafeLog::WriteToFile(L"comp_hosts.txt", Utils::Format(L"Select stadium for competition %08X (%s)", gCompRound->GetCompID().ToInt(), gCompRound->GetName()));
    if (continent != FifamContinent::None) {
        // Step 1 - find excluded stadiums
        Set<String> excludedStadiums = GetExcludedStadiums(continent == FifamContinent::Europe);
        //if (!excludedStadiums.empty()) {
        //    SafeLog::WriteToFile(L"comp_hosts.txt", L"  Excluded stadiums:");
        //    for (auto const &s : excludedStadiums)
        //        SafeLog::WriteToFile(L"comp_hosts.txt", L"    " + s);
        //}
        // Step 2 - find all unique stadiums on continent
        Map<String, CDBTeam *> uniqueStadiums;
        for (UInt c = 1; c <= 207; c++) {
            CDBCountry *country = &GetCountryStore()->m_aCountries[c];
            if (country->GetContinent() == continent.ToInt()) {
                for (Int t = 1; t <= country->GetLastTeamIndex(); t++) {
                    auto team = GetTeam(CTeamIndex::make(c, FifamClubTeamType::First, t));
                    if (team) {
                        String id = StadiumStringId(team);
                        if (!id.empty() && !Utils::Contains(excludedStadiums, id))
                            uniqueStadiums[id] = team;
                    }
                }
                auto nationalTeam = GetTeam(CTeamIndex::make(c, FifamClubTeamType::First, 0xFFFF));
                if (nationalTeam) {
                    String id = StadiumStringId(nationalTeam);
                    if (!id.empty() && !Utils::Contains(excludedStadiums, id))
                        uniqueStadiums[id] = nationalTeam;
                }
            }
        }
        //SafeLog::WriteToFile(L"comp_hosts.txt", L"  Stadium list:");
        if (uniqueStadiums.empty())
            return 0;
        // Step 3 - create sorted list of stadiums
        Vector<CDBTeam *> stadiumsSorted;
        for (auto const &[s, team] : uniqueStadiums)
            stadiumsSorted.push_back(team);
        std::sort(stadiumsSorted.begin(), stadiumsSorted.end(), [](CDBTeam *a, CDBTeam *b) {
            return StadiumCapacity(a) > StadiumCapacity(b);
        });
        // Step 4 - select random stadium
        UInt range = 0;
        UInt minCapacity = GetMinCapacityForCompetitionFinal(gCompRound->GetCompID());
        if (StadiumCapacity(stadiumsSorted[0]) >= minCapacity) {
            for (UInt i = 1; i < stadiumsSorted.size(); i++) {
                if (StadiumCapacity(stadiumsSorted[i]) >= minCapacity)
                    range = i;
            }
        }
        else
            range = (UInt)(roundf((Float)(stadiumsSorted.size() - 1) * 0.05f)); // select from first 5%
        UInt result = (range > 0) ? Random::Get(0, range) : 0;
        //UInt ti = 0;
        //for (auto const &team : stadiumsSorted) {
        //    String symbol;
        //    if (ti == result)
        //        symbol = L"+";
        //    else
        //        symbol = L" ";
        //    if (ti <= range)
        //        SafeLog::WriteToFile(L"comp_hosts.txt", Utils::Format(L"    %u. ", ti + 1) + symbol + L" " + StadiumStringId(team) + L" (" + team->GetName() + L")");
        //    ti++;
        //}
        return stadiumsSorted[result]->GetTeamID().ToInt();
    }
    return 0;
}

FifamContinent ContinentFromRegion(UChar region) {
    switch (region) {
    case FifamCompRegion::Europe:
        return FifamContinent::Europe;
    case FifamCompRegion::SouthAmerica:
        return FifamContinent::SouthAmerica;
    case FifamCompRegion::NorthAmerica:
        return FifamContinent::NorthAmerica;
    case FifamCompRegion::Africa:
        return FifamContinent::Africa;
    case FifamCompRegion::Asia:
        return FifamContinent::Asia;
    case FifamCompRegion::Oceania:
        return FifamContinent::Oceania;
    }
    return FifamContinent::None;
}

UInt METHOD OnGetChampionsLeagueHost(void *hosts) {
    if (gCompRound && gCompRound->GetCompID().countryId != FifamCompRegion::Europe) {
        UInt stadium = GetStadiumForContinentalCompetition(ContinentFromRegion(gCompRound->GetCompID().countryId));
        if (stadium != 0)
            return stadium;
    }
    return CallMethodAndReturn<UInt, 0x117A560>(hosts);
}

UInt METHOD OnGetEuropaLeagueHost(void *hosts) {
    if (gCompRound && gCompRound->GetCompID().countryId != FifamCompRegion::Europe) {
        UInt stadium = GetStadiumForContinentalCompetition(ContinentFromRegion(gCompRound->GetCompID().countryId));
        if (stadium != 0)
            return stadium;
    }
    return CallMethodAndReturn<UInt, 0x117A570>(hosts);
}

UInt METHOD OnGetEuroSuperCupHost(void *hosts) {
    if (gCompRound && (gCompRound->GetCompetitionType() != COMP_EURO_SUPERCUP || gCompRound->GetCompID().countryId != FifamCompRegion::Europe)) {
        UInt stadium = GetStadiumForContinentalCompetition(ContinentFromRegion(gCompRound->GetCompID().countryId));
        if (stadium != 0)
            return stadium;
    }
    return CallMethodAndReturn<UInt, 0x1177730>(hosts);
}

UInt METHOD OnGetCompTypeEuroSuperCupForHost(CDBCompetition *comp) {
    if (*raw_ptr<UInt>(gCompRound, 0x2080) & 2)
        return 0;
    auto type = comp->GetCompetitionType();
    if (type == COMP_CONTINENTAL_1 || type == COMP_CONTINENTAL_2 || type == COMP_CONFERENCE_LEAGUE || type == COMP_YOUTH_CHAMPIONSLEAGUE)
        return COMP_EURO_SUPERCUP;
    return type;
}

UInt METHOD OnGetCompTypeChampionsLeagueForHost(CDBCompetition *comp) {
    if (comp->GetCompID().countryId != FifamCompRegion::Europe && *raw_ptr<UInt>(gCompRound, 0x2080) & 2)
        return 0;
    return comp->GetCompetitionType();
}

UInt METHOD OnGetCompTypeEuropaLeagueForHost(CDBCompetition *comp) {
    if (comp->GetCompID().countryId != FifamCompRegion::Europe && *raw_ptr<UInt>(gCompRound, 0x2080) & 2)
        return 0;
    return comp->GetCompetitionType();
}

Bool METHOD OnGetIsCountryInCompetitionHosts(void *t, DUMMY_ARG, CCompID *compId, UChar countryId) {
    if (countryId == FifamCompRegion::Russia)
        return true;
    return CallMethodAndReturn<bool, 0x1179ED0>(t, compId, countryId);
}

template<UInt Addr>
void METHOD OnSetupCompetitionOneMatch(CDBCompetition *comp, DUMMY_ARG, CDBOneMatch *match) {
    UInt numSubs = CallMethodAndReturn<UInt, 0xF81990>(comp);
    CallMethod<0xE812A0>(match, numSubs);
    CallMethod<Addr>(comp, match);
}

int METHOD OnSortPoolCountryStrength(CDBPool *pool) {
    if (pool->GetCompID().countryId == 255) {
        pool->SortTeamIDs(NationalTeamSorter);
        return 0;
    }
    return pool->GetNumOfRegisteredTeams();
}

void METHOD OnSortPoolDrawWorldCup(CDBPool *pool) {
    if (pool->GetNumOfTeams() == 32)
        CallMethod<0x10F36B0>(pool);
    else if (pool->GetNumOfTeams() == 48) {
        pool->SortTeamIDs(1, 47, NationalTeamSorter);
        pool->RandomlySortTeams(1, 11);
        pool->RandomlySortTeams(12, 12);
        pool->RandomlySortTeams(24, 12);
        pool->RandomlySortTeams(36, 12);
        pool->DumpToFile();
    }
}

void SetHostTeamForNextCopaAmerica() {
    UInt compID = 0xFF210000;
    Int rndIndex = GetRandomInt(std::size(SouthAmerica_ParticipantsCountries));
    void *hosts = CallAndReturn<void *, 0x117C830>();
    CallMethod<0x117C040>(hosts, &compID, GetCurrentYear() + 4, SouthAmerica_ParticipantsCountries[rndIndex], 0);
}

CDBRound *CopaAmericaGetQuarterFinal(UInt region, UInt compType, UShort index) {
    return GetRoundByRoundType(region, compType, ROUND_QUARTERFINAL);
}

CDBRound *CopaAmericaGetSemiFinal(UInt region, UInt compType, UShort index) {
    return GetRoundByRoundType(region, compType, ROUND_SEMIFINAL);
}

CDBRound *CopaAmericaGet3rdPlace(UInt region, UInt compType, UShort index) {
    return GetRoundByRoundType(region, compType, ROUND_FINAL_3RD_PLACE);
}

CDBRound *CopaAmericaGetFinal(UInt region, UInt compType, UShort index) {
    return GetRoundByRoundType(region, compType, ROUND_FINAL);
}

CDBCompetition *OnGetCopaAmericaCompetition(CCompID const &compId) {
    CDBCompetition *comp = GetCompetition(compId);
    if (comp && comp->GetDbType() == DB_ROUND) {
        UInt r = comp->GetRoundType();
        if (r != ROUND_QUARTERFINAL && r != ROUND_SEMIFINAL && r != ROUND_FINAL_3RD_PLACE && r != ROUND_FINAL)
            return nullptr;
    }
    return comp;
}

CDBRound *WorldCupGetFinal(UInt region, UInt compType, UShort index) {
    return GetRoundByRoundType(region, compType, ROUND_FINAL);
}

CDBRound *GetWorldClubCupFinalForTransitionScreen(CCompID const &compID) {
    return GetRoundByRoundType(FifamCompRegion::Europe, COMP_TOYOTA, ROUND_FINAL);
}

CDBRound *gMyDBRound_RegisterMatch_Round = nullptr;

void METHOD MyDBRound_RegisterMatch(CDBRound *round, DUMMY_ARG, Int eventStartIndex, CDBOneMatch *match) {
    gMyDBRound_RegisterMatch_Round = round;
    CallMethod<0x1043BC0>(round, eventStartIndex, match);
    RoundPair const &pair = round->GetRoundPair(match->GetRoundPairIndex());
    if (pair.IsFinished()) {
        if (IsUEFALeaguePhaseMatchdayCompID(round->GetCompID()))
            UEFALeaguePhaseMatchdayProcessBonuses(round, pair);
        else {
            CTeamIndex loserID = pair.GetLoser();
            if (!loserID.isNull()) {
                CDBTeam *loserTeam = GetTeam(loserID);
                if (loserTeam) {
                    EAGMoney bonus = round->GetBonus(2);
                    if (bonus != 0) {
                        loserTeam->ChangeMoney(5, bonus, 0);
                        CEAMailData mailData;
                        mailData.SetMoney(bonus);
                        loserTeam->SendMail(3441, mailData, 1); // As a loser, you earned _MONEY in the previous cup match.
                        SafeLog::Write(Utils::Format(L"%s: team %s loser bonus - %I64d", CompetitionName(round), TeamName(loserTeam), bonus));
                    }
                }
            }
        }
    }
    gMyDBRound_RegisterMatch_Round = nullptr;
}

void PatchCompetitions(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectJump(0xF909BE, CupDraw_Clear);
        patch::RedirectJump(0xF90A95, CupDraw_Compare1);
        patch::RedirectJump(0xF90AE5, CupDraw_Group);
        patch::RedirectJump(0xF90CC1, CupDraw_Process);
        patch::SetUInt(0xF90BCA + 3, 16);
        patch::RedirectJump(0xF90B85, CupDraw_Compare2);
        patch::RedirectJump(0xF90BD3, CupDraw_R1);
        patch::RedirectJump(0xF90BE8, CupDraw_R2);
        patch::RedirectJump(0xF90BFD, CupDraw_R3);
        patch::RedirectJump(0xF90C11, CupDraw_R4);

        patch::RedirectCall(0xAC9F8E, OnGetIsCompetitionCupType);

        patch::RedirectJump(0x121DFF3, AddUefaPointsOnMatchFinished);
        patch::RedirectJump(0x121E380, AddUEFAPointsOnCompetitionLaunch);
        patch::RedirectCall(0x121E0EC, AddUEFAPoints_Get1stTeam);
        patch::RedirectCall(0x121E139, AddUEFAPoints_Get1stTeam);
        patch::RedirectCall(0x121E159, AddUEFAPoints_Get1stTeam);
        patch::RedirectCall(0x121E10E, AddUEFAPoints_Get2ndTeam);
        patch::RedirectCall(0x121E124, AddUEFAPoints_Get2ndTeam);
        patch::RedirectCall(0x121E17B, AddUEFAPoints_Get2ndTeam);
        patch::SetPointer(0xA3BB9D + 1, TeamIndexSort_UEFAPoints); // 08NewspaperChampionsLeague screen
        patch::RedirectCall(0x88F784, ParticipantsGetTeamID_LiechtensteinCheck);
        patch::RedirectCall(0x88F7F3, ParticipantsListBoxAddTeamWidget);
        patch::RedirectCall(0x88F805, ParticipantsListBoxAddTeamName);

        // round flags (away goals rule)
        patch::SetUChar(0x104903F + 4, 8); // CDBCup::RegisterMatch
        patch::SetUChar(0x104904E + 4, 8); // CDBCup::RegisterMatch
        patch::SetUChar(0x104CD35 + 4, 8); // CDBCup::DrawTeams
        patch::SetUChar(0x104CD22 + 4, 8); // CDBCup::DrawTeams
        patch::SetUChar(0x104CF0B + 5, 8); // CDBCup::DrawTeams
        patch::SetUChar(0x1043D95 + 4, 8); // CDBRound::RegisterMatch
        patch::SetUChar(0x104238A + 4, 8); // CDBRound::Launch
        patch::SetUChar(0x104239D + 4, 8); // CDBRound::Launch
        patch::SetUChar(0x10423B6 + 4, 8); // CDBRound::Enable2ndLeg?
        patch::SetUChar(0x1044F9C + 4, 8); // CDBRound::Launch

        // penalties score
        patch::Nop(0xA91467, 4);
        patch::Nop(0xA91476, 4);
        patch::Nop(0xA90440, 4);
        patch::Nop(0xA9044F, 4);
        patch::Nop(0xA8FFB6, 4);
        patch::Nop(0xA8FFC5, 4);
        patch::Nop(0xA8E613, 4);
        patch::Nop(0xA8E622, 4);
        patch::Nop(0xA8E0E0, 4);
        patch::Nop(0xA8E0EF, 4);

        // result for statistics
        patch::RedirectCall(0xA8FF5E, OnFormatResult);
        patch::RedirectCall(0xA9032E, OnFormatResult);
        patch::RedirectJump(0xA8D117, OnClearFormatResult);
        patch::RedirectCall(0xA901DA, StoreRoundPairForAggregateResult);
        patch::RedirectCall(0xA90377, OnGetRoundAggregateResult);

        ReadLeaguesConfig();

        //

        // Scotland
        //patch::SetUChar(0xEE2983 + 6, 245);
        //patch::SetUChar(0xF3FC9C + 1, 245);
        //patch::SetUChar(0x1067F5E + 1, 245);
        //patch::SetUChar(0x10680BF + 1, 245);
        //patch::SetUChar(0x10682F1 + 3, 245); // not finished
        //patch::SetUChar(0x106C451 + 1, 245);
        //patch::SetUChar(0x62BF8F + 3, 245);
        //patch::SetUChar(0x65400E + 3, 245);
        //patch::SetUChar(0xAA1C25 + 1, 245);
        //patch::SetUChar(0xE3F56D + 1, 245);
        //patch::SetUChar(0xEA109D + 1, 245);
        //patch::SetUChar(0xF6A9FD + 3, 245);
        //patch::SetUChar(0xF6CD78 + 3, 245);
        //patch::SetUChar(0xFF5EA2 + 3, 245);
        //patch::SetUChar(0xFF5F4B + 3, 245);

        patch::Nop(0x1068686, 2); // not sure

        patch::RedirectCall(0x1067F59, LeagueSplit_GetCompetitionCountryId);
        patch::RedirectCall(0x10680BA, LeagueSplit_GetCompetitionCountryId);
        patch::RedirectCall(0x10682EC, LeagueSplit_GetTeamCountryId);
        
        patch::RedirectCall(0x10680E6, LeagueSplit_GetCompId_Championship);
        
        patch::RedirectCall(0x1068311, LeagueSplit_sub_ECCC90_Champ);
        patch::RedirectCall(0x1068338, LeagueSplit_sub_ECCC90_Relegation);
        
        patch::RedirectCall(0x10682E5, LeagueSplit_GetCompId_Universal);
        
        patch::RedirectJump(0xEE2983, LeagueSplit_Cmp_CountryId);
        patch::RedirectCall(0xEE29D9, LeagueSplit_GetLeague_TitleRace);

        patch::RedirectCall(0xF3FC95, LeagueSplit_GetCompetitionCountryId);
        
        patch::RedirectCall(0x106C447, LeagueSplit_GetCompetitionCountryId);
        patch::RedirectCall(0x106C45C, LeagueSplit_GetCompId_League);
        patch::RedirectCall(0x106C470, LeagueSplit_GetCompId_Championship_StoreCountryId);
        patch::RedirectCall(0x106C482, LeagueSplit_GetLeagueFromStoredCountry);
        
        patch::RedirectCall(0x62BF8A, LeagueSplit_GetTeamCountryId);
        
        patch::RedirectCall(0x654009, LeagueSplit_GetTeamCountryId);
        
        patch::RedirectCall(0xAA1C20, LeagueSplit_GetCompetitionCountryId);
        patch::RedirectCall(0xAA1C2B, reinterpret_cast<void *>(0xF81C50));
        patch::SetUChar(0xAA1C30 + 2, FifamCompType::Relegation);
        
        patch::RedirectCall(0xE3F568, LeagueSplit_GetCompetitionCountryId);
        patch::RedirectCall(0xE3F573, reinterpret_cast<void *>(0xF81C50));
        patch::SetUChar(0xE3F578 + 2, FifamCompType::Relegation);
        
        patch::RedirectCall(0xEA1098, LeagueSplit_GetCompetitionCountryId);
        
        patch::RedirectCall(0xF6A9F8, LeagueSplit_GetTeamCountryId);
        
        patch::RedirectCall(0xF6CD73, LeagueSplit_GetTeamCountryId_StoreCountryId);
        patch::RedirectCall(0xF6CD84, LeagueSplit_GetLeagueFromStoredCountry_Team);
        
        patch::RedirectCall(0xFF5E9D, LeagueSplit_GetTeamCountryId);
        
        patch::RedirectCall(0xFF5F46, LeagueSplit_GetTeamCountryId);


        patch::RedirectCall(0x106336B, LeagueSplit_GetCompDbType);

        patch::RedirectCall(0x11F546E, OnGetLeagueCupToLaunch);

        //


        patch::RedirectCall(0x1044621, OnScriptProcess);
        patch::RedirectCall(0x104D7E5, OnScriptProcess);
        patch::RedirectCall(0x106B9CE, OnScriptProcess);
        patch::RedirectCall(0x10F1A56, OnScriptProcess);

        patch::RedirectCall(0x139EA9C, GetTeamInitTeamByID);

        patch::RedirectCall(0x10F47CB, ChampionsLeagueUniversalSort);

        patch::SetPointer(0x24B1874, GetPoolNumberOfTeamsFromCountry);
        //patch::SetUChar(0x10F109C + 3, 240);
        //patch::SetUChar(0x10F10E3 + 3, 240);

        patch::RedirectCall(0x88F9DD, OnFillEuropeanCompsParticipants);
        patch::RedirectCall(0x88F9ED, OnFillEuropeanCompsParticipants);
        //
        patch::RedirectCall(0x88F73C, EuropeanCompsParticipants_GetNumEntries);
        patch::RedirectCall(0x88F85D, EuropeanCompsParticipants_GetNumEntries);
        patch::RedirectCall(0x88F75D, EuropeanCompsParticipants_GetCountryAtPosition);
        //patch::RedirectCall(0x88F6F6, EuropeanCompsParticipants_GetPool);

        patch::SetUChar(0x88F814 + 1, 8); // max teams
        patch::RedirectCall(0x88FA6F, EuropeanCompsParticipantsInitColumns);

        patch::RedirectCall(0xFF203B, OnSetupAssessmentEntry);
        patch::RedirectCall(0x139D4A2, GetCountryAtAssessmentPositionLastYear<0>);
        patch::RedirectCall(0x139DA74, GetCountryAtAssessmentPositionLastYear<0>);
        patch::RedirectCall(0x139EFC8, GetCountryAtAssessmentPositionLastYear<255>);

        patch::RedirectCall(0x139EFAF, OnGetSpare);

        patch::RedirectCall(0xF97A3A, OnGetGameInstanceSetupCompetitionWinners);

        // Youth CL
        
        patch::RedirectCall(0x139E097, OnGetCompetition_GET_CHAMP);
        patch::RedirectCall(0x139E1B8, OnGetCompetition_GET_CHAMP_OR_RUNNER_UP);
        patch::RedirectCall(0x139E2F8, OnGetCompetition_GET_RUNNER_UP);
        /*
        patch::RedirectCall(0x139E0AD, OnGetChampion_GET_CHAMP);
        patch::RedirectCall(0x139E1CF, OnGetChampion_GET_CHAMP);
        patch::RedirectCall(0x139E1F4, OnGetRunnerUp_GET_RUNNER_UP);
        patch::RedirectCall(0x139E30F, OnGetRunnerUp_GET_RUNNER_UP);

        patch::RedirectCall(0x11F281F, OnSetupRootSetLeagueChampionTeam);

        patch::RedirectCall(0x11F402F, OnProcessRootGetCountryId);
        */
        
        //patch::SetUChar(0x23D4A84, 36);
        patch::SetPointer(0x5EF7D0 + 2, gInternationalCompsTypes);
        patch::SetUChar(0x5F03ED + 2, (UChar)(std::size(gInternationalCompsTypes) * 4));
        // B8 64 00 00 00
        patch::SetUInt(0x5EF7FC, 0x000064B8);
        patch::SetUChar(0x5EF7FC + 4, 0);
        patch::Nop(0x5EF7FC + 5, 2);

        patch::SetPointer(0xF815C8 + 1, gNewCompTypeNames);
        patch::SetPointer(0xF87462 + 1, gNewCompTypeNames);
        // comp name
        patch::RedirectCall(0xF9793F, OnSetCompetitionName);
        patch::RedirectCall(0xF97033, OnSetupRootInternational);
        patch::RedirectCall(0xF96D42, OnSetupRootContinentalEurope);
        patch::RedirectCall(0xF96D4D, OnSetupRootContinentalSouthAmerica);
        // comp types
        patch::SetPointer(0x11F2CFE + 2, gContinentalCompetitionTypes);
        patch::SetPointer(0x11F2D4F + 2, gContinentalCompetitionTypes);
        patch::SetUChar(0x11F2D6E + 2, (UChar)(std::size(gContinentalCompetitionTypes) * 4));

        // CStatsCupFixturesResults
        patch::SetUChar(0x703659 + 2, gNumCompetitionTypes - 1);
        patch::SetUChar(0x703592, 0xEB);
        patch::SetUChar(0x7035D6, 0xEB);
        patch::SetUChar(0x703515, 0xEB);
        patch::SetUChar(0x703564 + 2, gNumCompetitionTypes - 1);
        //patch::SetUChar(0x703610 + 2, COMP_INTERNATIONAL_FRIENDLY);

        // CStatsCupResults
        patch::SetUChar(0x704F59 + 2, gNumCompetitionTypes - 1);
        patch::SetUChar(0x704E4E, 0xEB);
        patch::SetUChar(0x704E92, 0xEB);
        patch::SetUChar(0x704ED6, 0xEB);
        patch::Nop(0x704DFB, 2);
        patch::Nop(0x704E00, 2);
        patch::SetUChar(0x704E25 + 2, gNumCompetitionTypes - 1);
        //patch::SetUChar(0x704F10 + 2, COMP_INTERNATIONAL_FRIENDLY);
        
        // CStatsCupTablesFinals
        patch::SetUChar(0x7061CE + 2, gNumCompetitionTypes - 1);
        //patch::SetUChar(0x7060A5 + 2, COMP_INTERNATIONAL_FRIENDLY);

        // standings
        patch::SetUChar(0xE3EBC1 + 2, gNumCompetitionTypes - 1);

        // unknown - weekly progress?
        patch::SetUChar(0x9776D1 + 2, gNumCompetitionTypes - 1);

        //patch::RedirectCall(0x977635, Comps1);
        //patch::Nop(0x97764C, 5);
        //patch::Nop(0x977624, 2);

        // matchday cup results
        patch::SetUChar(0xAA115E + 2, gNumCompetitionTypes - 1);

        patch::SetUChar(0xAA1008, 0xEB);
        patch::SetUChar(0xAA1077, 0xEB);
        patch::SetUChar(0xAA0FD7 + 2, gNumCompetitionTypes - 1);
        //patch::SetUChar(0xAA10E0 + 2, COMP_INTERNATIONAL_FRIENDLY);

        // next matchday info?
        patch::SetUChar(0xACE9CE + 2, gNumCompetitionTypes - 1);
        //patch::SetUChar(0xACE950 + 2, COMP_INTERNATIONAL_FRIENDLY);

        patch::RedirectJump(0x11F4ADC, LaunchCompetitions_Exec);
        patch::RedirectJump(0xF83180, TakesPlaceInThisYear);

        // fixture list
        patch::RedirectCall(0x96A3EF, SetupFixturesList);
        patch::RedirectJump(0x96A6B6, CheckFixtureList_1_Exec);
        patch::RedirectJump(0x96AAB5, CheckFixtureList_2_Exec);
        patch::SetUInt(0x96A4B6 + 4, std::size(gCompTypesClubYouth)); // Youth clubs
        patch::SetUInt(0x96A4D1 + 4, std::size(gCompTypesClub)); // Clubs
        patch::SetUInt(0x96A431 + 1, std::size(gCompTypesNationalTeam)); // National teams
        
        // competition short names
        patch::RedirectJump(0xF8B3E0, GetCompetitionShortName);

        patch::SetUChar(0xEFBCAF + 2, gNumCompetitionTypes - 4);

        // scouting
        patch::SetUChar(0x5F83CA + 2, gNumCompetitionTypes);

        //country id - 0x61727F - not sure

        // trophy stickers
        patch::SetUChar(0x129D807 + 2, gNumCompetitionTypes);
        patch::SetUChar(0x129D733 + 2, COMP_YOUTH_CHAMPIONSLEAGUE);
        patch::SetUInt(0x129D6F1 + 4, 0xFF210000);
        patch::RedirectCall(0x129D70D, OnAddTrophyStickerCompID);

        // club trophies
        patch::RedirectCall(0x669EAE, OnAddTrophyClubTrophiesScreen_Continental);
        patch::RedirectCall(0x669F19, OnAddTrophyClubTrophiesScreen_Other);

                                               // 3  4  5   6   7  8   9  10 11 12 13 14 15  16  17 18 19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35 36  37  38  39  40  41  42  43  44  45  46  47  48  49  50  51  52  53  54  55  56  57  58  59  60  61  62
        static UChar gCompWinYearsJumpTable[] = { 0, 1, 11, 11, 2, 11, 3, 4, 5, 6, 7, 5, 11, 11, 8, 9, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 5, 11, 10, 11,  5,  5, 11, 10, 10, 11, 10, 11, 10, 11, 10, 11,  5, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };
        patch::SetPointer(0xEFBCB8 + 3, gCompWinYearsJumpTable);
        patch::RedirectCall(0x7DA52D, OnAddMyCareerTrophyNationalTeam);
        patch::RedirectCall(0x7DA4DC, OnAddMyCareerTrophyContinental);

        // international goals
        patch::RedirectJump(0xF8DF90, GetCompetitionGroupByCompetitionId);

        // tournament status (groups)
        static UInt gTournamentStatusGroupsComps[] = { 17, 18, 32, 33, COMP_NAM_CUP,
             COMP_AFRICA_CUP, COMP_ASIA_CUP, COMP_OFC_CUP, COMP_U20_WORLD_CUP };
        patch::SetPointer(0x844334 + 2, gTournamentStatusGroupsComps);
        patch::SetUChar(0x8443A0 + 2, (UChar)std::size(gTournamentStatusGroupsComps) * 4);

        // tournament status (fixtures and tables)
        static UInt gTournamentStatusFixturesAndTablesComps[] = { 17, 18, 32, 33, COMP_EURO_NL, COMP_NAM_NL, COMP_NAM_CUP,
            COMP_AFRICA_CUP, COMP_ASIA_CUP, COMP_OFC_CUP, COMP_FINALISSIMA, COMP_U20_WORLD_CUP };
        patch::SetPointer(0x837760 + 3, gTournamentStatusFixturesAndTablesComps);
        patch::SetPointer(0x8377A8 + 3, gTournamentStatusFixturesAndTablesComps);
        patch::SetUChar(0x83776A + 2, (UChar)std::size(gTournamentStatusFixturesAndTablesComps));
        patch::SetUChar(0x837785 + 2, (UChar)std::size(gTournamentStatusFixturesAndTablesComps));
        patch::SetUInt(0x83778E, 0x3C24448B);
        //patch::SetUInt(0x837738 + 4, 120);

        // international hosts - national team (round)
        patch::RedirectCall(0x1044950, GetCompetitionTypeForNationalTeamHost);

        // Euro
        const UInt NumEuroTeams = 24;
        const UInt NumEuroGroups = 6;

        // Euro - welcome
        patch::SetUChar(0x836409 + 2, NumEuroTeams - 1);
        patch::SetUInt(0x835E8F + 4, NumEuroGroups);
        patch::SetChar(0x8359D1 + 2, NumEuroTeams - 18);

        // Euro - screens
        patch::RedirectJump(0x829880, GetNationalTeamECQualifiedTeamsScreenName);
        patch::RedirectJump(0x835590, GetNationalTeamTournamentWelcomeScreenName);
        patch::RedirectJump(0x831DE0, GetNationalTeamTournamentGroupsScreenName);
        patch::RedirectJump(0x8300E0, GetNationalTeamECTournamentFinalsScreenName);
        patch::RedirectJump(0x832940, GetNationalTeamTournamentReportScreenName);
        patch::RedirectJump(0x82E9E0, GetNationalTeamTournamentDreamTeamScreenName);
        patch::RedirectJump(0xE5F100, GetNationalTeamTournamentECUpsAndDownsScreenName);
        patch::RedirectJump(0x82A190, GetNationalTeamWCQualifiedTeams2ScreenName);
        patch::RedirectJump(0x82AB20, GetNationalTeamWCQualifiedTeams4ScreenName);
        patch::RedirectJump(0x830D50, GetNationalTeamWCTournamentFinalsScreenName);
        patch::RedirectJump(0xE5F020, GetNationalTeamTournamentWCUpsAndDownsScreenName);

        patch::RedirectCall(0x106D2F3, GetECLast16Round);
        patch::RedirectCall(0x106D571, GetECLast16Round);

        patch::RedirectCall(0x49B747, OnCreateNationalTeamTournamentFinalsEC);
        patch::SetUInt(0x49B714 + 1, 0x594 + sizeof(ECFinalsAdditionalData));
        patch::SetUInt(0x49B71B + 1, 0x594 + sizeof(ECFinalsAdditionalData));
        patch::SetUInt(0x830190 + 1, 1);
        patch::SetUInt(0x830184 + 2, 0x594 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x8301CA + 1, 1);
        patch::SetUInt(0x8301CF + 2, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUInt(0x830201 + 1, 30);
        patch::SetUInt(0x83021F + 2, 0x594 + 3 * 4 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x830225 + 1, 15);
        patch::SetUInt(0x830249 + 2, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUInt(0x83024F + 1, 30);

        patch::SetUInt(0x830AEC + 2, 0x594 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUChar(0x830B04 + 2, 60);
        patch::SetUInt(0x830B0B + 2, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUChar(0x830B23 + 2, 30);
        patch::SetUInt(0x830B3A + 3, 0x594 + offsetof(ECFinalsAdditionalData, mTeamIDs) + 2);
        patch::SetUInt(0x830B5F + 3, 0x594 + offsetof(ECFinalsAdditionalData, mTeamIDs) + 2);
        patch::SetUInt(0x830B44 + 3, 0x594 + offsetof(ECFinalsAdditionalData, mTeamIDs));
        patch::SetUInt(0x830B69 + 3, 0x594 + offsetof(ECFinalsAdditionalData, mTeamIDs));
        
        patch::RedirectCall(0x8309BF, GetECQuarterfinal);
        patch::RedirectCall(0x8309DE, GetECSemifinal);
        patch::RedirectCall(0x8309FD, GetECFinal);
        patch::RedirectCall(0x8309D0, OnSetupTeamsForECQuarterfinal);
        patch::SetUChar(0x8309CC + 1, 1);
        patch::SetUChar(0x8309EB + 1, 2);
        patch::SetUChar(0x830A0C + 1, 3);
        
        patch::SetUInt(0x8304CA + 2, 0x594 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x8304D0 + 2, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUInt(0x83052C + 3, 0x594 + 4 + offsetof(ECFinalsAdditionalData, mTeamIDs));
        patch::SetUInt(0x8304E2 + 2, 0x594 + 32 * 4 + offsetof(ECFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x8304E8 + 2, 0x594 + 16 * 4 + offsetof(ECFinalsAdditionalData, mFlags));
        patch::SetUInt(0x8304EE + 4, 16);
        patch::RedirectJump(0x8304F8, ECSetupTeamsForFinal);

        patch::SetUInt(0x830455 + 3, 0x594 + offsetof(ECFinalsAdditionalData, mFlags));

        patch::RedirectCall(0x835EC9, OnGetLeagueTournamentWelcome);

        // WC Quali
        patch::RedirectCall(0x106D68C, OnWCQualiGetNumOfRegisteredTeams_FirstLastPlace);

        // World Cup
        const UInt NumWorldCupTeams = 48;
        const UInt NumWorldCupGroups = 12;
        patch::RedirectCall(0x108B186, OnSortPoolDrawWorldCup);
        patch::RedirectCall(0x10F47E6, OnSortPoolDrawWorldCup);
        patch::RedirectCall(0x106D92A, GetWCLast32Round);
        patch::RedirectCall(0x106DB0C, GetWCLast32Round);
        patch::RedirectCall(0x83FB8A, GetWCFinal);

        // World Cup - welcome
        patch::SetUChar(0x8364A8 + 2, NumWorldCupTeams - 1);
        patch::SetUInt(0x835E9E + 4, NumWorldCupGroups);
        patch::SetUInt(0x8359DB + 1, NumWorldCupTeams);
        patch::SetUInt(0x49C0A4 + 1, 0x638 + sizeof(NTWelcomeAdditionalData));
        patch::SetUInt(0x49C0AB + 1, 0x638 + sizeof(NTWelcomeAdditionalData));
        patch::SetUInt(0x8364A2 + 2, 0x638 + offsetof(NTWelcomeAdditionalData, mFlags));
        patch::RedirectJump(0x8364CF, OnNTWelcome1);
        patch::SetUInt(0x836403 + 2, 0x638 + offsetof(NTWelcomeAdditionalData, mFlags));
        patch::RedirectJump(0x83642F, OnNTWelcome2);
        patch::SetUInt(0x835F2B + 3, 0x638 + offsetof(NTWelcomeAdditionalData, mTextBoxes));
        patch::SetUInt(0x835F88 + 3, 0x638 + offsetof(NTWelcomeAdditionalData, mCountryIDs));
        patch::SetUInt(0x836011 + 3, 0x638 + offsetof(NTWelcomeAdditionalData, mFlags));
        patch::SetUInt(0x8359ED + 2, 0x638 + offsetof(NTWelcomeAdditionalData, mFlags));
        patch::RedirectJump(0x8359F6, OnNTWelcome3);
        patch::SetUInt(0x835A11 + 2, sizeof(NTWelcomeAdditionalData::mFlags));

        // World Cup - groups
        patch::RedirectJump(0x8321A0, NationalTeamTournamentGroupsButtonReleased);

        // World Cup - finals
        patch::RedirectCall(0x49B637, OnCreateNationalTeamTournamentFinalsWC);
        patch::SetUInt(0x49B604 + 1, 0x6C0 + sizeof(WCFinalsAdditionalData));
        patch::SetUInt(0x49B60B + 1, 0x6C0 + sizeof(WCFinalsAdditionalData));
        patch::SetUInt(0x830F1B + 2, 0x6C0 + offsetof(WCFinalsAdditionalData, mTextBoxes));
        patch::RedirectJump(0x830F56, OnWCFinals1);
        patch::RedirectJump(0x831017, OnWCFinals2);
        patch::SetUInt(0x831024 + 2, 0x6C0 + offsetof(WCFinalsAdditionalData, mFlags));
        patch::SetUChar(0x831063 + 2, 64);
        patch::SetUInt(0x83106A + 1, 64);
        patch::SetUInt(0x831089 + 2, 0x6C0 + offsetof(WCFinalsAdditionalData, mTextBoxes) + 3 * 4);
        patch::SetUInt(0x83108F + 1, 32);
        patch::SetUInt(0x8310AD + 2, 0x6C0 + offsetof(WCFinalsAdditionalData, mFlags));
        patch::SetUInt(0x8310B3 + 1, 64);
        patch::Nop(0x831AC6, 13);
        patch::RedirectCall(0x831AD7, OnWCFinalsSetupRound);
        patch::SetPointer(0x241131C, OnWCFinalsSetupClick);
        patch::SetUChar(0x831AE0 + 2, 6);
        patch::RedirectCall(0x831B68, OnWCFinalsGetWCFinal);
        patch::SetUInt(0x831507 + 2, 0x6C0 + offsetof(WCFinalsAdditionalData, mTextBoxes));
        patch::SetUInt(0x83150D + 2, 0x6C0 + offsetof(WCFinalsAdditionalData, mFlags));
        patch::SetUInt(0x831589 + 3, 0x6C0 + offsetof(WCFinalsAdditionalData, mTeamIDs) + 4);
        patch::SetUInt(0x83135C + 2, 0x6C0 + offsetof(WCFinalsAdditionalData, mTextBoxes));
        patch::RedirectJump(0x831375, OnWCFinals3);
        patch::SetUInt(0x831417 + 3, offsetof(WCFinalsAdditionalData, mTeamIDs) + 2);
        patch::SetUInt(0x831421 + 3, offsetof(WCFinalsAdditionalData, mTeamIDs));
        patch::SetUInt(0x831448 + 3, offsetof(WCFinalsAdditionalData, mTeamIDs) + 2);
        patch::SetUInt(0x831452 + 3, offsetof(WCFinalsAdditionalData, mTeamIDs));
        patch::SetUInt(0x83137F + 2, 0x6C0 + offsetof(WCFinalsAdditionalData, mFlags));
        patch::SetUChar(0x83139B + 2, 64);
        patch::SetUInt(0x831495 + 3, 0x6C0 + offsetof(WCFinalsAdditionalData, mFlags));
        patch::SetUChar(0x8314AA + 2, 64);

        patch::RedirectCall(0x11F2B93, OnSetupRootInternationalComps);

        // translated competition name
        patch::RedirectCall(0x9A3EB6, ReadCompetitionName);
        patch::RedirectCall(0x1042116, ReadCompetitionName);
        patch::RedirectCall(0x1056196, ReadCompetitionName);

        // Champions League IDs - misc former cup opponents
        patch::SetUInt(0x897627 + 6, 0xF909000B);
        patch::SetUInt(0x897631 + 6, 0xF909000C);
        patch::SetUInt(0x89763B + 6, 0xF909000D);
        patch::SetUInt(0x897645 + 6, 0xF909000E);
        patch::SetUInt(0x89764F + 6, 0xF909000F);
        patch::SetUInt(0x897659 + 6, 0xF9090010);
        patch::SetUInt(0x897663 + 6, 0xF9090011);
        patch::SetUInt(0x89766D + 6, 0xF9090012);
        patch::SetUInt(0x8976B4 + 2, 0xF9090015);
        patch::SetUInt(0x8976D1 + 2, 0xF9090016);
        patch::SetUInt(0x8976FF + 2, 0xF9090017);

        // CL/EL Quali round pairs sorting
        patch::SetUInt(0x10439F1 + 2, 0xF9000000);
        patch::SetUInt(0x1043A06 + 2, 0xF9000000);
        patch::SetUInt(0x1043A1D + 2, 0xF9000000);
        patch::SetUInt(0x1043A34 + 2, 0xF9000000);
        patch::SetUInt(0x1043A4B + 2, 0xF9000000);
        patch::SetUInt(0x1043A62 + 2, 0xF9000000);
        patch::SetUInt(0x1043A79 + 2, 0xF9000000);
        patch::SetUInt(0x1043A90 + 2, 0xF9000000);
        patch::SetUInt(0x1043AA7 + 2, 0xF9000000);
        patch::RedirectCall(0x10439EC, IsCLELQuali);

        // Marketing pool for Conference League? TODO
        patch::SetUInt(0x10454AD + 2, 0xF9090017);

        patch::SetUInt(0x10F17B2 + 2, 0xF9090012);

        patch::SetUInt(0x10F1B51 + 2, 0xF9090008);

        // DONE: add Conference League
        patch::SetUInt(0x1132451 + 6, 0xF909000A);
        patch::SetUInt(0x113245B + 6, 0xF90A0008);
        patch::SetUInt(0x1132465 + 6, 0xF933000C);

        patch::SetUChar(0x10F28F4 + 1, 11);
        patch::SetUChar(0x10F2902 + 1, 12);
        patch::SetUChar(0x10F2917 + 1, 13);
        patch::SetUChar(0x10F292C + 1, 14);
        patch::SetUChar(0x10F2941 + 1, 15);
        patch::SetUChar(0x10F2956 + 1, 16);
        patch::SetUChar(0x10F296E + 1, 17);
        patch::SetUChar(0x10F2983 + 1, 18);

        patch::SetUInt(0x137A118 + 1, 10);
        patch::SetUChar(0x137A595 + 2, 23);

        patch::SetUChar(0x11F172C + 2, 21);
        patch::SetUChar(0x11F1731 + 1, 21);

        patch::SetUInt(0x10F4314 + 2, 0xF90AFFFF);

        patch::SetUInt(0x897857 + 6, 0xF90A0005);
        patch::SetUInt(0x897861 + 6, 0xF90A0006);
        patch::SetUInt(0x89786B + 6, 0xF90A0007);
        patch::SetUInt(0x897875 + 6, 0xF90A0008);
        patch::SetUInt(0x89787F + 6, 0xF90A0009);
        patch::SetUInt(0x897889 + 6, 0xF90A000A);
        patch::SetUInt(0x897893 + 6, 0xF90A000B);
        patch::SetUInt(0x89789D + 6, 0xF90A000C);
        patch::SetUInt(0x8978A7 + 6, 0xF90AFFFF);
        patch::SetUInt(0x8978B1 + 6, 0xF90AFFFF);
        patch::SetUInt(0x8978BB + 6, 0xF90AFFFF);
        patch::SetUInt(0x8978C5 + 6, 0xF90AFFFF);

        //patch::SetUInt(0x8978CF + 1, 8);
        patch::SetUChar(0x8978E1 + 1, 8);
        //patch::SetUChar(0x8978E5 + 2, 256 - 8);

        patch::SetUInt(0x89790C + 2, 0xF90A0011);
        patch::SetUInt(0x897929 + 2, 0xF90A0012);
        patch::SetUInt(0x897957 + 2, 0xF90A0013);
        patch::SetUInt(0x897977 + 2, 0xF90A0014);

        // Copa America IDs
        patch::RedirectCall(0x836D51, CopaAmericaGetQuarterFinal);
        patch::RedirectCall(0x836D70, CopaAmericaGetSemiFinal);
        patch::RedirectCall(0x836E10, CopaAmericaGet3rdPlace);
        patch::RedirectCall(0x836D8F, CopaAmericaGetFinal);
        patch::RedirectCall(0xF6CA82, OnGetCopaAmericaCompetition);
        patch::RedirectJump(0xF63000, SetHostTeamForNextCopaAmerica);

        // U-20 WC IDs
        patch::SetUInt(0x831A7E + 6, 0xFF1F000C);
        patch::SetUInt(0x831A88 + 6, 0xFF1F000D);
        patch::SetUInt(0x831A92 + 6, 0xFF1F000E);
        patch::SetUInt(0x831A9C + 6, 0xFF1F000F);
        patch::SetUInt(0x831AA6 + 6, 0xFF1F0010);

        patch::SetUInt(0xF60235 + 6, 0xFF1F000C);
        patch::SetUInt(0xF6023F + 6, 0xFF1F000D);
        patch::SetUInt(0xF60249 + 6, 0xFF1F000E);
        patch::SetUInt(0xF60253 + 6, 0xFF1F000F);
        patch::SetUInt(0xF6025D + 6, 0xFF1F0010);

        patch::SetUInt(0x12104A9 + 6, 0xFF1F000C);
        patch::SetUInt(0x12104B3 + 6, 0xFF1F000D);
        patch::SetUInt(0x12104BD + 6, 0xFF1F000E);
        patch::SetUInt(0x12104C7 + 6, 0xFF1F000F);
        patch::SetUInt(0x12104D1 + 6, 0xFF1F0010);

        // Simulation
        patch::RedirectCall(0x7BD305, GetCompetitionFinalForSimulationScreen);
        patch::RedirectCall(0x7BD385, GetCompetitionFinalForSimulationScreen);
        patch::RedirectCall(0x7BD315, GetCompetitionPoolForSimulationScreen);
        patch::RedirectCall(0x7BD395, GetCompetitionPoolForSimulationScreen);
        patch::RedirectCall(0x7BD1C9, OnStoreSimulationLbChampions);
        patch::RedirectCall(0x7BD405, GetCompetitionPoolForSimulationScreen_Supercup);
        patch::RedirectCall(0x7BD4F5, GetECFinalForSimulation);
        patch::RedirectCall(0x7BD505, GetECPoolForSimulation);
        patch::RedirectCall(0x7BD475, GetWCFinal);

        //patch::RedirectJump(0x11F46FF, LaunchQualiExec); TODO: make it flexible
        patch::RedirectCall(0x54678D, GetPoolForGameStartNationalTeam);
        // disable quali launch
        patch::Nop(0x11F4704, 2);
        patch::SetPointer(0x24B184C, OnPoolLaunch);

        // match events for international comps
        patch::RedirectJump(0x11F4311, (void *)0x11F43B0);
        patch::RedirectCall(0x11F43C1, OnClearInternationalCompsEvents);

        // club info achievements
        patch::RedirectCall(0x658A7D, OnGetTeamForClubAchievementsScreen);
        patch::RedirectCall(0x658B5F, OnGetTrophyPicForClubAchievementsScreen);
        patch::RedirectCall(0x658B8D, OnGetTrophyPicForClubAchievementsScreen);
        patch::RedirectCall(0x658C25, OnGetTrophyPicForClubAchievementsScreen);
        patch::RedirectCall(0x658BBB, OnGetTrophyPicForClubAchievementsScreen);
        patch::SetUChar(0x658BB5 + 4, COMP_CONFERENCE_LEAGUE);
        patch::SetUInt(0x23DFE08, COMP_CONFERENCE_LEAGUE);

        // international matches
        patch::RedirectCall(0x1044CE5, GetIsInternationalCompQuali);
        patch::RedirectCall(0x1044FFE, GetIsInternationalCompQuali);
        patch::RedirectCall(0x105E8C7, GetIsInternationalCompQuali);

        // test youth in 3d
        patch::RedirectCall(0xAE68EF, OnGetTeamIDMDSelectScreen_H);
        patch::RedirectCall(0xAE691B, OnGetTeamIDMDSelectScreen_A);

        static UInt watchCompIDs[] = {
            0xF9090000, 0xF90A0000, 0xF9330000, 0xF90D0000, 0xF9260000,
            0xFA090000, 0xFA0A0000,
            0xFB090000, 0xFB0A0000, 0xFB270000, 0xFB280000,
            0xFC090000, 0xFC0A0000, 0xFC270000,
            0xFD090000, 0xFD0A0000,
            0xFE090000
        };

        patch::SetUChar(0x674930 + 2, (UChar)(std::size(watchCompIDs) * 4));
        patch::SetPointer(0x6748C1 + 2, watchCompIDs);

        patch::RedirectJump(0xAC4929, WatchMatchesVideoTextCmpRegion);

        patch::SetPointer(0x3095F9C, GetIsSpecialMatch);

        // max ID for FindLastCompetitionOfType
        patch::SetUChar(0xF8B2F3 + 2, 120);

        // TEST PURPOSES ONLY!
        //patch::RedirectCall(0x800482, GetDateTrophyBook);

        patch::RedirectCall(0x104321D, SetupRoundMatchImportance);
        patch::RedirectCall(0x105702E, SetupLeagueMatchImportance);

        // team of the tournament fix
        patch::RedirectCall(0x110A585, GetInternationalPlayersCompetitionRoot);
        patch::RedirectCall(0x110A77E, InternationalPlayersCheck);
        patch::RedirectCall(0x110A790, InternationalPlayersCheck);

        // GET_EUROPEAN_ASSESSMENT_CUPWINNER
        patch::RedirectCall(0x139D4EE, GetEuropeanAssessmentCupRunnerUp); // return null team

        // skip Liechtenstein teams
        patch::RedirectCall(0x139D4D2, GetEuropeanAssessmentCupWinner);
        patch::RedirectCall(0x139DABB, GetEuropeanAssessmentTeamsGetTeamAtPosition);
        patch::RedirectCall(0x139E525, FillAssessmentReservesGetTeamAtPosition);

        // calendar icons
        //patch::SetUChar(0xEFDD85 + 2, gNumCompetitionTypes);
        //patch::SetUChar(0xEFDD41 + 2, 120);
        // temporary fix
        //patch::SetUInt(0xEFDCEC + 1, 50);
        patch::SetUChar(0xEFDD85 + 2, COMP_WORLD_CLUB_CHAMP); // TODO: ???

        // cup draw
        patch::RedirectCall(0xF90C8C, SetupCupDraw);
        patch::RedirectCall(0xF90CB8, SetupCupDraw);

        // remove special places for Germany, Spain and Scotland
        patch::RedirectJump(0xF8E60A, (void *)0xF8E6DF);
        patch::RedirectCall(0x11F1925, GetPositionPromotionsRelegationsPlacesInfo);
        patch::RedirectCall(0x11F2408, GetPositionPromotionsRelegationsPlacesInfo);

        // matches post-pone
        patch::RedirectJump(0xF84840, GetCanBePostPoned1);
        patch::RedirectJump(0xF84860, GetCanBePostPoned2);

        // special cups
        patch::RedirectCall(0x10499CF, OnAddCupTeam);

        // skip reserve teams for FA_CUP
        patch::RedirectCall(0x1049924, GetFACompCountryId_SkipReserveTeams);

        // relegation in stats screens

        // fixture results
        patch::RedirectCall(0x703815, OnGetCompForStatsScreen);
        patch::RedirectJump(0x703842, (void *)0x703859);
        // 
        patch::RedirectCall(0x6E2DDF, OnGetCompForStatsScreen);
        patch::RedirectJump(0x6E2E14, (void *)0x6E2E2F);
        // cup results
        patch::RedirectCall(0x7050A5, OnGetCompForStatsScreen);
        patch::RedirectJump(0x7050D2, (void *)0x7050E9);
        // cup tables finals
        patch::RedirectCall(0x706322, OnGetCompForStatsScreen);
        patch::RedirectJump(0x706357, (void *)0x706372);
        // 
        patch::RedirectCall(0x75FBBF, OnGetCompForStatsScreen);
        patch::RedirectJump(0x75FBF4, (void *)0x75FC0F);
        // 
        patch::RedirectCall(0x760FCF, OnGetCompForStatsScreen);
        patch::RedirectJump(0x761000, (void *)0x761017);
        // 
        patch::RedirectCall(0x977801, OnGetCompForStatsScreen);
        patch::RedirectJump(0x977816, (void *)0x97782D);
        // CMatchdayCupResults
        patch::RedirectCall(0xAA12AF, OnGetCompForStatsScreen);
        patch::RedirectJump(0xAA12C8, (void *)0xAA12E1);
        // 
        patch::RedirectCall(0xACEB13, OnGetCompForStatsScreen);
        patch::RedirectJump(0xACEB28, (void *)0xACEB41);
        // CStandingsCupResults
        patch::RedirectCall(0xE3ED5F, OnGetCompForStatsScreen);
        patch::RedirectJump(0xE3ED78, (void *)0xE3ED91);

        // Germany relegation
        //patch::SetUChar(0xF90730, 0xEB);
        //patch::RedirectJump(0xF90A77, (void *)0xF90A8C);

        // WC and Euro squad registration day offset
        patch::SetUInt(0x24D2A5C, 4);
        patch::SetUInt(0x24D2A9C, 4);

        patch::RedirectCall(0x11F49DF, OnRegisterSquadSelectionScreenForWC_EC);

        // team place in the league
        //patch::RedirectJump(0xEE0C40, GetLeagueInfo);
        patch::RedirectCall(0x10EC1B7, GetTeamLeaguePlace);

        // champions league sorter
        patch::RedirectCall(0x10F2402, OnGetCLPoolTeam);
        patch::RedirectCall(0x10F2433, OnGetCLTeamIntlPrestige);
        patch::SetUShort(0x10F2438, 0xC88B);
        patch::Nop(0x10F2438 + 2, 1);

        // remove fairness team screen
        patch::RedirectJump(0xF68AC2, (void *)0xF68B05);

        // fix reserve teams in relegation groups
        patch::RedirectCall(0x106C25D, OnAddYouthAndReserveTeams);
        patch::RedirectCall(0x106C264, OnSetupLeagueComposition);

        // remove GET_TAB_SPARE replacement
        patch::RedirectJump(0xF8AB72, (void *)0xF8AC75);

        // club info fixtures list
        patch::RedirectCall(0x651669, OnFillClubInfoFixturesList);
        patch::RedirectCall(0x6516CC, OnFillClubInfoFixturesList_NT);

        // World Club Cup for season transition screen
        patch::RedirectCall(0x8875D2, GetWorldClubCupFinalForTransitionScreen);

        // remove loading of all clubs when all league levels selected
       // patch::Nop(0xF954DC, 1);
       // patch::SetUChar(0xF954DC + 1, 0xE9);

        static UChar gCompSpectatorCalcType[] = {
            0, 2, 2, 3, 4, 4, 4, 5, 8, 10,
            6, 6, 7, 6, 6, 2, 2, 2, 2, 8,
            8, 8, 2, 9, 2, 2, 2, 2, 2, 2, 
            2, 2, 2, 2, 0, 30, 2, 2, 31, 32,
            32, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 6, 2
        };

        patch::SetPointer(0x62BE96 + 3, gCompSpectatorCalcType);
        patch::SetPointer(0x62C121 + 3, gCompSpectatorCalcType);

        // CDBCompetition::GetIndex
        patch::RedirectCall(0x10481CD, IsFinalRound_22);

        patch::RedirectCall(0xF0B19E, OnGetQualifiedForContinentalCompetition_SeasonGoals);
        patch::SetUChar(0xEE5DBF + 2, 2); // number of top teams for domestic cup season goals

        //patch::RedirectCall(0x66138D, OnGetLeagueForClubTableDevelopment);
        patch::RedirectCall(0x139E81B, OnScriptGetTabXToY);
        patch::RedirectCall(0x139E852, OnGetTabXToYTeamLeaguePositionDataGetTeamID);

        // reserve league levels
        patch::Nop(0xFD6E19, 22);
        patch::RedirectJump(0xFD6D70, OnGetLevelWithReserveTeams);

        // 3rd Continental Cup
        static unsigned int IntlCompsForLeaguePlaceColors[] = { COMP_CHAMPIONSLEAGUE, COMP_UEFA_CUP, COMP_CONFERENCE_LEAGUE };
        patch::SetUChar(0x121E1C1 + 4, (UChar)(std::size(IntlCompsForLeaguePlaceColors)));
        patch::SetPointer(0x121E213 + 3, IntlCompsForLeaguePlaceColors);

        // fill assessment reserves - Conference League, LE Cup England
        patch::SetUInt(0x139E48B + 2, 0xF9330000);
        patch::SetUChar(0x139E49B, 0xEB); // remove LE Cup France
        // CDBLeague::Launch - Conference League -1 spot for England (LE Cup winner)
        patch::SetUShort(0x106BBC5 + 3, 0x3F);
        patch::SetUChar(0x106BBCC + 3, 0x3F);

        // fix for competition save-load (not all teams are saved)
        patch::RedirectCall(0x10424BB, (void *)0xF82520);
        // CDBCompetition::Save, CDBCompetition::Load - process all teams; not needed atm
        //patch::SetUInt(0xF89616 + 2, 0xA8);
        //patch::SetUInt(0xF89636 + 2, 0xA8);
        //patch::SetUInt(0xF91B75 + 2, 0xA8);
        //patch::RedirectJump(0xF91B50, OnCompareNumCompetitionTeams);

        patch::RedirectJump(0x121D210, GetNumClubsInContinentalEuroCups);

        // widget all clubs
        static UInt intlCompTypesWorldAllClubs[] = {
            COMP_WORLD_CUP, COMP_QUALI_WC,
            COMP_EURO_CUP, COMP_QUALI_EC, COMP_EURO_NL, COMP_EURO_NL_Q,
            COMP_COPA_AMERICA,
            COMP_NAM_CUP, COMP_NAM_NL, COMP_NAM_NL_Q,
            COMP_AFRICA_CUP, COMP_AFRICA_CUP_Q,
            COMP_ASIA_CUP, COMP_ASIA_CUP_Q,
            COMP_OFC_CUP, COMP_OFC_CUP_Q,
            COMP_FINALISSIMA
        };
        patch::SetPointer(0x9DD323 + 2, intlCompTypesWorldAllClubs);
        patch::SetUChar(0x9DD3BE + 2, (UChar)std::size(intlCompTypesWorldAllClubs) * 4);
        patch::RedirectJump(0x9DD6C2, OnWidgetAllClubs);

        patch::RedirectCall(0x10449BD, OnGetChampionsLeagueHost);
        patch::RedirectCall(0x10449E8, OnGetEuropaLeagueHost);
        patch::RedirectCall(0x1044A13, OnGetEuroSuperCupHost);
        patch::RedirectCall(0x10449A0, OnGetCompTypeChampionsLeagueForHost);
        patch::RedirectCall(0x10449CB, OnGetCompTypeEuropaLeagueForHost);
        patch::RedirectCall(0x10449F6, OnGetCompTypeEuroSuperCupForHost);

        patch::RedirectCall(0xF652D1, OnGetIsCountryInCompetitionHosts);

        // max subs for DB_ROUND and DB_CUP
        patch::RedirectCall(0x104321D, OnSetupCompetitionOneMatch<0x1042570>);
        patch::RedirectCall(0x1049CCF, OnSetupCompetitionOneMatch<0x10496A0>);

        // sort by FIFA ranking for international comps
        patch::RedirectCall(0x10F4535, OnSortPoolCountryStrength);

        // Cup Draw pot/group WC/EC
        //patch::SetUInt(0x249562C + 4, 3); // Euro
        //patch::SetUInt(0x249562C + 4, 6); // WC

        patch::Nop(0x703613, 2); // CStatsCupFixturesResults - TOYOTA
        patch::Nop(0x9DD558, 6); // WidgetAllClubs - TOYOTA
        patch::Nop(0x704F13, 2); // CStatsCupResults - TOYOTA
        patch::Nop(0x7060A8, 6); // CStatsCupTablesFinals - TOYOTA
        patch::Nop(0xE3EB28, 6); // CStandingsCupResults - TOYOTA
        patch::Nop(0x977673, 2); // unknown - weekly progress? - TOYOTA
        patch::Nop(0xAA10E3, 2); // CMatchdayCupResults - TOYOTA
        patch::Nop(0xACE953, 2); // next matchday info? - TOYOTA
        patch::Nop(0x129D736, 6); // trophy stickers - TOYOTA

        patch::SetPointer(0x24AD3B8, MyDBRound_RegisterMatch); // use 3rd bonus in DB_ROUND as a payment for round loser

        // Club WC hosting teams
        static UInt ClubWorldCupHostTeams[4] = {
            0x009B0003, // Beijing Guoan FC
            0x00B70002, // Al Hilal
            0x005F2149, // Atlanta
            0x00B60004  // Al-Rayyan SC
        };
        patch::SetPointer(0x1044A87 + 3, ClubWorldCupHostTeams);
    }
}
