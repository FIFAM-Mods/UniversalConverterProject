#include "GameInterfaces.h"

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

CTeamIndex &CDBCompetition::GetChampion(bool checkIntl) {
    return *plugin::CallMethodAndReturn<CTeamIndex *, 0xF85B30>(this, checkIntl);
}

unsigned char CDBCompetition::GetNumTeamsFromCountry(int countryId) {
    return plugin::CallVirtualMethodAndReturn<unsigned char, 14>(this, countryId);
}

void CDBCompetition::AddTeamsFromCountry(unsigned char countryId, unsigned int numTeams) {
    plugin::CallMethod<0x139EB60>(this, countryId, numTeams);
}

bool CDBCompetition::AddTeam(CTeamIndex const &teamIndex, int position) {
    return plugin::CallVirtualMethodAndReturn<bool, 26>(this, &teamIndex, position);
}

bool CDBCompetition::IsTeamPresent(CTeamIndex const &teamIndex) {
    return plugin::CallMethodAndReturn<bool, 0xF82400>(this, &teamIndex);
}

void CDBLeague::SetStartDate(CJDate date) {
    plugin::CallMethod<0x1054390>(this, date);
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

unsigned char CAssessmentTable::GetCountryIdAtPosition(int position) {
    return plugin::CallMethodAndReturn<unsigned char, 0x121D030>(this, position);
}

void CCompID::SetFromInt(unsigned int value) {
    countryId = (value >> 24) & 0xFF;
    type = (value >> 16) & 0xFF;
    index = value & 0xFFFF;
}

unsigned int CCompID::ToInt() const {
    return index | (type << 16) | (countryId << 24);
}
