#include "Competitions.h"
#include "GameInterfaces.h"
#include "FifamCompID.h"
#include "FifamRoundID.h"
#include "shared.h"
#include "Log.h"
#include <map>
#include "license_check/license_check.h"

using namespace plugin;

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

Bool Europe_NationalTeamSorter(CTeamIndex const &a, CTeamIndex const &b) {
    if (!b.countryId)
        return true;
    if (!a.countryId)
        return false;
    Int posA = GetAssesmentTable()->GetCountryPosition(a.countryId);
    Int posB = GetAssesmentTable()->GetCountryPosition(b.countryId);
    return posA <= posB;
}

Bool NationalTeamSorter(CTeamIndex const &a, CTeamIndex const &b) {
    if (!b.countryId)
        return true;
    if (!a.countryId)
        return false;
    Int posA = (a.countryId < 208) ? GetCountryStore()->m_aCountries[a.countryId].GetLeagueAverageLevel() : 0;
    Int posB = (b.countryId < 208) ? GetCountryStore()->m_aCountries[b.countryId].GetLeagueAverageLevel() : 0;
    return posA <= posB;
}

UInt Oceania_ParticipantsCountries[] = {
    FifamCompRegion::Anguilla,
    FifamCompRegion::New_Zealand,
    FifamCompRegion::Fiji,
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
    FifamCompRegion::Venezuela,
    FifamCompRegion::Uruguay,
    FifamCompRegion::Bolivia,
    FifamCompRegion::Ecuador,
    FifamCompRegion::Paraguay,
    FifamCompRegion::Peru
};

UInt NorthAmerica_ParticipantsCountries_CL[] = {
    FifamCompRegion::Mexico,
    FifamCompRegion::United_States,
    FifamCompRegion::Canada
};

UInt NorthAmerica_ParticipantsCountries_EL[] = {
    FifamCompRegion::Canada,
    FifamCompRegion::Costa_Rica,
    FifamCompRegion::Honduras,
    FifamCompRegion::Panama,
    FifamCompRegion::El_Salvador,
    FifamCompRegion::Guatemala,
    FifamCompRegion::Nicaragua,
    FifamCompRegion::Belize
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
    FifamCompRegion::Uganda
};

UInt Asia_ParticipantsCountries_CL[] = {
    FifamCompRegion::United_Arab_Emirates,
    FifamCompRegion::Korea_Republic,
    FifamCompRegion::China_PR,
    FifamCompRegion::Saudi_Arabia,
    FifamCompRegion::Japan,
    FifamCompRegion::Qatar,
    FifamCompRegion::Iran,
    FifamCompRegion::Australia,
    FifamCompRegion::Uzbekistan,
    FifamCompRegion::Thailand,
    FifamCompRegion::Iraq,
    FifamCompRegion::Tajikistan,
    FifamCompRegion::Malaysia,
    FifamCompRegion::Hong_Kong,
    FifamCompRegion::India,
    FifamCompRegion::Syria,
    FifamCompRegion::Vietnam,
    FifamCompRegion::Jordan,
    FifamCompRegion::Kuwait,
    FifamCompRegion::Bahrain,
    FifamCompRegion::Philippines,
    FifamCompRegion::Singapore,
    FifamCompRegion::Indonesia,
    FifamCompRegion::Myanmar
};

UInt Asia_ParticipantsCountries_EL[] = {
    FifamCompRegion::Syria,
    FifamCompRegion::Vietnam,
    FifamCompRegion::Jordan,
    FifamCompRegion::Kuwait,
    FifamCompRegion::Bahrain,
    FifamCompRegion::Philippines,
    FifamCompRegion::Lebanon,
    FifamCompRegion::Singapore,
    FifamCompRegion::Indonesia,
    FifamCompRegion::Myanmar,
    FifamCompRegion::Turkmenistan,
    FifamCompRegion::Oman,
    FifamCompRegion::Maldives,
    FifamCompRegion::Palestinian_Authority,
    FifamCompRegion::Korea_DPR,
    FifamCompRegion::Kyrgyzstan,
    FifamCompRegion::Laos,
    FifamCompRegion::Bangladesh,
    FifamCompRegion::Yemen,
    FifamCompRegion::Cambodia,
    FifamCompRegion::Taiwan,
    FifamCompRegion::Afghanistan,
    FifamCompRegion::Nepal,
    FifamCompRegion::Bhutan,
    FifamCompRegion::Macao,
    FifamCompRegion::Brunei_Darussalam,
    FifamCompRegion::Guam,
    FifamCompRegion::Sri_Lanka,
    FifamCompRegion::Mongolia,
    FifamCompRegion::Pakistan
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
     { L"Q_U17_WC", 51 }, // U17 WC Quali
     { L"U17_WORLD_CUP", 52 }, // U17 WC
     { L"Q_U21_EC", 53 }, // U21 EC Quali
     { L"U21_EC", 54 }, // U21 EC
     { L"Q_U19_EC", 55 }, // U19 EC Quali
     { L"U19_EC", 56 }, // U19 EC
     { L"Q_U17_EC", 57 }, // U17 EC Quali
     { L"U17_EC", 58 }, // U17 EC
     { L"Q_OLYMPIC", 59 }, // Olympic Games Quali
     { L"OLYMPIC", 60 }, // Olympic Games
     { L"", 0 }
};

const unsigned char gNumCompetitionTypes = std::extent<decltype(gNewCompTypeNames)>::value + 9;

unsigned int gContinentalCompetitionTypes[] = {
    9, 10, 12, 11, 13, COMP_UIC, COMP_ICC, COMP_CONTINENTAL_1, COMP_CONTINENTAL_2 // TODO: set root for Youth CL
};

unsigned int gNewIntlComps[] = { 36, 37 };

unsigned int gInternationalCompsTypes[] = { COMP_EURO_NL_Q, 15, 16, COMP_EURO_NL, 17, 18, 32, 33, COMP_NAM_NL_Q, COMP_NAM_NL, COMP_NAM_CUP,
    COMP_AFRICA_CUP_Q, COMP_AFRICA_CUP, COMP_ASIA_CUP_Q, COMP_ASIA_CUP, COMP_OFC_CUP_Q, COMP_OFC_CUP }; // for NT matches info screen

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

    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_SOUTHAM_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_SOUTHAM_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_SOUTHAM_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::SouthAmerica, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_SOUTHAM_CONTINENTAL_2"));

    //Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_UEFA_CUP, GetTranslation("IDS_NAME_NORTHAM_2ND_CUP")); // remove later
    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_NORTHAM_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_NORTHAM_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_NORTHAM_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::NorthAmerica, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_NORTHAM_CONTINENTAL_2"));

    //Call<0xF904D0>(FifamCompRegion::Africa, COMP_UEFA_CUP, GetTranslation("IDS_NAME_AFRICA_2ND_CUP")); // remove later
    Call<0xF904D0>(FifamCompRegion::Africa, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_AFRICA_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::Africa, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_AFRICA_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::Africa, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_AFRICA_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::Africa, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_AFRICA_CONTINENTAL_2"));
    
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_ASIA_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_ASIA_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_ASIA_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::Asia, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_ASIA_CONTINENTAL_2"));

    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_EURO_SUPERCUP, GetTranslation("IDS_NAME_OCEANIA_SUPERCUP"));
    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_YOUTH_CHAMPIONSLEAGUE, GetTranslation("IDS_NAME_OCEANIA_YOUTH_LEAGUE"));
    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_CONTINENTAL_1, GetTranslation("IDS_NAME_OCEANIA_CONTINENTAL_1"));
    Call<0xF904D0>(FifamCompRegion::Oceania, COMP_CONTINENTAL_2, GetTranslation("IDS_NAME_OCEANIA_CONTINENTAL_2"));

    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_EURO_NL,
        CTeamIndex::make(FifamCompRegion::Portugal, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Netherlands, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_NAM_CUP,
        CTeamIndex::make(FifamCompRegion::United_States, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Jamaica, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_ASIA_CUP,
        CTeamIndex::make(FifamCompRegion::Qatar, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Japan, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_AFRICA_CUP,
        CTeamIndex::make(FifamCompRegion::Cameroon, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Egypt, 0, 0xFFFF));
    SetCompetitionWinnerAndRunnerUp(FifamCompRegion::International, COMP_OFC_CUP,
        CTeamIndex::make(FifamCompRegion::New_Zealand, 0, 0xFFFF), CTeamIndex::make(FifamCompRegion::Papua_New_Guinea, 0, 0xFFFF));
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
    case COMP_NAM_NL:
    case COMP_NAM_NL_Q:
        if (year % 2 == 1)
            return true;
        break;
    case COMP_QUALI_WC: // QUALI_WC
    case COMP_CONFED_CUP: // CONFED_CUP
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
        LaunchCompetition(255, COMP_NAM_NL_Q, 0);
        //LaunchCompetition(255, COMP_NAM_NL, 0);
    }
    if (year % 4 == 2) {
        LaunchCompetition(255, COMP_ASIA_CUP_Q, 0);
        //LaunchCompetition(255, COMP_ASIA_CUP, 0);
    }
    if (year % 4 == 3) {
        LaunchCompetition(255, COMP_COPA_AMERICA, 0);
        LaunchCompetition(255, COMP_OFC_CUP, 0);
    }
    if (year % 2 == 0) {
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

// not used
void *MatchdayScreen_GetCompById(unsigned int *pCompId) {
    gLastCompType = (*pCompId >> 16) & 0xFF;
    if ((gLastCompType >= 15 && gLastCompType <= 18) || gLastCompType == 36 || gLastCompType == 37)
        return GetCompetition(pCompId);
    return nullptr;
}

unsigned char gCompTypesClub[] =         { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 14, 8, 19, 20, 21, 23, 27,
    COMP_ICC, COMP_CONTINENTAL_1, COMP_CONTINENTAL_2 };
unsigned char gCompTypesClubYouth[] =    { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 14, 8, 19, 20, 21, 23,
    COMP_YOUTH_CHAMPIONSLEAGUE };
unsigned char gCompTypesNationalTeam[] = { 1, 3, 7, 6, 4, 9, 10, 11, 12, 13, 8, 19, 20, 21, 15, 16, 17, 18, 32, 33, 29,
    COMP_EURO_NL_Q, COMP_EURO_NL, COMP_NAM_NL_Q, COMP_NAM_NL, COMP_NAM_CUP, COMP_AFRICA_CUP, COMP_AFRICA_CUP_Q, COMP_ASIA_CUP, COMP_ASIA_CUP_Q, COMP_OFC_CUP_Q, COMP_OFC_CUP };
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
    if (comp->GetCompID().countryId == FifamCompRegion::International && comp->GetCompID().type == COMP_EURO_NL_Q && comp->GetCompID().index == 0) {
        //if (GetCurrentYear() > 2021) {
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
        //if (GetCurrentYear() > 2023) {
            comp->RandomlySortTeams(0, 4);
            comp->RandomlySortTeams(4, 4);
            comp->RandomlySortTeams(8, 4);

            comp->RandomlySortTeams(12, 4);
            comp->RandomlySortTeams(16, 4);
            comp->RandomlySortTeams(20, 4);

            comp->RandomlySortTeams(24, 4);
            comp->RandomlySortTeams(28, 4);
            comp->RandomlySortTeams(32, 2);
        //}
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
    CallMethod<0x10F2340>(comp, numGroups, numTeamsInGroup);
}

unsigned char METHOD GetPoolNumberOfTeamsFromCountry(CDBPool *pool, DUMMY_ARG, int countryId) {
    int numTeams = 0;
    CDBCompetition *comp = nullptr;
    for (int i = 0; i < pool->GetNumOfScriptCommands(); i++) {
        auto command = pool->GetScriptCommand(i);
        switch (command->m_nCommandId) {
        case 2: // RESERVE_ASSESSMENT_TEAMS
            if (countryId == GetAssesmentTable()->GetCountryIdAtPosition(LOWORD(command->m_params)))
                numTeams += HIWORD(command->m_params);
            break;
        case 4: // GET_EUROPEAN_ASSESSMENT_TEAMS
            if (countryId == GetAssesmentTable()->GetCountryIdAtPosition(LOWORD(command->m_params)))
                numTeams += HIBYTE(command->m_params);
            break;
        case 7: // GET_TAB_X_TO_Y
            if (countryId == command->m_competitionId.countryId)
                numTeams += HIBYTE(command->m_params);
            break;
        case 12: // GET_EUROPEAN_ASSESSMENT_CUPWINNER
            if (countryId == GetAssesmentTable()->GetCountryIdAtPosition(LOWORD(command->m_params)))
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
        default:
            continue;
        }
    }
    //if (pool->GetCompID().index == 0 && pool->GetCompID().countryId == 253) { // asia continental
    //    comp = GetCompetition((pool->GetCompID().ToInt() & 0xFFFF0001) | 1);
    //    if (comp && comp->GetDbType() == 5) // DB_POOL
    //        numTeams += comp->GetNumTeamsFromCountry(countryId);
    //}
    //else if (pool->GetCompID().countryId == 250 && pool->GetCompID().type == COMP_UEFA_CUP) { // south america continental
    //    auto copaLibertadoresPool = GetCompetition(250, COMP_CHAMPIONSLEAGUE, 0);
    //    if (copaLibertadoresPool) {
    //        numTeams -= copaLibertadoresPool->GetNumTeamsFromCountry(countryId);
    //        auto copaLibertadoresFinal = GetCompetition(250, COMP_CHAMPIONSLEAGUE, 22);
    //        if (copaLibertadoresFinal && copaLibertadoresFinal->GetChampion().countryId == countryId)
    //            ++numTeams;
    //    }
    //}
    return (numTeams > 0) ? numTeams : 0;
}

unsigned int gParticipantsRegion = 0;
unsigned int gParticipantsCompType = 0;
unsigned int gParticipantsListId = 0;

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
    gParticipantsRegion = region;
    gParticipantsCompType = compType;
    gParticipantsListId = listId;
    GetFirstManagerRegion(gParticipantsRegion);
    CallMethod<0x88F670>(obj, gParticipantsRegion, gParticipantsCompType, gParticipantsListId);
}

int METHOD EuropeanCompsParticipants_GetNumEntries(void *obj) {
    if (gParticipantsRegion == FifamCompRegion::Oceania && gParticipantsCompType == COMP_CHAMPIONSLEAGUE)
        return std::size(Oceania_ParticipantsCountries);
    else if (gParticipantsRegion == FifamCompRegion::SouthAmerica)
        return std::size(SouthAmerica_ParticipantsCountries);
    else if (gParticipantsRegion == FifamCompRegion::NorthAmerica && gParticipantsCompType == COMP_CHAMPIONSLEAGUE)
        return std::size(NorthAmerica_ParticipantsCountries_CL);
    else if (gParticipantsRegion == FifamCompRegion::NorthAmerica && gParticipantsCompType == COMP_UEFA_CUP)
        return std::size(NorthAmerica_ParticipantsCountries_EL);
    else if (gParticipantsRegion == FifamCompRegion::Africa)
        return std::size(Africa_ParticipantsCountries);
    else if (gParticipantsRegion == FifamCompRegion::Asia && gParticipantsCompType == COMP_CHAMPIONSLEAGUE)
        return std::size(Asia_ParticipantsCountries_CL);
    else if (gParticipantsRegion == FifamCompRegion::Asia && gParticipantsCompType == COMP_UEFA_CUP)
        return std::size(Asia_ParticipantsCountries_EL);
    return plugin::CallMethodAndReturn<int, 0x121D1C0>(obj);
}

unsigned char METHOD EuropeanCompsParticipants_GetCountryAtPosition(void *obj, DUMMY_ARG, int position) {
    if (gParticipantsRegion == FifamCompRegion::Oceania && gParticipantsCompType == COMP_CHAMPIONSLEAGUE) {
        if (position > 0 && position <= (int)std::size(Oceania_ParticipantsCountries))
            return Oceania_ParticipantsCountries[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::SouthAmerica) {
        if (position > 0 && position <= (int)std::size(SouthAmerica_ParticipantsCountries))
            return SouthAmerica_ParticipantsCountries[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::NorthAmerica && gParticipantsCompType == COMP_CHAMPIONSLEAGUE) {
        if (position > 0 && position <= (int)std::size(NorthAmerica_ParticipantsCountries_CL))
            return NorthAmerica_ParticipantsCountries_CL[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::NorthAmerica && gParticipantsCompType == COMP_UEFA_CUP) {
        if (position > 0 && position <= (int)std::size(NorthAmerica_ParticipantsCountries_EL))
            return NorthAmerica_ParticipantsCountries_EL[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::Africa) {
        if (position > 0 && position <= (int)std::size(Africa_ParticipantsCountries))
            return Africa_ParticipantsCountries[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::Asia && gParticipantsCompType == COMP_CHAMPIONSLEAGUE) {
        if (position > 0 && position <= (int)std::size(Asia_ParticipantsCountries_CL))
            return Asia_ParticipantsCountries_CL[position - 1];
        return 0;
    }
    else if (gParticipantsRegion == FifamCompRegion::Asia && gParticipantsCompType == COMP_UEFA_CUP) {
        if (position > 0 && position <= (int)std::size(Asia_ParticipantsCountries_EL))
            return Asia_ParticipantsCountries_EL[position - 1];
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
    Array<UInt, 2> firstSeasonLeagues = { 3, 7 };
    Array<UInt, 2> secondSeasonLeagues = { 4, 5 };
    Bool firstSeason = GetCurrentYear() % 2 == 0;
    if (promotionRound)
        firstSeason = !firstSeason;
    for (UInt i : (firstSeason ? firstSeasonLeagues : secondSeasonLeagues)) {
        auto l = GetLeague(FifamCompRegion::Germany, FifamCompType::League, i);
        if (l)
            ScriptGetTabXToY(dst, l, 1, 1);
    }
}

void OnGetSpare(CDBCompetition **ppComp) {
    if (*ppComp) {
        auto comp = *ppComp;
        auto id = comp->GetCompID();
        if (id.countryId == FifamCompRegion::Europe) {
            if (comp->GetDbType() == DB_ROUND) {
                if (id.type == COMP_YOUTH_CHAMPIONSLEAGUE) {
                    if (id.index == 12 || id.index == 13) // playoff and last16
                        comp->RandomizePairs();
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
                if (id.type == COMP_CHAMPIONSLEAGUE && id.index == 0 && comp->GetNumOfTeams() == 81 && comp->GetNumOfRegisteredTeams() == 79) {

                    const Bool DUMP_TO_LOG = true;

                    auto ClearID = [](CTeamIndex &teamIndex) {
                        teamIndex.countryId = 0;
                        teamIndex.type = 0;
                        teamIndex.index = 0;
                    };

                    auto CompareIDs = [](CTeamIndex const &a, CTeamIndex const &b) {
                        return a.countryId == b.countryId && a.type == b.type && a.index == b.index;
                    };

                    Array<CTeamIndex, 83> teams;
                    for (UInt i = 0; i < std::size(teams); i++)
                        ClearID(teams[i]);

                    auto finalCL = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, FifamRoundID::Final);
                    auto finalEL = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::UefaCup, FifamRoundID::Final);
                    if (finalCL)
                        teams[0] = finalCL->GetChampion();
                    if (finalEL)
                        teams[1] = finalEL->GetChampion();

                    CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);

                    if (DUMP_TO_LOG) {
                        SafeLog::Write(L"Champions League Pool");
                        for (UInt i = 0; i < comp->GetNumOfTeams(); i++)
                            SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), (pTeamIDs[i].countryId && GetTeam(pTeamIDs[i]))? GetTeam(pTeamIDs[i])->GetName() : L"n/a"));
                    }

                    for (UInt i = 0; i < 24; i++)
                        teams[i + 2] = pTeamIDs[i];

                    for (UInt i = 0; i < 44; i++)
                        teams[i + 28] = pTeamIDs[i + 24];

                    for (UInt i = 0; i < 11; i++)
                        teams[i + 72] = pTeamIDs[i + 68];

                    if (DUMP_TO_LOG) {
                        SafeLog::Write(L"Reordered map");
                        for (UInt i = 0; i < std::size(teams); i++) {
                            String teamNam;
                            SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, teams[i].ToInt(), (teams[i].countryId && GetTeam(teams[i])) ? GetTeam(teams[i])->GetName() : L"n/a"));
                        }
                    }

                    Bool movedChampTeam = false;
                    Bool movedNonChampTeam = false;

                    if (teams[0].countryId != 0) {
                        for (UInt i = 2; i < std::size(teams); i++) {
                            if (CompareIDs(teams[0], teams[i])) {
                                ClearID(teams[i]);
                                if (i < 28) {
                                    teams[26] = teams[28];
                                    ClearID(teams[28]);
                                    movedChampTeam = true;
                                }
                                break;
                            }
                        }
                    }

                    if (teams[1].countryId != 0) {
                        for (UInt i = 2; i < std::size(teams); i++) {
                            if (CompareIDs(teams[1], teams[i])) {
                                ClearID(teams[i]);
                                if (i < 28) {
                                    teams[27] = teams[72];
                                    ClearID(teams[72]);
                                    movedNonChampTeam = true;
                                }
                                break;
                            }
                        }
                    }

                    // 5 teams max
                    if (teams[0].countryId != 0 && teams[0].countryId == teams[1].countryId && !movedChampTeam && !movedNonChampTeam) {
                        auto assessmentPos = GetAssesmentTable()->GetCountryPositionLastYear(teams[0].countryId);
                        if (assessmentPos < 3) {
                            ClearID(teams[2 + assessmentPos * 4 + 3]);
                            teams[26] = teams[28];
                            ClearID(teams[28]);
                            movedChampTeam = true;
                        }
                    }
                    if (DUMP_TO_LOG) {
                        SafeLog::Write(L"Sorted map");
                        for (UInt i = 0; i < std::size(teams); i++) {
                            String teamNam;
                            SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, teams[i].ToInt(), (teams[i].countryId && GetTeam(teams[i])) ? GetTeam(teams[i])->GetName() : L"n/a"));
                        }
                    }

                    CallMethod<0xF82440>(comp);

                    auto AddTeams = [](CTeamIndex *dst, CTeamIndex const *from, CTeamIndex const *to, UInt numTeams) {
                        UInt numAdded = 0;
                        for (CTeamIndex const *p = from; p != to; p++) {
                            if (numAdded < numTeams) {
                                if (p->countryId != 0)
                                    dst[numAdded++] = *p;
                            }
                            else
                                break;
                        }
                    };

                    AddTeams(&pTeamIDs[0], &teams[0], &teams[28], 26);
                    // in case if CL/EL winners are not available
                    UInt numTeamsInGroup = 26;
                    if (!pTeamIDs[24].countryId)
                        numTeamsInGroup = 24;
                    else if (!pTeamIDs[25].countryId)
                        numTeamsInGroup = 25;
                    if (numTeamsInGroup != 26) {
                        for (UInt i = 28; i < 72; i++) {
                            if (teams[i].countryId != 0) {
                                pTeamIDs[numTeamsInGroup] = teams[i];
                                ClearID(teams[i]);
                                numTeamsInGroup++;
                            }
                            if (numTeamsInGroup == 26)
                                break;
                        }
                    }
                    AddTeams(&pTeamIDs[26], &teams[28], &teams[72], 44);
                    AddTeams(&pTeamIDs[70], &teams[72], &teams[83], 11);

                    *raw_ptr<UInt>(comp, 0xA4) = 81;

                    if (DUMP_TO_LOG) {
                        SafeLog::Write(L"Modified Champions League Pool");
                        for (UInt i = 0; i < comp->GetNumOfTeams(); i++) {
                            String teamNam;
                            SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), (pTeamIDs[i].countryId && GetTeam(pTeamIDs[i])) ? GetTeam(pTeamIDs[i])->GetName() : L"n/a"));
                        }
                    }

                    // round must have at least one correct match - put team from next round into last round
                    if (!pTeamIDs[68].countryId && !pTeamIDs[69].countryId && pTeamIDs[67].countryId != 0 && pTeamIDs[66].countryId != 0 && pTeamIDs[65].countryId != 0) {
                        pTeamIDs[68] = pTeamIDs[67];
                        pTeamIDs[67] = pTeamIDs[65];
                        ClearID(pTeamIDs[65]);
                    }

                    if (DUMP_TO_LOG) {
                        SafeLog::Write(L"Modified Champions League Pool 2");
                        for (UInt i = 0; i < comp->GetNumOfTeams(); i++) {
                            String teamNam;
                            SafeLog::Write(Utils::Format(L"%2d. %08X %s", i, pTeamIDs[i].ToInt(), (pTeamIDs[i].countryId && GetTeam(pTeamIDs[i])) ? GetTeam(pTeamIDs[i])->GetName() : L"n/a"));
                        }
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
                if (id.countryId == FifamCompRegion::SouthAmerica && id.type == COMP_CHAMPIONSLEAGUE && id.index == 15) // last16
                    comp->RandomizePairs();
                else if (id.countryId == FifamCompRegion::NorthAmerica && id.type == COMP_CONTINENTAL_1 && id.index == 2) // last16
                    comp->RandomizePairs();
                else if (id.countryId == FifamCompRegion::Africa && id.type == COMP_UEFA_CUP && id.index == 3) // playoff round
                    comp->RandomizePairs();
                else if (id.countryId == FifamCompRegion::Africa && id.type == COMP_CHAMPIONSLEAGUE && id.index == 8) // quarterfinal
                    comp->RandomizePairs();
                else if (id.countryId == FifamCompRegion::Africa && id.type == COMP_UEFA_CUP && id.index == 9) // quarterfinal
                    comp->RandomizePairs();
                //else if (id.countryId == FifamCompRegion::Asia && id.type == COMP_CHAMPIONSLEAGUE && id.index == 17) // last16
                //    comp->RandomizePairs4x4();
                else if (id.countryId == FifamCompRegion::Asia && id.type == COMP_UEFA_CUP && (id.index == 23 || id.index == 26 || id.index == 27)) // semifinals
                    comp->RandomizePairs();
                else if (id.countryId == FifamCompRegion::Oceania && id.type == COMP_CHAMPIONSLEAGUE && id.index == 7) // quarterfinal
                    comp->RandomizePairs();
                else {
                    comp->SortTeams(ChampionsLeagueRoundSorter);
                    comp->RandomizePairs();
                }
            }
            return;
        }
        else if (id.countryId == FifamCompRegion::International) {
            if (comp->GetDbType() == DB_ROUND) {
                if (id.type == COMP_QUALI_WC && (id.index == 28)) { // Euro Quali WC play-off
                    comp->SortTeamIDs(Europe_NationalTeamSorter);
                    comp->RandomizePairs();
                }
                else
                    comp->RandomizePairs();
            }
            return;
        }
        else if (id.countryId == FifamCompRegion::Germany) {
            if (comp->GetDbType() == DB_ROUND && id.type == COMP_RELEGATION) {
                AddGermanyRegionalligaPromotionTeams(comp, true);
            }
            else if (comp->GetDbType() == DB_POOL && id.index == 2) {
                AddGermanyRegionalligaPromotionTeams(comp, false);
            }
            return;
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
}

void METHOD OnAddMyCareerTrophyContinental(void *vec, DUMMY_ARG, void *pValue) {
    CallMethod<0x416950>(vec, pValue);
    UChar compType = COMP_CONTINENTAL_1;
    CallMethod<0x416950>(vec, &compType);
    compType = COMP_CONTINENTAL_1;
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
        || compType == COMP_OLYMPIC || compType == COMP_OFC_CUP_Q)
    {
        return COMP_WORLD_CUP;
    }
    return compType;
}

Char const *GetNationalTeamECQualifiedTeamsScreenName() {
    if (GetCurrentYear() == 2020)
        return "Screens/10NationalTeamECQualifiedTeams_2020_FF12.xml";
    return "Screens/10NationalTeamECQualifiedTeams.xml";
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
        if (game->GetCurrentYear() == 2020)
            return "Screens/10NationalTeamECTournamentWelcome_2020_FF12.xml";
        return "Screens/10NationalTeamECTournamentWelcome.xml";
    }
    return "Screens/10NationalTeamECTournamentWelcome.xml";
}

Char const *GetNationalTeamTournamentGroupsScreenName() {
    if (*(UChar *)0x3111C74 == 2) {
        auto game = CDBGame::GetInstance();
        if (game->GetCurrentYear() == 2020)
            return "Screens/11NationalTeamECTournamentGroups_2020_FF12.xml";
        return "Screens/11NationalTeamECTournamentGroups.xml";
    }
    return "Screens/11NationalTeamWCTournamentGroups.xml";
}

Char const *GetNationalTeamECTournamentFinalsScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetCurrentYear() == 2020)
        return "Screens/10NationalTeamECTournamentFinals_2020_FF12.xml";
    return "Screens/10NationalTeamECTournamentFinals.xml";
}

Char const *GetNationalTeamTournamentReportScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010TournamentReport.xml";
    if (game->TestFlag(4) || !game->TestFlag(2)) {
        if (game->GetCurrentYear() == 2020)
            return "Screens/10NationalTeamECTournamentReport_2020_FF12.xml";
        return "Screens/10NationalTeamECTournamentReport.xml";
    }
    return "Screens/10NationalTeamWCTournamentReport.xml";
}

Char const *GetNationalTeamTournamentDreamTeamScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetIsWorldCupMode())
        return "Screens/11NationalTeamWC2010DreamTeam.xml";
    if (game->TestFlag(4) || !game->TestFlag(2)) {
        if (game->GetCurrentYear() == 2020)
            return "Screens/10NationalTeamECTournamentDreamTeam_2020_FF12.xml";
        return "Screens/10NationalTeamECTournamentDreamTeam.xml";
    }
    return "Screens/10NationalTeamWCTournamentDreamTeam.xml";
}

Char const *GetNationalTeamTournamentECUpsAndDownsScreenName() {
    auto game = CDBGame::GetInstance();
    if (game->GetCurrentYear() == 2020)
        return "Screens/13NationalTeamECUpsAndDowns_2020_FF12.xml";
    return "Screens/13NationalTeamECUpsAndDowns.xml";
}

UInt gTmpAddress = 0;
wchar_t gTmpMessage[256];

void __declspec(naked) GetNumOfTeamsInComp() {
    __asm {
        mov eax, dword ptr[esp]
        mov gTmpAddress, eax
        test ecx, ecx
        jz SHOW_ERR
        mov eax, [ecx + 0xA8]
        //cdq
        //sub eax, edx
        //sar eax, 1
        retn
        SHOW_ERR :
    }
        swprintf(gTmpMessage, L"0x%X", gTmpAddress);
        MessageBoxW(NULL, gTmpMessage, L"Season 2019 (v.1.3) - Exception", MB_ICONERROR);
    __asm {
        xor eax, eax
        retn
    }
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
    return GetRoundByRoundType(255, compType, 15);
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

CDBRound *GetWCFinalForSimulation(Int, Int, Int) {
    return GetRoundByRoundType(255, COMP_WORLD_CUP, 15);
}

CDBRound *GetCompetitionFinalForSimulationScreen(UInt region, UInt type, UInt index) {
    GetFirstManagerRegion(region);
    return GetRoundByRoundType(region, type, 15);
}

CDBCompetition *GetCompetitionPoolForSimulationScreen(UInt region, UInt type, UInt index) {
    if (region == FifamCompRegion::NorthAmerica && type == COMP_EURO_SUPERCUP)
        return nullptr;
    GetFirstManagerRegion(region);
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

void METHOD OnSetupRootInternationalComps(void *vec, DUMMY_ARG, UInt *pCompId) {
    CallMethod<0x6E3FD0>(vec, pCompId);
    *pCompId = 0xFF240000;
    CallMethod<0x6E3FD0>(vec, pCompId);
    *pCompId = 0xFF250000;
    CallMethod<0x6E3FD0>(vec, pCompId);
    for (UInt i = 41; i <= 60; i++) {
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

void METHOD OnGetCalendarMatchesBuffer(void *a, DUMMY_ARG, UInt b, void **c) {
    CallMethod<0xEFDBB0>(a, b, c);
    void *cal = *c;
    UInt size = *raw_ptr<UInt>(cal, 4);
    Error(L"%d", size);
}

void METHOD OnSetupAssessmentEntry(CAssessmentTable *table, DUMMY_ARG, UChar countryIndex, UShort y1, UShort y2, UShort y3, UShort y4, UShort y5, UShort y6) {
    if (countryIndex == 207) // Kosovo
        CallMethod<0x121D410>(table, countryIndex, 0, 0, 0, 0, 250, 150);
    else
        CallMethod<0x121D410>(table, countryIndex, y1, y2, y3, y4, y5, y6);
}

UChar METHOD GetCountryAtAssessmentPosition(CAssessmentTable *table, DUMMY_ARG, UInt position) {
    UInt lpos = table->GetCountryPositionLastYear(FifamCompRegion::Liechtenstein) + 1;
    if (lpos > position)
        return table->GetCountryIdAtPositionLastYear(position);
    return table->GetCountryIdAtPositionLastYear(position + 1);
}

CDBCompetition *gCurrentScriptComp = nullptr;

void METHOD OnScriptProcess(CDBCompetition *comp) {
    gCurrentScriptComp = comp;
    auto compId = comp->GetCompID();
    if (compId.countryId == FifamCompRegion::Germany && compId.type == FifamCompType::League) { // Regionalliga
        if (compId.index == 3 || compId.index == 7 || compId.index == 4 || compId.index == 5) {
            Bool firstSeason = GetCurrentYear() % 2 == 1;
            if (compId.index == 3 || compId.index == 7)
                CallMethod<0x10503E0>(comp, firstSeason ? 1 : 0, firstSeason ? 0 : 1);
            else
                CallMethod<0x10503E0>(comp, firstSeason ? 0 : 1, firstSeason ? 1 : 0);
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
    CDBRound *r = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::Spain, 0, 1));
    r = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::UefaCup, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::England, 0, 1));

    r = GetRoundByRoundType(FifamCompRegion::SouthAmerica, FifamCompType::ChampionsLeague, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::Argentina, 0, 1));
    r = GetRoundByRoundType(FifamCompRegion::SouthAmerica, FifamCompType::UefaCup, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::Brazil, 0, 1));

    r = GetRoundByRoundType(FifamCompRegion::NorthAmerica, FifamCompType::ChampionsLeague, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::Mexico, 0, 1));
    r = GetRoundByRoundType(FifamCompRegion::NorthAmerica, FifamCompType::UefaCup, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::United_States, 0, 1));

    r = GetRoundByRoundType(FifamCompRegion::Asia, FifamCompType::ChampionsLeague, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::Japan, 0, 1));
    r = GetRoundByRoundType(FifamCompRegion::Asia, FifamCompType::UefaCup, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::China_PR, 0, 1));

    r = GetRoundByRoundType(FifamCompRegion::Africa, FifamCompType::ChampionsLeague, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::Morocco, 0, 1));
    r = GetRoundByRoundType(FifamCompRegion::Africa, FifamCompType::UefaCup, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::Egypt, 0, 1));

    r = GetRoundByRoundType(FifamCompRegion::Oceania, FifamCompType::ChampionsLeague, 15);
    if (r && (!r->GetChampion().countryId || !GetTeam(r->GetChampion())))
        r->SetChampion(CTeamIndex::make(FifamCompRegion::New_Zealand, 0, 1));

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
}

void METHOD OnAddTrophyClubTrophiesScreen_Continental(void *vec, DUMMY_ARG, UChar *pCompType) {
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
    WatchMatchesVideoTextCmpRegion_RET1:
        mov eax, 0xAC492E
        jmp eax
    }
}

Bool GetIsSpecialMatch(void *match, Int, UInt *unk) {
    *unk = 0;
    CCompID compId;
    CallMethod<0xE80190>(match, &compId);
    if (compId.type == COMP_U20_WC_Q || compId.type == COMP_U20_WORLD_CUP)
        return false;
    return CallMethodAndReturn<Bool, 0xE82EC0>(match);
}

CTeamIndex *METHOD OnGetTeamIDMDSelectScreen_H(void *match, DUMMY_ARG, CTeamIndex *ret_teamID) {
    CallMethod<0xE7FCF0>(match, ret_teamID);
    if ((ret_teamID->type == 2 || ret_teamID->type == 4) && ret_teamID->index != 0xFFFF)
        ret_teamID->type = 0;
    return ret_teamID;
}

CTeamIndex *METHOD OnGetTeamIDMDSelectScreen_A(void *match, DUMMY_ARG, CTeamIndex *ret_teamID) {
    CallMethod<0xE7FD00>(match, ret_teamID);
    if ((ret_teamID->type == 2 || ret_teamID->type == 4) && ret_teamID->index != 0xFFFF)
        ret_teamID->type = 0;
    return ret_teamID;
}

void METHOD SetupLeagueMatchImportance(CDBLeague *league, DUMMY_ARG, void *match) {
    if (league->GetCompID().type == COMP_ICC) {
        CallMethod<0x137DB90>(league, match); // friendly
        return;
    }
    CallMethod<0x1053430>(league, match); // league
}

void METHOD SetupRoundMatchImportance(CDBRound *round, DUMMY_ARG, void *match) {
    if (round->GetCompID().type == COMP_ICC) {
        CallMethod<0x137DB90>(round, match); // friendly
        return;
    }
    CallMethod<0x1042570>(round, match); // round
}

CDBCompetition *gInternationalPlayersCompetition = nullptr;

CDBRoot * METHOD GetInternationalPlayersCompetitionRoot(CDBCompetition *comp) {
    gInternationalPlayersCompetition = comp;
    return CallMethodAndReturn<CDBRoot *, 0xF8B680>(comp);
}

bool METHOD InternationalPlayersCheck(CDBGame *game, DUMMY_ARG, Int flag) {
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

CTeamIndex & METHOD GetEuropeanAssessmentCupRunnerUp(CDBCompetition *comp) {
    static CTeamIndex nullTeam = CTeamIndex::make(0, 0, 0);
    return nullTeam;
}

Bool METHOD OnAddCupTeam(CDBCup *cup, DUMMY_ARG, CTeamIndex const &teamIndex, Int putAt) {
    if (cup->GetCompID().ToInt() == 0x28040001) {
        CDBCompetition *iccPool = GetCompetition(FifamCompRegion::Europe, FifamCompType::ICC, 0);
        if (iccPool && iccPool->IsTeamPresent(teamIndex))
            return false;
    }
    return CallMethodAndReturn<Bool, 0xF85C80>(cup, &teamIndex, putAt);
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
    if (compId.countryId == FifamCompRegion::Europe && roundType != 14)
        Call<0xF4AB00>(compId, roundType, unk);
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
    UChar compTypes[] = { COMP_UIC, COMP_ICC, COMP_YOUTH_CHAMPIONSLEAGUE, COMP_CONTINENTAL_1, COMP_CONTINENTAL_2 };
    for (UChar compType : compTypes)
        *CallMethodAndReturn<UChar *, 0x651180>(m, &compType) = 1;
    return CallMethodAndReturn<UChar *, 0x651180>(m, arg);
}

UChar *METHOD OnFillClubInfoFixturesList_NT(void *m, DUMMY_ARG, UChar *arg) {
    UChar compTypes[] = { COMP_EURO_NL_Q, COMP_EURO_NL, COMP_NAM_NL_Q, COMP_NAM_NL, COMP_NAM_CUP, COMP_AFRICA_CUP_Q, COMP_AFRICA_CUP, COMP_ASIA_CUP_Q, COMP_ASIA_CUP, COMP_OFC_CUP_Q, COMP_OFC_CUP };
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
        (outCompId->type == FifamCompType::ChampionsLeague || outCompId->type == FifamCompType::UefaCup))
    {
        outCompId->index = 0;
    }
    return result;
}

// NOTE: QUALI_EC 0 and QUALI_WC 8 are hardcoded now in the plugin

void METHOD OnPoolLaunch(CDBPool* pool) {
    if (pool->GetCompetitionType() == COMP_QUALI_EC && pool->GetCompID().index == 0 && (GetCurrentYear() % 4) != 2)
        return;
    if (pool->GetCompetitionType() == COMP_QUALI_WC && pool->GetCompID().index == 8 && (GetCurrentYear() % 4) != 0)
        return;
    CallMethod<0x10F1A40>(pool);
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
                CDBGame* game = CDBGame::GetInstance();
                if (game && GetCurrentYear() == 2020) {
                    Set<UInt> eastTeams = {
                        0x5F0002, // New York City FC
                        0x5F0003, // Atlanta United F.C
                        0x5F0005, // Philadelphia Union
                        0x5F0009, // Toronto FC
                        0x5F000A, // D.C. United
                        0x5F000C, // New York Red Bulls
                        0x5F000E, // New England Revolution
                        0x5F0011, // Chicago Fire FC
                        0x5F0012, // Montreal Impact
                        0x5F0014, // Columbus Crew SC
                        0x5F0016, // Orlando City SC
                        0x5F0018, // FC Cincinnati
                    };
                    if (!eastTeams.contains(teamId->ToInt()))
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
                CDBGame* game = CDBGame::GetInstance();
                if (game && GetCurrentYear() == 2020) {
                    Set<UInt> westTeams = {
                        0x5F0001, // Los Angeles Football Club
                        0x5F0004, // Seattle Sounders FC
                        0x5F0006, // Real Salt Lake
                        0x5F0007, // Minnesota United FC
                        0x5F0008, // LA Galaxy
                        0x5F000B, // Portland Timbers
                        0x5F000D, // FC Dallas
                        0x5F000F, // San Jose Earthquakes
                        0x5F0010, // Colorado Rapids
                        0x5F0013, // Houston Dynamo
                        0x5F0015, // Sporting Kansas City
                        0x5F0017, // Vancouver Whitecaps FC
                    };
                    if (!westTeams.contains(teamId->ToInt()))
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

void PatchCompetitions(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

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

        //patch::SetPointer(0x24B1874, GetPoolNumberOfTeamsFromCountry);
        patch::SetUChar(0x10F109C + 3, 240);
        patch::SetUChar(0x10F10E3 + 3, 240);

        patch::RedirectCall(0x88F9DD, OnFillEuropeanCompsParticipants);
        patch::RedirectCall(0x88F9ED, OnFillEuropeanCompsParticipants);
        //
        patch::RedirectCall(0x88F73C, EuropeanCompsParticipants_GetNumEntries);
        patch::RedirectCall(0x88F85D, EuropeanCompsParticipants_GetNumEntries);
        patch::RedirectCall(0x88F75D, EuropeanCompsParticipants_GetCountryAtPosition);
        //patch::RedirectCall(0x88F6F6, EuropeanCompsParticipants_GetPool);

        patch::SetUChar(0x88F814 + 1, 8);
        patch::RedirectCall(0x88FA6F, EuropeanCompsParticipantsInitColumns);

        patch::RedirectCall(0xFF203B, OnSetupAssessmentEntry);
        //patch::RedirectCall(0x139D4A2, GetCountryAtAssessmentPosition);
        //patch::RedirectCall(0x139DA74, GetCountryAtAssessmentPosition);
        //patch::RedirectCall(0x139EFC8, GetCountryAtAssessmentPosition);

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

                                               // 3  4  5   6   7  8   9  10 11 12 13 14 15  16  17 18 19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35 36  37  38  39  40  41  42  43  44  45  46  47  48  49  50  51  52  53  54  55  56  57  58  59
        static UChar gCompWinYearsJumpTable[] = { 0, 1, 11, 11, 2, 11, 3, 4, 5, 6, 7, 5, 11, 11, 8, 9, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 5, 11, 10, 11,  5,  5, 11, 10, 10, 11, 10, 11, 10, 11, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };
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
            COMP_AFRICA_CUP, COMP_ASIA_CUP, COMP_OFC_CUP, COMP_U20_WORLD_CUP };
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

        patch::RedirectCall(0x11F2B93, OnSetupRootInternationalComps);

        // translated competition name
        patch::RedirectCall(0x9A3EB6, ReadCompetitionName);
        patch::RedirectCall(0x1042116, ReadCompetitionName);
        patch::RedirectCall(0x1056196, ReadCompetitionName);

        // Champions League IDs
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

        patch::SetUInt(0x10439F1 + 2, 0xF9000000);
        patch::SetUInt(0x1043A06 + 2, 0xF9000000);
        patch::SetUInt(0x1043A1D + 2, 0xF9000000);
        patch::SetUInt(0x1043A34 + 2, 0xF9000000);
        patch::SetUInt(0x1043A4B + 2, 0xF9000000);
        patch::SetUInt(0x1043A62 + 2, 0xF9000000);
        patch::SetUInt(0x1043A79 + 2, 0xF9000000);
        patch::SetUInt(0x1043A90 + 2, 0xF9000000);
        patch::SetUInt(0x1043AA7 + 2, 0xF9000000);

        patch::SetUInt(0x10454AD + 2, 0xF9090018);

        patch::SetUInt(0x10F17B2 + 2, 0xF9090014);

        patch::SetUInt(0x10F1B51 + 2, 0xF909000A);

        patch::SetUInt(0x1132451 + 6, 0xF909000B);
        patch::SetUInt(0x113245B + 6, 0xF90A000A);
        patch::SetUInt(0x1132465 + 6, 0xF90A0012);

        patch::SetUChar(0x10F28F4 + 1, 11);
        patch::SetUChar(0x10F2902 + 1, 12);
        patch::SetUChar(0x10F2917 + 1, 13);
        patch::SetUChar(0x10F292C + 1, 14);
        patch::SetUChar(0x10F2941 + 1, 15);
        patch::SetUChar(0x10F2956 + 1, 16);
        patch::SetUChar(0x10F296E + 1, 17);
        patch::SetUChar(0x10F2983 + 1, 18);

        patch::SetUInt(0x137A118 + 1, 10);
        patch::SetUChar(0x137A595 + 2, 24);

        patch::SetUChar(0x11F172C + 2, 22);
        patch::SetUChar(0x11F1731 + 1, 22);

        patch::SetUInt(0x10F4314 + 2, 0xF90A0016);

        patch::SetUInt(0x897857 + 6, 0xF90A000A);
        patch::SetUInt(0x897861 + 6, 0xF90A000B);
        patch::SetUInt(0x89786B + 6, 0xF90A000C);
        patch::SetUInt(0x897875 + 6, 0xF90A000D);
        patch::SetUInt(0x89787F + 6, 0xF90A000E);
        patch::SetUInt(0x897889 + 6, 0xF90A000F);
        patch::SetUInt(0x897893 + 6, 0xF90A0010);
        patch::SetUInt(0x89789D + 6, 0xF90A0011);
        patch::SetUInt(0x8978A7 + 6, 0xF90A0012);
        patch::SetUInt(0x8978B1 + 6, 0xF90A0013);
        patch::SetUInt(0x8978BB + 6, 0xF90A0014);
        patch::SetUInt(0x8978C5 + 6, 0xF90A0015);

        patch::SetUInt(0x89790C + 2, 0xF90A0017);
        patch::SetUInt(0x897929 + 2, 0xF90A0018);
        patch::SetUInt(0x897957 + 2, 0xF90A0019);
        patch::SetUInt(0x897977 + 2, 0xF90A001A);

        // Copa America IDs
        patch::SetUChar(0x836D48 + 1, 4);
        patch::SetUChar(0x836D67 + 1, 5);
        patch::SetUChar(0x836D86 + 1, 7);
        patch::SetUChar(0x836E07 + 1, 6);

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
        patch::RedirectCall(0x7BD405, GetCompetitionFinalForSimulationScreen);
        patch::RedirectCall(0x7BD315, GetCompetitionPoolForSimulationScreen);
        patch::RedirectCall(0x7BD395, GetCompetitionPoolForSimulationScreen);
        patch::RedirectCall(0x7BD405, GetCompetitionPoolForSimulationScreen);
        patch::RedirectCall(0x7BD4F5, GetECFinalForSimulation);
        patch::RedirectCall(0x7BD505, GetECPoolForSimulation);
        patch::RedirectCall(0x7BD475, GetWCFinalForSimulation);

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

        // international matches
        patch::RedirectCall(0x1044CE5, GetIsInternationalCompQuali);
        patch::RedirectCall(0x1044FFE, GetIsInternationalCompQuali);
        patch::RedirectCall(0x105E8C7, GetIsInternationalCompQuali);

        // test youth in 3d
        patch::RedirectCall(0xAE68EF, OnGetTeamIDMDSelectScreen_H);
        patch::RedirectCall(0xAE691B, OnGetTeamIDMDSelectScreen_A);

        static UInt watchCompIDs[] = {
            0xF9090000, 0xF90A0000, 0xF90D0000, 0xF9260000,
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
        patch::RedirectCall(0x139D4EE, GetEuropeanAssessmentCupRunnerUp);

        // calendar icons
        //patch::SetUChar(0xEFDD85 + 2, gNumCompetitionTypes);
        //patch::SetUChar(0xEFDD41 + 2, 120);
        // temporary fix
        //patch::SetUInt(0xEFDCEC + 1, 50);
        patch::SetUChar(0xEFDD85 + 2, COMP_WORLD_CLUB_CHAMP);

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
        patch::SetUInt(0x8875CA + 4, 0xF90D0008);

        // remove loading of all clubs when all league levels selected
       // patch::Nop(0xF954DC, 1);
       // patch::SetUChar(0xF954DC + 1, 0xE9);

        static UChar gCompSpectatorCalcType[] = {
            0, 2, 2, 3, 4, 4, 4, 5, 8, 10, 6, 6, 7, 6, 6, 2, 2, 2, 2, 8, 8, 8, 2, 9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0,
            30, 2, 2, 31, 32, 32, 2, 2, 2, 2, 2, 2, 2, 2, 2
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
    }
}
