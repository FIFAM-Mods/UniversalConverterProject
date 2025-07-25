#include "GameInterfaces.h"
#include "GfxCoreHook.h"
#include "Utils.h"
#ifndef SHARED_DO_NOT_LOG
#include "shared.h"
#endif
#include "FifamCompRegion.h"
#include "FifamLanguage.h"
#include "FifamBeg.h"

using namespace plugin;

CJDate CJDate::GetTranslated(int numYears) {
    CJDate result;
    plugin::CallMethod<0x1495EFF>(this, &result, numYears);
    return result;
}

unsigned short CJDate::GetYear() {
    return plugin::CallMethodAndReturn<unsigned short, 0x1494CC9>(this);
}

unsigned char CJDate::GetMonth() {
    return plugin::CallMethodAndReturn<unsigned char, 0x1494CA2>(this);
}

unsigned char CJDate::GetDays() {
    return plugin::CallMethodAndReturn<unsigned char, 0x1494C7B>(this);
}

void CJDate::GetYMD(unsigned short *outY, unsigned char *outM, unsigned char *outD) {
    plugin::CallMethod<0x1494AA2>(this, outY, outM, outD);
}

void CJDate::GetYMD(unsigned int *outY, unsigned int *outM, unsigned int *outD) {
    plugin::CallMethod<0x1494A81>(this, outY, outM, outD);
}

void CJDate::Set(unsigned short year, unsigned char month, unsigned char day) {
    CallMethod<0x1494AE2>(this, year, month, day);
}

UInt CJDate::Value() { return value; }

void CJDate::Set(UInt _value) {
    value = _value;
}

UInt CJDate::GetDayOfWeek() {
    return CallMethodAndReturn<UInt, 0x1494FBA>(this);
}

CJDate CJDate::DateFromDayOfWeek(UChar dayOfWeek, UChar month, UShort year) {
    CJDate date;
    Call<0x14966C4>(&date, dayOfWeek, month, year);
    return date;
}

void CJDate::AddRandomDaysCount(UChar direction) {
    CallMethod<0x149633E>(this, direction);
}

CJDate CJDate::AddYears(Int years) {
    CJDate outDate;
    CallMethod<0x1495EFF>(this, &outDate, years);
    return outDate;
}

String CJDate::ToStr() {
	return Utils::Format(L"%02d.%02d.%04d", GetDays(), GetMonth(), GetYear());
}

Bool CJDate::IsNull() {
    return value == 0;
}

Bool CJDate::operator>(const CJDate &other) const {
    return value > other.value;
}

Bool CJDate::operator<(const CJDate &other) const {
    return value < other.value;
}

CDBGame *CDBGame::GetInstance() {
    return plugin::CallAndReturn<CDBGame *, 0xF61410>();
}

CJDate CDBGame::GetStartDate() {
    CJDate result;
    plugin::CallMethod<0xF49950>(this, &result);
    return result;
}

CJDate CDBGame::GetCurrentDate() {
    CJDate result;
    plugin::CallMethod<0xF498D0>(this, &result);
    return result;
}

CJDate CDBGame::GetCurrentSeasonStartDate() {
    CJDate result;
    plugin::CallMethod<0xF499A0>(this, &result);
    return result;
}

CJDate CDBGame::GetCurrentSeasonEndDate() {
    CJDate result;
    plugin::CallMethod<0xF49A00>(this, &result);
    return result;
}

unsigned short CDBGame::GetCurrentYear() {
    return plugin::CallMethodAndReturn<unsigned short, 0xF498C0>(this);
}

bool CDBGame::GetIsWorldCupMode() {
    return plugin::CallMethodAndReturn<bool, 0xF4A340>(this);
}

bool CDBGame::TestFlag(unsigned int flag) {
    return plugin::CallMethodAndReturn<bool, 0xF49CA0>(this, flag);
}

void CDBGame::SetCurrentDate(CJDate date) {
    plugin::CallMethod<0xF49960>(this, date);
}

CDBGameOptions &CDBGame::GetOptions() {
    return *plugin::CallMethodAndReturn<CDBGameOptions *, 0xF49D70>(this);
}

bool CDBGame::IsCountryPlayable(UChar countryId) {
    return plugin::CallMethodAndReturn<bool, 0xF49AA0>(this, countryId);
}

UShort CDBGame::GetCurrentSeasonNumber() {
    return plugin::CallMethodAndReturn<UShort, 0xF49A80>(this);
}

bool CDBGameOptions::CheckFlag(unsigned int flag) {
    return plugin::CallMethodAndReturn<bool, 0x10410E0>(this, flag);
}

std::wstring CCompID::ToStr() const {
    std::wstring typeName = plugin::Format(L"UNKNOWN (%d)", type);
    if (type == 0) typeName = L"ROOT";
    else if (type == 1) typeName = L"LEAGUE";
    else if (type == 2) typeName = L"UNKNOWN_2";
    else if (type == 3) typeName = L"FA_CUP";
    else if (type == 4) typeName = L"LE_CUP";
    else if (type == 5) typeName = L"CHALLENGE_SHIELD";
    else if (type == 6) typeName = L"CONFERENCE_CUP";
    else if (type == 7) typeName = L"SUPERCUP";
    else if (type == 8) typeName = L"RELEGATION";
    else if (type == 9) typeName = L"CHAMPIONSLEAGUE";
    else if (type == 10) typeName = L"UEFA_CUP";
    else if (type == 11) typeName = L"TOYOTA";
    else if (type == 12) typeName = L"EURO_SUPERCUP";
    else if (type == 13) typeName = L"WORLD_CLUB_CHAMP";
    else if (type == 14) typeName = L"UIC";
    else if (type == 15) typeName = L"QUALI_WC";
    else if (type == 16) typeName = L"QUALI_EC";
    else if (type == 17) typeName = L"WORLD_CUP";
    else if (type == 18) typeName = L"EURO_CUP";
    else if (type == 19) typeName = L"REL1";
    else if (type == 20) typeName = L"REL2";
    else if (type == 21) typeName = L"REL3";
    else if (type == 22) typeName = L"INDOOR";
    else if (type == 23) typeName = L"FRIENDLY";
    else if (type == 24) typeName = L"POOL";
    else if (type == 25) typeName = L"UNKNOWN_25";
    else if (type == 26) typeName = L"UNKNOWN_26";
    else if (type == 27) typeName = L"UNKNOWN_27";
    else if (type == 28) typeName = L"RESERVE";
    else if (type == 29) typeName = L"UNKNOWN_29";
    else if (type == 30) typeName = L"UNKNOWN_30";
    else if (type == 31) typeName = L"U20_WORLD_CUP";
    else if (type == 32) typeName = L"CONFED_CUP";
    else if (type == 33) typeName = L"COPA_AMERICA";
    return plugin::Format(L"{ %d, %s, %d }", countryId, typeName.c_str(), index);
}

CCompID CCompID::Make(unsigned char _country, unsigned char _type, unsigned short _index) {
    CCompID result;
    result.countryId = _country;
    result.type = _type;
    result.index = _index;
    return result;
}

CCompID CCompID::Make(unsigned int value) {
    CCompID result;
    result.SetFromInt(value);
    return result;
}

CCompID CCompID::BaseCompID() const {
    CCompID compId = CCompID::Make(ToInt() & 0xFFFF0000);
    return compId;
}

unsigned int CDBCompetition::GetCompetitionType() {
    return plugin::CallMethodAndReturn<unsigned int, 0xF81C50>(this);
}

UChar CDBCompetition::GetRegion() {
    return CallMethodAndReturn<UChar, 0xF81C40>(this);
}

unsigned int CDBCompetition::GetDbType() {
    return plugin::CallMethodAndReturn<unsigned int, 0xF81960>(this);
}

wchar_t const *CDBCompetition::GetName() {
    return plugin::CallMethodAndReturn<wchar_t const *, 0xF81930>(this);
}

CCompID CDBCompetition::GetCompID() {
    CCompID result;
    plugin::CallMethod<0xF81C20>(this, &result);
    return result;
}

unsigned int CDBCompetition::GetNumOfRegisteredTeams() {
    return plugin::CallMethodAndReturn<unsigned int, 0xF82510>(this);
}

void CDBCompetition::SetNumOfRegisteredTeams(unsigned int number) {
    *raw_ptr<unsigned int>(this, 0xA4) = number;
}

unsigned int CDBCompetition::GetNumOfTeams() {
    return plugin::CallMethodAndReturn<unsigned int, 0xF82520>(this);
}

unsigned char CDBCompetition::GetFlags() {
    return *plugin::raw_ptr<unsigned char>(this, 0x288);
}

void CDBCompetition::DumpToFile() {
    plugin::CallMethod<0xF85430>(this);
}

int CDBCompetition::GetNumOfScriptCommands() {
    return plugin::CallMethodAndReturn<int, 0xF82620>(this);
}

CScriptCommand *CDBCompetition::GetScriptCommand(int index) {
    return plugin::CallMethodAndReturn<CScriptCommand *, 0xF86090>(this, index);
}

CTeamIndex &CDBCompetition::GetTeamID(int position) {
    return plugin::CallMethodAndReturn<CTeamIndex &, 0xF82490>(this, position);
}

CTeamIndex &CDBCompetition::GetChampion(bool checkIntl) {
    return *plugin::CallMethodAndReturn<CTeamIndex *, 0xF85B30>(this, checkIntl);
}

CTeamIndex &CDBCompetition::GetRunnerUp() {
    return *plugin::CallMethodAndReturn<CTeamIndex *, 0xF82070>(this);
}

void CDBCompetition::SetChampion(CTeamIndex const &teamId) {
    plugin::CallMethod<0xF82050>(this, &teamId);
}

void CDBCompetition::SetRunnerUp(CTeamIndex const &teamId) {
    plugin::CallMethod<0xF82060>(this, &teamId);
}

unsigned char CDBCompetition::GetNumTeamsFromCountry(int countryId) {
    return plugin::CallVirtualMethodAndReturn<unsigned char, 14>(this, countryId);
}

void CDBCompetition::AddTeamsFromCountry(unsigned char countryId, unsigned int numTeams) {
    plugin::Call<0x139EB60>(this, countryId, numTeams);
}

bool CDBCompetition::AddTeam(CTeamIndex const &teamIndex, int position) {
    return plugin::CallVirtualMethodAndReturn<bool, 26>(this, &teamIndex, position);
}

bool CDBCompetition::IsTeamPresent(CTeamIndex const &teamIndex) {
    return plugin::CallMethodAndReturn<bool, 0xF82400>(this, &teamIndex);
}

unsigned int CDBCompetition::GetNumMatchdays() {
    return plugin::CallVirtualMethodAndReturn<unsigned int, 8>(this);
}

bool CDBCompetition::SwapTeams(int oldIndex, int newIndex) {
    return plugin::CallMethodAndReturn<bool, 0xF85E00>(this, oldIndex, newIndex);
}

void CDBCompetition::SortTeamIDs(unsigned int startPos, unsigned int numTeams, Function<Bool(CTeamIndex const &, CTeamIndex const &)> const &sorter) {
    if (GetNumOfRegisteredTeams() > 1 && numTeams > 1 && (startPos + numTeams) <= GetNumOfTeams()) {
        CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(this, 0xA0);
        std::sort(pTeamIDs + startPos, pTeamIDs + startPos + numTeams, sorter);
    }
}

void CDBCompetition::SortTeams(unsigned int startPos, unsigned int numTeams, Function<Bool(CDBTeam *, CDBTeam *)> const &sorter) {
    if (GetNumOfRegisteredTeams() > 1 && numTeams > 1 && (startPos + numTeams) <= GetNumOfTeams()) {
        CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(this, 0xA0);
        std::sort(pTeamIDs + startPos, pTeamIDs + startPos + numTeams, [&](CTeamIndex const &teamIndexA, CTeamIndex const &teamIndexB) {
            CDBTeam *teamA = GetTeam(teamIndexA.firstTeam());
            CDBTeam *teamB = GetTeam(teamIndexB.firstTeam());
            return sorter(teamA, teamB);
        });
    }
}

void CDBCompetition::SortTeamIDs(Function<Bool(CTeamIndex const &, CTeamIndex const &)> const &sorter) {
    CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(this, 0xA0);
    auto numTeams = GetNumOfRegisteredTeams();
    if (numTeams > 1)
        std::sort(pTeamIDs, pTeamIDs + numTeams, sorter);
}

void CDBCompetition::SortTeams(Function<Bool(CDBTeam *, CDBTeam *)> const &sorter) {
    CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(this, 0xA0);
    auto numTeams = GetNumOfRegisteredTeams();
    if (numTeams > 1) {
        std::sort(pTeamIDs, pTeamIDs + numTeams, [&](CTeamIndex const &teamIndexA, CTeamIndex const &teamIndexB) {
            CDBTeam *teamA = GetTeam(teamIndexA.firstTeam());
            CDBTeam *teamB = GetTeam(teamIndexB.firstTeam());
            return sorter(teamA, teamB);
        });
    }
}

void CDBCompetition::RandomlySortTeams(UInt startPos, UInt numTeams) {
    auto numRegisteredTeams = GetNumOfRegisteredTeams();
    if (numRegisteredTeams > 1 && startPos < numRegisteredTeams) {
        if (numTeams == 0 || (startPos + numTeams) > numRegisteredTeams)
            numTeams = numRegisteredTeams - startPos;
        if (numTeams > 1) {
            for (UInt i = numTeams - 1; i > 0; --i) {
                UInt j = GetRandomInt(i + 1);
                SwapTeams(startPos + i, startPos + j);
            }
        }
    }
}

void CDBCompetition::SortTeamsForKORoundAfterGroupStage() {
    if (GetNumOfTeams() <= 2 || GetNumOfRegisteredTeams() != GetNumOfTeams() || (GetNumOfTeams() % 2) == 1)
        return;
    struct TeamWithGroup {
        CTeamIndex teamID = CTeamIndex::null();
        UChar group = 0;
    };
    auto teamIDs = GetTeams();
    UInt numTeamsToSort = teamIDs.size() / 2;
    Vector<TeamWithGroup> teams(teamIDs.size());
    for (UInt i = 0; i < teams.size(); i++) {
        teams[i].teamID = teamIDs[i];
        teams[i].group = (i % numTeamsToSort) + 1;
    }
    UInt numIterations = 0;
    Utils::Shuffle(teams, 0, numTeamsToSort);
    Utils::Shuffle(teams, numTeamsToSort, numTeamsToSort);
    for (; numIterations < 10'000; numIterations++) {
        Bool valid = true;
        for (UInt i = 0; i < numTeamsToSort; i++) {
            if (teams[i].group == teams[numTeamsToSort + i].group) {
                valid = false;
                break;
            }
        }
        if (valid)
            break;
        Utils::Shuffle(teams, 0, numTeamsToSort);
    }
#ifndef SHARED_DO_NOT_LOG
    SafeLog::Write(Utils::Format(L"SortTeamsForKORoundAfterGroupStage numIterations: %d", numIterations));
#endif
    for (UInt i = 0; i < teamIDs.size(); i++) {
        teamIDs[i] = teams[i].teamID;
        if (i < numTeamsToSort) {
#ifndef SHARED_DO_NOT_LOG
            SafeLog::Write(Utils::Format(L"%s (%c) - %s (%c)",
                TeamName(teamIDs[i]), teams[i].group + 'A' - 1,
                TeamName(teamIDs[numTeamsToSort + i]), teams[numTeamsToSort + i].group + 'A' - 1));
#endif
        }
    }
    SetTeams(teamIDs);
}

void CDBCompetition::RandomizeHomeAway() {
    if (GetNumOfRegisteredTeams() > 1) {
        UInt half = GetNumOfTeams() / 2;
        UInt pairsToSort = 0;
        if (GetNumOfTeams() == GetNumOfRegisteredTeams())
            pairsToSort = half;
        else if (GetNumOfRegisteredTeams() >= half)
            pairsToSort = half - (GetNumOfTeams() - GetNumOfRegisteredTeams());
        if (pairsToSort > 0) {
            for (UInt i = 0; i < pairsToSort; i++) {
                if (rand() % 2)
                    SwapTeams(i, i + half);
            }
        }
    }
}

void CDBCompetition::RandomizePairs() {
    if (GetNumOfTeams() == GetNumOfRegisteredTeams()) {
        RandomlySortTeams(0, GetNumOfRegisteredTeams() / 2);
        RandomlySortTeams(GetNumOfRegisteredTeams() / 2);
    }
    else if (GetNumOfRegisteredTeams() >= GetNumOfTeams() / 2)
        RandomlySortTeams(0, GetNumOfTeams() / 2);
    else
        RandomlySortTeams(0, GetNumOfRegisteredTeams());
}

void CDBCompetition::RandomizePairs4x4() {
    UInt blockSize = GetNumOfTeams() / 4;
    if (GetNumOfTeams() == GetNumOfRegisteredTeams()) {
        RandomlySortTeams(0, blockSize);
        RandomlySortTeams(blockSize, blockSize);
        RandomlySortTeams(blockSize * 2, blockSize);
        RandomlySortTeams(blockSize * 3, blockSize);
    }
    else if (GetNumOfRegisteredTeams() >= GetNumOfTeams() / 2) {
        RandomlySortTeams(0, blockSize);
        RandomlySortTeams(blockSize, blockSize);
    }
    else if (GetNumOfRegisteredTeams() >= GetNumOfTeams() / 4) {
        RandomlySortTeams(0, blockSize);
    }
    else
        RandomlySortTeams(0, GetNumOfRegisteredTeams());
}

CCompID CDBCompetition::GetPredecessor(Int index) {
    CCompID result;
    plugin::CallMethod<0xF81F80>(this, &result, index);
    return result;
}

CCompID CDBCompetition::GetSuccessor(Int index) {
    CCompID result;
    plugin::CallMethod<0xF81FE0>(this, &result, index);
    return result;
}

unsigned int CDBCompetition::GetLevel() {
    return plugin::CallVirtualMethodAndReturn<unsigned int, 18>(this);
}

unsigned int CDBCompetition::GetRoundType() {
    return CallMethodAndReturn<unsigned int, 0xF81CF0>(this);
}

int CDBCompetition::GetTeamIndex(CTeamIndex const& teamId) {
    return CallMethodAndReturn<int, 0xF85D40>(this, &teamId);
}

bool CDBCompetition::Finish() {
    return plugin::CallVirtualMethodAndReturn<bool, 6>(this);
}

void CDBCompetition::Launch() {
    plugin::CallVirtualMethod<4>(this);
}

Bool CDBCompetition::IsContinental() {
    return CallMethodAndReturn<Bool, 0x11F0370>(this);
}

Bool CDBCompetition::IsLaunched() {
    return CallMethodAndReturn<Bool, 0xF81B30>(this);
}

Bool CDBCompetition::IsFinished() {
    return CallVirtualMethodAndReturn<Bool, 1>(this);
}

Bool CDBCompetition::CanLaunchAllSuccessors() {
	return CallVirtualMethodAndReturn<Bool, 2>(this);
}

EAGMoney CDBCompetition::GetBonus(UInt bonusId) {
    EAGMoney result;
    CallVirtualMethod<27>(this, &result, bonusId);
    return result;
}

CDBRound *CDBCompetition::AsRound() {
    return CallMethodAndReturn<CDBRound *, 0xF84780>(this);
}

CDBRoot *CDBCompetition::GetRoot() {
    return CallMethodAndReturn<CDBRoot *, 0xF8B680>(this);
}

CTeamIndex *CDBCompetition::Teams() {
    return *raw_ptr<CTeamIndex *>(this, 0xA0);
}

Vector<CTeamIndex> CDBCompetition::GetTeams() {
    Vector<CTeamIndex> result(GetNumOfTeams());
    for (UInt i = 0; i < result.size(); i++)
        result[i] = Teams()[i];
    return result;
}

Vector<CTeamIndex> CDBCompetition::GetTeams(UInt startPos, UInt count) {
    Vector<CTeamIndex> result(count);
    UInt counter = 0;
    for (UInt i = startPos; i < (startPos + count); i++)
        result[counter++] = Teams()[i];
    return result;
}

Vector<CTeamIndex> CDBCompetition::GetRegisteredTeams() {
    Vector<CTeamIndex> result(GetNumOfRegisteredTeams());
    for (UInt i = 0; i < result.size(); i++)
        result[i] = Teams()[i];
    return result;
}

void CDBCompetition::SetTeams(Vector<CTeamIndex> const &teams, UInt numRegisteredTeams) {
    UInt numTeamsToCopy = Utils::Min(GetNumOfTeams(), teams.size());
    for (UInt i = 0; i < numTeamsToCopy; i++)
        Teams()[i] = teams[i];
    if (numTeamsToCopy < GetNumOfTeams()) {
        for (UInt i = numTeamsToCopy; i < GetNumOfTeams(); i++)
            Teams()[i].clear();
    }
    SetNumOfRegisteredTeams(numRegisteredTeams);
}

void CDBCompetition::SetTeams(Vector<CTeamIndex> const &teams) {
    SetTeams(teams, GetNumOfTeams());
}

void CDBCompetition::SetRegisteredTeams(Vector<CTeamIndex> const &teams) {
    SetTeams(teams, teams.size());
}

CDBCompetition *CDBCompetition::PrevContinental() {
    return CallMethodAndReturn<CDBCompetition *, 0xF8B6B0>(this);
}

CDBCompetition *CDBCompetition::NextContinental() {
    return CallMethodAndReturn<CDBCompetition *, 0xF8B6C0>(this);
}

void CDBLeague::SetStartDate(CJDate date) {
    plugin::CallMethod<0x1054390>(this, date);
}

int CDBLeague::GetEqualPointsSorting() {
    return plugin::CallMethodAndReturn<int, 0x10502F0>(this);
}

void CDBLeague::SortTeams(TeamLeaguePositionData *infos, int sortingFlags, int goalsMinMinute, int goalsMaxMinute, int minMatchday, int maxMatchday) {
    plugin::CallVirtualMethod<40>(this, infos, sortingFlags, goalsMinMinute, goalsMaxMinute, minMatchday, maxMatchday);
}

unsigned int CDBLeague::GetCurrentMatchday() {
    return plugin::CallMethodAndReturn<int, 0x1050310>(this);
}

CMatches *CDBLeague::GetMatches() {
    return raw_ptr<CMatches>(this, 0x22C8);
}

CRoleFactory *GetRoleFactory() {
    return CallAndReturn<CRoleFactory *, 0x124B4F0>();
}

CAssessmentTable *GetAssesmentTable() {
    return plugin::CallAndReturn<CAssessmentTable *, 0x121D940>();
}

CDBCompetition *GetCompetition(unsigned char region, unsigned char type, unsigned short index) {
    return plugin::CallAndReturn<CDBCompetition *, 0xF8AFC0>(region, type, index);
}

CDBCompetition *GetCompetition(CCompID const &id) {
    return plugin::CallAndReturn<CDBCompetition *, 0xF8AF50>(&id);
}

CDBCompetition *GetCompetition(unsigned int *id) {
    return plugin::CallAndReturn<CDBCompetition *, 0xF8AF50>(id);
}

CDBCompetition *GetCompetition(unsigned int id) {
    return plugin::CallAndReturn<CDBCompetition *, 0xF8AF50>(&id);
}

CDBLeague *GetLeague(unsigned char region, unsigned char type, unsigned short index) {
    return plugin::CallAndReturn<CDBLeague *, 0xF8C620>(region, type, index);
}

CDBLeague *GetLeague(CCompID const &id) {
    return plugin::CallAndReturn<CDBLeague *, 0xF8C600>(&id);
}

CDBLeague *GetLeague(unsigned int *id) {
    return plugin::CallAndReturn<CDBLeague *, 0xF8C600>(id);
}

CDBLeague *GetLeague(unsigned int id) {
    return plugin::CallAndReturn<CDBLeague *, 0xF8C600>(&id);
}

CDBRound *GetRound(unsigned char region, unsigned char type, unsigned short index) {
    return plugin::CallAndReturn<CDBRound *, 0xF8B150>(region, type, index);
}

CDBRound *GetRound(CCompID const &id) {
    return plugin::CallAndReturn<CDBRound *, 0xF8B130>(&id);
}

CDBRound *GetRound(unsigned int *id) {
    return plugin::CallAndReturn<CDBRound *, 0xF8B130>(id);
}

CDBRound *GetRound(unsigned int id) {
    return plugin::CallAndReturn<CDBRound *, 0xF8B130>(&id);
}

CDBRound *GetRoundByRoundType(unsigned char region, unsigned char type, unsigned char roundType) {
    return plugin::CallAndReturn<CDBRound *, 0xF8B190>(region, type, roundType);
}

CDBPool *GetPool(unsigned char region, unsigned char type, unsigned short index) {
    return plugin::CallAndReturn<CDBPool *, 0xF8C5C0>(region, type, index);
}

FmMap<UInt, CDBCompetition *> &GetCompetitions() {
    return *(FmMap<UInt, CDBCompetition *> *)0x3127D8C;
}

CDBCountry *GetCountry(UChar countryId) {
    return &GetCountryStore()->m_aCountries[countryId];
}

CDBTeam *GetTeam(CTeamIndex teamId) {
    return plugin::CallAndReturn<CDBTeam *, 0xEC8F70>(teamId);
}

CDBTeam *GetTeamByUniqueID(unsigned int uniqueID) {
    return plugin::CallAndReturn<CDBTeam *, 0xF0CCA0>(uniqueID);
}

CDBPlayer *GetPlayer(Int playerId) {
    return CallAndReturn<CDBPlayer *, 0xF97C70>(playerId);
}

CDBEmployee *GetEmployee(Int employeeId) {
    return CallAndReturn<CDBEmployee *, 0xEA2A00>(employeeId);
}

CDBStaff *GetStaff(Int staffId) {
    return CallAndReturn<CDBStaff *, 0x11027F0>(staffId);
}

WideChar const *GetCityName(Int cityId) {
    return CallAndReturn<WideChar const *, 0x11C14A0>(cityId);
}

CCountryStore *GetCountryStore() {
    return plugin::CallAndReturn<CCountryStore *, 0x415100>();
}

wchar_t const *GetTranslation(const char *key) {
    return plugin::CallMethodAndReturn<wchar_t const *, 0x14A9B78>(0x31E3FA8, key);
}

bool IsTranslationPresent(const char *key) {
    return plugin::CallMethodAndReturn<bool, 0x14A9BA4>(0x31E3FA8, key);
}

wchar_t const *GetTranslationIfPresent(const char *key) {
    if (IsTranslationPresent(key))
        return GetTranslation(key);
    return nullptr;
}

CJDate GetCurrentDate() {
	auto game = CDBGame::GetInstance();
	if (game)
		return game->GetCurrentDate();
	static CJDate DUMMY_DATE;
	DUMMY_DATE.Set(0);
	return DUMMY_DATE;
}

unsigned short GetCurrentYear() {
    auto game = CDBGame::GetInstance();
    if (game)
        return game->GetCurrentYear();
    return 0;
}

unsigned char GetCurrentMonth() {
    auto game = CDBGame::GetInstance();
    if (game)
        return game->GetCurrentDate().GetMonth();
    return 0;
}

unsigned short GetStartingYear() {
    auto game = CDBGame::GetInstance();
    if (game)
        return game->GetStartDate().GetYear();
    return 0;
}

unsigned short GetCurrentSeasonStartYear() {
    auto game = CDBGame::GetInstance();
    if (game)
        return game->GetCurrentSeasonStartDate().GetYear();
    return 0;
}

unsigned short GetCurrentSeasonEndYear() {
    auto game = CDBGame::GetInstance();
    if (game)
        return game->GetCurrentSeasonEndDate().GetYear();
    return 0;
}

wchar_t const *GetCountryLogoPath(wchar_t *out, unsigned char countryId, int size) {
    return plugin::CallAndReturn<wchar_t const *, 0xD3D800>(out, countryId, size);
}

int gfxGetVarInt(char const *varName, int defaultValue) {
    void *expVars = plugin::CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x239120));
    return plugin::CallMethodAndReturnDynGlobal<int>(GfxCoreAddress(0x239370), expVars, varName, defaultValue);
}

char const *gfxGetVarString(char const *varName) {
    void *expVars = plugin::CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x239120));
    return plugin::CallMethodAndReturnDynGlobal<char const *>(GfxCoreAddress(0x2392A0), expVars, varName);
}

void SetVarInt(char const *varName, int value) {
    void *varFM = *(void **)0x30ABBC4;
    if (varFM)
        plugin::CallVirtualMethod<1>(varFM, varName, value);
}

void SetVarString(char const *varName, char const *value) {
    void *varFM = *(void **)0x30ABBC4;
    if (varFM)
        plugin::CallVirtualMethod<0>(varFM, varName, value);
}

int GetRandomInt(int endValue) {
    return plugin::CallAndReturn<int, 0x149E320>(endValue);
}

void SetCompetitionWinnerAndRunnerUp(unsigned int region, unsigned int type, CTeamIndex const &winner, CTeamIndex const &runnerUp) {
    auto r = GetRoundByRoundType(region, type, 15);
    if (r) {
        if (winner.countryId && !r->GetChampion().countryId)
            r->SetChampion(winner);
        if (runnerUp.countryId && !r->GetRunnerUp().countryId)
            r->SetRunnerUp(runnerUp);
        if (r->GetDbType() == DB_ROUND)
            CallMethod<0x1043900>(r);
    }
}

Bool FmFileImageExists(String const &filepathWithoutExtension, String &resultPath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3859EA));
    if (fmFs) {
        String fpstr = filepathWithoutExtension + L".tga";
        if (CallVirtualMethodAndReturn<Bool, 9>(fmFs, fpstr.c_str())) {
            resultPath = fpstr;
            return true;
        }
    }
    resultPath.clear();
    return false;
}

Bool FmFileExists(Path const &filepath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3859EA));
    if (fmFs)
        return CallVirtualMethodAndReturn<Bool, 9>(fmFs, filepath.c_str());
    return false;
}

UInt FmFileGetSize(Path const &filepath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3859EA));
    if (fmFs)
        return CallVirtualMethodAndReturn<UInt, 0>(fmFs, filepath.c_str());
    return 0;
}

Bool FmFileRead(Path const &filepath, void *outData, UInt size) {
    void *fmFs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3859EA));
    if (fmFs) {
        UInt outBuf[2] = { 0, 0 };
        CallVirtualMethod<2>(fmFs, outBuf, filepath.c_str(), outData, size);
        return outBuf[0] == (UInt)outData && outBuf[1] == size;
    }
    return false;
}

void *CreateTextBox(void *screen, char const *name) {
    return CallMethodAndReturn<void *, 0xD44240>(screen, name);
}

void *CreateTextButton(void *screen, char const *name) {
    return CallMethodAndReturn<void *, 0xD44360>(screen, name);
}

void *CreateImage(void *screen, char const *name) {
    return CallMethodAndReturn<void *, 0xD44380>(screen, name);
}

void *GetTransform(void *screen, char const *name) {
    return CallMethodAndReturn<void *, 0xD43060>(screen, name);
}

void SetTextBoxColorRGBA(void *tb, UChar r, UChar g, UChar b, UChar a) {
    UInt clr = (a << 24) | (r << 16) | (g << 8) | b;
    for (UInt i = 0; i < 4; i++)
        CallMethod<0x144888F>(tb, i, clr);
}

void SetTextBoxColorRGB(void *tb, UChar r, UChar g, UChar b) {
    UInt clr = (r << 16) | (g << 8) | b;
    for (UInt i = 0; i < 4; i++) {
        UInt oldClr = CallMethodAndReturn<UInt, 0x14488CB>(tb, i);
        CallMethod<0x144888F>(tb, i, (oldClr & 0xFF000000) | clr);
    }
}

void SetTextBoxColorRGBA(void *tb, UInt clr) {
    for (UInt i = 0; i < 4; i++)
        CallMethod<0x144888F>(tb, i, clr);
}

void SetTextBoxColorRGB(void *tb, UInt clr) {
    for (UInt i = 0; i < 4; i++) {
        UInt oldClr = CallMethodAndReturn<UInt, 0x14488CB>(tb, i);
        CallMethod<0x144888F>(tb, i, (oldClr & 0xFF000000) | clr);
    }
}

void SetImageColorRGBA(void *img, UChar r, UChar g, UChar b, UChar a) {
    void *baseImage = *raw_ptr<void *>(img, 0x60);
    if (baseImage) {
        UInt clr = (a << 24) | (r << 16) | (g << 8) | b;
        CallVirtualMethod<7>(baseImage, clr);
    }
}

void SetImageColorRGB(void *img, UChar r, UChar g, UChar b) {
    void *baseImage = *raw_ptr<void *>(img, 0x60);
    if (baseImage) {
        UInt clr = (r << 16) | (g << 8) | b;
        UInt oldClr = CallVirtualMethodAndReturn<UInt, 8>(baseImage);
        CallVirtualMethod<7>(baseImage, (oldClr & 0xFF000000) | clr);
    }
}

void SetImageColorRGBA(void *img, UInt clr) {
    void *baseImage = *raw_ptr<void *>(img, 0x60);
    if (baseImage)
        CallVirtualMethod<7>(baseImage, clr);
}

void SetImageColorRGB(void *img, UInt clr) {
    void *baseImage = *raw_ptr<void *>(img, 0x60);
    if (baseImage) {
        UInt oldClr = CallVirtualMethodAndReturn<UInt, 8>(baseImage);
        CallVirtualMethod<7>(baseImage, (oldClr & 0xFF000000) | clr);
    }
}

void SetTextBoxColors(void *tb, UInt clr) {
    Call<0xD32810>(tb, clr);
}

void SetEnabled(void* widget, Bool enabled) {
    if (widget)
        CallVirtualMethod<9>(widget, enabled);
}

void SetVisible(void *widget, Bool visible) {
    if (widget)
        CallVirtualMethod<11>(widget, visible);
}

UInt GetId(void *widget) {
    if (widget)
        return CallVirtualMethodAndReturn<UInt, 23>(widget);
    return 0;
}

void SetText(void* widget, WideChar const* text) {
    if (widget)
        CallVirtualMethod<78>(widget, text);
}

unsigned char SetImageFilename(void* widget, std::wstring const& path) {
    if (widget)
        return CallAndReturn<unsigned char, 0xD32860>(widget, path.c_str(), 0, 0);
    return 0;
}

unsigned char SetImageFilename(void *widget, wchar_t const *filename, int u1, int u2) {
    return CallAndReturn<unsigned char, 0xD32860>(widget, filename, u1, u2);
}

void SetTransformColor(void *trfm, Float *color) {
    if (trfm)
        CallVirtualMethod<37>(trfm, color);
}

void SetTransformEnabled(void *trfm, Bool enabled) {
    if (trfm)
        CallVirtualMethod<22>(trfm, enabled);
}

void SetTransformVisible(void *trfm, Bool visible) {
    if (trfm)
        CallVirtualMethod<24>(trfm, 0x100, visible);
}

String TeamName(CDBTeam *team) {
    if (team)
        return team->GetName();
    return L"n/a";
}

String TeamNameWithCountry(CDBTeam *team) {
    if (team)
        return Format(L"%s (%s)", team->GetName(), GetCountryStore()->m_aCountries[team->GetCountryId()].GetName());
    return L"n/a";
}

String TeamTag(CDBTeam *team) {
    if (team)
        return Utils::Format(L"%08X %s", team->GetTeamID().ToInt(), team->GetName());
    return L"n/a";
}

String TeamTagWithCountry(CDBTeam *team) {
    if (team)
        return Utils::Format(L"%08X %s (%s)", team->GetTeamID().ToInt(), team->GetName(), GetCountryStore()->m_aCountries[team->GetCountryId()].GetName());
    return L"n/a";
}

String TeamName(CTeamIndex const &teamId) {
    auto team = GetTeam(teamId);
    if (team)
        return team->GetName();
    return L"n/a";
}

String TeamNameWithCountry(CTeamIndex const &teamId) {
    auto team = GetTeam(teamId);
    if (team)
        return Format(L"%s (%s)", team->GetName(), GetCountryStore()->m_aCountries[teamId.countryId].GetName());
    return L"n/a";
}

String TeamTag(CTeamIndex const& teamId) {
    auto team = GetTeam(teamId);
    return Utils::Format(L"%08X %s", teamId.ToInt(), team ? team->GetName() : L"n/a");
}

String TeamTagWithCountry(CTeamIndex const& teamId) {
    auto team = GetTeam(teamId);
    return Utils::Format(L"%08X %s (%s)", teamId.ToInt(), team ? team->GetName() : L"n/a", CountryName(teamId.countryId));
}

String ThousandSeparators(unsigned int value) {
    String str = std::to_wstring(value);
    int len = str.length();
    int dlen = 3;
    while (len > dlen) {
        str.insert(len - dlen, 1, '\'');
        dlen += 4;
        len += 1;
    }
    return str;
}

String StadiumNameWithCapacity(CStadiumDevelopment *stadium) {
    return Utils::Format(L"%s (%s)", stadium->GetStadiumName(), ThousandSeparators(stadium->GetNumSeats()));
}

String StadiumName(CDBTeam *team) {
    if (team) {
        return Utils::Format(L"%s (%s)", team->GetStadiumDevelopment()->GetStadiumName(),
            ThousandSeparators(team->GetStadiumDevelopment()->GetNumSeats()));
    }
    return L"n/a";
}

String StadiumNameWithCountry(CDBTeam *team) {
    if (team) {
        return Utils::Format(L"%s (%s, %s)", team->GetStadiumDevelopment()->GetStadiumName(),
            ThousandSeparators(team->GetStadiumDevelopment()->GetNumSeats()),
            CountryName(team->GetCountryId()));
    }
    return L"n/a";
}

String StadiumTag(CDBTeam *team) {
    if (team)
        return Utils::Format(L"%08X %s", team->GetTeamID().ToInt(), StadiumName(team));
    return L"n/a";
}

String StadiumTagWithCountry(CDBTeam *team) {
    if (team)
        return Utils::Format(L"%08X %s", team->GetTeamID().ToInt(), StadiumNameWithCountry(team));
    return L"n/a";
}

String StadiumName(CTeamIndex const &teamId) {
    return StadiumName(GetTeam(teamId));
}

String StadiumNameWithCountry(CTeamIndex const &teamId) {
    return StadiumNameWithCountry(GetTeam(teamId));
}

String StadiumTag(CTeamIndex const &teamId) {
    return StadiumTag(GetTeam(teamId));
}

String StadiumTagWithCountry(CTeamIndex const &teamId) {
    return StadiumTagWithCountry(GetTeam(teamId));
}

String CompetitionTag(CDBCompetition* comp) {
    if (comp)
        return Utils::Format(L"%s %s", comp->GetCompID().ToStr(), comp->GetName());
    return L"n/a";
}

String CompetitionTag(CCompID const& compId) {
    CDBCompetition *comp = GetCompetition(compId);
    return Utils::Format(L"%s %s", compId.ToStr(), comp ? comp->GetName() : L"n/a");
}

String CompetitionName(CDBCompetition *comp) {
    if (comp)
        return Utils::Format(L"%s", comp->GetName());
    return L"n/a";
}

String CompetitionName(CCompID const &compId) {
    return CompetitionName(GetCompetition(compId));
}

String CountryName(UChar countryId) {
    if (countryId >= 1 && countryId <= 207)
        return GetCountryStore()->m_aCountries[countryId].GetName();
    return L"n/a";
}

String CountryTag(UChar countryId) {
    Bool isValid = countryId >= 1 && countryId <= 207;
    return Utils::Format(L"%d %s", countryId, CountryName(countryId));
}

String FlagsToStr(UInt value) {
    Vector<String> vecFlags;
    FifamBeg beg = FifamBeg::MakeFromInt(value);
    beg.ToStr();
    for (UInt i = 0; i < 32; i++) {
        UInt f = 1 << i;
        if (value & f) {
            if (FifamBeg::Present(f))
                vecFlags.push_back(FifamBeg::MakeFromInt(f).ToStr());
            else
                vecFlags.push_back(Utils::Format(L"UNKNOWN_", f));
        }
    }
    return Utils::Join(vecFlags, L',');
}

String CompetitionType(CDBCompetition *comp) {
    static const wchar_t *compTypeNames[] = {
        L"DB_ROOT", L"DB_LEAGUE", L"DB_SPARE", L"DB_CUP", L"DB_ROUND", L"DB_POOL", L"DB_FRIENDLY", L"DB_CUSTOM"
    };
    if (comp) {
        UChar type = comp->GetDbType();
        if (type < std::size(compTypeNames))
            return compTypeNames[type];
        return Utils::Format(L"UNKNOWN_%u", type);
    }
    return String();
}

Bool GetHour() {
    void *match = *(void **)0x3124748;
    if (match)
        return CallMethodAndReturn<UChar, 0xE811C0>(match);
    return 0;
}

Bool GetIsCloudy() {
    void *match = *(void **)0x3124748;
    if (match) {
        UChar weather = CallMethodAndReturn<UChar, 0xE81160>(match);
        if (weather == 2 || weather == 3)
            return true;
    }
    return false;
}

Bool GetIsRainy() {
    void *match = *(void **)0x3124748;
    if (match) {
        UChar weather = CallMethodAndReturn<UChar, 0xE81160>(match);
        if (weather == 0)
            return true;
    }
    return false;
}

Bool GetIsSnowy() {
    void *match = *(void **)0x3124748;
    if (match) {
        UChar weather = CallMethodAndReturn<UChar, 0xE81160>(match);
        if (weather == 1)
            return true;
    }
    return false;
}

UInt GetLighting() {
    return gfxGetVarInt("LIGHTING", 1);
}

unsigned char CAssessmentTable::GetCountryIdAtPosition(int position) {
    return plugin::CallMethodAndReturn<unsigned char, 0x121D030>(this, position);
}

unsigned char CAssessmentTable::GetCountryIdAtPositionLastYear(int position) {
    return plugin::CallMethodAndReturn<unsigned char, 0x121CFF0>(this, position);
}

int CAssessmentTable::GetCountryPosition(int countryId) {
    return plugin::CallMethodAndReturn<int, 0x121CEA0>(this, countryId);
}

CAssessmentInfo *CAssessmentTable::GetInfoForCountry(unsigned char countryId) {
    return plugin::CallMethodAndReturn<CAssessmentInfo *, 0x121D040>(this, countryId);
}

int CAssessmentTable::GetCountryPositionLastYear(unsigned char countryId) {
    return GetInfoForCountry(countryId)->m_nPositionIndex;
}

float CAssessmentTable::GetTotalPointsForCountry(unsigned char countryId) {
    return plugin::CallMethodAndReturn<float, 0x121D500>(this, countryId);
}

void CCompID::SetFromInt(unsigned int value) {
    countryId = (value >> 24) & 0xFF;
    type = (value >> 16) & 0xFF;
    index = value & 0xFFFF;
}

unsigned int CCompID::ToInt() const {
    return index | (type << 16) | (countryId << 24);
}

CTeamIndex CDBTeam::GetTeamID() {
    return plugin::CallMethodAndReturn<CTeamIndex, 0xEC9440>(this);
}

unsigned int CDBTeam::GetTeamUniqueID() {
    return CallMethodAndReturn<unsigned int, 0xEC9490>(this);
}

unsigned char CDBTeam::GetNationalPrestige() {
    return plugin::CallMethodAndReturn<unsigned char, 0xED1480>(this);
}

void CDBTeam::SetNationalPrestige(UChar prestige) {
    plugin::CallMethod<0xED1490>(this, prestige);
}

unsigned char CDBTeam::GetInternationalPrestige() {
    return plugin::CallMethodAndReturn<unsigned char, 0xED14B0>(this);
}

void CDBTeam::SetInternationalPrestige(UChar prestige) {
    plugin::CallMethod<0xED14C0>(this, prestige);
}

wchar_t *CDBTeam::GetName(bool first) {
    return plugin::CallMethodAndReturn<wchar_t *, 0xEEB670>(this, first);
}

wchar_t *CDBTeam::GetShortName(CTeamIndex const &teamID, bool includeYouth) {
    return plugin::CallMethodAndReturn<wchar_t *, 0xEDD3E0>(this, &teamID, includeYouth);
}

wchar_t *CDBTeam::GetClickableTeamName(CTeamIndex const &teamID, bool includeYouth) {
    return plugin::CallMethodAndReturn<wchar_t *, 0xEDD3A0>(this, &teamID, includeYouth);
}

CTeamSponsor &CDBTeam::GetSponsor() {
    return plugin::CallMethodAndReturn<CTeamSponsor &, 0xED5170>(this);
}

bool CDBTeam::IsRivalWith(CTeamIndex const &teamIndex) {
    return CallMethodAndReturn<bool, 0xEE40D0>(this, &teamIndex);
}

CDBTeamKit *CDBTeam::GetKit() {
    return CallMethodAndReturn<CDBTeamKit *, 0xED3D60>(this);
}

bool CDBTeam::IsManagedByAI(bool flag) {
    return CallMethodAndReturn<bool, 0xECA230>(this, flag);
}

UChar CDBTeam::GetColorId(UInt colorType) {
    return CallMethodAndReturn<UChar, 0xED2F20>(this, colorType);
}

UInt CDBTeam::GetColorRGBA(UInt colorType) {
    return CallMethodAndReturn<UInt, 0xED2FC0>(this, colorType);
}

UInt CDBTeam::GetNumPlayers() {
    return CallMethodAndReturn<UInt, 0xEC9820>(this);
}

UInt CDBTeam::GetPlayer(UChar index) {
    return CallMethodAndReturn<UInt, 0xEC9950>(this, index);
}

UInt CDBTeam::GetFifaID() {
    return CallMethodAndReturn<UInt, 0xEC9550>(this);
}

bool CDBTeam::CanBuyOnlyBasquePlayers() {
    return CallMethodAndReturn<bool, 0xEC9520>(this);
}

bool CDBTeam::YouthPlayersAreBasques() {
    return *raw_ptr<UInt>(this, 0xF4) & 0x1000000;
}

UChar CDBTeam::GetYouthPlayersCountry() {
    return CallMethodAndReturn<UChar, 0xEC9510>(this);
}

UChar CDBTeam::GetCountryId() {
    return GetTeamID().countryId;
}

CDBCountry *CDBTeam::GetCountry() {
    return ::GetCountry(GetCountryId());
}

CStadiumDevelopment *CDBTeam::GetStadiumDevelopment() {
    return CallMethodAndReturn<CStadiumDevelopment *, 0xECFFC0>(this);
}

Int CDBTeam::GetManagerId() {
    return CallMethodAndReturn<UInt, 0xECA1A0>(this);
}

void CDBTeam::SetFlag(UInt flag, Bool enable) {
    CallMethod<0xEC9590>(this, flag, enable);
}

Char CDBTeam::SendMail(UInt mailId, CEAMailData const &mailData, Int flag) {
    return CallMethodAndReturn<Char, 0xEE1F10>(this, mailId, &mailData, flag);
}

void CDBTeam::ChangeMoney(UInt type, EAGMoney const &money, UInt flag) {
    CallMethod<0xECA910>(this, type, &money, flag);
}

void CDBTeam::OnCompetitionElimination(CCompID const &compID, UInt cupRoundId) {
    CallMethod<0xEEE0B0>(this, &compID, cupRoundId);
}

UChar CDBTeam::GetFirstTeamDivision() {
    return CallMethodAndReturn<UChar, 0xECC400>(this);
}

UChar CDBTeam::GetFirstTeamDivisionLastSeason() {
    return CallMethodAndReturn<UChar, 0xECC6B0>(this);
}

CClubFans *CDBTeam::GetClubFans() {
    return CallMethodAndReturn<CClubFans *, 0xED01B0>(this);
}

CTeamStaff *CDBTeam::GetTeamStaff() {
    return *raw_ptr<CTeamStaff *>(this, 0x49B4);
}

CTeamFanshops *CDBTeam::GetFanShops() {
    return CallMethodAndReturn<CTeamFanshops *, 0xECFFF0>(this);
}

CClubHistory *CDBTeam::GetClubHistory() {
    return CallMethodAndReturn<CClubHistory *, 0xED1BE0>(this);
}

Bool CDBTeam::IsPlayerPresent(UInt playerId) {
    for (UInt i = 0; i < GetNumPlayers(); i++) {
        if (GetPlayer(i) == playerId)
            return true;
    }
    return false;
}

CTeamIndex &CDBLeagueBase::GetTeamAtPosition(int position) {
    return plugin::CallMethodAndReturn<CTeamIndex &, 0x10CBE20>(this, position);
}

unsigned int CTeamIndex::ToInt() const {
    return index | (countryId << 16) | (type << 24);
}

CTeamIndex CTeamIndex::firstTeam() const {
    CTeamIndex result;
    result.index = index;
    result.countryId = countryId;
    result.type = 0;
    return result;
}

bool CTeamIndex::isNull() const {
    return countryId == 0;
}

void CTeamIndex::clear() {
    index = 0;
    countryId = 0;
    type = 0;
}

CTeamIndex CTeamIndex::make(unsigned char CountryId, unsigned char Type, unsigned short Index) {
    CTeamIndex result;
    result.countryId = CountryId;
    result.index = Index;
    result.type = Type;
    return result;
}

CTeamIndex CTeamIndex::make(unsigned int value) {
    CTeamIndex result;
    *(unsigned int *)(&result) = value;
    return result;
}

CTeamIndex CTeamIndex::null() {
    CTeamIndex result;
    result.clear();
    return result;
}

Bool operator<(EAGMoney const &a, EAGMoney const &b) { return CallAndReturn<Bool, 0x0149CFFA>(&a, &b); }

Bool operator>(EAGMoney const &a, EAGMoney const &b) { return CallAndReturn<Bool, 0x0149D03A>(&a, &b); }

Bool operator<=(EAGMoney const &a, EAGMoney const &b) { return CallAndReturn<Bool, 0x0149D07A>(&a, &b); }

Bool operator>=(EAGMoney const &a, EAGMoney const &b) { return CallAndReturn<Bool, 0x0149D0BA>(&a, &b); }

Bool operator==(EAGMoney const &a, EAGMoney const &b) { return CallAndReturn<Bool, 0x0149D0FA>(&a, &b); }

Bool operator!=(EAGMoney const &a, EAGMoney const &b) { return CallAndReturn<Bool, 0x0149D138>(&a, &b); }

Bool operator>(Int64 a, EAGMoney const &b) { return CallAndReturn<Bool, 0x0149D1BA>(a, &b); }

Bool operator==(Int64 a, EAGMoney const &b) { return CallAndReturn<Bool, 0x0149D286>(a, &b); }

Bool operator<(EAGMoney const &a, Int64 b) { return CallAndReturn<Bool, 0x0149D30A>(&a, b); }

Bool operator>(EAGMoney const &a, Int64 b) { return CallAndReturn<Bool, 0x0149D34E>(&a, b); }

Bool operator<=(EAGMoney const &a, Int64 b) { return CallAndReturn<Bool, 0x0149D392>(&a, b); }

Bool operator>=(EAGMoney const &a, Int64 b) { return CallAndReturn<Bool, 0x0149D3D6>(&a, b); }

Bool operator==(EAGMoney const &a, Int64 b) { return CallAndReturn<Bool, 0x0149D41A>(&a, b); }

Bool operator!=(EAGMoney const &a, Int64 b) { return CallAndReturn<Bool, 0x0149D45C>(&a, b); }

EAGMoney operator+(EAGMoney const &a, EAGMoney const &b) {
    EAGMoney result;
    Call<0x0149D49E>(&result, &a, &b);
    return result;
}

EAGMoney &operator+=(EAGMoney &a, EAGMoney const &b) {
    Call<0x0149D4D8>(&a, &b);
    return a;
}

EAGMoney operator+(EAGMoney const &a, Int64 b) {
    EAGMoney result;
    Call<0x0149D515>(&result, &a, b);
    return result;
}

EAGMoney operator+(EAGMoney const &a, Int b) {
    EAGMoney result;
    Call<0x0149D5A1>(&result, &a, b);
    return result;
}

EAGMoney &operator+=(EAGMoney &a, Int64 b) {
    Call<0x0149D644>(&a, b);
    return a;
}

EAGMoney &operator+=(EAGMoney &a, Int b) {
    Call<0x0149D670>(&a, b);
    return a;
}

EAGMoney operator+(EAGMoney const &a, Double b) {
    EAGMoney result;
    Call<0x0149D69C>(&result, &a, b);
    return result;
}

EAGMoney &operator+=(EAGMoney &a, Double b) {
    Call<0x0149D77A>(&a, b);
    return a;
}

EAGMoney operator-(EAGMoney const &a, EAGMoney const &b) {
    EAGMoney result;
    Call<0x0149D7D0>(&result, &a, &b);
    return result;
}

EAGMoney &operator-=(EAGMoney &a, EAGMoney const &b) {
    Call<0x0149D80A>(&a, &b);
    return a;
}

EAGMoney operator-(EAGMoney const &a, Int64 b) {
    EAGMoney result;
    Call<0x0149D82A>(&result, &a, b);
    return result;
}

EAGMoney operator-(EAGMoney const &a, Int b) {
    EAGMoney result;
    Call<0x0149D8B8>(&result, &a, b);
    return result;
}

EAGMoney &operator-=(EAGMoney &a, Int64 b) {
    Call<0x0149D902>(&a, b);
    return a;
}

EAGMoney &operator-=(EAGMoney &a, Int b) {
    Call<0x0149D976>(&a, b);
    return a;
}

EAGMoney &operator-=(EAGMoney &a, Double b) {
    Call<0x0149D9A6>(&a, b);
    return a;
}

EAGMoney operator*(EAGMoney const &a, Double b) {
    EAGMoney result;
    Call<0x0149DAE0>(&result, &a, b);
    return result;
}

EAGMoney operator*(Double a, EAGMoney const &b) {
    EAGMoney result;
    Call<0x0149DB17>(&result, a, &b);
    return result;
}

EAGMoney &operator*=(EAGMoney &a, Double b) {
    Call<0x0149DB4E>(&a, b);
    return a;
}

EAGMoney operator/(EAGMoney const &a, Double b) {
    EAGMoney result;
    Call<0x0149DB6B>(&result, &a, b);
    return result;
}

EAGMoney &operator/=(EAGMoney &a, Double b) {
    Call<0x0149DBA2>(&a, b);
    return a;
}

EAGMoney &operator/=(EAGMoney &a, EAGMoney const &b) {
    Call<0x0149DBBF>(&a, &b);
    return a;
}

EAGMoney operator-(EAGMoney const &rhs) {
    EAGMoney result;
    Call<0x0149DBD0>(&result, &rhs);
    return result;
}

UInt GetCurrentMetric() {
    void *metrics = CallAndReturn<void *, 0x14AC159>();
    return CallMethodAndReturn<UInt, 0x14AC3A0>(metrics);
}

bool operator==(CTeamIndex const &a, CTeamIndex const &b) {
    return a.index == b.index && a.countryId == b.countryId && a.type == b.type;
}

UChar CDBCountry::GetLeagueAverageLevel() {
    return plugin::CallMethodAndReturn<UChar, 0xFD6FA0>(this);
}

UChar CDBCountry::GetCountryId() {
    return plugin::CallMethodAndReturn<UChar, 0xFD6410>(this);
}

UInt CDBCountry::GetContinent() {
    return plugin::CallMethodAndReturn<UInt, 0xFD6830>(this);
}

const WideChar *CDBCountry::GetName() {
    return plugin::CallMethodAndReturn<const WideChar *, 0xFD67F0>(this);
}

const WideChar *CDBCountry::GetAbbr() {
    return plugin::CallMethodAndReturn<const WideChar *, 0xFD6800>(this);
}

const WideChar *CDBCountry::GetContinentName() {
    return plugin::CallMethodAndReturn<const WideChar *, 0xFD68D0>(this);
}

const Int CDBCountry::GetNumClubs() {
    return CallMethodAndReturn<Int, 0xFD6910>(this);
}

const UInt CDBCountry::GetNumRegenPlayers() {
    return plugin::CallMethodAndReturn<UInt, 0xFDC7C0>(this);
}

const UChar CDBCountry::GetLanguage(UChar number) {
    return plugin::CallMethodAndReturn<UInt, 0xFD71B0>(this, number);
}

const UChar CDBCountry::GetFirstLanguage() {
    return plugin::CallMethodAndReturn<UInt, 0xFD71A0>(this);
}

const Bool CDBCountry::IsPlayerInNationalTeam(UInt playerId) {
    return plugin::CallMethodAndReturn<Bool, 0xFD7CD0>(this, playerId);
}

void CDBCountry::SetFifaRanking(Float value) {
	*raw_ptr<Float>(this, 0x284) = value;
}

Float CDBCountry::GetFifaRanking() {
	return *raw_ptr<Float>(this, 0x284);
}

TeamLeaguePositionData::TeamLeaguePositionData() {
    m_teamID.countryId = 0;
    m_teamID.index = 0;
    m_teamID.type = 0;
    m_nGames = 0;
    m_nPoints = 0;
    m_nGoalsScored = 0;
    m_nGoalsAgainst = 0;
    m_nWins = 0;
    m_nDraws = 0;
    m_nLoses = 0;
    m_nAwayGoals = 0;
    m_nPosition = 0;
}

UChar CDBPlayer::GetPlayingForm() {
    return CallMethodAndReturn<UChar, 0xFA2AD0>(this);
}

UInt CDBPlayer::GetAge() {
    return CallMethodAndReturn<UInt, 0xF9B2F0>(this);
}

Char CDBPlayer::GetTalent(CDBEmployee *employee) {
    return CallMethodAndReturn<Char, 0xFB2590>(this, employee);
}

Char CDBPlayer::GetMainPosition() {
    return CallMethodAndReturn<Char, 0xFAAE10>(this);
}

UChar CDBPlayer::GetLevel(Char position, Bool special) {
    return CallMethodAndReturn<unsigned char, 0xFAAD60>(this, position, special);
}

CTeamIndex CDBPlayer::GetCurrentTeam() {
    CTeamIndex result;
    CallMethod<0xFB5240>(this, &result);
    return result;
}

EAGMoney CDBPlayer::GetMarketValue(CDBEmployee *employee) {
    EAGMoney result;
    CallMethod<0xF9A980>(this, &result, employee);
    return result;
}

EAGMoney CDBPlayer::GetDemandValue() {
    EAGMoney result;
    CallMethod<0xF9B6A0>(this, &result);
    return result;
}

EAGMoney CDBPlayer::GetMinRelFee() {
    EAGMoney result;
    CallMethod<0xF9B6E0>(this, &result);
    return result;
}

void CDBPlayer::SetDemandValue(EAGMoney const &money) {
    CallMethod<0xFC2670>(this, &money);
}

void CDBPlayer::SetMinRelFee(EAGMoney const &money) {
    CallMethod<0xFC27A0>(this, &money);
}

void CDBPlayer::SetNationality(UChar index, UChar nation) {
    CallMethod<0xF9AF00>(this, index, nation);
}

void CDBPlayer::SetIsBasque(bool basque) {
    CallMethod<0xF9AF50>(this, basque);
}

UChar CDBPlayer::GetNumKnownLanguages() {
    return CallMethodAndReturn<UChar, 0xF9B230>(this);
}

UChar CDBPlayer::GetLanguage(UChar number) {
    return CallMethodAndReturn<UChar, 0xF9B220>(this, number);
}

void CDBPlayer::SetLanguage(UChar index, UChar language) {
    CallMethod<0xF9B200>(this, index, language);
}

UInt CDBPlayer::GetID() {
    return *plugin::raw_ptr<UInt>(this, 0xD8);
}

Bool CDBPlayer::IsInNationalTeam() {
    return CallMethodAndReturn<Bool, 0xFB2BC0>(this);
}

Bool CDBPlayer::IsRetiredFromNationalTeam() {
    return CallMethodAndReturn<Bool, 0xFB2B20>(this);
}

Bool CDBPlayer::IsInU21NationalTeam() {
    return CallMethodAndReturn<Bool, 0xFB1D20>(this);
}

Bool CDBPlayer::IsEndOfCareer() {
    return CallMethodAndReturn<Bool, 0xFB1D30>(this);
}

UChar CDBPlayer::GetNumPlannedYearsForCareer() {
    return CallMethodAndReturn<UChar, 0xF9C2C0>(this);
}

CPlayerStats const *CDBPlayer::GetStatsConst() const {
    return CallMethodAndReturn<CPlayerStats const *, 0xF9CE50>(this);
}

CPlayerStats *CDBPlayer::GetStats() {
    return CallMethodAndReturn<CPlayerStats *, 0xF9CDD0>(this);
}

UChar CDBPlayer::GetPotential() {
    return CallMethodAndReturn<UChar, 0xF9CC00>(this);
}

UChar CDBPlayer::GetPositionRole() {
    return CallMethodAndReturn<UChar, 0xFB4AF0>(this);
}

Char CDBPlayer::GetAbility(UInt ability, CDBEmployee *employee) {
    return CallMethodAndReturn<UChar, 0xFA57D0>(this, ability, employee);
}

UInt CDBPlayer::GetEmpicsID() {
    return CallMethodAndReturn<UInt, 0xF98600>(this);
}

CJDate CDBPlayer::GetBirthdate() {
    CJDate result;
    CallMethod<0xF9B2E0>(this, &result);
    return result;
}

UChar CDBPlayer::GetNationality(UChar number) {
    return CallMethodAndReturn<UChar, 0xF9AF10>(this, number);
}

CTeamIndex CDBPlayer::GetNationalTeam() {
    CTeamIndex result;
    CallMethod<0xF9AF60>(this, &result);
    return result;
}

String CDBPlayer::GetName(bool shortForm) {
    WideChar playerName[256];
    CallMethod<0x13255C0>(0, this, playerName, shortForm);
    return playerName;
}

NameDesc CDBPlayer::GetNameDesc() {
    NameDesc desc;
    CallMethod<0xFA25E0>(this, &desc);
    return desc;
}

FmVec<CDBSponsorContractAdBoards> &CTeamSponsor::GetAdBoardSponsors() {
    return *raw_ptr<FmVec<CDBSponsorContractAdBoards>>(this, 0x80);
}

CDBSponsorContractBase &CTeamSponsor::GetMainSponsor() {
    return plugin::CallMethodAndReturn<CDBSponsorContractBase &, 0x11B2F40>(this);
}

Bool CDBSponsorContractBase::IsActive() {
    return plugin::CallVirtualMethodAndReturn<Bool, 13>(this);
}

SponsorPlacement &CDBSponsorContractBase::GetPlacement() {
    return plugin::CallMethodAndReturn<SponsorPlacement &, 0x11B8530>(this);
}

UInt CDBTeamKit::GetPartType(UChar kitType, UChar shirtPart) {
    return CallMethodAndReturn<UInt, 0xFFCBD0>(this, kitType, shirtPart);
}

UChar CDBTeamKit::GetPartColor(UChar kitType, UChar shirtPart, UChar colorIndex) {
    return CallMethodAndReturn<UInt, 0xFFCC20>(this, kitType, shirtPart, colorIndex);
}

Bool CDBTeamKit::HasUserKitType(UInt kitType) {
    return CallMethodAndReturn<Bool, 0xFFD940>(this, kitType);
}

Bool CDBTeamKit::HasUserKit() {
    return CallMethodAndReturn<Bool, 0xFFDD00>(this);
}

Bool CDBTeamKit::UserKitInUse() {
    return CallMethodAndReturn<Bool, 0xFFDD80>(this);
}

WideChar const *CDBTeamKit::GetBadgePath() {
    return CallMethodAndReturn<WideChar const *, 0xFFCDA0>(this);
}

WideChar const *CDBTeamKit::GetUserKitPath(UInt kitType) {
    return CallMethodAndReturn<WideChar const *, 0xFFDDB0>(this, kitType);
}

UChar CDBTeamKit::GetShirtBadgePosition(UChar kitType) {
    return CallMethodAndReturn<UChar, 0xFFCD60>(this, kitType);
}

void CAssessmentInfo::AddPoints(float points) {
    CallMethod<0x121DE60>(this, points);
}

void CRandom::SetSeed(Int seed1, Int seed2) {
    Call<0x149E2F6>(seed1, seed2);
}

void CRandom::GetSeed(Int &seed1, Int &seed2) {
    Call<0x149E30B>(&seed1, &seed2);
}

Int CRandom::GetRandomInt(Int maxExclusive) {
    return CallAndReturn<Int, 0x149E320>(maxExclusive);
}

Int CRandom::GetRandomBetween(Int min, Int max) {
    return CallAndReturn<Int, 0x149E332>(min, max);
}

Float CRandom::GetRandomFloat(Float maxExclusive) {
    return CallAndReturn<Float, 0x149E347>(maxExclusive);
}

CTeamIndex CDBOneMatch::GetHostTeamID() {
    CTeamIndex teamID;
    CallMethod<0xE814C0>(this, &teamID);
    return teamID;
}

CDBTeam *CDBOneMatch::GetHostTeam() {
    CTeamIndex id = GetHostTeamID();
    if (id.countryId)
        return ::GetTeam(id);
    return nullptr;
}

CCompID CDBOneMatch::GetCompID() {
    CCompID compId;
    CallMethod<0xE80190>(this, &compId);
    return compId;
}

UInt CDBOneMatch::GetCompIDInt() {
    UInt compId = 0;
    CallMethod<0xE80190>(this, &compId);
    return compId;
}

CTeamIndex CDBOneMatch::GetTeamID(Bool home) {
    CTeamIndex teamID;
    CallMethod<0xE7FD30>(this, &teamID, home);
    return teamID;
}

CDBTeam *CDBOneMatch::GetTeam(Bool home) {
    CTeamIndex teamID;
    CallMethod<0xE7FD30>(this, &teamID, home);
    if (teamID.countryId != 0)
        return ::GetTeam(teamID);
    return nullptr;
}

CTeamIndex CDBOneMatch::GetHomeTeamID() {
    CTeamIndex teamID;
    CallMethod<0xE7FCF0>(this, &teamID);
    return teamID;
}

CDBTeam *CDBOneMatch::GetHomeTeam() {
    CTeamIndex teamID;
    CallMethod<0xE7FCF0>(this, &teamID);
    if (teamID.countryId != 0)
        return ::GetTeam(teamID);
    return nullptr;
}

CTeamIndex CDBOneMatch::GetAwayTeamID() {
    CTeamIndex teamID;
    CallMethod<0xE7FD00>(this, &teamID);
    return teamID;
}

CDBTeam *CDBOneMatch::GetAwayTeam() {
    CTeamIndex teamID;
    CallMethod<0xE7FD00>(this, &teamID);
    if (teamID.countryId != 0)
        return ::GetTeam(teamID);
    return nullptr;
}

Bool CDBOneMatch::CheckFlag(UInt flag) {
	return CallMethodAndReturn<Bool, 0xE80230>(this, flag);
}

void CDBOneMatch::GetResult(UChar & outHome, UChar & outAway) {
	CallMethod<0xE7FF20>(this, &outHome, &outAway);
}

UInt CDBOneMatch::GetRoundPairIndex() {
    return CallMethodAndReturn<UInt, 0xE801B0>(this);
}

CDBOneMatch *GetCurrentMatch() {
    return *(CDBOneMatch **)0x3124748;
}

UInt GetGuiColor(UInt colorId) {
    return CallAndReturn<UInt, 0x4E8C10>(colorId);
}

Bool IsEuropeanCountry(Int countryId) {
    return CallAndReturn<Bool, 0xFF8020>(countryId);
}

Bool IsNonEuropeanCountry(Int countryId) {
    return CallAndReturn<Bool, 0xFD8A60>(countryId);
}

Bool IsEuropeanUnion(Int countryId) {
    return CallAndReturn<Bool, 0xFD8A30>(countryId);
}

Bool IsAfricanCaribbeanAndPacificGroupOfStates(Int countryId) {
    return CallAndReturn<Bool, 0xFD8BD0>(countryId);
}

Bool IsPlayerForeignerForCompetition(CDBPlayer *player, CCompID const &compID) {
    return CallAndReturn<Bool, 0x13D2510>(player, &compID);
}

void *FmNew(UInt size) {
    return CallAndReturn<void *, 0x15738F3>(size);
}

void FmDelete(void *data) {
    Call<0x157347A>(data);
}

bool BinaryReaderIsVersionGreaterOrEqual(void *reader, UInt year, UInt build) {
    return CallMethodAndReturn<bool, 0x1338EA0>(reader, year, build);
}

void BinaryReaderReadString(void *reader, WideChar *out, UInt maxLen) {
    CallMethod<0x1338700>(reader, out, maxLen);
}

void BinaryReaderReadUInt32(void *reader, UInt *out) {
    CallMethod<0x1338B10>(reader, out);
}

bool BinaryReaderCheckFourcc(void *reader, UInt fourcc) {
    return CallMethodAndReturn<bool, 0x1338EC0>(reader, fourcc);
}

void BinaryReaderReadUInt16(void *reader, UShort *out) {
    CallMethod<0x13389E0>(reader, out);
}

void BinaryReaderReadUInt8(void *reader, UChar *out) {
    CallMethod<0x1338890>(reader, out);
}

void BinaryReaderReadFloat(void *reader, Float *out) {
    CallMethod<0x1338BF0>(reader, out);
}

void SaveGameReadString(void *save, WideChar *out, UInt maxLen) {
    CallMethod<0x1080EB0>(save, out, maxLen);
}

void SaveGameWriteString(void *save, WideChar const *str) {
    CallMethod<0x1080130>(save, str);
}

void SaveGameReadInt8(void *save, UChar &out) {
    CallMethod<0x1080390>(save, &out);
}

void SaveGameReadInt32(void *save, UInt &out) {
    CallMethod<0x10802D0>(save, &out);
}

UInt SaveGameReadInt32(void *save) {
    return CallMethodAndReturn<UInt, 0x10802F0>(save);
}

void SaveGameReadFloat(void *save, Float &out) {
    CallMethod<0x1080650>(save, &out);
}

Float SaveGameReadFloat(void *save) {
    return CallMethodAndReturn<Float, 0x1080670>(save);
}

void SaveGameReadFloatArray(void *save, Float *values, UInt count) {
    CallMethod<0x1080630>(save, values, count);
}

UInt SaveGameReadSize(void *save) {
    return CallMethodAndReturn<UInt, 0x1080780>(save);
}

void SaveGameReadData(void *save, void *data, UInt size) {
    CallMethod<0x1080690>(save, data, size);
}

void SaveGameWriteInt8(void *save, UChar value) {
    CallMethod<0x107F400>(save, value);
}

void SaveGameWriteInt32(void *save, UInt value) {
    CallMethod<0x107F380>(save, value);
}

void SaveGameWriteFloat(void *save, Float value) {
    CallMethod<0x107F5F0>(save, value);
}

void SaveGameWriteFloatArray(void *save, Float const *values, UInt count) {
    CallMethod<0x107F5D0>(save, values, count);
}

void SaveGameWriteSize(void *save, UInt value) {
    CallMethod<0x107F670>(save, value);
}

void SaveGameWriteData(void *save, void *data, UInt size) {
    CallMethod<0x107F610>(save, data, size);
}

void SaveGameWriteString(void *save, FmString const &value) {
    CallMethod<0x1080C80>(save, &value);
}

UInt SaveGameLoadGetVersion(void *save) {
    return CallMethodAndReturn<UInt, 0x107F730>(save);
}

CDBPlayer *FindPlayerByStringID(WideChar const *stringID) {
    return CallAndReturn<CDBPlayer *, 0xFAF750>(stringID);
}

CParameterFiles *CParameterFiles::Instance() {
    return CallAndReturn<CParameterFiles *, 0x103E540>();
}

UChar CParameterFiles::GetProbabilityOfForeignPlayersInYouthTeam() {
    return CallMethodAndReturn<UChar, 0x1028120>(this);
}

UChar CParameterFiles::GetProbabilityOfBasquePlayersInYouthTeam() {
    return CallMethodAndReturn<UChar, 0x1028140>(this);
}

UChar CPlayerStats::GetNumInternationalCaps() const {
    return CallMethodAndReturn<UChar, 0x1005F40>(this);
}

Int CPlayerStats::GetStatsForMatches(CMatchStatistics &outStats, UInt &numConsecutiveHomeMatches, UInt &numMatchesWithMark, UInt &numMOTMs, CCompID const &compID, UInt minMinutesForMark, Bool bLocalizedMarks, Bool bWithFriendlies, UInt compIDMask, CJDate startDate, CDBTeam *team, Bool bWithoutQuali) const {
    return CallMethodAndReturn<Int, 0x1008FD0>(this, &outStats, &numConsecutiveHomeMatches, &numMatchesWithMark, &numMOTMs, &compID, minMinutesForMark, bLocalizedMarks, bWithFriendlies, compIDMask, startDate, team, bWithoutQuali);
}

void CPlayerStats::SetNumPlayerOfTheMonth(UChar count) { CallMethod<0x1006090>(this, count); }

void CPlayerStats::SetNumPlayerOfTheYear(UChar count) { CallMethod<0x10060B0>(this, count); }

void CPlayerStats::SetNumFifaWorldPlayerAwards(UChar count) { CallMethod<0x10060F0>(this, count); }

void CPlayerStats::SetNumEuropeanPlayerOfTheYear(UChar count) { CallMethod<0x1006120>(this, count); }

void CPlayerStats::AddEuroCupWin() { CallMethod<0x1006930>(this); }

void CPlayerStats::AddWorldCupWin() { CallMethod<0x1006950>(this); }

UChar CPlayerStats::GetNumPlayerOfTheMonth() const { return CallMethodAndReturn<UChar, 0x10060A0>(this); }

UInt CPlayerStats::GetNumPlayerOfTheYear() const { return CallMethodAndReturn<UInt, 0x10060E0>(this); }

UInt CPlayerStats::GetNumFifaWorldPlayerAwards() const { return CallMethodAndReturn<UInt, 0x1006110>(this); }

UChar CPlayerStats::GetNumEuropeanPlayerOfTheYear() const { return CallMethodAndReturn<UChar, 0x1006130>(this); }

UInt CPlayerStats::GetNumECWins() const { return CallMethodAndReturn<UInt, 0x10062D0>(this); }

UInt CPlayerStats::GetNumWCWins() const { return CallMethodAndReturn<UInt, 0x10062E0>(this); }

CDBPlayer *CPlayerStats::GetPlayer() const {
    return *raw_ptr<CDBPlayer *>(this, 0);
}

Int CPlayerStats::GetPlayerId() const {
    return CallMethodAndReturn<Int, 0x1006970>(this);
}

NetComStorageIterator NetComStorageBegin(eNetComStorage storageType) {
    NetComStorageIterator it;
    Call<0x156F100>(&it, storageType);
    return it;
}

NetComStorageIterator NetComStorageEnd(eNetComStorage storageType) {
    NetComStorageIterator it;
    Call<0x156F160>(&it, storageType);
    return it;
}

void NetComStorageNext(NetComStorageIterator &it) {
    Call<0x156F1E0>(&it);
}

UInt CStadiumDevelopment::GetNumSeats() {
    return CallMethodAndReturn<UInt, 0xF74220>(this);
}

const WideChar *CStadiumDevelopment::GetStadiumName(Bool withSponsor) {
    return CallMethodAndReturn<const WideChar *, 0xF73BD0>(this, withSponsor);
}

CDBTeam *CStadiumDevelopment::GetTeam() {
    return *raw_ptr<CDBTeam *>(this, 0x52C);
}

void CDBEmployeeBase::SetBirthDate(CJDate const &date) {
    CallMethod<0xE7E940>(this, &date);
}

WideChar const *CDBEmployeeBase::GetName(WideChar const *nameBuf) {
    return CallMethodAndReturn<WideChar const *, 0xE7E9E0>(this, nameBuf);
}

CDBEmployeeBase *CDBEmployee::GetBase() {
    return raw_ptr<CDBEmployeeBase>(this, 0x10);
}

void CDBEmployee::SetBirthDate(CJDate const &date) {
    CallMethod<0xEA2EF0>(this, &date);
}

void CDBEmployee::SetFirstNationality(UChar countryId) {
    CallMethod<0xEA2C20>(this, countryId);
}

void CDBEmployee::SetSecondNationality(UChar countryId) {
    CallMethod<0xEA2C40>(this, countryId);
}

void CDBEmployee::SetLanguage(UChar languageId) {
    CallMethod<0xEA35B0>(this, languageId);
}

void CDBEmployee::SetAdditionalLanguageLevel(UChar index, UChar languageId, UChar level) {
    CallMethod<0xEA35E0>(this, index, languageId, level);
}

CTeamIndex CDBEmployee::GetTeamID() {
    CTeamIndex teamID;
    CallMethod<0xEA2C10>(this, &teamID);
    return teamID;
}

void CDBEmployee::GetTeamID(CTeamIndex &out) {
    CallMethod<0xEA2C00>(this, &out);
}

WideChar const *CDBEmployee::GetName(WideChar const *nameBuf) {
    return GetBase()->GetName(nameBuf);
}

CWorker *CDBStaff::GetWorker() {
    return *raw_ptr<CWorker *>(this, 0xC);
}

void CDBStaff::SetBirthDate(CJDate const &date) {
    CallMethod<0x11029C0>(this, &date);
}

CJDate const &CDBStaff::GetBirthdate() {
    return CallMethodAndReturn<CJDate const &, 0x11029D0>(this);
}

void CDBStaff::SetNationality(UChar index, UChar countryId) {
    CallMethod<0x11029F0>(this, index, countryId);
}

UChar CDBStaff::GetNationality(UChar index) {
    return CallMethodAndReturn<UChar, 0x1102A10>(this, index);
}

WideChar const *CDBStaff::GetName(WideChar const *nameBuf) {
    return CallMethodAndReturn<WideChar const *, 0x11028D0>(this, nameBuf);
}

UInt CDBStaff::GetAge() {
    return CallMethodAndReturn<UInt, 0x11029E0>(this);
}

void CDBStaff::SetTalent(UInt talent) {
    CallMethod<0x1102AE0>(this, talent);
}

UInt CDBStaff::GetTalent() {
    return CallMethodAndReturn<UInt, 0x1102AF0>(this);
}

unsigned int CDBRound::GetNumOfPairs() {
    return plugin::CallMethodAndReturn<unsigned int, 0x1042260>(this);
}

void CDBRound::GetRoundPair(unsigned int pairIndex, RoundPair &out) {
    plugin::CallMethod<0x1043BA0>(this, pairIndex, &out);
}

RoundPair &CDBRound::GetRoundPair(unsigned int pairIndex) {
    return *raw_ptr<RoundPair>(this, 0x2088 + sizeof(RoundPair) * pairIndex);
}

Bool CDBRound::GetTeamResult(CTeamIndex teamID, UChar &goalsFor, UChar &goalsAgainst, Bool &home) {
    return CallMethodAndReturn<Bool, 0x1043360>(this, teamID, &goalsFor, &goalsAgainst, &home);
}

UInt CDBRound::GetLegFlags(UInt leg) const {
    return CallMethodAndReturn<UInt, 0x10423C0>(this, leg);
}

RoundPair::RoundPair() {
    plugin::CallMethod<0x10ED6C0>(this);
}

Bool RoundPair::AreTeamsValid() const {
    return plugin::CallMethodAndReturn<bool, 0x10EDE40>(this);
}

void RoundPair::GetResult(UChar &outResult1, UChar &outResult2, UInt &outFlags, UChar flags) const {
    CallMethod<0x10ED490>(this, &outResult1, &outResult2, &outFlags, flags);
}

void *RoundPair::GetResultString(void *str, UChar flags, const wchar_t *team1name, const wchar_t *team2name) const {
    return plugin::CallMethodAndReturn<void *, 0x10ED930>(this, str, flags, team1name, team2name);
}

Bool RoundPair::IsFinished() const {
    return CallMethodAndReturn<Bool, 0x10ED450>(this);
}

CTeamIndex const &RoundPair::GetWinner() const {
    return CallMethodAndReturn<CTeamIndex const &, 0x10EDD80>(this);
}

CTeamIndex const &RoundPair::GetLoser() const {
    return CallMethodAndReturn<CTeamIndex const &, 0x10EDE00>(this);
}

Bool RoundPair::TestFlag(UInt flag) const {
    return CallMethodAndReturn<Bool, 0x10ED420>(this, flag);
}

CTeamIndex const &RoundPair::Get1stTeam() const {
    return CallMethodAndReturn<CTeamIndex const &, 0x10ED3F0>(this);
}

CTeamIndex const &RoundPair::Get2ndTeam() const {
    return CallMethodAndReturn<CTeamIndex const &, 0x10ED400>(this);
}

Int RoundPair::GetMatchEventsStartIndex(UInt leg) const {
    return CallMethodAndReturn<Int, 0x10ED630>(this, leg);
}

void RoundPair::SetMatchEventsStartIndex(Int index, UInt leg) {
    CallMethod<0x10ED620>(this, index, leg);
}

Bool RoundPair::IsWinner(Bool b2ndTeam) const {
    return CallMethodAndReturn<Bool, 0x10EDE20>(this, b2ndTeam);
}

SyncFile::SyncFile() {
    plugin::CallMethod<0x14BDD8D>(this);
}

SyncFile::SyncFile(UInt bufferSize) {
    plugin::CallMethod<0x14BDDC9>(this, bufferSize);
}

SyncFile::~SyncFile() {
    plugin::CallMethod<0x14BDE22>(this);
}

Bool SyncFile::Load(WideChar const *filePath, UInt mode, UInt version) {
    return plugin::CallMethodAndReturn<Bool, 0x14BDE5F>(this, filePath, mode, version);
}

Bool SyncFile::Close() {
    return plugin::CallMethodAndReturn<Bool, 0x14BE00B>(this);
}

UInt SyncFile::Mode() {
    return plugin::CallMethodAndReturn<UInt, 0x14BE07D>(this);
}

UInt SyncFile::Version() {
    return plugin::CallMethodAndReturn<UInt, 0x14BE08C>(this);
}

Bool SyncFile::Chunk(UInt id, UInt minVersion) {
    return plugin::CallMethodAndReturn<Bool, 0x14BE0AA>(this, id, minVersion);
}

Bool SyncFile::Data(void *buf, void *pDefaultValue, UInt minVersion, UInt size) {
    return plugin::CallMethodAndReturn<Bool, 0x14BE194>(this, buf, pDefaultValue, minVersion, size);
}

Bool SyncFile::DataArray(void *buf, UInt count, void *pDefaultValue, UInt minVersion, UInt size) {
    return plugin::CallMethodAndReturn<Bool, 0x14BE25F>(this, buf, count, pDefaultValue, minVersion, size);
}

Bool SyncFile::String(WideChar *out, UInt maxLen, WideChar const *defaultValue, UInt minVersion) {
    return plugin::CallMethodAndReturn<Bool, 0x14BE461>(this, out, maxLen, defaultValue, minVersion);
}

Bool SyncFile::IsVersionGreaterOrEqual(UInt version) {
    return plugin::CallMethodAndReturn<Bool, 0x14BE86F>(this, version);
}

Bool SyncFile::UInt32(UInt *pValue, UInt *pDefaultValue, UInt minVersion) {
    return plugin::CallMethodAndReturn<Bool, 0x10CF840>(this, pValue, pDefaultValue, minVersion);
}

Bool SyncFile::UInt16(UShort *pValue, UShort *pDefaultValue, UInt minVersion) {
    return plugin::CallMethodAndReturn<Bool, 0x10CF8E0>(this, pValue, pDefaultValue, minVersion);
}

Bool SyncFile::UInt8(UChar *pValue, UChar *pDefaultValue, UInt minVersion) {
    return plugin::CallMethodAndReturn<Bool, 0x10CF880>(this, pValue, pDefaultValue, minVersion);
}

WideChar const * CStatsBaseScrWrapper::GetName() {
	return *raw_ptr<WideChar const *>(this, 0xC);
}

UInt CStatsBaseScrWrapper::GetType() {
	return m_nType;
}

void *CStatsBaseScrWrapper::DeletingDestructor(UChar flags) {
	Call<0x1573B5B>(*raw_ptr<WideChar const *>(this, 0xC));
	if (flags & 1)
		Call<0x157347A>(this);
	return this;
}

Bool IsLiechtensteinClubFromSwitzerland(CTeamIndex const &teamIndex) {
    if (teamIndex.countryId == FifamCompRegion::Switzerland) {
        CDBTeam *team = GetTeam(teamIndex);
        if (team) {
            UInt uid = team->GetTeamUniqueID();
            return uid == 0x002F0013  // FC Vaduz
                || uid == 0x002F1015  // USV Eschen/Mauren
                || uid == 0x002F0032; // FC Balzers
        }
    }
    return false;
}

UChar GetTeamCountryId_LiechtensteinCheck(CTeamIndex const &teamIndex) {
    if (IsLiechtensteinClubFromSwitzerland(teamIndex))
        return FifamCompRegion::Liechtenstein;
    return teamIndex.countryId;
}

CEAMailData::CEAMailData() {
    CallMethod<0x1010500>(this);
}

CEAMailData::~CEAMailData() {
    CallMethod<0x100D500>(this);
}

void CEAMailData::SetRoundType(UChar roundType) {
    CallMethod<0x100DA30>(this, roundType);
}

UChar CEAMailData::GetRoundType() const {
    return CallMethodAndReturn<UChar, 0x100DA40>(this);
}

void CEAMailData::SetCompetition(CCompID const &compID) {
    CallMethod<0x100F900>(this, &compID);
}

CCompID CEAMailData::GetCompetition() const {
    CCompID result;
    CallMethod<0x100F910>(this, &result);
    return result;
}

void CEAMailData::SetMoney(EAGMoney const &money) {
    CallMethod<0x100DA70>(this, &money);
}

EAGMoney CEAMailData::GetMoney() const {
    EAGMoney result;
    CallMethod<0x100DA90>(this, &result);
    return result;
}

void CEAMailData::SetArrayValue(UInt index, Int value) {
    CallMethod<0x1010680>(this, index, value);
}

Int CEAMailData::GetArrayValue(UInt index) const {
    return CallMethodAndReturn<Int, 0x1010670>(this, index);
}

void CEAMailData::SetFirstTeam(CTeamIndex const &teamID) {
    CallMethod<0x1010610>(this, &teamID);
}

void CEAMailData::Format(WideChar *dst, UInt maxLen, WideChar const *format, CEAMailData const &mailData) {
    Call<0x14F6AD2>(dst, maxLen, format, &mailData);
}

void CEAMailData::SetPlayer(Int index, Int playerId) {
    CallMethod<0x1010640>(this, index, playerId);
}

void CEAMailData::SetTeam(Int index, CTeamIndex const &teamID) {
    CallMethod<0x1010620>(this, index, &teamID);
}

void CEAMailData::SetTerm(WideChar const *term) {
    CallMethod<0x100DA00>(this, term);
}

void CFMListBox::SetVisible(Bool visible) {
    CallVirtualMethod<20>(this, visible);
}

void CFMListBox::Clear() {
    CallMethod<0xD1AF40>(this);
}

Int CFMListBox::GetNumRows() {
    return CallMethodAndReturn<Int, 0xD18600>(this);
}

Int CFMListBox::GetNumColumns() {
    return CallMethodAndReturn<Int, 0xD18610>(this);
}

Int CFMListBox::GetMaxRows() {
    return CallMethodAndReturn<Int, 0xD18640>(this);
}

Int CFMListBox::GetTotalRows() {
    return CallMethodAndReturn<Int, 0xD18620>(this);
}

void CFMListBox::AddColumnInt(Int64 value, UInt color, Int unk) {
    CallMethod<0xD22BE0>(this, value, color, unk);
}

void CFMListBox::AddColumnFloat(Float value, UInt color, Int unk) {
    CallMethod<0xD1CE20>(this, value, color, unk);
}

void CFMListBox::AddColumnString(WideChar const *str, UInt color, Int unk) {
    CallMethod<0xD1EF40>(this, str, color, unk);
}

void CFMListBox::AddTeamWidget(CTeamIndex const &teamID) {
    CallMethod<0xD1E620>(this, &teamID);
}

void CFMListBox::AddTeamName(CTeamIndex const &teamID, UInt color, Int unk) {
    CallMethod<0xD1F060>(this, &teamID, color, unk);
}

void CFMListBox::AddCompetition(CCompID const &compID, UInt color, Int unk) {
    CallMethod<0xD1D8D0>(this, &compID, color, unk);
}

void CFMListBox::AddCountryFlag(UInt countryId, Int unk) {
    CallMethod<0xD1E7F0>(this, countryId, unk);
}

void CFMListBox::AddColumnImage(WideChar const *imagePath) {
    CallMethod<0xD1E4C0>(this, imagePath);
}

void CFMListBox::SetRowColor(UInt rowIndex, UInt color) {
    CallMethod<0xD18C30>(this, rowIndex, color);
}

void CFMListBox::NextRow(Int unk) {
    CallMethod<0xD18920>(this, unk);
}

void CFMListBox::Create(CXgFMPanel *panel, const char *name) {
    CallMethod<0xD1EEE0>(this, panel, name);
}

Int64 CFMListBox::GetCellValue(UInt row, UInt column) {
    return CallMethodAndReturn<Int64, 0xD189A0>(this, row, column);
}

void CFMListBox::SetCellValue(UInt row, UInt column, Int64 value) {
    return CallMethod<0xD18A00>(this, row, column, value);
}

void CFMListBox::SetFont(Char const *fontName) {
    CallMethod<0xD1A310>(this, fontName);
}

CXgTextBox *CFMListBox::GetCellTextBox(UInt rowIndex, UInt columnIndex) {
    return CallMethodAndReturn<CXgTextBox *, 0xD19DB0>(this, rowIndex, columnIndex);
}

void CFMListBox::Sort(UInt columnIndex, Bool descendingOrder) {
    CallMethod<0xD18510>(this, columnIndex, descendingOrder);
}

EAGMoney::EAGMoney() {
    mValue = 0;
}

EAGMoney::EAGMoney(Int value, eCurrency currency) {
    CallMethod<0x149C16C>(this, value, currency);
}

EAGMoney::EAGMoney(Int64 value, eCurrency currency) {
    CallMethod<0x149C282>(this, value, currency);
}

EAGMoney::EAGMoney(Double value, eCurrency currency) {
    CallMethod<0x149C4AE>(this, value, currency);
}

Int64 EAGMoney::GetValue() const {
    return mValue;
}

Bool EAGMoney::Set(Int64 value, eCurrency currency) {
    return CallMethodAndReturn<Bool, 0x149C6BB>(this, value, currency);
}

Bool EAGMoney::IsValidCurrency(eCurrency currency) const {
    return CallMethodAndReturn<Bool, 0x149CF59>(this, currency);
}

Int64 EAGMoney::GetValueInCurrency(eCurrency currency) const {
    return CallMethodAndReturn<Int64, 0x149C9D7>(this, currency);
}

EAGMoney &EAGMoney::operator=(Int64 rhs) {
    mValue = rhs;
    return *this;
}

EAGMoney EAGMoney::operator-() {
    EAGMoney result;
    CallMethod<0x149CF0A>(this, &result);
    return result;
}

void CompetitionHosts::AddHostCountries(CCompID const &compId, UShort year, UChar hostCountry1, UChar hostCountry2) {
    CallMethod<0x117C040>(this, &compId, year, hostCountry1, hostCountry2);
}

UChar CompetitionHosts::GetHostCountry(CCompID const &compId, UShort year, UInt hostIndex) {
    return CallMethodAndReturn<UChar, 0x1179F80>(this, &compId, year, hostIndex);
}

UChar CompetitionHosts::GetFirstHostCountry(CCompID const &compId, UShort year) {
    return CallMethodAndReturn<UChar, 0x117A030>(this, &compId, year);
}

UChar CompetitionHosts::GetSecondHostCountry(CCompID const &compId, UShort year) {
    return CallMethodAndReturn<UChar, 0x117A050>(this, &compId, year);
}

UChar CompetitionHosts::GetNumberOfHostCountries(CCompID const &compId, UShort year) {
    return CallMethodAndReturn<UChar, 0x117A070>(this, &compId, year);
}

Bool CompetitionHosts::AddHostStadium(CCompID const &compId, UShort year, CTeamIndex teamID) {
    return CallMethodAndReturn<Bool, 0x117C0A0>(this, &compId, year, teamID);
}

Bool CompetitionHosts::CountryHostedTournament(CCompID const &compId, UChar countryId) {
    return CallMethodAndReturn<Bool, 0x1179ED0>(this, &compId, countryId);
}

Bool CompetitionHosts::IsNationalTeamHost(CCompID const &compId, UShort year, CTeamIndex const &teamID) {
    return CallMethodAndReturn<Bool, 0x117A230>(this, &compId, year, &teamID);
}

UInt CompetitionHosts::GetNumOfStadiums(CCompID const &compId, UShort year) {
    return CallMethodAndReturn<UInt, 0x117A250>(this, &compId, year);
}

CTeamIndex CompetitionHosts::GetHostStadium(CCompID const &compId, UShort year, UInt stadiumIndex) {
    return CallMethodAndReturn<CTeamIndex, 0x117A310>(this, &compId, year, stadiumIndex);
}

void CompetitionHosts::SelectHostStadiums() {
    CallMethod<0x117C560>(this);
}

void CompetitionHosts::SelectHostStadiums(CCompID const &compId, UShort year) {
    CallMethod<0x117C760>(this, &compId, year);
}

CTeamIndex CompetitionHosts::GetChampionsLeagueHost() {
    return CallMethodAndReturn<CTeamIndex, 0x117A560>(this);
}

CTeamIndex CompetitionHosts::GetUefaCupHost() {
    return CallMethodAndReturn<CTeamIndex, 0x117A570>(this);
}

CTeamIndex CompetitionHosts::GetEuroSupercupHost() {
    return CallMethodAndReturn<CTeamIndex, 0x1177730>(this);
}

CompetitionHosts *GetCompHosts() {
    return CallAndReturn<CompetitionHosts *, 0x117C830>();
}

CDBYouthcampList *GetYouthcampList() {
    return CallAndReturn<CDBYouthcampList *, 0x11C5330>();
}

void *GetApp() {
    void *xlibFactory = CallAndReturn<void *, 0x459720>();
    return CallVirtualMethodAndReturn<void *, 33>(xlibFactory);
}

void ClampTilesToImageBounds(UShort *dst, UShort *src, UInt numTiles, UShort total) {
    CallDynGlobal(GfxCoreAddress(0x3D6910), dst, src, numTiles, total);
}

void *opNew(UInt size) {
    return CallAndReturn<void *, 0x15773B5>();
}

void opDelete(void *data) {
    Call<0x1573B66>(data);
}

Bool GetFirstManagerRegion(UInt &outRegion) {
    for (UInt i = 0; i <= 5; i++) {
        if (plugin::CallAndReturn<Bool, 0xFF7F60>(i)) {
            outRegion = 249 + i;
            return true;
        }
    }
    return false;
}

CNamePools *GetNamePools() {
    return CallAndReturn<CNamePools *, 0x1499D1D>();
}

UChar GetCountryFirstLanguage(UChar countryId) {
    CDBCountry *country = GetCountry(countryId);
    return country ? country->GetFirstLanguage() : FifamLanguage::English;
}

Int GetIDForObject(UChar type, Int id) {
    return CallAndReturn<Int, 0x133E310>(type, id);
}

void *GetObjectByID(Int id) {
    return CallAndReturn<void *, 0x156EF70>(id);
}

CDBMatchesGoalsLeagueList *GetPlayerMatchesGoalsList(Int playerId) {
    return CallAndReturn<CDBMatchesGoalsLeagueList *, 0x103F7B0>(playerId);
}

CDBPlayerCareerList *GetPlayerCareerList(Int playerId) {
    return CallAndReturn<CDBPlayerCareerList *, 0x10D6020>(playerId);
}

CDBNetwork &GetNetwork() {
    return *(CDBNetwork *)0x3185EF0;
}

CCompID CDBRoot::GetFirstContinentalCompetition() {
    CCompID result;
    CallMethod<0x11F0A10>(this, &result);
    return result;
}

void CDBRoot::SetFirstContinentalCompetition(CDBCompetition *comp) {
    CallMethod<0x11F09F0>(this, comp);
}

Bool CDBRoot::LaunchesInThisSeason(UInt phase) {
    return CallMethodAndReturn<Bool, 0x11F03C0>(this, phase);
}

CDBMatchEventEntries *CDBRoot::GetEvents() {
    return *raw_ptr<CDBMatchEventEntries *>(this, 0x2128);
}

CDBTeam *CClubFans::GetTeam() {
    return *raw_ptr<CDBTeam *>(this, 0);
}

Int CClubFans::GetNumFans() {
    return *raw_ptr<Int>(this, 0x24);
}

void CClubFans::SetNumFans(Int numFans) {
    CallMethod<0x122DF00>(this, numFans);
}

Int CClubFans::AddFans(Int numFans) {
    return CallMethodAndReturn<Int, 0x122DF30>(this, numFans);
}

UInt CTeamFanshops::GetNumFanShops() {
    return CallMethodAndReturn<UInt, 0x124CF30>(this);
}

CFanShop *CTeamFanshops::GetFanShop(UShort index) {
    return CallMethodAndReturn<CFanShop *, 0x124CF50>(this, index);
}

CMatch::CMatch() {
    CallMethod<0xF82800>(this);
}

Int CMatch::GetMatchEventsStartIndex() const {
    return CallMethodAndReturn<Int, 0xF828C0>(this);
}

void CMatch::SetMatchEventsStartIndex(Int index) {
    CallMethod<0xF828B0>(this, index);
}

void CMatches::GetMatch(UInt matchday, UInt matchIndex, CMatch &match) const {
    CallMethod<0xF82C40>(this, matchday, matchIndex, &match);
}

void CMatches::SetMatch(UInt matchday, UInt matchIndex, CMatch const &match) {
    CallMethod<0xF82CD0>(this, matchday, matchIndex, &match);
}

void CDBMatchEventEntries::Clear() {
    CallMethod<0x1004E40>(this);
}

void CXgComboCompound::Clear() {
    CallVirtualMethod<81>(this);
}

Int CXgComboCompound::GetNumberOfItems() {
    return CallVirtualMethodAndReturn<Int, 69>(this);
}

void CXgComboCompound::SetCurrentIndex(Int index) {
    CallVirtualMethod<70>(this, index);
}

Int CXgComboCompound::GetCurrentIndex() {
    return CallVirtualMethodAndReturn<Int, 71>(this);
}

Int CXgComboBox::AddItem(WideChar const *text, Int64 value) {
    return CallVirtualMethodAndReturn<Int, 83>(this, text, value);
}

void CXgComboBox::SetValueAtIndex(Int index, Int64 value) {
    CallVirtualMethod<91>(this, index, value);
}

Int64 CXgComboBox::GetValueAtIndex(Int index) {
    return CallVirtualMethodAndReturn<Int64, 92>(this, index);
}

void CXgComboBox::SetCurrentValue(Int64 value) {
    CallVirtualMethod<93>(this, value);
}

Int64 CXgComboBox::GetCurrentValue(Int64 defaultValue) {
    return CallVirtualMethodAndReturn<Int64, 94>(this, defaultValue);
}

void CXgCheckBox::SetIsChecked(Bool checked) {
    CallVirtualMethod<84>(this, checked);
}

Bool CXgCheckBox::GetIsChecked() {
    return GetCheckState() != 0;
}

UChar CXgCheckBox::GetCheckState() {
    return CallVirtualMethodAndReturn<UChar, 85>(this);
}

void CXgBaseControl::SetEnabled(Bool enabled) {
    CallVirtualMethod<9>(this, enabled);
}

void CXgBaseControl::SetVisible(Bool visible) {
    CallVirtualMethod<11>(this, visible);
}

CGuiNode *CXgBaseControl::GetGuiNode() {
    return CallVirtualMethodAndReturn<CGuiNode *, 23>(this);
}

void CXgBaseControl::SetTooltip(WideChar const *text) {
    CallVirtualMethod<50>(this, text);
}

CXgTextBox *CXgFMPanel::GetTextBox(Char const *name) {
    return CallMethodAndReturn<CXgTextBox *, 0xD44240>(this, name);
}

CXgTextButton *CXgFMPanel::GetTextButton(Char const *name) {
    return CallMethodAndReturn<CXgTextButton *, 0xD44360>(this, name);
}

CXgImage *CXgFMPanel::GetImage(Char const *name) {
    return CallMethodAndReturn<CXgImage *, 0xD44380>(this, name);
}

CTrfmNode *CXgFMPanel::GetTransform(Char const *name) {
    return CallMethodAndReturn<CTrfmNode *, 0xD43060>(this, name);
}

CXgCheckBox *CXgFMPanel::GetCheckBox(Char const *name) {
    return CallMethodAndReturn<CXgCheckBox *, 0xD44260>(this, name);
}

CXgComboBox *CXgFMPanel::GetComboBox(Char const *name) {
    return CallMethodAndReturn<CXgComboBox *, 0xD442C0>(this, name);
}

Int CXgFMPanel::SetPlayerPortrait(CXgVisibleControl *control, Int playerId, Bool unk) {
    return CallMethodAndReturn<Int, 0xD4F0B0>(this, control, playerId, unk);
}

void CXgFMPanel::SetPlayerImage(CXgVisibleControl *control, Int playerId, WideChar const *filePath) {
    CallMethod<0xD4EAD0>(this, control, playerId, filePath);
}

void CXgFMPanel::SetPlayerName(CXgVisibleControl *control, Int playerId) {
    CallMethod<0xD51710>(this, control, playerId);
}

void CXgFMPanel::SetTeamBadge(CXgVisibleControl *control, CTeamIndex teamID) {
    CallMethod<0xD516C0>(this, control, teamID);
}

void CXgFMPanel::SetTeamName(CXgVisibleControl *control, CTeamIndex const &teamID) {
    CallMethod<0xD51670>(this, control, &teamID);
}

void CXgFMPanel::SetCountryFlag(CXgVisibleControl *control, UInt countryId) {
    CallMethod<0xD4EDD0>(this, control, countryId);
}

void CXgFMPanel::SetCountryFlag(CXgVisibleControl *control, UInt countryId, UInt size) {
    CallMethod<0xD4EBF0>(this, control, countryId, size);
}

void CXgFMPanel::SetCountryName(CXgVisibleControl *control, UInt countryId) {
    CallMethod<0xD4EB40>(this, control, countryId);
}

void CXgFMPanel::SetCompetitionBadge(CXgVisibleControl *control, CCompID compID, UInt size) {
    CallMethod<0xD4EEB0>(this, control, compID, size);
}

void CXgFMPanel::SetCompetitionName(CXgVisibleControl *control, CCompID compID) {
    CallMethod<0xD52560>(this, control, compID);
}

VisibleControlAppearance *CXgVisibleControl::GetAppearance() {
    return CallVirtualMethodAndReturn<VisibleControlAppearance *, 27>(this);
}

Rect *CXgVisibleControl::GetRect() {
    return CallVirtualMethodAndReturn<Rect *, 30>(this);
}

void CXgTextBox::SetColor(UInt colorType, UInt colorValue) {
    CallVirtualMethod<74>(this, colorType, colorValue);
}

UInt CXgTextBox::GetColor(UInt colorType) {
    return CallVirtualMethodAndReturn<UInt, 75>(this, colorType);
}

void CXgTextBox::SetFont(Char const *fontName) {
    CallVirtualMethod<76>(this, fontName);
}

Char const *CXgTextBox::GetFont() {
    return CallVirtualMethodAndReturn<Char const *, 77>(this);
}

void CXgTextBox::SetText(WideChar const *text) {
    CallVirtualMethod<78>(this, text);
}

WideChar const *CXgTextBox::GetText() {
    return CallVirtualMethodAndReturn<WideChar const *, 79>(this);
}

PixelFormat::PixelFormat(UInt depth, UInt rMask, UInt gMask, UInt bMask, UInt aMask) {
    CallMethodDynGlobal(GfxCoreAddress(0x3D91D0), this, depth, rMask, gMask, bMask, aMask);
}

void PixelFormat::Convert(UChar* dst, UInt dstSize, PixelFormat *srcFormat, UChar *src, UInt srcSize, UInt mask) {
    CallMethodDynGlobal(GfxCoreAddress(0x3D93B0), this, dst, dstSize, srcFormat, src, srcSize, mask);
}

UShort CNamePools::AddName(UChar languageId, UInt nameType, WideChar const *name) {
    return CallMethodAndReturn<UShort, 0x1499DDA>(this, languageId, nameType, name);
}

WideChar const *CNamePools::GetNameByIndex(UChar nameType, UInt languageId, UShort index) {
    return CallMethodAndReturn<WideChar const *, 0x1499EC3>(this, languageId, nameType, index);
}

WideChar const *CNamePools::FormatName(NameDesc const &nameDesc, Bool bShortForm, WideChar *buf, UInt bufSize) {
    return CallMethodAndReturn<WideChar const *, 0x149A35E>(this, &nameDesc, bShortForm, buf, bufSize);
}

String CNamePools::FormatName(NameDesc const &nameDesc, Bool bShortForm) {
    WideChar buf[1024];
    FormatName(nameDesc, bShortForm, buf, std::size(buf));
    return buf;
}

WideChar const *NameDesc::ToName(WideChar *buf) {
    return CallAndReturn<WideChar const *, 0x149A616>(this, buf);
}

WideChar const *NameDesc::ToPlayerStringID(CJDate birthdate, Int empicsId, WideChar *buf) {
    return CallAndReturn<WideChar const *, 0x149A918>(this, birthdate, empicsId, buf);
}

String NameDesc::ToPlayerStringID(CJDate birthdate, Int empicsId) {
    WideChar buf[256];
    ToPlayerStringID(birthdate, empicsId, buf);
    return buf;
}

Bool NetComStorageIterator::operator==(const NetComStorageIterator &other) const {
    return blockId == other.blockId && index == other.index;
}

Bool NetComStorageIterator::operator!=(const NetComStorageIterator &other) const {
    return !(*this == other);
}

UShort CDBMatchesGoalsLeagueList::GetNumEntries() {
    return CallMethodAndReturn<UShort, 0x103F340>(this);
}

UChar CDBMatchesGoalsLeagueList::GetCountryId(Int entryId) {
    return CallMethodAndReturn<UChar, 0x103ED00>(this, entryId);
}

UShort CDBMatchesGoalsLeagueList::GetYear(Int entryId) {
    return CallMethodAndReturn<UShort, 0x103EDE0>(this, entryId);
}

UShort CDBMatchesGoalsLeagueList::GetLeagueLevel(Int entryId, Bool firstTeam) {
    return CallMethodAndReturn<UShort, 0x103EDB0>(this, entryId, firstTeam);
}

UChar CDBMatchesGoalsLeagueList::GetNumGoals(Int entryId, Int compType) {
    return CallMethodAndReturn<UChar, 0x103F100>(this, entryId, compType);
}

UChar CDBMatchesGoalsLeagueList::GetNumAssists(Int entryId, Int compType) {
    return CallMethodAndReturn<UChar, 0x103F130>(this, entryId, compType);
}

UChar CDBMatchesGoalsLeagueList::GetNumMatches(Int entryId, Int compType) {
    return CallMethodAndReturn<UChar, 0x103EE10>(this, entryId, compType);
}

UInt CDBPlayerCareerList::GetNumEntries() {
    return CallMethodAndReturn<UInt, 0x10D56B0>(this);
}

CDBPlayerCareerEntry *CDBPlayerCareerList::GetEntry(UInt index) {
    return CallMethodAndReturn<CDBPlayerCareerEntry *, 0x10D59F0>(this, index);
}

CTeamIndex CDBPlayerCareerList::GetTeamID(UInt entryId) {
    CTeamIndex result;
    CallMethod<0x4894E0>(this, &result, entryId);
    return result;
}

CJDate CDBPlayerCareerList::GetStartDate(UInt entryId) {
    CJDate result;
    CallMethod<0x489500>(this, &result, entryId);
    return result;
}

CJDate CDBPlayerCareerList::GetEndDate(UInt entryId) {
    CJDate result;
    CallMethod<0x489520>(this, &result, entryId);
    return result;
}

CMatchStatistics::CMatchStatistics() {
    CallMethod<0x4A33A0>(this);
}

CNetworkEvent *CDBNetwork::AddEvent(UShort eventId, Short shortId, Int intId, void *unk) {
    return CallMethodAndReturn<CNetworkEvent *, 0x11206A0>(this, eventId, shortId, intId, unk);
}

UShort CDBPlayerCareerEntry::GetMatches(Bool bFirstTeam) const {
    return CallMethodAndReturn<UShort, 0x10D5260>(this, bFirstTeam);
}

UShort CDBPlayerCareerEntry::GetGoals(Bool bFirstTeam) const {
    return CallMethodAndReturn<UShort, 0x10D5280>(this, bFirstTeam);
}

CTeamIndex CDBPlayerCareerEntry::GetTeamID() const {
    CTeamIndex result;
    CallMethod<0x10D5610>(this, &result);
    return result;
}

CJDate CDBPlayerCareerEntry::GetStartDate() const {
    CJDate result;
    CallMethod<0x10D5230>(this, &result);
    return result;
}

CJDate CDBPlayerCareerEntry::GetEndDate() const {
    CJDate result;
    CallMethod<0x10D5250>(this, &result);
    return result;
}

Bool CDBPlayerCareerEntry::OnLoan() const {
    return CallMethodAndReturn<Bool, 0x10D5430>(this);
}
