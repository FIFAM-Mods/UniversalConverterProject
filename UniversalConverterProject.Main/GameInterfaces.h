#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"
#include "CompetitionsShared.h"

enum eGuiColor {
    COL_BLANK,
    COL_TEXT_STANDARD,
    COL_TEXT_STANDARD_ACTIVE,
    COL_TEXT_STANDARD_INACTIVE,
    COL_TEXT_STANDARD_DISABLED,
    COL_TEXT_STANDARD_MOUSEOVER,
    COL_TEXT_INVERTED,
    COL_TEXT_INDICATOR_HIGHLIGHT,
    COL_TEXT_INDICATOR_LINK,
    COL_TEXT_TICKER_STANDARD,
    COL_TEXT_TICKER_RSS,
    COL_GEN_FINANCES_EARNINGS,
    COL_GEN_FINANCES_EXPENDITURES,
    COL_GEN_DISTINCT_1,
    COL_GEN_DISTINCT_2,
    COL_GEN_DISTINCT_3,
    COL_GEN_GRADUAL_VPOS,
    COL_GEN_GRADUAL_POS,
    COL_GEN_GRADUAL_AVG,
    COL_GEN_GRADUAL_NEG,
    COL_GEN_GRADUAL_VNEG,
    COL_GEN_GRADUAL_PROGRESS,
    COL_GEN_CAPTIONCOLOR_1,
    COL_GEN_CAPTIONCOLOR_2,
    COL_GEN_CAPTIONCOLOR_3,
    COL_GEN_CAPTIONCOLOR_4,
    COL_GEN_CAPTIONCOLOR_5,
    COL_GEN_CAPTIONCOLOR_6,
    COL_GEN_CAPTIONCOLOR_7,
    COL_GEN_CAPTIONCOLOR_8,
    COL_GEN_CAPTIONCOLOR_9,
    COL_GEN_CAPTIONCOLOR_10,
    COL_GEN_CAPTIONCOLOR_11,
    COL_GEN_CAPTIONCOLOR_12,
    COL_GEN_CAPTIONCOLOR_1T,
    COL_GEN_CAPTIONCOLOR_2T,
    COL_GEN_CAPTIONCOLOR_3T,
    COL_GEN_CAPTIONCOLOR_4T,
    COL_GEN_CAPTIONCOLOR_5T,
    COL_GEN_CAPTIONCOLOR_6T,
    COL_GEN_CAPTIONCOLOR_7T,
    COL_GEN_CAPTIONCOLOR_8T,
    COL_GEN_CAPTIONCOLOR_9T,
    COL_GEN_CAPTIONCOLOR_10T,
    COL_GEN_CAPTIONCOLOR_11T,
    COL_GEN_CAPTIONCOLOR_12T,
    COL_BG_FILL_EMPTY,
    COL_BG_FILL_STANDARD,
    COL_BG_FILL_ACTIVE,
    COL_BG_FILL_INACTIVE,
    COL_BG_FILL_DISABLED,
    COL_BG_FILL_MOUSEOVER,
    COL_BG_TABLEPOS_CHAMPION,
    COL_BG_TABLEPOS_INTCOMP1ST,
    COL_BG_TABLEPOS_INTCOMP2ND,
    COL_BG_TABLEPOS_INTCOMP3RD,
    COL_BG_TABLEPOS_PROMOTION,
    COL_BG_TABLEPOS_PROMOTIONMATCH,
    COL_BG_TABLEPOS_RELEGATIONMATCH,
    COL_BG_TABLEPOS_RELEGATION,
    COL_BG_PLAYERSTATE_FIRST11,
    COL_BG_PLAYERSTATE_BENCH,
    COL_BG_PLAYERSTATE_NOTINSQUAD
};

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
	String ToStr();
};

enum eCurrency : Int {
    CURRENCY_EUR = 0,
    CURRENCY_USD = 1,
    CURRENCY_GBP = 2,
    CURRENCY_PLN = 3
};

class EAGMoney {
public:
    Int64 mValue;

    EAGMoney();
    EAGMoney(Int value, eCurrency currency = CURRENCY_EUR);
    EAGMoney(Int64 value, eCurrency currency = CURRENCY_EUR);
    EAGMoney(Double value, eCurrency currency = CURRENCY_EUR);
    Int64 GetValue() const;
    Bool Set(Int64 value, eCurrency currency = CURRENCY_EUR);
    Bool IsValidCurrency(eCurrency currency = CURRENCY_EUR) const;
    Int64 GetValueInCurrency(eCurrency currency = CURRENCY_EUR) const;
    EAGMoney &operator=(Int64 rhs);
    EAGMoney operator-();
};

Bool operator<(EAGMoney const &a, EAGMoney const &b);
Bool operator>(EAGMoney const &a, EAGMoney const &b);
Bool operator<=(EAGMoney const &a, EAGMoney const &b);
Bool operator>=(EAGMoney const &a, EAGMoney const &b);
Bool operator==(EAGMoney const &a, EAGMoney const &b);
Bool operator!=(EAGMoney const &a, EAGMoney const &b);
Bool operator>(Int64 a, EAGMoney const &b);
Bool operator==(Int64 a, EAGMoney const &b);
Bool operator<(EAGMoney const &a, Int64 b);
Bool operator>(EAGMoney const &a, Int64 b);
Bool operator<=(EAGMoney const &a, Int64 b);
Bool operator>=(EAGMoney const &a, Int64 b);
Bool operator==(EAGMoney const &a, Int64 b);
Bool operator!=(EAGMoney const &a, Int64 b);
EAGMoney operator+(EAGMoney const &a, EAGMoney const &b);
EAGMoney &operator+=(EAGMoney &a, EAGMoney const &b);
EAGMoney operator+(EAGMoney const &a, Int64 b);
EAGMoney operator+(EAGMoney const &a, Int b);
EAGMoney &operator+=(EAGMoney &a, Int64 b);
EAGMoney &operator+=(EAGMoney &a, Int b);
EAGMoney operator+(EAGMoney const &a, Double b);
EAGMoney &operator+=(EAGMoney &a, Double b);
EAGMoney operator-(EAGMoney const &a, EAGMoney const &b);
EAGMoney &operator-=(EAGMoney &a, EAGMoney const &b);
EAGMoney operator-(EAGMoney const &a, Int64 b);
EAGMoney operator-(EAGMoney const &a, Int b);
EAGMoney &operator-=(EAGMoney &a, Int64 b);
EAGMoney &operator-=(EAGMoney &a, Int b);
EAGMoney &operator-=(EAGMoney &a, Double b);
EAGMoney operator*(EAGMoney const &a, Double b);
EAGMoney operator*(Double a, EAGMoney const &b);
EAGMoney &operator*=(EAGMoney &a, Double b);
EAGMoney operator/(EAGMoney const &a, Double b);
EAGMoney &operator/=(EAGMoney &a, Double b);
EAGMoney &operator/=(EAGMoney &a, EAGMoney const &b);
EAGMoney operator-(EAGMoney const &rhs);

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
class CStringBase {
protected:
    void *_vtable;
    UInt _size;
    T *_data;
    UInt _capacity;
};

template <typename T>
class CDynamicStringTemplate : public CStringBase<T> {
    UInt _alignment;
public:
    UInt size() const {
        return this->_size;
    }

    UInt capacity() const {
        return this->_capacity;
    }

    T *data() {
        return this->_data;
    }

    T *c_str() const {
        return this->_data;
    }
};

using FmString = CDynamicStringTemplate<WideChar>;

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
    T *end_buf;

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

template<typename TKey, typename TValue>
class FmMap {
public:
    class Proxy {
    public:
        FmMap *myMap;
    };

    class Node {
    public:
        Node *left;
        Node *parent;
        Node *right;
        struct {
            TKey key;
            TValue value;
        };
        unsigned char color;
        bool isNil;
        char _pad[2];
    };

    class Iterator {
    public:
        Proxy *proxy;
        Node *node;

        Iterator(Node *n, Proxy *p) : node(n), proxy(p) {}

        Iterator &operator++() {
            if (!node || node->isNil)
                return *this;
            if (!node->right->isNil) {
                node = node->right;
                while (!node->left->isNil)
                    node = node->left;
            }
            else {
                Node *p = nullptr;
                while ((p = node->parent) && node == p->right)
                    node = p;
                node = p;
            }
            return *this;
        }

        bool operator!=(const Iterator &other) const {
            return node != other.node;
        }

        bool operator==(const Iterator &other) const {
            return node == other.node;
        }

        std::pair<const TKey &, TValue &> operator*() {
            return { node->key, node->value };
        }
    };

    Proxy *proxy;
    int _unk[5];
    Node *head;
    UInt _size;

    size_t size() {
        return _size;
    }

    Iterator begin() {
        return Iterator(head->left, proxy);
    }

    Iterator end() {
        return Iterator(head, proxy);
    }

    Iterator find(const TKey &key) {
        Node *current = head->parent;
        while (current && !current->isNil) {
            if (key < current->key)
                current = current->left;
            else if (key > current->key)
                current = current->right;
            else
                return Iterator(current, proxy);
        }
        return end();
    }
};

template<typename TKey>
class FmSet {
public:
    class Proxy {
    public:
        FmSet *myMap;
    };

    class Node {
    public:
        Node *left;
        Node *parent;
        Node *right;
        TKey key;
        unsigned char color;
        bool isNil;
        char _pad[2];
    };

    class Iterator {
    public:
        Proxy *proxy;
        Node *node;

        Iterator(Node *n, Proxy *p) : node(n), proxy(p) {}

        Iterator &operator++() {
            if (!node || node->isNil)
                return *this;
            if (!node->right->isNil) {
                node = node->right;
                while (!node->left->isNil)
                    node = node->left;
            }
            else {
                Node *p = nullptr;
                while ((p = node->parent) && node == p->right)
                    node = p;
                node = p;
            }
            return *this;
        }

        bool operator!=(const Iterator &other) const {
            return node != other.node;
        }

        const TKey &operator*() const {
            return node->key;
        }
    };

    Proxy *proxy;
    int _unk[5];
    Node *head;
    UInt _size;

    size_t size() {
        return _size;
    }

    Iterator begin() {
        return Iterator(head->left, proxy);
    }

    Iterator end() {
        return Iterator(head, proxy);
    }

    Iterator find(const TKey &key) {
        Node *current = head->parent;
        while (current && !current->isNil) {
            if (key < current->key)
                current = current->left;
            else if (key > current->key)
                current = current->right;
            else
                return Iterator(current, proxy);
        }
        return end();
    }
};

template<typename T>
class SimpleContainer {
    T *mData;
    UInt mSize;
    UInt mCapacity;
public:
    UInt size() {
        return mSize;
    }

    UInt capacity() {
        return mCapacity;
    }

    Bool empty() {
        return !mData || mSize == 0;
    }

    T *operator[](UInt index) {
        return &mData[index];
    }
};

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
    UShort GetCurrentSeasonNumber();
};

struct CTeamIndex {
    unsigned short index;
    unsigned char countryId;
    unsigned char type;

    unsigned int ToInt() const;
    CTeamIndex firstTeam() const;
    bool isNull() const;
    void clear();
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
    Char GetAbility(UInt ability, CDBEmployee *employee = nullptr);
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

struct CCompID {
    unsigned short index = 0;
    unsigned char type = 0;
    unsigned char countryId = 0;

    void SetFromInt(unsigned int value);
    unsigned int ToInt() const;
    std::wstring ToStr() const;
    static CCompID Make(unsigned char _country, unsigned char _type, unsigned short _index);
    static CCompID Make(unsigned int value);
    CCompID BaseCompID() const;
};

struct CScriptCommand {
    unsigned int m_nCommandId;
    CCompID m_competitionId;
    unsigned int m_params;
};

class CDBCompetition {
public:
    unsigned int GetCompetitionType();
    UChar GetRegion();
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
    void SortTeamsForKORoundAfterGroupStage();
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
    Bool IsFinished();
	Bool CanLaunchAllSuccessors();
    EAGMoney GetBonus(UInt bonusId);
    class CDBRound *AsRound();
    class CDBRoot *GetRoot();
    CTeamIndex *Teams();
    Vector<CTeamIndex> GetTeams();
    Vector<CTeamIndex> GetTeams(UInt startPos, UInt count);
    Vector<CTeamIndex> GetRegisteredTeams();
    void SetTeams(Vector<CTeamIndex> const &teams, UInt numRegisteredTeams);
    void SetTeams(Vector<CTeamIndex> const &teams);
    void SetRegisteredTeams(Vector<CTeamIndex> const &teams);
    CDBCompetition *PrevContinental();
    CDBCompetition *NextContinental();
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
    char firstHalfResult1[2];
    char firstHalfResult2[2];
    unsigned int m_nFlags;
    int m_anMatchEventsStartIndex[3];

    RoundPair();
    Bool AreTeamsValid() const;
    void GetResult(UChar &outResult1, UChar &outResult2, UInt &outFlags, UChar flags) const;
    void *GetResultString(void *str, UChar flags, const wchar_t *team1name, const wchar_t *team2name) const;
    Bool IsFinished() const;
    CTeamIndex const &GetWinner() const;
    CTeamIndex const &GetLoser() const;
    Bool TestFlag(UInt flag) const;
    CTeamIndex const &Get1stTeam() const;
    CTeamIndex const &Get2ndTeam() const;
    Int GetMatchEventsStartIndex(UInt leg) const;
    void SetMatchEventsStartIndex(Int index, UInt leg);
    Bool IsWinner(Bool b2ndTeam) const;
};

struct MatchGoalInfo {
    int scorer;
    unsigned char teamSide;
    unsigned char minute;
    char field_6;
    bool isOwnGoal;
};

class CDBMatchEventEntry {
public:
    UChar flags;
    UChar minute;
    UShort eventType;
    Int playerScorerId;
    Int playerAssistantId;
    Int field_C;
    UChar reason1_or_value;
    UChar reason2;
    UChar goalsHome;
    UChar goalsAway;
};

class CDBMatchEventEntries : public SimpleContainer<CDBMatchEventEntry> {
public:
    void Clear();
};

class CDBRoot : public CDBCompetition {
public:
    CCompID GetFirstContinentalCompetition();
    void SetFirstContinentalCompetition(CDBCompetition *comp);
    Bool LaunchesInThisSeason(UInt phase);
    CDBMatchEventEntries *GetEvents();
};

class CDBPool : public CDBCompetition {};

class CDBRound : public CDBCompetition {
public:
    unsigned int GetNumOfPairs();
    void GetRoundPair(unsigned int pairIndex, RoundPair &out);
    RoundPair &GetRoundPair(unsigned int pairIndex);
    Bool GetTeamResult(CTeamIndex teamID, UChar &goalsFor, UChar &goalsAgainst, Bool &home);
    UInt GetLegFlags(UInt leg) const;
};

class CDBCup : public CDBCompetition {};

class CDBLeagueBase : public CDBCompetition {
public:
    CTeamIndex &GetTeamAtPosition(int position);
};

class CMatch{
public:
    UChar homeGoals;
    UChar awayGoals;
    UChar firstHalfHome;
    UChar firstHalfAway;
    UInt flags;
    Int firstEventIndex;

    CMatch();
    Int GetMatchEventsStartIndex() const;
    void SetMatchEventsStartIndex(Int index);
};

static_assert(sizeof(CMatch) == 0xC, "Failed");

class CMatches {
public:
    UInt m_nNumMatchdays;
    UInt m_nNumMatches;
    void *m_pMatches;

    void GetMatch(UInt matchday, UInt matchIndex, CMatch &match) const;
    void SetMatch(UInt matchday, UInt matchIndex, CMatch const &match);
};

class CDBLeague : public CDBLeagueBase {
public:
    void SetStartDate(CJDate date);
    int GetEqualPointsSorting();
    void SortTeams(TeamLeaguePositionData *infos, int sortingFlags, int goalsMinMinute, int goalsMaxMinute, int minMatchday, int maxMatchday);
    unsigned int GetCurrentMatchday();
    CMatches *GetMatches();
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
    Bool UserKitInUse();
    WideChar const *GetBadgePath();
    WideChar const *GetUserKitPath(UInt kitType);
    UChar GetShirtBadgePosition(UChar kitType);
};

class CStadiumDevelopment {
public:
    CDBTeam *GetTeam();
    UInt GetNumSeats();
    const WideChar *GetStadiumName(Bool withSponsor = true);
};

class CEAMailData {
    UChar data[0x278];
public:
    CEAMailData();
    ~CEAMailData();
    void SetRoundType(UChar roundType);
    UChar GetRoundType() const;
    void SetCompetition(CCompID const &compID);
    CCompID GetCompetition() const;
    void SetMoney(EAGMoney const &money);
    EAGMoney GetMoney() const;
    void SetArrayValue(UInt index, Int value);
    Int GetArrayValue(UInt index) const;
    void SetFirstTeam(CTeamIndex const &teamID);
};

class CClubFans {
public:
    class CDBTeam *GetTeam();
    Int GetNumFans();
    void SetNumFans(Int numFans);
    Int AddFans(Int numFans);
};

enum eClubRole {
    ROLE_ASSISTANT_COACH1 = 0x6,
    ROLE_ASSISTANT_COACH2 = 0x7,
    ROLE_AMATEUR_COACH = 0x8,
    ROLE_YOUTH_COACH = 0x9,
    ROLE_FITNESS_COACH = 0xA,
    ROLE_GOALKEEPER_COACH = 0xB,
    ROLE_TEAM_DOCTOR = 0xC,
    ROLE_SPECIALIST_BONE = 0xD,
    ROLE_SPECIALIST_KNEE = 0xE,
    ROLE_SPECIALIST_MUSCLE = 0xF,
    ROLE_MASSEUR = 0x10,
    ROLE_PSYCHOLOGIST = 0x11,
    ROLE_GENERAL_MANAGER = 0x12,
    ROLE_MARKETING_MANAGER = 0x13,
    ROLE_CONSTRUCTION_MANAGER = 0x14,
    ROLE_SPORTS_DIRECTOR = 0x15,
    ROLE_FAN_REPRESENTATIVE = 0x16,
    ROLE_SPOKESPERSON = 0x17,
    ROLE_LAWYER = 0x18,
    ROLE_GENERAL_SCOUT1 = 0x19,
    ROLE_GENERAL_SCOUT2 = 0x1A,
    ROLE_GENERAL_SCOUT3 = 0x1B,
    ROLE_GENERAL_SCOUT4 = 0x1C,
    ROLE_GENERAL_SCOUT5 = 0x1D,
    ROLE_GENERAL_SCOUT6 = 0x1E
};

enum eClubPosition {
    POSITION_MANAGER = 0x1,
    POSITION_CHIEF_EXEC = 0x2,
    POSITION_PRESIDENT = 0x3,
    POSITION_ASSISTANT_COACH = 0x6,
    POSITION_AMATEUR_COACH = 0x7,
    POSITION_YOUTH_COACH = 0x8,
    POSITION_FITNESS_COACH = 0x9,
    POSITION_GOALKEEPER_COACH = 0xA,
    POSITION_TEAM_DOCTOR = 0xB,
    POSITION_SPECIALIST_BONE = 0xC,
    POSITION_SPECIALIST_KNEE = 0xD,
    POSITION_SPECIALIST_MUSCLE = 0xE,
    POSITION_MASSEUR = 0xF,
    POSITION_PSYCHOLOGIST = 0x10,
    POSITION_GENERAL_MANAGER = 0x11,
    POSITION_MARKETING_MANAGER = 0x12,
    POSITION_CONSTRUCTION_MANAGER = 0x13,
    POSITION_SPORTS_DIRECTOR = 0x14,
    POSITION_FAN_REPRESENTATIVE = 0x15,
    POSITION_SPOKESPERSON = 0x16,
    POSITION_LAWYER = 0x17,
    POSITION_GENERAL_SCOUT = 0x18,
    POSITION_NONE = 0x19
};

enum eDepartment {
    DEPARTMENT_COACHES = 0,
    DEPARTMENT_MEDICINE = 1,
    DEPARTMENT_MANAGERS = 2,
    DEPARTMENT_PR = 3,
    DEPARTMENT_SCOUTS = 4,
};

class CRole {
public:
    UInt id;
    UInt positionId;
    CDynamicStringTemplate<WideChar> name;
    UChar importance;
private:
    Char _pad1D;
public:
    UInt locationType;
    UInt locationId;
};

static_assert(sizeof(CRole) == 0x28, "Failed");

class CRoleFactory {
    void *vtable;
public:
    FmMap<UInt, CRole *> roles;
};

CRoleFactory *GetRoleFactory();

class CDepartment {
public:
    UInt id;
    UInt type;
    FmSet<UInt> roles;
};

class CStructure {
public:
    CDepartment *departments[5];
};

class CTeamStaff {
public:
    CTeamIndex teamId;
    CStructure structure;
    FmMap<UInt, Int> roleToWorkerId;
};

class CFanShop {
public:
    UChar countryId;
    Char _pad1;
    UShort abroadID;
    Short field_4;
    Char _pad6[2];
    UInt lastMp;
    Int field_C;
    UInt sumOfAllMp;
    Int field_14;
    Char field_18;
    Char _pad19[3];
    Int roleID;
    UChar flags;
    Char _pad21[3];
    CDBTeam *pTeam;
};

static_assert(sizeof(CFanShop) == 0x28, "Failed");

class CTeamFanshops {
public:
    FmVec<CFanShop> vecShops;
    CDBTeam *pTeam;

    UInt GetNumFanShops();
    CFanShop *GetFanShop(UShort index);
};

class CDBTeam {
public:
    CTeamIndex GetTeamID();
    unsigned int GetTeamUniqueID();
    unsigned char GetNationalPrestige();
    void SetNationalPrestige(UChar prestige);
    unsigned char GetInternationalPrestige();
    void SetInternationalPrestige(UChar prestige);
    wchar_t *GetName(bool first = true);
    wchar_t *GetShortName(CTeamIndex const &teamID, bool includeYouth = false);
    wchar_t *GetClickableTeamName(CTeamIndex const &teamID, bool includeYouth = false);
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
    Char SendMail(UInt mailId, CEAMailData const &mailData, Int flag);
    void ChangeMoney(UInt type, EAGMoney const &money, UInt flag);
    void OnCompetitionElimination(CCompID const &compID, UInt cupRoundId);
    UChar GetFirstTeamDivision();
    UChar GetFirstTeamDivisionLastSeason();
    CClubFans *GetClubFans();
    CTeamStaff *GetTeamStaff();
    CTeamFanshops *GetFanShops();
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
    unsigned int m_nNumEntries;
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
    const Int GetNumClubs();
    const UInt GetNumRegenPlayers();
    const UChar GetLanguage(UChar number);
    const Bool IsPlayerInNationalTeam(UInt playerId);
	void SetFifaRanking(Float value);
	Float GetFifaRanking();
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
CDBRound *GetRound(CCompID const &id);
CDBRound *GetRound(unsigned int *id);
CDBRound *GetRound(unsigned int id);
CDBRound *GetRoundByRoundType(unsigned char region, unsigned char type, unsigned char roundType);
CDBPool *GetPool(unsigned char region, unsigned char type, unsigned short index);

FmMap<UInt, CDBCompetition *> &GetCompetitions();

CDBCountry *GetCountry(UChar countryId);
CDBTeam *GetTeam(CTeamIndex teamId);
CDBTeam *GetTeamByUniqueID(unsigned int uniqueID);
CDBPlayer *GetPlayer(Int playerId);
CDBEmployee *GetEmployee(Int employeeId);
CDBStaff *GetStaff(Int staffId);
WideChar const *GetCityName(Int cityId);

CCountryStore *GetCountryStore();

wchar_t const *GetTranslation(const char *key);
bool IsTranslationPresent(const char *key);
wchar_t const *GetTranslationIfPresent(const char *key);
CJDate GetCurrentDate();
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
void *CreateTextButton(void *screen, char const *name);
void *CreateImage(void *screen, char const *name);
void *GetTransform(void *screen, char const *name);
void SetTextBoxColorRGBA(void *tb, UChar r, UChar g, UChar b, UChar a);
void SetTextBoxColorRGB(void *tb, UChar r, UChar g, UChar b);
void SetTextBoxColorRGBA(void *tb, UInt clr);
void SetTextBoxColorRGB(void *tb, UInt clr);
void SetImageColorRGBA(void *img, UChar r, UChar g, UChar b, UChar a);
void SetImageColorRGB(void *img, UChar r, UChar g, UChar b);
void SetImageColorRGBA(void *img, UInt clr);
void SetImageColorRGB(void *img, UInt clr);
void SetTextBoxColors(void *tb, UInt clr);
void SetEnabled(void *widget, Bool enabled);
void SetVisible(void* widget, Bool visible);
UInt GetId(void *widget);
void SetText(void* widget, WideChar const* text);
unsigned char SetImageFilename(void* widget, std::wstring const& path);
unsigned char SetImageFilename(void *widget, wchar_t const *filename, int u1, int u2);
void SetTransformColor(void *trfm, Float *color);
void SetTransformEnabled(void *trfm, Bool enabled);
void SetTransformVisible(void *trfm, Bool visible);

class CRandom {
public:
    void SetSeed(Int seed1, Int seed2);
    void GetSeed(Int &seed1, Int &seed2);
    // Returns a random integer in [0, maxExclusive)
    static Int GetRandomInt(Int maxExclusive);
    // Returns a random integer in [min, max] (16-bit clamped)
    Int GetRandomBetween(Int min, Int max);
    // Returns a random float in [0.0f, maxExclusive)
    Float GetRandomFloat(Float maxExclusive);
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
String StadiumName(CDBTeam *team);
String StadiumNameWithCountry(CDBTeam *team);
String StadiumTag(CDBTeam *team);
String StadiumTagWithCountry(CDBTeam *team);
String StadiumName(CTeamIndex const &teamId);
String StadiumNameWithCountry(CTeamIndex const &teamId);
String StadiumTag(CTeamIndex const &teamId);
String StadiumTagWithCountry(CTeamIndex const &teamId);
String CompetitionTag(CDBCompetition* comp);
String CompetitionTag(CCompID const& compId);
String CompetitionName(CDBCompetition *comp);
String CompetitionName(CCompID const &compId);
String CountryName(UChar countryId);
String CountryTag(UChar countryId);

String FlagsToStr(UInt value);
String CompetitionType(CDBCompetition *comp);

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
	Bool CheckFlag(UInt flag);
	void GetResult(UChar &outHome, UChar &outAway);
    UInt GetRoundPairIndex();
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
bool BinaryReaderCheckFourcc(void *reader, UInt fourcc);
void BinaryReaderReadUInt16(void *reader, UShort *out);
void BinaryReaderReadUInt8(void *reader, UChar *out);
void BinaryReaderReadFloat(void *reader, Float *out);
void SaveGameReadString(void *save, WideChar *out, UInt maxLen);
void SaveGameWriteString(void *save, WideChar const *str);
void SaveGameReadInt8(void *save, UChar &out);
void SaveGameReadInt32(void *save, UInt &out);
UInt SaveGameReadInt32(void *save);
void SaveGameReadFloat(void *save, Float &out);
Float SaveGameReadFloat(void *save);
void SaveGameReadFloatArray(void *save, Float *values, UInt count);
void SaveGameWriteInt8(void *save, UChar value);
void SaveGameWriteInt32(void *save, UInt value);
void SaveGameWriteFloat(void *save, Float value);
void SaveGameWriteFloatArray(void *save, Float const *values, UInt count);
void SaveGameWriteSize(void *save, UInt value);
void SaveGameWriteString(void *save, FmString const &value);
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

class CStatsBaseScrWrapper {
public:
	UChar m_name[0x14];
	UInt m_nType;

	virtual WideChar const *GetName();
	virtual UInt GetType();
	virtual void *CreateScreen(void *guiInstance) = 0;
	virtual void *DeletingDestructor(UChar flags);
};

Bool IsLiechtensteinClubFromSwitzerland(CTeamIndex const &teamIndex);
UChar GetTeamCountryId_LiechtensteinCheck(CTeamIndex const &teamIndex);

struct CompMatchResult {
    CTeamIndex team1;
    CTeamIndex team2;
    UShort year;
    UChar result1stLeg[2];
    UChar result2ndtLeg[2];
    Char _padE[2];
    Int flags;
    CTeamIndex firstPlaceTeam;
    Int place;
    UChar roundType;
    Char _pad1D[3];
};

class CTrfmNode {
public:
};

class CGuiNode {
public:
};

struct MessageDataBase {
    Char const *name;
};

struct GuiMessage {
    CGuiNode *node;
    Char const *nodeUid;
    CGuiNode *node2;
    Char const *node2Uid;
    MessageDataBase *pData;
};

class CXgBaseControl {
public:
    void SetEnabled(Bool enabled);
    void SetVisible(Bool visible);
    CGuiNode *GetGuiNode();
};

struct Rect {
    Short x; Short y; Short width; Short height;
};

struct VisibleControlAppearance {
    Rect rect;
    UInt blendCol;
    Float depth;
};

class CXgVisibleControl : public CXgBaseControl {
public:
    VisibleControlAppearance *GetAppearance();
    Rect *GetRect();
};

class CXgBaseButton : public CXgVisibleControl {
public:
};

class CXgButton : public CXgBaseButton {
public:
};

class CXgTextBox : public CXgBaseButton {
public:
    void SetColor(UInt colorType, UInt colorValue);
    UInt GetColor(UInt colorType);
    void SetFont(Char const *fontName);
    Char const *GetFont();
    void SetText(WideChar const *text);
    WideChar const *GetText();
};

class CXgImage : public CXgBaseButton {
public:
};

class CXgTextButton : public CXgButton {
public:
};

class CXgComboCompound : public CXgVisibleControl {
public:
    void Clear();
    Int GetNumberOfItems();
    void SetCurrentIndex(Int index);
    Int GetCurrentIndex();
};

class CXgComboBox : public CXgComboCompound {
public:
    Int AddItem(WideChar const *text, Int64 value);
    void SetValueAtIndex(Int index, Int64 value);
    Int64 GetValueAtIndex(Int index);
    void SetCurrentValue(Int64 value);
    Int64 GetCurrentValue(Int64 defaultValue);
};

class CXgCheckBox : public CXgButton {
public:
    void SetIsChecked(Bool checked);
    Bool GetIsChecked();
    UChar GetCheckState();
};

enum FMListBoxColumnType {
    LBT_COUNTRY = 1,
    LBT_FLAG = 2,
    LBT_CLUB = 4,
    LBT_INT = 9,
    LBT_FLOAT = 12,
    LBT_IMAGE = 58,
    LBT_END = 63
};

enum FMListBoxColumnFormatting {
    LBF_FLOAT = 200,
    LBF_SHORTNAME = 203,
    LBF_NAME = 204,
    LBF_NONE = 210,
    LBF_END = 228
};

class CXgFMPanel;

class CFMListBox {
public:
    void SetVisible(Bool visible);
    void Clear();
    Int GetNumRows();
    Int GetNumColumns();
    Int GetMaxRows();
    Int GetTotalRows();
    void AddColumnInt(Int64 value, UInt color, Int unk);
    void AddColumnFloat(Float value, UInt color, Int unk);
    void AddColumnString(WideChar const *str, UInt color, Int unk);
    void AddTeamWidget(CTeamIndex const &teamID);
    void AddTeamName(CTeamIndex const &teamID, UInt color, Int unk);
    void AddCountryFlag(UInt countryId, Int unk);
    void AddColumnImage(WideChar const *imagePath);
    void SetRowColor(UInt rowIndex, UInt color);
    void NextRow(Int unk);
    void Create(CXgFMPanel *panel, const char *name);
    Int64 GetCellValue(UInt row, UInt column);
    void SetCellValue(UInt row, UInt column, Int64 value);
    void SetFont(Char const *fontName);
    CXgTextBox *GetCellTextBox(UInt rowIndex, UInt columnIndex);
    template <typename... ArgTypes>
    static void InitColumnTypes(CFMListBox *listBox, ArgTypes... args) {
        Call<0xD19660>(listBox, args...);
    }
    template <typename... ArgTypes>
    static void InitColumnFormatting(CFMListBox *listBox, ArgTypes... args) {
        Call<0xD196A0>(listBox, args...);
    }
};

class CXgFMPanel {
public:
    CXgTextBox *GetTextBox(Char const *name);
    CXgTextButton *GetTextButton(Char const *name);
    CXgImage *GetImage(Char const *name);
    CTrfmNode *GetTransform(Char const *name);
    CXgCheckBox *GetCheckBox(Char const *name);
    CXgComboBox *GetComboBox(Char const *name);
};

class CompetitionHosts {
public:
    void AddHostCountries(CCompID const &compId, UShort year, UChar hostCountry1, UChar hostCountry2);
    UChar GetHostCountry(CCompID const &compId, UShort year, UInt hostIndex);
    UChar GetFirstHostCountry(CCompID const &compId, UShort year);
    UChar GetSecondHostCountry(CCompID const &compId, UShort year);
    UChar GetNumberOfHostCountries(CCompID const &compId, UShort year);
    Bool AddHostStadium(CCompID const &compId, UShort year, CTeamIndex teamID);
    Bool CountryHostedTournament(CCompID const &compId, UChar countryId);
    Bool IsNationalTeamHost(CCompID const &compId, UShort year, CTeamIndex const &teamID);
    UInt GetNumOfStadiums(CCompID const &compId, UShort year);
    CTeamIndex GetHostStadium(CCompID const &compId, UShort year, UInt stadiumIndex);
    void SelectHostStadiums();
    void SelectHostStadiums(CCompID const &compId, UShort year);
    CTeamIndex GetChampionsLeagueHost();
    CTeamIndex GetUefaCupHost();
    CTeamIndex GetEuroSupercupHost();
};

CompetitionHosts *GetCompHosts();

class CDBYouthcamp {
public:
    Int id;
    Int countryId;
    Int cityId;
    Int level1;
    Int level2;
    Int field_14;
    Int numPlayers;
    Int numSignedPlayers;
    Int field_20;
    CTeamIndex teamID;
    Int youthCoachRoleId;
    Int playerIds[100];
    Int bestSignedPlayerId;
    Int maxSignedPlayerRating;
    Int field_1C4;
    Char field_1C8;
    Char field_1C9;
    Char _pad1CA[2];
    CJDate date;
};

static_assert(sizeof(CDBYouthcamp) == 0x1D0, "Failed");

class CDBYouthcampList {
public:
    FmVec<CDBYouthcamp> countries[208];
};

CDBYouthcampList *GetYouthcampList();

void *GetApp();
