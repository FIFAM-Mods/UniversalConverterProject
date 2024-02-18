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
    void Set(UInt _value);
    UInt Value();
    UInt GetDayOfWeek();
    static CJDate DateFromDayOfWeek(UChar dayOfWeek, UChar month, UShort year);
    void AddRandomDaysCount(UChar direction);
    CJDate AddYears(Int years);
};

using EAGMoney = Int64;

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

UInt GetCurrentMetric();

struct RGBAReal {
    Float r, g, b, a;

    void Set(Float _r, Float _g, Float _b, Float _a) {
        r = _r; g = _g; b = _b; a = _a;
    }

    void SetRGB(UChar _r, UChar _g, UChar _b) {
        r = Float(_r) / 255.0f;
        g = Float(_g) / 255.0f;
        b = Float(_b) / 255.0f;
    }

    void SetInt(UInt value) {
        r = Float((value >> 16) & 0xFF) / 255.0f;
        g = Float((value >> 8) & 0xFF) / 255.0f;
        b = Float(value & 0xFF) / 255.0f;
    }
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

#pragma pack(push, 4)
template<typename TKey, typename TValue>
class FmMap {
public:
    class Proxy {
        FmMap *container;
    };

    class Node {
        Node *left;
        Node *parent;
        Node *right;
        struct {
            TKey key;
            TValue value;
        };
        UChar color;
        UChar isNil;
    };

    class Iterator {
        Proxy *proxy;
        Node *node;
    };

    Proxy *proxy;
private:
    UInt _unk[5];
public:
    Node *head;
    UInt size;
};
#pragma pack(pop)

class CDBGameOptions {
public:
    bool CheckFlag(unsigned int flag);
};

class CDBGame {
public:
    static CDBGame *GetInstance();
    CJDate GetStartDate();
    CJDate GetCurrentDate();
    CJDate GetCurrentSeasonStartDate();
    CJDate GetCurrentSeasonEndDate();
    unsigned short GetCurrentYear();
    bool GetIsWorldCupMode();
    bool TestFlag(unsigned int flag);
    void SetCurrentDate(CJDate date);
    CDBGameOptions &GetOptions();
    bool IsCountryPlayable(UChar countryId);
};

struct CTeamIndex {
    unsigned short index;
    unsigned char countryId;
    unsigned char type;

    unsigned int ToInt() const;
    CTeamIndex firstTeam() const;
    bool isNull() const;
    static CTeamIndex make(unsigned char CountryId, unsigned char Type, unsigned short Index);
    static CTeamIndex make(unsigned int value);
    static CTeamIndex null();
};

bool operator==(CTeamIndex const &a, CTeamIndex const &b);

class CDBCountry;
class CDBTeam;
class CDBEmployee;
class CDBStaff;
class CWorker;

class CPlayerStats {
public:
    UChar GetNumInternationalCaps();
};

enum ePlayerPositionRole {
    PLAYER_POSITION_ROLE_GK,
    PLAYER_POSITION_ROLE_DEF,
    PLAYER_POSITION_ROLE_MID,
    PLAYER_POSITION_ROLE_ATT
};

class CDBPlayer {
public:
    // 0 to 20
    UChar GetPlayingForm();
    UInt GetAge();
    Char GetTalent(CDBEmployee *employee = nullptr);
    Char GetMainPosition();
    UChar GetLevel(Char position, Bool special = false);
    CTeamIndex GetCurrentTeam();
    EAGMoney GetMarketValue(CDBEmployee *employee = nullptr);
    EAGMoney GetDemandValue();
    EAGMoney GetMinRelFee();
    UChar GetNationality(UChar number = 0);
    CTeamIndex GetNationalTeam();
    String GetName(bool shortForm = false);
    void SetDemandValue(EAGMoney const &money);
    void SetMinRelFee(EAGMoney const &money);
    void SetNationality(UChar index, UChar nation);
    void SetIsBasque(bool basque);
    UChar GetNumKnownLanguages();
    UChar GetLanguage(UChar number = 0);
    void SetLanguage(UChar index, UChar language);
    UInt GetID();
    Bool IsInNationalTeam();
    Bool IsRetiredFromNationalTeam();
    Bool IsInU21NationalTeam();
    Bool IsEndOfCareer();
    UChar GetNumPlannedYearsForCareer();
    CPlayerStats *GetStats();
    UChar GetPotential();
    UChar GetPositionRole();
};

class CDBStaff {
public:
    CWorker *GetWorker();
    void SetBirthDate(CJDate const &date);
    CJDate const &GetBirthdate();
    void SetNationality(UChar index, UChar countryId);
    UChar GetNationality(UChar index);
    WideChar const *GetName(WideChar const *nameBuf = nullptr);
    UInt GetAge();
    void SetTalent(UInt talent);
    UInt GetTalent();
};

class CDBEmployeeBase {
public:
    void SetBirthDate(CJDate const &date);
    WideChar const *GetName(WideChar const *nameBuf = nullptr);
};

class CDBEmployee {
public:
    CDBEmployeeBase *GetBase();
    void SetBirthDate(CJDate const &date);
    void SetFirstNationality(UChar countryId);
    void SetSecondNationality(UChar countryId);
    void SetLanguage(UChar languageId);
    void SetAdditionalLanguageLevel(UChar index, UChar languageId, UChar level);
    CTeamIndex GetTeamID();
    void GetTeamID(CTeamIndex &out);
    WideChar const *GetName(WideChar const *nameBuf = nullptr);
};

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
    unsigned int GetCompetitionType();
    unsigned int GetDbType();
    wchar_t const *GetName();
    CCompID GetCompID();
    unsigned int GetNumOfRegisteredTeams();
    void SetNumOfRegisteredTeams(unsigned int number);
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
    void SortTeamIDs(unsigned int startPos, unsigned int numTeams, Function<Bool(CTeamIndex const &, CTeamIndex const &)> const &sorter);
    void SortTeams(unsigned int startPos, unsigned int numTeams, Function<Bool(CDBTeam *, CDBTeam *)> const &sorter);
    void SortTeamIDs(Function<Bool(CTeamIndex const &, CTeamIndex const &)> const &sorter);
    void SortTeams(Function<Bool(CDBTeam *, CDBTeam *)> const &sorter);
    void RandomlySortTeams(UInt startPos = 0, UInt numTeams = 0);
    void RandomizeHomeAway();
    void RandomizePairs();
    void RandomizePairs4x4();
    CCompID GetPredecessor(Int index);
    CCompID GetSuccessor(Int index);
    unsigned int GetLevel();
    unsigned int GetRoundType();
    int GetTeamIndex(CTeamIndex const& teamId);
    bool Finish();
    void Launch();
    Bool IsContinental();
    Bool IsLaunched();
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

enum eRoundType {
    ROUND_QUALI = 1,
    ROUND_QUALI2 = 2,
    ROUND_QUALI3 = 3,
    ROUND_PREROUND1 = 4,
    ROUND_1 = 5,
    ROUND_2 = 6,
    ROUND_3 = 7,
    ROUND_4 = 8,
    ROUND_5 = 9,
    ROUND_GROUP1 = 10,
    ROUND_GROUP2 = 11,
    ROUND_LAST_16 = 12,
    ROUND_QUARTERFINAL = 13,
    ROUND_SEMIFINAL = 14,
    ROUND_FINAL = 15,
    ROUND_FINAL_3RD_PLACE = 16,
    ROUND_NONE = 255
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

struct RoundPair {
    CTeamIndex m_n1stTeam;
    CTeamIndex m_n2ndTeam;
    char result1[2];
    char result2[2];
    char field_C[2];
    char field_E[2];
    unsigned int m_nFlags;
    int m_anMatchEventsStartIndex[3];

    RoundPair();
    bool AreTeamsValid();
    void *GetResultString(void *str, UChar flags, const wchar_t *team1name, const wchar_t *team2name);
};

struct MatchGoalInfo {
    int scorer;
    unsigned char teamSide;
    unsigned char minute;
    char field_6;
    bool isOwnGoal;
};

class CDBRoot : public CDBCompetition {};
class CDBPool : public CDBCompetition {};

class CDBRound : public CDBCompetition {
public:
    unsigned int GetNumOfPairs();
    void GetRoundPair(unsigned int pairIndex, RoundPair &out);
};

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
    unsigned int GetCurrentMatchday();
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

class CDBTeamKit {
public:
    UInt GetPartType(UChar kitType, UChar shirtPart);
    UChar GetPartColor(UChar kitType, UChar shirtPart, UChar colorIndex);
    Bool HasUserKitType(UInt kitType);
    Bool HasUserKit();
};

class CStadiumDevelopment {
public:
    CDBTeam *GetTeam();
    UInt GetNumSeats();
};

class CDBTeam {
public:
    CTeamIndex GetTeamID();
    unsigned int GetTeamUniqueID();
    unsigned char GetNationalPrestige();
    unsigned char GetInternationalPrestige();
    wchar_t *GetName(bool first = true);
    wchar_t *GetShortName(CTeamIndex const &teamID, bool includeYouth = false);
    CTeamSponsor &GetSponsor();
    bool IsRivalWith(CTeamIndex const &teamIndex);
    CDBTeamKit *GetKit();
    bool IsManagedByAI(bool flag = true);
    UChar GetColorId(UInt colorType);
    UInt GetColorRGBA(UInt colorType);
    UInt GetNumPlayers();
    UInt GetPlayer(UChar index);
    UInt GetFifaID();
    bool CanBuyOnlyBasquePlayers();
    bool YouthPlayersAreBasques();
    UChar GetYouthPlayersCountry();
    UChar GetCountryId();
    CDBCountry *GetCountry();
    Bool IsPlayerPresent(UInt playerId);
    CStadiumDevelopment *GetStadiumDevelopment();
    Int GetManagerId();
    void SetFlag(UInt flag, Bool enable);
};

struct CAssessmentInfo {
    unsigned char m_nCountryIndex;
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

    void AddPoints(float points);
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
    float GetTotalPointsForCountry(unsigned char countryId);
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
    UChar GetCountryId();
    UInt GetContinent();
    const WideChar *GetName();
    const WideChar *GetAbbr();
    const WideChar *GetContinentName();
    const Int GetLastTeamIndex();
    const UInt GetNumRegenPlayers();
    const UChar GetLanguage(UChar number);
    const Bool IsPlayerInNationalTeam(UInt playerId);
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
CDBPool *GetPool(unsigned char region, unsigned char type, unsigned short index);

CDBCountry *GetCountry(UChar countryId);
CDBTeam *GetTeam(CTeamIndex teamId);
CDBTeam *GetTeamByUniqueID(unsigned int uniqueID);
CDBPlayer *GetPlayer(Int playerId);
CDBEmployee *GetEmployee(Int employeeId);

CCountryStore *GetCountryStore();

wchar_t const *GetTranslation(const char *key);
bool IsTranslationPresent(const char *key);
wchar_t const *GetTranslationIfPresent(const char *key);
unsigned short GetCurrentYear();
unsigned char GetCurrentMonth();
unsigned short GetStartingYear();
unsigned short GetCurrentSeasonStartYear();
unsigned short GetCurrentSeasonEndYear();
wchar_t const *GetCountryLogoPath(wchar_t *out, unsigned char countryId, int size);

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

struct ShapeWriterImageRegion {
    Char const *name;
    UInt width;
    UInt height;
    UInt offsetX;
    UInt offsetY;
    Int field_14;
};

struct FshWriterWriteInfo {
    UInt format;
    Char const *name;
    UInt width;
    UInt height;
    ShapeWriterImageRegion *regions;
    UInt numRegions;

    enum Format {
        Format5551 = 0,
        Format4444 = 1,
        Format565 = 2,
        Format8888 = 3,
    };
};

void *CreateTextBox(void *screen, char const *name);
void SetTextBoxColorRGBA(void *tb, UChar r, UChar g, UChar b, UChar a);
void SetTextBoxColorRGB(void *tb, UChar r, UChar g, UChar b);
void SetTextBoxColorRGBA(void *tb, UInt clr);
void SetTextBoxColorRGB(void *tb, UInt clr);
void SetImageColorRGBA(void *img, UChar r, UChar g, UChar b, UChar a);
void SetImageColorRGB(void *img, UChar r, UChar g, UChar b);
void SetImageColorRGBA(void *img, UInt clr);
void SetImageColorRGB(void *img, UInt clr);
void SetTextBoxColors(void *tb, UInt clr);
void SetVisible(void* widget, bool visible);
void SetText(void* widget, WideChar const* text);
unsigned char SetImageFilename(void* widget, std::wstring const& path);
unsigned char SetImageFilename(void *widget, wchar_t const *filename, int u1, int u2);

class CRandom {
public:
    static Int GetRandomInt(Int maxExclusive);
};

class CParameterFiles {
public:
    static CParameterFiles *Instance();
    UChar GetProbabilityOfForeignPlayersInYouthTeam();
    UChar GetProbabilityOfBasquePlayersInYouthTeam();
};

String TeamName(CDBTeam *team);
String TeamNameWithCountry(CDBTeam *team);
String TeamTag(CDBTeam *team);
String TeamTagWithCountry(CDBTeam *team);
String TeamName(CTeamIndex const &teamId);
String TeamNameWithCountry(CTeamIndex const &teamId);
String TeamTag(CTeamIndex const& teamId);
String TeamTagWithCountry(CTeamIndex const& teamId);
String CompetitionTag(CDBCompetition* comp);
String CompetitionTag(CCompID const& compId);
String CountryName(UChar countryId);
String CountryTag(UChar countryId);

Bool GetHour();
Bool GetIsCloudy();
Bool GetIsRainy();
Bool GetIsSnowy();
UInt GetLighting();

class CDBOneMatch {
public:
    CTeamIndex GetHostTeamID();
    CDBTeam *GetHostTeam();
    CCompID GetCompID();
    UInt GetCompIDInt();
    CTeamIndex GetTeamID(Bool home);
    CDBTeam *GetTeam(Bool home);
    CTeamIndex GetHomeTeamID();
    CDBTeam *GetHomeTeam();
    CTeamIndex GetAwayTeamID();
    CDBTeam *GetAwayTeam();
};

CDBOneMatch *GetCurrentMatch();

UInt GetGuiColor(UInt colorId);

Bool IsEuropeanCountry(Int countryId);
Bool IsNonEuropeanCountry(Int countryId);
Bool IsEuropeanUnion(Int countryId);
Bool IsAfricanCaribbeanAndPacificGroupOfStates(Int countryId);
Bool IsPlayerForeignerForCompetition(CDBPlayer *player, CCompID const &compID);

void *FmNew(UInt size);
void FmDelete(void *data);
bool BinaryReaderIsVersionGreaterOrEqual(void *reader, UInt year, UInt build);
void BinaryReaderReadString(void *reader, WideChar *out, UInt maxLen);
void BinaryReaderReadUInt32(void *reader, UInt *out);
void SaveGameReadString(void *save, WideChar *out, UInt maxLen);
void SaveGameWriteString(void *save, WideChar const *str);
void SaveGameReadInt8(void *save, UChar &out);
void SaveGameReadInt32(void *save, UInt &out);
UInt SaveGameReadInt32(void *save);
void SaveGameWriteInt8(void *save, UChar value);
void SaveGameWriteInt32(void *save, UInt value);
UInt SaveGameLoadGetVersion(void *save);
CDBPlayer *FindPlayerByStringID(WideChar const *stringID);

enum eNetComStorage {
    STORAGE_PLAYERS = 1,
    STORAGE_CAREER_LISTS = 2,
    STORAGE_STAT_LISTS = 3,
    STORAGE_TEAMS = 4,
    STORAGE_EMPLOYEES = 6,
    STORAGE_STAFF = 7,
    STORAGE_CREDITS_LISTS = 8,
    STORAGE_MAILS = 11,
    STORAGE_STADIUMS = 12,
    STORAGE_CONTRACT_OFFERS = 14,
    STORAGE_TRANSFERS = 15,
    STORAGE_EMPLOYEE_JOBS = 19,
    STORAGE_COMPLETED_TRANSFERS = 20,
    STORAGE_REFEREES = 21,
    STORAGE_DRAFT_LISTS = 22
};

struct NetComStorageIterator {
    void *pStorage;
    UInt blockId;
    Int index;
    UInt id;
    void *object;
};

NetComStorageIterator NetComStorageBegin(eNetComStorage storageType);
NetComStorageIterator NetComStorageEnd(eNetComStorage storageType);
void NetComStorageNext(NetComStorageIterator &it);

class SyncFile {
    FILE *m_pFile;
    UInt m_nMode;
    UInt m_nVersion;
    WideChar m_szFilePath[256];
    Int field_20C;
    void *m_pUserBufferData;
    UInt m_pUserBufferSize;

public:
    enum Mode {
        Read = 1,
        Write = 2
    };

    SyncFile();
    SyncFile(UInt bufferSize);
    ~SyncFile();
    Bool Load(WideChar const *filePath, UInt mode, UInt version);
    Bool Close();
    UInt Mode();
    UInt Version();
    Bool Chunk(UInt id, UInt minVersion);
    Bool Data(void *buf, void *pDefaultValue, UInt minVersion, UInt size);
    Bool DataArray(void *buf, UInt count, void *pDefaultValue, UInt minVersion, UInt size);
    Bool String(WideChar *out, UInt maxLen, WideChar const *defaultValue, UInt minVersion);
    Bool IsVersionGreaterOrEqual(UInt version);
    Bool UInt32(UInt *pValue, UInt *pDefaultValue, UInt minVersion);
    Bool UInt16(UShort *pValue, UShort *pDefaultValue, UInt minVersion);
    Bool UInt8(UChar *pValue, UChar *pDefaultValue, UInt minVersion);
};
