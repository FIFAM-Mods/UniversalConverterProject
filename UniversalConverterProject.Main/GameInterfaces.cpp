#include "GameInterfaces.h"
#include "GfxCoreHook.h"
#include "Utils.h"
#include "shared.h"

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

unsigned char CDBCompetition::GetCompetitionType() {
    return plugin::CallMethodAndReturn<unsigned char, 0xF81C50>(this);
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
        if (numTeams == 0 || (numTeams - startPos) > numRegisteredTeams)
            numTeams = numRegisteredTeams - startPos;
        if (numTeams > 1) {
            if (numTeams == 2) {
                if (GetRandomInt(2) == 1)
                    SwapTeams(startPos, startPos + 1);
            }
            else {
                for (UInt i = 0; i < numTeams / 2 + 1; i++) {
                    Int rnd1 = GetRandomInt(numTeams);
                    Int rnd2 = GetRandomInt(numTeams);
                    SwapTeams(startPos + rnd1, startPos + rnd2);
                }
            }
        }
    }
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

void CDBLeague::SetStartDate(CJDate date) {
    plugin::CallMethod<0x1054390>(this, date);
}

int CDBLeague::GetEqualPointsSorting() {
    return plugin::CallMethodAndReturn<int, 0x10502F0>(this);
}

void CDBLeague::SortTeams(TeamLeaguePositionData *infos, int sortingFlags, int goalsMinMinute, int goalsMaxMinute, int minMatchday, int maxMatchday) {
    plugin::CallVirtualMethod<40>(this, infos, sortingFlags, goalsMinMinute, goalsMaxMinute, minMatchday, maxMatchday);
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

CDBRound *GetRoundByRoundType(unsigned char region, unsigned char type, unsigned char roundType) {
    return plugin::CallAndReturn<CDBRound *, 0xF8B190>(region, type, roundType);
}

CDBTeam *GetTeam(CTeamIndex teamId) {
    return plugin::CallAndReturn<CDBTeam *, 0xEC8F70>(teamId);
}

CDBTeam *GetTeamByUniqueID(unsigned int uniqueID) {
    return plugin::CallAndReturn<CDBTeam *, 0xF0CCA0>(uniqueID);
}

CCountryStore *GetCountryStore() {
    return plugin::CallAndReturn<CCountryStore *, 0x415100>();
}

wchar_t const *GetTranslation(const char *key) {
    return plugin::CallMethodAndReturn<wchar_t const *, 0x14A9B78>(0x31E3FA8, key);
}

unsigned short GetCurrentYear() {
    auto game = CDBGame::GetInstance();
    if (game)
        return game->GetCurrentYear();
    return 0;
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
    plugin::CallVirtualMethod<1>(*(void **)0x30ABBC4, varName, value);
}

void SetVarString(char const *varName, char const *value) {
    plugin::CallVirtualMethod<0>(*(void **)0x30ABBC4, varName, value);
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
        else {
            fpstr = filepathWithoutExtension + L".kit";
            if (CallVirtualMethodAndReturn<Bool, 9>(fmFs, fpstr.c_str())) {
                resultPath = fpstr;
                return true;
            }
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
    return false;
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

unsigned char CDBTeam::GetInternationalPrestige() {
    return plugin::CallMethodAndReturn<unsigned char, 0xED14B0>(this);
}

wchar_t *CDBTeam::GetName(bool first) {
    return plugin::CallMethodAndReturn<wchar_t *, 0xEEB670>(this, first);
}

CTeamSponsor &CDBTeam::GetSponsor() {
    return plugin::CallMethodAndReturn<CTeamSponsor &, 0xED5170>(this);
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

CTeamIndex CTeamIndex::make(unsigned char CountryId, unsigned char Type, unsigned short Index) {
    CTeamIndex result;
    result.countryId = CountryId;
    result.index = Index;
    result.type = Type;
    return result;
}

UChar CDBCountry::GetLeagueAverageLevel() {
    return plugin::CallMethodAndReturn<UChar, 0xFD6FA0>(this);
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