#include "LeagueSplit.h"
#include "FifamReadWrite.h"
#include "FifamCompRegion.h"
#include "FifamCompType.h"
#include "FifamBeg.h"
#include "FifamNation.h"
#include "shared.h"

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

void METHOD MyLeagueGetTopScorers(CDBLeague *league, DUMMY_ARG, TopScorersBuffer &buf, Int minuteMin, Int minuteMax,
    UInt matchdayMin, UInt matchdayMax, UChar flags, Bool clearBuffer, Int sortCriteria1, Int sortCriteria2)
{
    if (IsCompetitionLeagueWithSplit(league->GetCompID())) {
        Bool initialClear = false;
        if (matchdayMin < league->GetNumMatchdays())
            CallMethod<0x106E0A0>(league, &buf, minuteMin, minuteMax, matchdayMin, matchdayMax, flags, clearBuffer, sortCriteria1, sortCriteria2);
        else
            initialClear = clearBuffer;
        if (matchdayMax >= league->GetNumMatchdays() && IsCompetitionLeagueWithSplit(league->GetCompID())) {
            UInt splitMin = (matchdayMin <= league->GetNumMatchdays()) ? 0 : matchdayMin - league->GetNumMatchdays();
            UInt splitMax = matchdayMax - league->GetNumMatchdays();
            for (UInt i = 0; i < 2; i++) {
                auto relegation = GetLeague(CCompID::Make(league->GetCompID().countryId, COMP_RELEGATION, i));
                if (relegation)
                    CallMethod<0x106E0A0>(relegation, &buf, minuteMin, minuteMax, splitMin, splitMax, flags, (i == 0) ? initialClear : false, sortCriteria1, sortCriteria2);
            }
        }
    }
    else
        CallMethod<0x106E0A0>(league, &buf, minuteMin, minuteMax, matchdayMin, matchdayMax, flags, clearBuffer, sortCriteria1, sortCriteria2);
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

const UInt OriginalNewspaperTopScorerSize = 0x4D8;
const UInt OriginalNewspaperGoalkeeperSize = 0x4F0;

struct NewspaperTopScorerGkExtension {
    CXgImage *ImgPicture;
    CXgTextBox *TbPhoto;
    CXgTextBox *TbName[2];
    CXgTextBox *TbDescWithPhoto[2];
    CXgTextBox *TbDescNoPhoto[2];
    CXgTextBox *TbTrophy[3];
    CXgTextBox *TbNotDrawn;
};

template<Bool IsGK>
void METHOD OnNewspaperTopScorerGkCreateUI(CXgFMPanel *screen) {
    if (IsGK)
        CallMethod<0xA494F0>(screen);
    else
        CallMethod<0xA6BBA0>(screen);
    auto ext = raw_ptr<NewspaperTopScorerGkExtension>(screen, IsGK ? OriginalNewspaperGoalkeeperSize : OriginalNewspaperTopScorerSize);
    ext->ImgPicture = screen->GetImage("ImgPicture");
    ext->TbPhoto = screen->GetTextBox("TbPhoto");
    ext->TbName[0] = screen->GetTextBox("TbName");
    ext->TbName[1] = screen->GetTextBox("TbNameShadow");
    ext->TbDescWithPhoto[0] = screen->GetTextBox("TbDescWithPhoto");
    ext->TbDescWithPhoto[1] = screen->GetTextBox("TbDescWithPhotoShadow");
    ext->TbDescNoPhoto[0] = screen->GetTextBox("TbDescNoPhoto");
    ext->TbDescNoPhoto[1] = screen->GetTextBox("TbDescNoPhotoShadow");
    for (UInt i = 0; i < 3; i++)
        ext->TbTrophy[i] = screen->GetTextBox(Utils::Format("TbTrophy%d", i + 1).c_str());
    ext->TbNotDrawn = screen->GetTextBox("TbNotDrawn");
}

void SetupNewspaperTopScorerGkExtension(NewspaperTopScorerGkExtension *ext, CCompID const &compId, Array<Bool, 3> winners, Bool isGk) {
    String compIdStr = Utils::Format(L"%08X", compId.ToInt());
    String basePath = isGk ? L"art_fm\\lib\\Trophies\\goalkeeper\\" : L"art_fm\\lib\\Trophies\\topscorer\\";
    String imgPath;
    if (GetFilenameForImageIfExists(imgPath, basePath + L"picture", compIdStr))
        SetImageFilename(ext->ImgPicture, imgPath.c_str(), 4, 4);
    Bool hasPhoto = GetFilenameForImageIfExists(imgPath, basePath + L"photo", compIdStr);
    if (hasPhoto)
        SetImageFilename(ext->TbPhoto, imgPath.c_str(), 4, 4);
    ext->TbPhoto->SetVisible(hasPhoto);
    auto nameText = GetTranslationIfPresent(Utils::Format(isGk ? "IDS_BESTGK_NAME_%08X" : "IDS_TOPSCORER_NAME_%08X", compId.ToInt()).c_str());
    auto descText = GetTranslationIfPresent(Utils::Format(isGk ? "IDS_BESTGK_DESC_%08X" : "IDS_TOPSCORER_DESC_%08X", compId.ToInt()).c_str());
    for (UInt i = 0; i < 2; i++) {
        if (nameText)
            ext->TbName[i]->SetText(nameText);
        ext->TbName[i]->SetEnabled(nameText ? true : false);
        if (descText) {
            if (hasPhoto)
                ext->TbDescWithPhoto[i]->SetText(descText);
            else
                ext->TbDescNoPhoto[i]->SetText(descText);
        }
        ext->TbDescWithPhoto[i]->SetVisible(hasPhoto && descText);
        ext->TbDescNoPhoto[i]->SetVisible(!hasPhoto && descText);
    }
    Bool hasTrophyPic = GetFilenameForImageIfExists(imgPath, basePath, compIdStr);
    for (UInt i = 0; i < 3; i++) {
        if (hasTrophyPic && winners[i]) {
            SetImageFilename(ext->TbTrophy[i], imgPath.c_str(), 4, 4);
            ext->TbTrophy[i]->SetVisible(true);
        }
        else
            ext->TbTrophy[i]->SetVisible(false);
    }
}

CTeamIndex GetPlayerLastTeamFromLeagueInCurrentSeason(UInt playerId, CDBLeague *league) {
    CDBPlayerCareerList *cl = (CDBPlayerCareerList *)GetObjectByID(GetIDForObject(2, playerId));
    if (cl && cl->GetNumEntries()) {
        CJDate seasonStartDate = Game()->GetCurrentSeasonStartDate();
        for (Int e = cl->GetNumEntries() - 1; e >= 0; e--) {
            if (cl->GetEndDate(e).IsNull() || cl->GetEndDate(e) > seasonStartDate) {
                CTeamIndex teamID = cl->GetTeamID(e);
                if (!teamID.isNull()) {
                    auto team = GetTeam(teamID);
                    if (team && league->IsTeamPresent(teamID))
                        return teamID;
                }
            }
        }
    }
    return CTeamIndex();
}

void METHOD SetupNewspaperTopScorer(CXgFMPanel *screen) {
    UInt numPlayersAdded = 0;
    Array<Bool, 3> winners = {};
    auto Tile = raw_ptr<CXgImage *>(screen, 0x48C);
    auto TbCrest = raw_ptr<CXgTextBox *>(screen, 0x498);
    auto TbPic = raw_ptr<CXgTextBox *>(screen, 0x4A4);
    auto TbPlayer = raw_ptr<CXgTextBox *>(screen, 0x4B0);
    auto TbTeam = raw_ptr<CXgTextBox *>(screen, 0x4BC);
    auto TbGoals = raw_ptr<CXgTextBox *>(screen, 0x4C8);
    for (UInt i = 0; i < 3; i++) {
        Tile[i]->SetVisible(false);
        TbCrest[i]->SetVisible(false);
        TbPic[i]->SetVisible(false);
        TbPlayer[i]->SetVisible(false);
        TbTeam[i]->SetVisible(false);
        TbGoals[i]->SetVisible(false);
    }
    auto ext = raw_ptr<NewspaperTopScorerGkExtension>(screen, OriginalNewspaperTopScorerSize);
    CCompID leagueID;
    auto team = CurrentUser().GetTeam();
    if (team) {
        auto league = GetLeague(team->GetFirstTeamLeagueID());
        if (league) {
            leagueID = league->GetCompID();
            TopScorersBuffer topScorers;
            MyLeagueGetTopScorers(league, 0, topScorers, 0, 120, 0, 120, 3, true, 1, 2);
            UInt winnerIndex = 0;
            for (UInt i = 0; i < Utils::Min(3u, topScorers.Size()); i++) {
                auto topScorer = topScorers.At(i);
                auto player = GetPlayer(topScorer->GetPlayerId());
                if (player) {
                    Tile[numPlayersAdded]->SetVisible(true);
                    Bool isWinner = numPlayersAdded == 0;
                    if (isWinner)
                        winnerIndex = i;
                    else
                        isWinner = topScorer->GetGoals() == topScorers.At(winnerIndex)->GetGoals();
                    screen->SetPlayerName(TbPlayer[numPlayersAdded], player->GetID());
                    TbPlayer[numPlayersAdded]->SetVisible(true);
                    screen->SetPlayerPortrait(TbPic[numPlayersAdded], player->GetID());
                    TbPic[numPlayersAdded]->SetVisible(true);
                    String goalsStr = Utils::Format(GetTranslation("ID_NUM_GOALS"), topScorer->GetGoals());
                    TbGoals[numPlayersAdded]->SetText(goalsStr.c_str());
                    TbGoals[numPlayersAdded]->SetVisible(true);
                    CTeamIndex teamToDisplay;
                    //CTeamIndex playerTeamID = player->GetCurrentTeam();
                    //if (!playerTeamID.isNull()) {
                    //    auto playerTeam = GetTeam(playerTeamID);
                    //    if (playerTeam && playerTeam->GetFirstTeamLeagueID() == leagueID)
                    //        teamToDisplay = playerTeamID;
                    //}
                    if (teamToDisplay.isNull())
                        teamToDisplay = GetPlayerLastTeamFromLeagueInCurrentSeason(player->GetID(), league);
                    if (!teamToDisplay.isNull()) {
                        screen->SetTeamName(TbTeam[numPlayersAdded], teamToDisplay);
                        TbTeam[numPlayersAdded]->SetVisible(true);
                        screen->SetTeamBadge(TbCrest[numPlayersAdded], teamToDisplay);
                        TbCrest[numPlayersAdded]->SetVisible(true);
                    }
                    winners[numPlayersAdded] = isWinner;
                    numPlayersAdded++;
                }
            }
        }
    }
    ext->TbNotDrawn->SetVisible(numPlayersAdded == 0);
    SetupNewspaperTopScorerGkExtension(ext, leagueID, winners, false);
}

struct GkAppearanceInfo {
    UChar minuteIn = 0, minuteOut = 0, concededGoals = 0;
    Bool onTheBench = false, homeTeam = false;
};

struct NewspaperGoalkeeperInfo {
    UInt playerId = 0;
    CTeamIndex teamID;
    UChar matchesPlayed = 0;
    UChar cleanSheets = 0;
    UShort goalsConceded = 0;
    UShort minutesPlayed = 0;
    Float coefficient = 0.0f;
};

void METHOD SetupNewspaperGoalkeeper(CXgFMPanel *screen) {
    UInt numPlayersAdded = 0;
    Array<Bool, 3> winners = {};
    auto Tile = raw_ptr<CXgImage *>(screen, 0x48C);
    auto TbCrest = raw_ptr<CXgTextBox *>(screen, 0x498);
    auto TbPic = raw_ptr<CXgTextBox *>(screen, 0x4A4);
    auto TbPlayer = raw_ptr<CXgTextBox *>(screen, 0x4B0);
    auto TbTeam = raw_ptr<CXgTextBox *>(screen, 0x4BC);
    auto TbGoals = raw_ptr<CXgTextBox *>(screen, 0x4C8);
    auto TbMatches = raw_ptr<CXgTextBox *>(screen, 0x4D4);
    auto TbConcededGoals = raw_ptr<CXgTextBox *>(screen, 0x4E0);
    for (UInt i = 0; i < 3; i++) {
        Tile[i]->SetVisible(false);
        TbCrest[i]->SetVisible(false);
        TbPic[i]->SetVisible(false);
        TbPlayer[i]->SetVisible(false);
        TbTeam[i]->SetVisible(false);
        TbGoals[i]->SetVisible(false);
        TbMatches[i]->SetVisible(false);
        TbConcededGoals[i]->SetVisible(false);
    }
    auto ext = raw_ptr<NewspaperTopScorerGkExtension>(screen, OriginalNewspaperGoalkeeperSize);
    CCompID leagueID;
    auto team = CurrentUser().GetTeam();
    if (team) {
        auto mainLeague = GetLeague(team->GetFirstTeamLeagueID());
        if (mainLeague) {
            leagueID = mainLeague->GetCompID();
            Bool IsEngland = leagueID.index == 0 && leagueID.countryId == FifamNation::England;
            UInt leagueMatchdays = mainLeague->GetNumMatchdays();
            UInt leagueSplitMatchdays = 0;
            Vector<CDBLeague *> leaguesToCheck;
            leaguesToCheck.push_back(mainLeague);
            if (IsCompetitionLeagueWithSplit(leagueID)) {
                for (UInt i = 0; i < 2; i++) {
                    auto relegation = GetLeague(CCompID::Make(leagueID.countryId, COMP_RELEGATION, i));
                    if (relegation) {
                        leaguesToCheck.push_back(relegation);
                        if (relegation->GetNumMatchdays() > leagueSplitMatchdays)
                            leagueSplitMatchdays = relegation->GetNumMatchdays();
                    }
                }
                leagueMatchdays += leagueSplitMatchdays;
            }
            Map<UInt, NewspaperGoalkeeperInfo> goalkeepersMap;
            for (auto league : leaguesToCheck) {
                auto root = league->GetRoot();
                for (UInt matchday = 0; matchday < league->GetNumMatchdays(); matchday++) {
                    for (UInt matchIndex = 0; matchIndex < league->GetMatchesInMatchday(); matchIndex++) {
                        CMatch match;
                        league->GetMatch(matchday, matchIndex, match);
                        if (match.CheckFlag(FifamBeg::_1stPlayed)) {
                            CDBMatchEventEntry event;
                            CTeamIndex teams[2]{};
                            league->GetFixtureTeams(matchday, matchIndex, teams[0], teams[1]);
                            if (!teams[0].isNull() && !teams[1].isNull()) {
                                Map<UInt, GkAppearanceInfo> appInfo;
                                Int eventIndex = match.GetMatchEventsStartIndex();
                                root->GetMatchEvent(eventIndex, event);
                                while (event.GetEventType() != MET_END) {
                                    if (event.GetEventType() == MET_PLAYER_APPEARANCE) {
                                        UInt playerIndex = 0;
                                        while (event.GetEventType() == MET_PLAYER_APPEARANCE) {
                                            for (UInt i = 0; i < 3; ++i) {
                                                UInt appPlayer = event.GetValue(i);
                                                if (appPlayer) {
                                                    auto &app = appInfo[appPlayer];
                                                    app.homeTeam = event.IsHomeTeam();
                                                    app.onTheBench = (playerIndex > 10);
                                                }
                                                playerIndex++;
                                            }
                                            root->GetMatchEvent(++eventIndex, event);
                                        }
                                    }
                                    if (event.GetEventType() == MET_END)
                                        break;
                                    else if (event.GetEventType() == MET_CONCEDED_GOAL) {
                                        if (event.GetMinute() != 250) {
                                            UInt playerId = event.GetPlayerAffected();
                                            if (playerId) {
                                                auto &gk = goalkeepersMap[playerId];
                                                gk.goalsConceded += 1;
                                                gk.playerId = playerId;
                                                appInfo[playerId].concededGoals += 1;
                                            }
                                        }
                                    }
                                    else if (event.GetEventType() == MET_YELRED_CARD || event.GetEventType() == MET_RED_CARD || event.GetEventType() == MET_SUBST) {
                                        UInt outPlayer = event.GetValue(0);
                                        if (outPlayer)
                                            appInfo[outPlayer].minuteOut = event.GetMinute();
                                        if (event.GetEventType() == MET_SUBST) {
                                            UInt inPlayer = event.GetValue(1);
                                            if (inPlayer)
                                                appInfo[inPlayer].minuteIn = event.GetMinute();
                                        }
                                    }
                                    root->GetMatchEvent(++eventIndex, event);
                                }
                                for (auto const &[playerId, app] : appInfo) {
                                    if (!app.onTheBench || app.minuteIn) {
                                        CDBPlayer *player = GetPlayer(playerId);
                                        if (player && (Utils::Contains(goalkeepersMap, playerId) || player->IsGoalkeeper())) {
                                            auto &gk = goalkeepersMap[playerId];
                                            gk.playerId = playerId;
                                            gk.teamID = app.homeTeam ? teams[0] : teams[1];
                                            Int minutesPlayed = Utils::Clamp(app.minuteOut ?
                                                app.minuteOut - app.minuteIn : event.GetMinute() - app.minuteIn, 1, 120);
                                            if (minutesPlayed >= 60)
                                                gk.matchesPlayed += 1;
                                            Bool cleanSheet = app.concededGoals == 0 && minutesPlayed >= 90
                                                && !app.onTheBench && !app.minuteIn && !app.minuteOut;
                                            if (cleanSheet)
                                                gk.cleanSheets += 1;
                                            gk.minutesPlayed += minutesPlayed;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            Vector<NewspaperGoalkeeperInfo> goalkeepers;
            goalkeepers.reserve(goalkeepersMap.size());
            UChar minMatches = (UChar)((Float)leagueMatchdays * 0.74f); // 38 => 28, 46 => 34, 8 => 5
            if (minMatches < 1)
                minMatches = 1;
            for (auto &[playerId, gk] : goalkeepersMap) {
                if (IsEngland) {
                    if (gk.cleanSheets > 0)
                        goalkeepers.push_back(gk);
                }
                else {
                    if (gk.matchesPlayed >= minMatches) {
                        gk.coefficient = std::round((Float)gk.goalsConceded / (Float)gk.matchesPlayed * 100.0f) / 100.0f;
                        goalkeepers.push_back(gk);
                    }
                }
            }
            if (IsEngland) {
                Utils::Sort(goalkeepers, [](NewspaperGoalkeeperInfo const &a, NewspaperGoalkeeperInfo const &b) {
                    if (a.cleanSheets > b.cleanSheets)
                        return true;
                    if (b.cleanSheets > a.cleanSheets)
                        return false;
                    return a.minutesPlayed > b.minutesPlayed;
                });
            } 
            else {
                Utils::Sort(goalkeepers, [](NewspaperGoalkeeperInfo const &a, NewspaperGoalkeeperInfo const &b) {
                    return a.coefficient < b.coefficient;
                });
            }
            UInt winnerIndex = 0;
            for (UInt i = 0; i < Utils::Min(3u, goalkeepers.size()); i++) {
                auto const &gk = goalkeepers[i];
                auto player = GetPlayer(gk.playerId);
                if (player) {
                    Tile[numPlayersAdded]->SetVisible(true);
                    Bool isWinner = numPlayersAdded == 0;
                    if (isWinner)
                        winnerIndex = i;
                    else {
                        if (IsEngland) {
                            isWinner = gk.cleanSheets == goalkeepers[winnerIndex].cleanSheets
                                && gk.minutesPlayed == goalkeepers[winnerIndex].minutesPlayed;
                        }
                        else
                            isWinner = gk.coefficient == goalkeepers[winnerIndex].coefficient;
                    }
                    screen->SetPlayerName(TbPlayer[numPlayersAdded], player->GetID());
                    TbPlayer[numPlayersAdded]->SetVisible(true);
                    screen->SetPlayerPortrait(TbPic[numPlayersAdded], player->GetID());
                    TbPic[numPlayersAdded]->SetVisible(true);
                    // England: matches (TbMatches), clean sheets (TbConcededGoals)
                    // Spain: matches (TbMatches), conceded goals (TbConcededGoals), coefficient (TbGoals)
                    String matchesStr = Utils::Format(GetTranslation("IDS_NEWSPAPER_MATCHES"), gk.matchesPlayed);
                    TbMatches[numPlayersAdded]->SetText(matchesStr.c_str());
                    TbMatches[numPlayersAdded]->SetVisible(true);
                    String concededGoalsStr = IsEngland ?
                        Utils::Format(GetTranslation("IDS_NEWSPAPER_CLEANSHEETS"), gk.cleanSheets) :
                        Utils::Format(GetTranslation("IDS_NEWSPAPER_CONGOALS"), gk.goalsConceded);
                    TbConcededGoals[numPlayersAdded]->SetText(concededGoalsStr.c_str());
                    TbConcededGoals[numPlayersAdded]->SetVisible(true);
                    String coefficientStr = IsEngland ?
                        Utils::Format(GetTranslation("IDS_NEWSPAPER_MINUTES"), gk.minutesPlayed) :
                        Utils::Format(GetTranslation("IDS_NEWSPAPER_COEFFICIENT"), gk.coefficient);
                    TbGoals[numPlayersAdded]->SetText(coefficientStr.c_str());
                    TbGoals[numPlayersAdded]->SetVisible(true);
                    if (!gk.teamID.isNull()) {
                        screen->SetTeamName(TbTeam[numPlayersAdded], gk.teamID);
                        TbTeam[numPlayersAdded]->SetVisible(true);
                        screen->SetTeamBadge(TbCrest[numPlayersAdded], gk.teamID);
                        TbCrest[numPlayersAdded]->SetVisible(true);
                    }
                    winners[numPlayersAdded] = isWinner;
                    numPlayersAdded++;
                }
            }
        }
    }
    ext->TbNotDrawn->SetVisible(numPlayersAdded == 0);
    SetupNewspaperTopScorerGkExtension(ext, leagueID, winners, true);
}

Int METHOD OnPlayerInfoGetSeasonStats(CPlayerStats *stats, DUMMY_ARG, CMatchStatistics &outStats, UInt &numConsecutiveHomeMatches,
    UInt &numMatchesWithMark, UInt &numMOTMs, Bool bCurrentClubOnly, CCompID const &compID, UInt minMinutesForMark, Bool bLocalizedMarks,
    Bool bWithFriendlies, CTeamIndex teamID, UInt compIDMask, Bool bWithoutQuali)
{
    Int result = stats->GetSeasonStats(outStats, numConsecutiveHomeMatches, numMatchesWithMark, numMOTMs, bCurrentClubOnly,
        compID, minMinutesForMark, bLocalizedMarks, bWithFriendlies, teamID, compIDMask, bWithoutQuali);
    if (IsCompetitionLeagueWithSplit(compID)) {
        for (UInt i = 0; i < 2; i++) {
            result += stats->GetSeasonStats(outStats, numConsecutiveHomeMatches, numMatchesWithMark, numMOTMs, bCurrentClubOnly,
                CCompID::Make(compID.countryId, COMP_RELEGATION, i), minMinutesForMark, bLocalizedMarks, bWithFriendlies, teamID,
                compIDMask, bWithoutQuali);
        }
    }
    return result;
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

        // newspaper
        patch::SetUInt(0xA854A4 + 1, OriginalNewspaperTopScorerSize + sizeof(NewspaperTopScorerGkExtension));
        patch::SetUInt(0xA854AB + 1, OriginalNewspaperTopScorerSize + sizeof(NewspaperTopScorerGkExtension));
        patch::SetUInt(0xA855B4 + 1, OriginalNewspaperGoalkeeperSize + sizeof(NewspaperTopScorerGkExtension));
        patch::SetUInt(0xA855BB + 1, OriginalNewspaperGoalkeeperSize + sizeof(NewspaperTopScorerGkExtension));
        patch::SetPointer(0x244D15C, OnNewspaperTopScorerGkCreateUI<false>);
        patch::SetPointer(0x244AAC4, OnNewspaperTopScorerGkCreateUI<true>);
        patch::RedirectCall(0xA6DDAE, SetupNewspaperTopScorer);
        patch::RedirectCall(0xA4C04E, SetupNewspaperGoalkeeper);

        // player info stats
        patch::RedirectCall(0x5CDD7E, OnPlayerInfoGetSeasonStats);
        patch::RedirectCall(0x5D343D, OnPlayerInfoGetSeasonStats); // print

    }
}
