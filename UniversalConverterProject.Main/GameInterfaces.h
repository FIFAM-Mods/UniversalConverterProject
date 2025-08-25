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
    UInt value = 0;
public:
    CJDate();
    CJDate(UInt year, UInt month, UInt day);
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
    Bool IsNull();
    Bool operator>(const CJDate &other) const;
    Bool operator<(const CJDate &other) const;
    Bool operator==(const CJDate &other) const;
    Bool operator>=(const CJDate &other) const;
    Bool operator<=(const CJDate &other) const;
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
    class Proxy {
    public:
        FmVec *myVec;
    };

    Proxy *proxy;
    UInt _size;
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

    T &operator[](UInt index) {
        return begin[index];
    }

    FmVec() {
        plugin::CallMethod<0x40F430>(this);
    }

    ~FmVec() {
        plugin::CallMethod<0x40EAA0>(this);
    }
};

template<typename T>
class FmList {
public:
    class Proxy {
    public:
        FmList *myVec;
    };

    class Node {
    public:
        Node *next;
        Node *prev;
        T value;
    };

    class Iterator {
    public:
        Proxy *proxy;
        Node *node;

        Iterator(Node *n, Proxy *p) : node(n), proxy(p) {}

        Iterator &operator++() {
            node = node->next;
            return *this;
        }

        Iterator &operator--() {
            node = node->prev;
            return *this;
        }

        bool operator!=(const Iterator &other) const {
            return node != other.node;
        }

        bool operator==(const Iterator &other) const {
            return node == other.node;
        }

        T &operator*() {
            return node->value;
        }
    };

    size_t size() {
        return _size;
    }

    Iterator begin() {
        return Iterator(head->next, proxy);
    }

    Iterator end() {
        return Iterator(head, proxy);
    }

    Proxy *proxy;
    Int _unk[4];
    Node *head;
    UInt _size;
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

class CGameEvent {
public:
    CJDate m_date;
    UInt m_nEntityId;
    UShort m_nEventId;
    UShort m_nFlags;
    Int m_nParam1;
    Int m_nParam2;
    Int m_nParam3;
    Int m_nParam4;
    Int m_nParam5;

    CGameEvent(CJDate const &date, UInt entityId, UShort eventId, UShort flags,
        Int param1, Int param2, Int param3, Int param4, Int param5);
};

class CGameEvents {
public:
    Bool PopEvent(UInt eventType, CJDate date, CGameEvent &out);
    void AddEvent(UInt type, CGameEvent const &event);
};

CGameEvents &GameEvents();

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

CDBGame *Game();
Bool HidePlayerLevel();

struct CTeamIndex {
    unsigned short index = 0;
    unsigned char countryId = 0;
    unsigned char type = 0;

    unsigned int ToInt() const;
    CTeamIndex firstTeam() const;
    bool isNull() const;
    void clear();
    static CTeamIndex make(unsigned char CountryId, unsigned char Type, unsigned short Index);
    static CTeamIndex make(unsigned int value);
    static CTeamIndex null();
    Bool isFirstTeam();
    Bool isReserveTeam();
};

bool operator==(CTeamIndex const &a, CTeamIndex const &b);

class CDBCountry;
class CDBTeam;
class CDBEmployee;
class CDBStaff;
class CWorker;

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

bool operator==(CCompID const &a, CCompID const &b);

struct CMatchEvent {
    UInt m_nType;
    UChar m_nMinute;
    UChar m_nReason;
    Char field_6;
    Char field_7;
};

class CMatchStatistics {
public:
    UInt m_nMinutesPlayed;
    UInt m_nGoals;
    UInt m_nAssists;
    Float m_fMark;
    UInt m_nForm;
    UInt m_nYellowCards;
    UInt m_nYelRedCards;
    UInt m_nRedCards;
    UChar m_bManOfTheMatch;
    Char _pad21[3];
    UInt m_nSubInMinute;
    CMatchEvent events[32];
    UChar field_128;
    Char _pad129[3];

    CMatchStatistics();
};

static_assert(sizeof(CMatchStatistics) == 0x12C, "Failed");

class CDBPlayer;

class CPlayerStats {
public:
    UChar GetNumInternationalCaps() const;
    Int GetMatchStats(CCompID const &compId, Int eventIndex, CMatchStatistics &outStats) const;
    Int GetSeasonStats(CMatchStatistics &outStats, UInt &numConsecutiveHomeMatches, UInt &numMatchesWithMark, UInt &numMOTMs, Bool bCurrentClubOnly, CCompID const &compID, UInt minMinutesForMark, Bool bLocalizedMarks, Bool bWithFriendlies, CTeamIndex teamID, UInt compIDMask, Bool bWithoutQuali) const;
    Int GetStatsForMatches(CMatchStatistics &outStats, UInt &numConsecutiveHomeMatches, UInt &numMatchesWithMark, UInt &numMOTMs, CCompID const &compID, UInt minMinutesForMark, Bool bLocalizedMarks, Bool bWithFriendlies, UInt compIDMask, CJDate startDate, CDBTeam *team, Bool bWithoutQuali) const;
    void SetNumPlayerOfTheMonth(UChar count);
    void SetNumPlayerOfTheYear(UChar count);
    void SetNumFifaWorldPlayerAwards(UChar count);
    void SetNumEuropeanPlayerOfTheYear(UChar count);
    void AddEuroCupWin();
    void AddWorldCupWin();
    UChar GetNumPlayerOfTheMonth() const;
    UInt GetNumPlayerOfTheYear() const;
    UInt GetNumFifaWorldPlayerAwards() const;
    UChar GetNumEuropeanPlayerOfTheYear() const;
    UInt GetNumECWins() const;
    UInt GetNumWCWins() const;
    CDBPlayer *GetPlayer() const;
    UInt GetPlayerId() const;
};

enum ePlayerPositionRole {
    PLAYER_POSITION_ROLE_GK,
    PLAYER_POSITION_ROLE_DEF,
    PLAYER_POSITION_ROLE_MID,
    PLAYER_POSITION_ROLE_ATT
};

struct NameDesc {
    UChar languageId;
    UChar flags;
    UShort firstName;
    UShort lastName;

    WideChar const *ToName(WideChar *buf = nullptr);
    WideChar const *ToPlayerStringID(CJDate birthdate, Int empicsId, WideChar *buf);
    String ToPlayerStringID(CJDate birthdate, Int empicsId);
};

static_assert(sizeof(NameDesc) == 0x6, "Failed");

class CDBPlayer {
public:
    // 0 to 20
    UChar GetPlayingForm();
    UInt GetAge();
    Char GetTalent(CDBEmployee *employee = nullptr);
    Char GetBestPosition();
    UChar GetBasicLevel(Char position, CDBEmployee *employee = nullptr);
    UChar GetLevel(Char position = -1, CDBEmployee *employee = nullptr);
    CTeamIndex GetCurrentTeam();
    EAGMoney GetMarketValue(CDBEmployee *employee = nullptr);
    EAGMoney GetDemandValue();
    EAGMoney GetMinRelFee();
    UChar GetNationality(UChar number = 0);
    CTeamIndex GetNationalTeam();
    String GetName(bool shortForm = false);
    NameDesc GetNameDesc();
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
    CPlayerStats const *GetStatsConst() const;
    CPlayerStats *GetStats();
    UChar GetPotential();
    UChar GetPositionRole();
    Char GetAbility(UInt ability, CDBEmployee *employee = nullptr);
    UInt GetEmpicsID();
    CJDate GetBirthdate();
    UChar GetShirtNumber(Bool firstTeam = true);
    Bool IsGoalkeeper();
    Bool IsCaptain();
    UChar GetTeamPart();
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
    UChar GetPlayerKnowledge(CDBPlayer *player);
};

CDBEmployee *GetManagerWhoLooksForPlayer(UInt playerId);

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

enum eGoalType {
    GOAL_INSIDEBOX = 0,
    GOAL_OUTSIDEBOX = 1,
    GOAL_HEADER = 2,
    GOAL_VOLLEY = 3,
    GOAL_FREEKICK = 4,
    GOAL_PENALTY = 5,
    GOAL_OWNGOAL = 6
};

enum eMatchEvent {
    MET_START = 0x0,
    MET_GOAL = 0x1,
    MET_OWNGOALS = 0x2,
    MET_GOALSHOT = 0x3,
    MET_SAVED_SHOT = 0x4,
    MET_CONCEDED_GOAL = 0x5,
    MET_POST_SHOT = 0x6,
    MET_BAR_SHOT = 0x7,
    MET_WIDE_SHOT = 0x8,
    MET_YEL_CARD = 0x9,
    MET_YELRED_CARD = 0xA,
    MET_RED_CARD = 0xB,
    MET_INJURY = 0xC,
    MET_FOUL = 0xD,
    MET_WRONGDECISION = 0xE,
    MET_FREEKICK = 0xF,
    MET_CORNERKICK = 0x10,
    MET_SUBST = 0x11,
    MET_CHANCE = 0x12,
    MET_PENALTY_KICK = 0x13,
    MET_PENALTY_AET = 0x14,
    MET_MANOFTHEMATCH = 0x15,
    MET_PLAYER_APPEARANCE = 0x16,
    MET_WHISTLE = 0x18,
    MET_OFFSIDE = 0x1A,
    MET_INJURY_MINOR = 0x1B,
    MET_END = 0x1C,
    MET_PLAYER_MARK = 0x1E,
    MET_PLAYER_FORM = 0x1F,
    MET_ATTENDANCE = 0x20,
};

class CDBMatchEventEntry {
public:
    UChar bHomeTeam : 1;
    UChar bAdditionalInfo : 1;
    UChar bMissedPenalty : 1;
    UChar minute;
    UShort eventType;
    Int value1;
    Int value2;
    UChar playerPositions[3];
    Char _padF;
    union {
        Int value3;
        struct { // Additional info
            UChar reason1;
            UChar reason2;
            UChar goalsHome;
            UChar goalsAway;
        };
    };

    CDBMatchEventEntry();
    UInt GetEventType();
    UChar GetMinute();
    UInt GetPlayerInitiator();
    UInt GetPlayerAffected();
    UInt GetValue(UInt index);
    UInt GetReason1();
    UInt GetReason2();
    Bool IsHomeTeam();
    Bool HasAdditionalData();
    Bool IsMissedPenalty();
    UChar GetPlayerPosition(UChar index);
};

static_assert(sizeof(CDBMatchEventEntry) == 0x14, "Failed");

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
    void GetMatchEvent(Int index, CDBMatchEventEntry &event);
    CDBMatchEventEntry &GetMatchEvent(Int index);
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
    void AddFlag(UInt flag);
    Bool CheckFlag(UInt flag);
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
    void GetMatch(UInt matchday, UInt matchIndex, CMatch &match) const;
    void GetFixtureTeams(UInt matchday, UInt matchIndex, UChar &team1number, UChar &team2number) const;
    void GetFixtureTeams(UInt matchday, UInt matchIndex, CTeamIndex &team1, CTeamIndex &team2) const;
    UInt GetMatchesInMatchday();
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
    static void Format(WideChar *dst, UInt maxLen, WideChar const *format, CEAMailData const &mailData);
    void SetPlayer(Int index, UInt playerId);
    void SetTeam(Int index, CTeamIndex const &teamID);
    void SetTerm(WideChar const *term);
    void SetStaff(Int index, UInt staffId);
    void SetManager(UInt managerId);
    void SetManager2(UInt managerId);
    void SetPlayerPosition(Int index, UChar position);
    void SetStaffPosition(UChar position);
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

class CompWinYear {
public:
    CCompID compID;
    UShort year;
    Char _pad6[2];
};

class WinYearsList {
public:
    CompWinYear *entries;
    UInt count;
    UInt capacity;
};

class ClubHistoryRecord {
public:
    WideChar m_szOpponentName[22];
    UChar m_nScore1;
    UChar m_nScore2;
    UShort m_nYear;
    UInt m_nAttendance;
};

class CClubHistory {
public:
    ClubHistoryRecord m_aRecords1[5];
    ClubHistoryRecord m_aRecords2[5];
    WinYearsList m_winYearsList;
};

static_assert(sizeof(CClubHistory) == 0x214, "Failed");

enum eTeamPart {
    TEAMPART_GOALKEEPER = 0,
    TEAMPART_DEFENSE = 1,
    TEAMPART_MIDFIELD = 2,
    TEAMPART_OFFENSE = 3,
    TEAMPART_NONE = 5
};

class IPlayerRoles {
public:
};

class CLineUpController {
public:
    UInt GetFieldPlayer(UChar index);
    UInt GetBenchPlayer(UChar index);
    UInt GetPlayer(UChar index);
    UChar GetNumPlayersOnField();
    UChar GetNumPlayersOnBench();
    UChar GetNumPlayersWithPosition(UInt position);
    UChar GetNumPlayersOfTeamPart(UInt teamPart);
    UInt GetPlayerTeamPart(UInt playerId);
    UInt GetSlotTeamPart(UChar slotIndex);
    Bool GetFormationName(UInt formationId, WideChar *outName, UInt maxLen);
    void GetFormationName(WideChar *outName, UInt maxLen);
};

class LineUp {
public:
    CLineUpController *GetLineUpController();
};

class LineUpSettings {
public:
    IPlayerRoles *GetRoles();
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
    CCompID GetFirstTeamLeagueID();
    CCompID GetTeamLeagueID(CTeamIndex teamID);
    CClubFans *GetClubFans();
    CTeamStaff *GetTeamStaff();
    CTeamFanshops *GetFanShops();
    CClubHistory *GetClubHistory();
    LineUp *GetLineUp(CTeamIndex const &teamID);
    LineUpSettings *GetLineUpSettings(CTeamIndex teamID);
    UInt GetAllStaff(FmVec<UInt> const &vec);
    UInt GetAllStaff(FmVec<CDBStaff> const &vec);
    UInt GetStaffIdWithRole(UChar role);
    CDBStaff *GetStaffWithRole(UChar role);
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
    const UChar GetFirstLanguage();
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
CDBPlayer *GetPlayer(UInt playerId);
CDBEmployee *GetEmployee(UInt employeeId);
CDBStaff *GetStaff(UInt staffId);
WideChar const *GetCityName(UInt cityId);

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
Bool IsPlayerCaptain(UInt playerId, IPlayerRoles *roles, CLineUpController *lineUpController);

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

Bool GetFilenameForImageIfExists(String &out, String const &folder, String const &filename);

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
String PlayerName(CDBPlayer *player);
String PlayerName(UInt playerId);

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
    void Dump(WideChar const *dumpFolder);
};

CDBOneMatch *GetCurrentMatch();

class CDBMatchlist {
public:
    CDBOneMatch *GetMatch(UInt index);
    UInt GetNumMatches();
};

CDBMatchlist &DBMatchlist();

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
UInt SaveGameReadSize(void *save);
void SaveGameReadData(void *save, void *data, UInt size);
void SaveGameWriteInt8(void *save, UChar value);
void SaveGameWriteInt32(void *save, UInt value);
void SaveGameWriteFloat(void *save, Float value);
void SaveGameWriteFloatArray(void *save, Float const *values, UInt count);
void SaveGameWriteSize(void *save, UInt value);
void SaveGameWriteData(void *save, void *data, UInt size);
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

    Bool operator==(const NetComStorageIterator &other) const;
    Bool operator!=(const NetComStorageIterator &other) const;
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
    Char const *GetUid() const;
    Bool32 CheckFlag(UInt flag) const;
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
    Bool IsVisible();
    CGuiNode *GetGuiNode();
    void SetTooltip(WideChar const *text);
    void *CastTo(UInt typeId);
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
    void SetBlendCol(UInt color);
    UInt GetBlendCol();
};

class CXgBaseButton : public CXgVisibleControl {
public:
};

class CXgListBoxCompound : public  CXgVisibleControl {
public:
    void SetColor(UInt colorType, UInt colorValue);
    UInt GetColor(UInt colorType);
};

class CXgFmListBox : public  CXgListBoxCompound {
public:
    static CXgFmListBox *Cast(CXgBaseControl *control);
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
    LBT_FLAG_2 = 3,
    LBT_CLUB = 4,
    LBT_CLUB_BADGE = 5,
    LBT_PLAYER = 6,
    LBT_EMPLOYEE = 7,
    LBT_PERSON_3 = 8,
    LBT_INT = 9,
    LBT_FLOAT = 12,
    LBT_MONEY_1 = 15,
    LBT_MONEY_2 = 17,
    LBT_MONEY_3 = 30,
    LBT_COMP_LOGO = 36,
    LBT_COMP_NAME = 37,
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

class CGuiInstance;

class CXgPanel {
public:
    CGuiInstance *GetGuiInstance();
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
    void AddColumnInt(Int64 value, UInt color = 0xFF1A1A1A, Int unk = 0);
    void AddColumnFloat(Float value, UInt color = 0xFF1A1A1A, Int unk = 0);
    void AddColumnString(WideChar const *str, UInt color = 0xFF1A1A1A, Int unk = 0);
    void AddTeamWidget(CTeamIndex const &teamID);
    void AddTeamName(CTeamIndex const &teamID, UInt color = 0xFF1A1A1A, Int unk = 0);
    void AddCompetition(CCompID const &compID, UInt color = 0xFF1A1A1A, Int unk = 0);
    void AddCountryFlag(UInt countryId, Int unk = 0);
    void AddColumnImage(WideChar const *imagePath);
    void SetRowColor(UInt rowIndex, UInt color);
    void NextRow(Int unk = 0);
    void Create(CXgFMPanel *panel, const char *name);
    Int64 GetCellValue(UInt row, UInt column);
    void SetCellValue(UInt row, UInt column, Int64 value);
    void SetFont(Char const *fontName);
    void SetHeaderFont(Char const *fontName);
    CXgTextBox *GetCellTextBox(UInt rowIndex, UInt columnIndex);
    void Sort(UInt columnIndex, Bool descendingOrder = false);
    CXgFmListBox *GetXgListBox();

    template <typename... ArgTypes>
    static void InitColumnTypes(CFMListBox *listBox, ArgTypes... args) {
        Call<0xD19660>(listBox, args...);
    }
    template <typename... ArgTypes>
    static void InitColumnFormatting(CFMListBox *listBox, ArgTypes... args) {
        Call<0xD196A0>(listBox, args...);
    }
};

class CXgFMPanel : public CXgPanel {
public:
    CXgTextBox *GetTextBox(Char const *name);
    CXgTextButton *GetTextButton(Char const *name);
    CXgImage *GetImage(Char const *name);
    CTrfmNode *GetTransform(Char const *name);
    CXgCheckBox *GetCheckBox(Char const *name);
    CXgComboBox *GetComboBox(Char const *name);
    CXgVisibleControl *GetControl(Char const *name);
    CXgVisibleControl *GetControlIfExists(Char const *name);
    Int SetPlayerPortrait(CXgVisibleControl *control, UInt playerId, Bool unk = false);
    void SetPlayerImage(CXgVisibleControl *control, UInt playerId, WideChar const *filePath);
    void SetPlayerName(CXgVisibleControl *control, UInt playerId);
    void SetTeamBadge(CXgVisibleControl *control, CTeamIndex teamID);
    void SetTeamName(CXgVisibleControl *control, CTeamIndex teamID);
    void SetCountryFlag(CXgVisibleControl *control, UInt countryId);
    void SetCountryFlag(CXgVisibleControl *control, UInt countryId, UInt size);
    void SetCountryName(CXgVisibleControl *control, UInt countryId);
    void SetCompetitionBadge(CXgVisibleControl *control, CCompID compID, UInt size);
    void SetCompetitionName(CXgVisibleControl *control, CCompID compID);
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
    UInt playerIds[100];
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

struct ImageDesc {
    UShort m_nWidth;
    UShort m_nHeight;
    UChar m_nDepth;
    UChar _pad5[3];
    UInt m_nMaskRed;
    UInt m_nMaskGreen;
    UInt m_nMaskBlue;
    UInt m_nMaskAlpha;
    UInt m_nInternalFormat;
    UChar m_nNumTilesX;
    UChar m_nNumTilesY;
    UShort m_tilesX[32];
    UShort m_tilesY[32];
    Char _pad9E[2];
};

static_assert(sizeof(ImageDesc) == 0xA0, "Failed");

class CImage_ {
public:
    void *vtable;
    void* m_pData;
    UInt m_nDataSize;
    ImageDesc m_desc;
    WideChar m_szTypeName[4];
    Bool m_bDataAllocated;
    Char _padB5[3];
};

struct PngReadData {
    UChar *data;
    UInt size;
    UInt offset;
};

class CXPNG_ : public CImage_ {
public:
    void *pPngStruct;
    void *pPngInfo;
    PngReadData readData;
};

static_assert(sizeof(CXPNG_) == 0xCC, "Failed");

class PixelFormat {
public:
    UInt depth;
    UInt rMask;
    UInt gMask;
    UInt bMask;
    UInt aMask;
    UInt rShift;
    UInt gShift;
    UInt bShift;
    UInt aShift;
    UInt rBits;
    UInt gBits;
    UInt bBits;
    UInt aBits;

    PixelFormat(UInt depth, UInt rMask, UInt gMask, UInt bMask, UInt aMask);
    void Convert(UChar* dst, UInt dstSize, PixelFormat *srcFormat, UChar* src, UInt srcSize, UInt mask);
};

static_assert(sizeof(PixelFormat) == 0x34, "Failed");

void ClampTilesToImageBounds(UShort *dst, UShort *src, UInt numTiles, UShort total);

void *opNew(UInt size);
void opDelete(void *data);

Bool GetFirstManagerRegion(UInt &outRegion);

struct HistoricalPlayerEntry {
    UShort year;
    NameDesc name;
    CTeamIndex teamID;
    Float level;
    UChar age;
    UChar position;
    Char _pad12[2];
    Int count;
    UInt playerId;
    Float mark;
    UChar countryId;
    Char _pad21[3];
};

static_assert(sizeof(HistoricalPlayerEntry) == 0x24, "Failed");

class CNamePools {
public:
    UShort AddName(UChar languageId, UInt nameType, WideChar const *name);
    WideChar const *GetNameByIndex(UChar nameType, UInt languageId, UShort index);
    WideChar const *FormatName(NameDesc const &nameDesc, Bool bShortForm, WideChar *buf, UInt bufSize);
    String FormatName(NameDesc const &nameDesc, Bool bShortForm = false);
};

CNamePools *GetNamePools();
UChar GetCountryFirstLanguage(UChar countryId);

struct GeoPrimState {
    Int nPrimitiveType;
    Int nShading;
    Int bCullEnable;
    Int nCullDirection;
    Int nDepthTestMetod;
    Int nAlphaBlendMode;
    Int bAlphaTestEnable;
    Int nAlphaCompareValue;
    Int nAlphaTestMethod;
    Int bTextureEnable;
    Int nTransparencyMethod;
    Int nFillMode;
    Int nBlendOperation;
    Int nSrcBlend;
    Int nDstBlend;
    Float fNumPatchSegments;
    Int nZWritesEnable;
};

UInt GetIDForObject(UChar type, UInt id);
void *GetObjectByID(UInt id);

enum MatchesGoalsCompType {
    MG_COMP_LEAGUE,
    MG_COMP_RESERVE,
    MG_COMP_CUP,
    MG_COMP_CONTINENTAL,
    MG_COMP_INTERNATIONAL
};

class CDBMatchesGoalsLeagueList {
public:
    UShort GetNumEntries();
    UChar GetCountryId(Int entryId);
    UShort GetYear(Int entryId);
    UShort GetLeagueLevel(Int entryId, Bool firstTeam);
    UChar GetNumGoals(Int entryId, Int compType);
    UChar GetNumAssists(Int entryId, Int compType);
    UChar GetNumMatches(Int entryId, Int compType);
    UChar GetMOTM(Int entryId, Int compType);
    Float GetAverageMark(Int entryId);
};

CDBMatchesGoalsLeagueList *GetPlayerMatchesGoalsList(UInt playerId);

class CDBPlayerCareerEntry {
public:
    UShort GetMatches(Bool bFirstTeam) const;
    UShort GetGoals(Bool bFirstTeam) const;
    CTeamIndex GetTeamID() const;
    CJDate GetStartDate() const;
    CJDate GetEndDate() const;
    Bool OnLoan() const;
};

class CDBPlayerCareerList {
public:
    UInt GetNumEntries();
    CDBPlayerCareerEntry *GetEntry(UInt index);
    CTeamIndex GetTeamID(UInt entryId);
    CJDate GetStartDate(UInt entryId);
    CJDate GetEndDate(UInt entryId);
};

CDBPlayerCareerList *GetPlayerCareerList(UInt playerId);

enum eNetworkEventId {
    NETWORKEVENT_WORLD_PLAYER_GALA = 205
};

class CNetworkEvent {
public:
    UShort m_nShortID;
    UShort m_nEventID;
    UInt m_nIntID;
    void *field_8;
    Char m_szTextParam[64];
    UInt m_nStatus;
    Bool m_bActive;
    Bool m_bAsynchronous;
    Char _pad52[2];
};

static_assert(sizeof(CNetworkEvent) == 0x54, "Failed");

class CDBNetwork {
public:
    CNetworkEvent *AddEvent(UShort eventId, Short shortId, Int intId, void *unk = nullptr);
};

CDBNetwork &GetNetwork();

class CNetComData {
    void *vtable;
    UInt m_nStructSize;
    UInt m_nId;
};

class CCurrentUser {
public:
    CDBTeam *GetTeam();
    void SetUseDefaultColors(Bool useDefaultColors);
    void SetTeamColors(CDBTeam *team);
};

CCurrentUser &CurrentUser();

class CGuiFrame {
public:
    void ApplyColorGroups(CGuiInstance *guiInstance);
};

CGuiFrame *GetGuiFrame();

class CBuffer {
public:
    void *m_pData;
    UInt m_nSize;

    CBuffer(void *data, UInt size);
};

class CMinMlGen {
public:
    class CNode;

    class CAttrParser {
    public:
        void *vtable;
        CNode *node;
        Int field_8;

        Char const *GetString(Char const *attribName, Char const *defaultValue = nullptr);
    };

    class CBaseNode {
    public:
    };

    class CNode : public CBaseNode {
    public:
        CNode &FindChildNode(Char const *nodeName);
        CAttrParser &GetAttrParser(CBuffer const &buf);
    };
};

void SetControlCountryFlag(CXgVisibleControl *control, UChar countryId);

class TopScorer {
public:
    UInt m_nPlayerId;
    CJDate m_birthdate;
    NameDesc m_name;
    UShort m_nGoals;
    UShort m_nPenalties;
    UShort m_nDoubles;
    UShort m_nHomeMatches;
    UShort m_nAssists;
    UShort m_nMOTMs;
    UShort m_nMatches;

    UShort GetGoals() const;
    UShort GetPenalties() const;
    UShort GetDoubles() const;
    UShort GetHomeMatches() const;
    UShort GetAssists() const;
    UShort GetMatches() const;
    UShort GetMOTMs() const;
    UInt GetGoalsAndAssists() const;
    UInt GetPlayerId() const;
    Float GetGoalsPerMatch() const;
    NameDesc *GetNameDesc() const;
    WideChar const *GetPlayerName() const;
    WideChar *GetPlayerName(WideChar *buf, UInt maxLen) const;
};

static_assert(sizeof(TopScorer) == 0x1C, "Failed");

class TopScorersBuffer {
public:
    UInt m_nCapacity;
    UInt m_nSize;
    TopScorer *m_pData;

    TopScorersBuffer(UInt capacity = 20);
    ~TopScorersBuffer();
    UInt Size() const;
    TopScorer *At(UInt index);
};
