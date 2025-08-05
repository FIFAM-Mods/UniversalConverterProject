#include "LeagueSplit.h"
#include "GameInterfaces.h"
#include "FifamReadWrite.h"
#include "FifamCompRegion.h"
#include "FifamCompType.h"

using namespace plugin;

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
    FifamReader leagueSplitReader("plugins\\ucp\\league_split.csv", 14);
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
    FifamReader specialLeagueCupsReader("plugins\\ucp\\special_league_cups.csv", 14);
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
    FifamReader leagueTablesReader("plugins\\ucp\\league_tables.csv", 14);
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

CCompID *METHOD LeagueSplit_GetCompId_League(CDBCompetition *comp, DUMMY_ARG, CCompID *out) {
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
        cmp[esp + 0x44], eax
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

UChar OnGetLevelWithReserveTeams(Int countryId) {
    if (countryId >= 1 && countryId <= 207 && gCountryReserveLevelId[countryId - 1] != 0)
        return gCountryReserveLevelId[countryId - 1] - 1;
    return 255;
}

Bool METHOD OnAddCupTeam(CDBCup *cup, DUMMY_ARG, CTeamIndex const &teamIndex, Int putAt) {
    if (cup->GetCompID().ToInt() == 0x2F030000 && IsLiechtensteinClubFromSwitzerland(teamIndex)) // { 47, FA_CUP, 0 }
        return false;
    return CallMethodAndReturn<Bool, 0xF85C80>(cup, &teamIndex, putAt);
}

unsigned char METHOD GetFACompCountryId_SkipReserveTeams(void *comp) {
    return 45;
}

void METHOD MyLeagueGetTopScorers(CDBLeague *league, DUMMY_ARG, void *buf, Int minuteMin, Int minuteMax,
    UInt matchdayMin, UInt matchdayMax, UChar flags, Bool clearBuffer, Int sortCriteria1, Int sortCriteria2)
{
    if (IsCompetitionLeagueWithSplit(league->GetCompID())) {
        Bool initialClear = false;
        if (matchdayMin < league->GetNumMatchdays())
            CallMethod<0x106E0A0>(league, buf, minuteMin, minuteMax, matchdayMin, matchdayMax, flags, clearBuffer, sortCriteria1, sortCriteria2);
        else
            initialClear = clearBuffer;
        if (matchdayMax >= league->GetNumMatchdays() && IsCompetitionLeagueWithSplit(league->GetCompID())) {
            UInt splitMin = (matchdayMin <= league->GetNumMatchdays()) ? 0 : matchdayMin - league->GetNumMatchdays();
            UInt splitMax = matchdayMax - league->GetNumMatchdays();
            for (UInt i = 0; i < 2; i++) {
                auto relegation = GetLeague(CCompID::Make(league->GetCompID().countryId, COMP_RELEGATION, i));
                if (relegation)
                    CallMethod<0x106E0A0>(relegation, buf, minuteMin, minuteMax, splitMin, splitMax, flags, (i == 0) ? initialClear : false, sortCriteria1, sortCriteria2);
            }
        }
    }
    else
        CallMethod<0x106E0A0>(league, buf, minuteMin, minuteMax, matchdayMin, matchdayMax, flags, clearBuffer, sortCriteria1, sortCriteria2);
}

UInt METHOD StatsTables_GetLeagueCurrentMatchday(CDBLeague *league) {
    UInt currentMatchday = league->GetCurrentMatchday();
    UInt leagueSplitMatchday = 0;
    if (currentMatchday == league->GetNumMatchdays() && IsCompetitionLeagueWithSplit(league->GetCompID())) {
        for (UInt i = 0; i < 2; i++) {
            auto relegation = GetLeague(CCompID::Make(league->GetCompID().countryId, COMP_RELEGATION, i));
            if (relegation && relegation->GetCurrentMatchday() > leagueSplitMatchday)
                leagueSplitMatchday = relegation->GetCurrentMatchday();
        }
        //leagueSplitMatchday++;
    }
    return currentMatchday + leagueSplitMatchday;
}

void PatchLeagueSplit(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadLeaguesConfig();

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

        // remove special places for Germany, Spain and Scotland
        patch::RedirectJump(0xF8E60A, (void *)0xF8E6DF);
        patch::RedirectCall(0x11F1925, GetPositionPromotionsRelegationsPlacesInfo);
        patch::RedirectCall(0x11F2408, GetPositionPromotionsRelegationsPlacesInfo);

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

        // team place in the league
        //patch::RedirectJump(0xEE0C40, GetLeagueInfo);
        patch::RedirectCall(0x10EC1B7, GetTeamLeaguePlace);

        // reserve league levels
        patch::Nop(0xFD6E19, 22);
        patch::RedirectJump(0xFD6D70, OnGetLevelWithReserveTeams);

        // include league split in league top scorer calculation
        patch::SetPointer(0x24ADF30, MyLeagueGetTopScorers);

        // stats screens - extend max matchday
        patch::RedirectCall(0x785608, StatsTables_GetLeagueCurrentMatchday);
        patch::RedirectCall(0x7853E6, StatsTables_GetLeagueCurrentMatchday);
    }
}
