#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"

class CJDate {
    UInt value;
public:
    CJDate GetTranslated(int numYears);
    unsigned short GetYear();
    unsigned char GetMonth();
    unsigned char GetDays();
    void GetYMD(unsigned short *outY, unsigned char *outM, unsigned char *outD);
    void GetYMD(unsigned int *outY, unsigned int *outM, unsigned int *outD);
    void Set(unsigned short year, unsigned char month, unsigned char day);
    UInt Value();
};

using EAGMoney = UInt64;

enum eMetricCode {
    METRIC_NONE = 0,
    METRIC_UNITED_STATES = 1,
    METRIC_FRANCE = 33,
    METRIC_SPAIN = 34,
    METRIC_ITALY = 39,
    METRIC_SWITHERLAND = 41,
    METRIC_AUSTRIA = 43,
    METRIC_UNITED_KINGDOM = 44,
    METRIC_POLAND = 48,
    METRIC_GERMANY = 49,
    METRIC_JAPAN = 81,
    METRIC_SOUTH_KOREA = 82
};


template<typename T>
class FmVec {
public:
    T *data;
private:
    UInt internalSize;
public:
    UInt capacity;
    T *begin;
    T *end;

    UInt size() {
        return (UInt)(end - begin);
    }

    Bool empty() {
        return !begin || !end || end <= begin;
    }

    T *operator[](UInt index) {
        return &begin[index];
    }
};

class CDBGame {
public:
    static CDBGame *GetInstance();
    CJDate GetStartDate();
    CJDate GetCurrentDate();
    CJDate GetCurrentSeasonStartDate();
    unsigned short GetCurrentYear();
    bool GetIsWorldCupMode();
    bool TestFlag(unsigned int flag);
    void SetCurrentDate(CJDate date);
};

struct CTeamIndex {
    unsigned short index;
    unsigned char countryId;
    unsigned char type;

    unsigned int ToInt() const;
    CTeamIndex firstTeam() const;
    static CTeamIndex make(unsigned char CountryId, unsigned char Type, unsigned short Index);
};

class CDBTeam;
class CDBEmployee;

class CDBPlayer {
public:
    // 0 to 20
    UChar GetPlayingForm();
    UInt GetAge();
    Char GetTalent(CDBEmployee *employee = nullptr);
    Char GetMainPosition();
    UChar GetLevel(Char position, Bool special = false);
};

class CDBCountry;

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
    COMP_U17_WC_Q = 51, // U17 WC Quali
    COMP_U17_WORLD_CUP = 52, // U17 WC
    COMP_U21_EC_Q = 53, // U21 EC Quali
    COMP_U21_EC = 54, // U21 EC
    COMP_U19_EC_Q = 55, // U19 EC Quali
    COMP_U19_EC = 56, // U19 EC
    COMP_U17_EC_Q = 57, // U17 EC Quali
    COMP_U17_EC = 58, // U17 EC
    COMP_OLYMPIC_Q = 59, // Olympic Games Quali
    COMP_OLYMPIC = 60, // Olympic Games
};

struct CCompID {
    unsigned short index = 0;
    unsigned char type = 0;
    unsigned char countryId = 0;

    void SetFromInt(unsigned int value);
    unsigned int ToInt() const;
    std::wstring ToStr() const;
    static CCompID Make(unsigned char _country, unsigned char _type, unsigned short _index);
};

struct CScriptCommand {
    unsigned int m_nCommandId;
    CCompID m_competitionId;
    unsigned int m_params;
};

class CDBCompetition {
public:
    unsigned char GetCompetitionType();
    unsigned int GetDbType();
    wchar_t const *GetName();
    CCompID GetCompID();
    unsigned int GetNumOfRegisteredTeams();
    unsigned int GetNumOfTeams();
    unsigned char GetFlags();
    void DumpToFile();
    int GetNumOfScriptCommands();
    CScriptCommand *GetScriptCommand(int index);
    CTeamIndex &GetTeamID(int position);
    CTeamIndex &GetChampion(bool checkIntl = false);
    CTeamIndex &GetRunnerUp();
    void SetChampion(CTeamIndex const &teamId);
    void SetRunnerUp(CTeamIndex const &teamId);
    unsigned char GetNumTeamsFromCountry(int countryId);
    void AddTeamsFromCountry(unsigned char countryId, unsigned int numTeams);
    bool AddTeam(CTeamIndex const &teamIndex, int position = -1);
    bool IsTeamPresent(CTeamIndex const &teamIndex);
    unsigned int GetNumMatchdays();
    bool SwapTeams(int oldIndex, int newIndex);
    void SortTeamIDs(Function<Bool(CTeamIndex const &, CTeamIndex const &)> const &sorter);
    void SortTeams(Function<Bool(CDBTeam *, CDBTeam *)> const &sorter);
    void RandomlySortTeams(UInt startPos = 0, UInt numTeams = 0);
    void RandomizeHomeAway();
    void RandomizePairs();
    void RandomizePairs4x4();
    CCompID GetPredecessor(Int index);
    CCompID GetSuccessor(Int index);
    unsigned int GetLevel();
};

enum CompDbType {
    DB_ROOT = 0,
    DB_LEAGUE = 1,
    DB_SPARE = 2,
    DB_CUP = 3,
    DB_ROUND = 4,
    DB_POOL = 5,
    DB_FRIENDLY = 6,
    DB_CUSTOM = 7
};

struct TeamLeaguePositionData {
    CTeamIndex m_teamID;
    unsigned int m_nGames;
    unsigned int m_nPoints;
    unsigned int m_nGoalsScored;
    unsigned int m_nGoalsAgainst;
    unsigned short m_nWins;
    unsigned short m_nDraws;
    unsigned short m_nLoses;
    unsigned short m_nAwayGoals;
    unsigned short m_nPosition;
    char _pad1E[2];

    TeamLeaguePositionData();
};

class CDBRoot : public CDBCompetition {};
class CDBPool : public CDBCompetition {};
class CDBRound : public CDBCompetition {};
class CDBCup : public CDBCompetition {};
class CDBLeagueBase : public CDBCompetition {
public:
    CTeamIndex &GetTeamAtPosition(int position);
};

class CDBLeague : public CDBLeagueBase {
public:
    void SetStartDate(CJDate date);
    int GetEqualPointsSorting();
    void SortTeams(TeamLeaguePositionData *infos, int sortingFlags, int goalsMinMinute, int goalsMaxMinute, int minMatchday, int maxMatchday);
};

struct SponsorPlacement {
    UInt index;
    UChar countryId;
    Char _pad5[3];
};

class CDBSponsorContractBase {
    void *vtable;
public:
    Int field_4;
    EAGMoney m_money;
    Int field_10;
    Int m_nStatus;
    CJDate m_startDate;
    CJDate m_endDate;
    UInt m_nUniqueId;
    CTeamIndex m_teamID;
    SponsorPlacement m_placement;
    Char field_30;
private:
    Char _pad31[3];
public:

    Bool IsActive();
    SponsorPlacement &GetPlacement();
};

static_assert(sizeof(CDBSponsorContractBase) == 0x34, "Failed");

class CDBSponsorContractAdBoards : public CDBSponsorContractBase {
    Int field_34;
    CJDate field_38;
    Char field_3C;
    Char field_3D[3];
};

static_assert(sizeof(CDBSponsorContractAdBoards) == 0x40, "Failed");

class CTeamSponsor {
public:
    FmVec<CDBSponsorContractAdBoards> &GetAdBoardSponsors();
    CDBSponsorContractBase &GetMainSponsor();
};

class CDBTeam {
public:
    CTeamIndex GetTeamID();
    unsigned int GetTeamUniqueID();
    unsigned char GetInternationalPrestige();
    wchar_t *GetName(bool first = true);
    CTeamSponsor &GetSponsor();
};

struct CAssessmentInfo {
    char m_nCountryIndex;
    char m_nIndex1;
    char field_2;
    unsigned char m_nPositionIndex;
    float m_fYear_6;
    float m_fYear_5;
    float m_fYear_4;
    float m_fYear_3;
    float m_fYear_2;
    float m_fLastYear;
    float m_fCurrent;
    int m_date;
};

class CAssessmentTable {
public:
    int m_nNumEntries;
    CAssessmentInfo m_aEntries[208];
    CTeamIndex m_fairnessTeamIDs[208];
    char field_2084[1664];
    CJDate field_2704;

    unsigned char GetCountryIdAtPosition(int position);
    unsigned char GetCountryIdAtPositionLastYear(int position);
    int GetCountryPosition(int countryId);
    CAssessmentInfo *GetInfoForCountry(unsigned char countryId);
    int GetCountryPositionLastYear(unsigned char countryId);
};

CAssessmentTable *GetAssesmentTable();

enum eContinentId {
    CONTINENT_EUROPE = 0,
    CONTINENT_SOUTH_AMERICA = 1,
    CONTINENT_NORTH_AMERICA = 2,
    CONTINENT_AFRICA = 3,
    CONTINENT_ASIA = 4,
    CONTINENT_OCEANIA = 5,
    CONTINENT_NONE = 6
};


class CDBCountry {
    UChar data[0x10C8];
public:
    UChar GetLeagueAverageLevel();
    UInt GetContinent();
    const WideChar *GetName();
    const WideChar *GetAbbr();
    const WideChar *GetContinentName();
};

class CCountryStore {
    void *vtable;
public:
    UInt field_4;
    CDBCountry m_aCountries[208];
};

CDBCompetition *GetCompetition(unsigned char region, unsigned char type, unsigned short index);
CDBCompetition *GetCompetition(CCompID const &id);
CDBCompetition *GetCompetition(unsigned int *id);
CDBCompetition *GetCompetition(unsigned int id);
CDBLeague *GetLeague(unsigned char region, unsigned char type, unsigned short index);
CDBLeague *GetLeague(CCompID const &id);
CDBLeague *GetLeague(unsigned int *id);
CDBLeague *GetLeague(unsigned int id);
CDBRound *GetRound(unsigned char region, unsigned char type, unsigned short index);
CDBRound *GetRoundByRoundType(unsigned char region, unsigned char type, unsigned char roundType);

CDBTeam *GetTeam(CTeamIndex teamId);
CDBTeam *GetTeamByUniqueID(unsigned int uniqueID);

CCountryStore *GetCountryStore();

wchar_t const *GetTranslation(const char *key);
unsigned short GetCurrentYear();

int gfxGetVarInt(char const *varName, int defaultValue);
char const *gfxGetVarString(char const *varName);
void SetVarInt(char const *varName, int value);
void SetVarString(char const *varName, char const *value);

int GetRandomInt(int endValue);

void SetCompetitionWinnerAndRunnerUp(unsigned int region, unsigned int type, CTeamIndex const &winner, CTeamIndex const &runnerUp);

Bool FmFileExists(Path const &filepath);
Bool FmFileImageExists(String const &filepathWithoutExtension, String &resultPath);
UInt FmFileGetSize(Path const &filepath);
Bool FmFileRead(Path const &filepath, void *outData, UInt size);
