#pragma once
#include "plugin-std.h"

class CJDate {
    unsigned int value;
public:
    CJDate GetTranslated(int numYears);
    unsigned short GetYear();
    unsigned char GetMonth();
    unsigned char CJDate::GetDays();
    void GetYMD(unsigned short *outY, unsigned char *outM, unsigned char *outD);
    void GetYMD(unsigned int *outY, unsigned int *outM, unsigned int *outD);
};

class CDBGame {
public:
    static CDBGame *GetInstance();
    CJDate GetStartDate();
    CJDate GetCurrentDate();
    unsigned short GetCurrentYear();
    bool GetIsWorldCupMode();
    bool TestFlag(unsigned int flag);
};

struct CTeamIndex {
    unsigned short index;
    unsigned char countryId;
    unsigned char type;
};

enum eCompetitionType : unsigned char {
    COMP_ROOT = 0,
    COMP_LEAGUE = 1,
    COMP_UNKNOWN_2 = 2,
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
    COMP_COPA_AMERICA = 33
};

struct CCompID {
    unsigned short index = 0;
    unsigned char type = 0;
    unsigned char countryId = 0;

    void SetFromInt(unsigned int value);
    unsigned int ToInt() const;
    std::wstring ToStr() const;
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
    CTeamIndex &GetChampion(bool checkIntl = false);
    unsigned char GetNumTeamsFromCountry(int countryId);
    void AddTeamsFromCountry(unsigned char countryId, unsigned int numTeams);
    bool AddTeam(CTeamIndex const &teamIndex, int position = -1);
    bool IsTeamPresent(CTeamIndex const &teamIndex);
    unsigned int GetNumMatchdays();
};

class CDBRoot : public CDBCompetition {};
class CDBPool : public CDBCompetition {};
class CDBRound : public CDBCompetition {};
class CDBCup : public CDBCompetition {};
class CDBLeagueBase : public CDBCompetition {};

class CDBLeague : public CDBLeagueBase {
public:
    void SetStartDate(CJDate date);
};

class CDBTeam;
class CDBPlayer;

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
};

CAssessmentTable *GetAssesmentTable();

CDBCompetition *GetCompetition(unsigned char region, unsigned char type, unsigned short index);
CDBCompetition *GetCompetition(CCompID const &id);
CDBCompetition *GetCompetition(unsigned int *id);
CDBCompetition *GetCompetition(unsigned int id);
CDBLeague *GetLeague(CCompID const &id);
CDBLeague *GetLeague(unsigned int *id);
CDBLeague *GetLeague(unsigned int id);
CDBRound *GetRound(unsigned char region, unsigned char type, unsigned short index);
CDBRound *GetRoundByRoundType(unsigned char region, unsigned char type, unsigned char roundType);

wchar_t const *GetTranslation(const char *key);
unsigned short GetCurrentYear();
