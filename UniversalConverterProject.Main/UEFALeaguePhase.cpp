#include "UEFALeaguePhase.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"
#include "shared.h"
#include <random>
#include <chrono>

using namespace plugin;

namespace cldraw {

enum class DrawType {
    DrawCL, DrawEL, DrawCO
};

unsigned char fixturesCL1[] = {
    27,28,16,23,17,5,2,20,9,36,26,22,34,24,29,18,19,33,15,3,6,4,7,31,21,13,25,14,32,1,11,10,12,8,30,35,
    24,30,36,34,10,7,13,17,3,19,1,27,4,25,23,26,33,6,18,11,31,21,28,2,20,32,5,29,22,9,8,16,35,15,14,12,
    17,15,32,25,10,18,28,29,31,33,16,36,7,23,9,3,35,26,11,19,30,13,12,22,27,4,1,2,24,20,6,34,8,5,14,21,
    23,31,33,20,29,32,3,35,19,8,5,13,22,16,9,17,26,6,15,28,18,27,34,30,2,14,4,10,21,24,25,1,7,12,36,11,
    34,12,33,17,13,24,27,11,1,30,2,7,4,8,6,21,26,10,25,36,35,31,32,14,28,16,29,22,19,15,20,3,5,9,23,18,
    31,5,20,19,11,9,13,4,15,26,24,7,18,2,8,28,30,23,12,33,22,35,17,25,10,32,3,1,21,34,16,6,14,29,36,27,
    32,28,11,35,12,13,29,3,15,16,25,23,5,22,33,36,14,1,18,30,8,26,17,31,34,4,10,20,19,27,21,2,7,6,9,24,
    28,19,13,34,3,18,27,25,1,11,2,33,4,32,24,12,31,10,20,17,16,14,22,21,6,15,23,5,35,8,26,29,30,9,36,7
};

unsigned char fixturesEL1[] = {
    10,32,19,5,20,3,24,35,21,17,4,36,33,18,22,8,28,13,12,26,23,6,1,30,7,29,2,27,31,34,14,16,11,15,9,25,
    34,21,13,9,15,24,16,11,25,23,18,28,3,33,8,1,35,20,29,10,30,2,5,4,36,12,27,22,32,7,17,31,26,19,6,14,
    21,32,11,19,7,20,2,34,24,26,13,33,15,18,17,27,25,1,29,8,5,35,36,3,12,4,23,16,14,30,6,31,28,22,9,10,
    30,23,2,8,19,25,31,24,21,9,32,11,33,36,16,6,22,29,26,7,1,15,10,12,20,13,34,28,27,18,4,17,3,5,35,14,
    29,32,10,21,30,15,20,27,34,17,25,14,28,5,3,22,24,2,36,26,13,23,31,16,4,19,33,6,18,1,11,35,8,12,9,7,
    12,29,7,11,27,4,23,21,16,36,17,13,22,10,26,33,35,31,1,3,5,24,19,30,32,25,15,34,14,2,6,9,18,20,8,28,
    21,20,30,29,10,7,5,16,27,28,12,14,19,15,23,36,25,31,35,9,2,13,34,1,32,33,4,6,17,8,22,24,26,11,3,18,
    1,21,7,2,29,27,20,34,24,12,36,30,13,10,31,4,15,5,33,19,16,25,14,22,6,26,18,17,28,35,11,3,8,23,9,32
};

unsigned char fixturesCO1[] = {
    12,17,18,31,7,15,19,29,13,30,32,20,34,22,10,6,11,33,16,36,3,24,1,4,2,26,21,14,28,35,25,27,5,9,23,8,
    36,19,8,25,9,18,29,3,20,16,26,28,4,11,33,23,31,12,27,1,17,34,22,21,30,10,14,32,15,5,35,7,6,2,24,13,
    28,17,30,21,14,15,4,16,10,32,35,13,23,24,36,25,8,3,1,34,9,27,2,12,20,7,26,11,33,29,5,19,6,31,18,22,
    12,28,13,18,7,1,19,20,32,2,34,36,29,30,25,5,11,8,31,26,27,14,24,9,3,35,21,4,22,6,15,33,16,10,17,23,
    13,1,3,5,2,20,32,33,34,8,28,6,14,11,12,7,10,21,15,19,29,18,22,26,4,30,16,17,35,31,23,25,24,27,36,9,
    7,29,8,13,19,12,1,23,9,10,21,35,25,16,30,34,20,28,26,15,33,4,5,36,11,22,31,24,27,32,6,14,17,2,18,3
};

#ifdef CL_DRAWING_TEST
struct Team {
    wstring name;
    wstring countryId;

    Team() {

    }

    Team(wstring _name, wstring _country) {
        name = _name;
        countryId = _country;
    }
};
using CountryIdType = wstring;
#define TEAM_NAME(x) (x).name
#define COUNTRY_NAME(x) x
#else
using Team = CTeamIndex;
using CountryIdType = UChar;
#define TEAM_NAME(x) TeamName(x)
#define COUNTRY_NAME(x) CountryName(x)
#endif

default_random_engine &rng() {
    static unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
    static default_random_engine _rng = default_random_engine(seed);
    return _rng;
}

unsigned int random_number(unsigned int _min, unsigned int _max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> distrib(_min, _max);
    return distrib(gen);
}

struct DrawResult {
    vector<unsigned char> fixtures;
    size_t score = INT32_MAX;
    size_t score_oppsFromSameCountry = INT32_MAX;
};

void CalcDrawResultScore(DrawResult &d, vector<Team> const &teams, vector<pair<unsigned char, unsigned char>> &errors) {
    errors.clear();
    for (size_t i = 0; i < d.fixtures.size() / 2; i++) {
        Team const &t1 = teams[d.fixtures[i * 2] - 1];
        Team const &t2 = teams[d.fixtures[i * 2 + 1] - 1];
        if (t1.countryId == t2.countryId)
            errors.emplace_back(d.fixtures[i * 2], d.fixtures[i * 2 + 1]);
    }
    d.score = errors.size();
}

void CalcDrawResultScore_TooManyOppsFromSameCountry(DrawResult &d, vector<Team> const &teams, vector<pair<unsigned char, pair<CountryIdType, unsigned char>>> &errors) {
    errors.clear();
    d.score_oppsFromSameCountry = 0;
    vector<vector<unsigned char>> opponents(teams.size());
    for (size_t i = 0; i < d.fixtures.size() / 2; i++) {
        opponents[d.fixtures[i * 2] - 1].push_back(d.fixtures[i * 2 + 1]);
        opponents[d.fixtures[i * 2 + 1] - 1].push_back(d.fixtures[i * 2]);
    }
    for (size_t i = 0; i < opponents.size(); i++) {
        if (opponents[i].size()) {
            map<CountryIdType, unsigned char> countryCounter;
            for (unsigned char opponent : opponents[i]) {
                CountryIdType country = teams[opponent - 1].countryId;
                if (countryCounter.count(country))
                    countryCounter[country]++;
                else
                    countryCounter[country] = 1;
            }
            for (auto const &[country, count] : countryCounter) {
                if (count > 2) {
                    d.score_oppsFromSameCountry += (size_t)pow(count - 2, 2);
                    errors.emplace_back((unsigned char)(i + 1), make_pair(country, count));
                }
            }
        }
    }

    d.score = errors.size();
}

DrawResult drawIteration(DrawType type, size_t iteration, size_t numPots, size_t numMatchdays, vector<Team> const &teams) {
    DrawResult d;
    size_t numTeamsInPot = teams.size() / numPots;
    d.fixtures.resize(numMatchdays * teams.size());
    vector<unsigned char> teamIds(teams.size());
    for (size_t i = 0; i < teams.size(); i++)
        teamIds[i] = (unsigned char)(i + 1);
    for (size_t i = 0; i < numPots; i++)
        std::shuffle(teamIds.begin() + numTeamsInPot * i, teamIds.begin() + numTeamsInPot * (i + 1), rng());
    unsigned char *pFixtures = nullptr;
    if (type == DrawType::DrawCL || type == DrawType::DrawEL) {
        if (iteration % 2)
            pFixtures = fixturesEL1;
        else
            pFixtures = fixturesCL1;
    }
    else
        pFixtures = fixturesCO1;
    for (size_t i = 0; i < d.fixtures.size(); i++)
        d.fixtures[i] = teamIds[pFixtures[i] - 1];
    return d;
}

#ifdef CL_DRAWING_TEST
double
#else
void
#endif
draw(DrawType drawType, size_t numPots, size_t numMatchdays, vector<Team> const &teams, DrawResult &d) {
#ifdef CL_DRAWING_TEST
    auto t_start = std::chrono::high_resolution_clock::now();
#endif
    size_t best = INT32_MAX;
    vector<DrawResult> drawResults;
#ifdef CL_DRAWING_TEST
    size_t drawResultsMaxSize = 0;
    size_t iterCount = 0;
#endif
    size_t maxIterations1 = 5'000'000;
    size_t maxIterations2 = 2'000'000;
    if (drawType == DrawType::DrawEL) {
        maxIterations1 = 10'000;
        maxIterations2 = 2'000;
    }
    else if (drawType == DrawType::DrawCO) {
        maxIterations1 = 1'000;
        maxIterations2 = 200;
    }
    for (size_t i = 1; i < maxIterations1; i++) {
#ifdef CL_DRAWING_TEST
        iterCount++;
#endif
        DrawResult d = drawIteration(drawType, i, numPots, numMatchdays, teams);
        vector<pair<unsigned char, unsigned char>> errors;
        CalcDrawResultScore(d, teams, errors);
        if (d.score <= best) {
            if (d.score < best) {
                best = d.score;
                drawResults.clear();
            }
            drawResults.push_back(d);
#ifdef CL_DRAWING_TEST
            if (drawResults.size() > drawResultsMaxSize)
                drawResultsMaxSize = drawResults.size();
#endif
        }
        if (i >= maxIterations2 && best == 0)
            break;
    }

    if (drawResults.empty())
        drawResults.push_back(drawIteration(drawType, 0, numPots, numMatchdays, teams));

    for (auto &d : drawResults) {
        vector<pair<unsigned char, pair<CountryIdType, unsigned char>>> errors;
        CalcDrawResultScore_TooManyOppsFromSameCountry(d, teams, errors);
    }

    std::sort(drawResults.begin(), drawResults.end(), [](DrawResult const &a, DrawResult const &b) {
        return a.score_oppsFromSameCountry < b.score_oppsFromSameCountry;
    });
    d = drawResults[0];

    wstring message;
#ifdef CL_DRAWING_TEST
    message += L"----------------------------------------------------------\n";
    if (drawType == DrawType::DrawCL)
        message += L"-- CHAMPIONS LEAGUE\n";
    else if (drawType == DrawType::DrawEL)
        message += L"-- EUROPA LEAGUE\n";
    else if (drawType == DrawType::DrawCO)
        message += L"-- CONFERENCE LEAGUE\n";
    message += L"----------------------------------------------------------\n\n";
    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    message += L"Drawing took " + to_wstring(elapsed_time_ms / 1000.0) + L" seconds and " + ThousandSeparators(to_wstring(iterCount)) + L" iterations\n";
    message += L"Found " + ThousandSeparators(to_wstring(drawResults.size())) + L" optimal draw " + (drawResults.size() == 1 ? L"result" : L"results") + L"\n";
    message += L"drawResultsMaxSize: " + to_wstring(drawResultsMaxSize) + L"\n\n";
#endif
    vector<pair<unsigned char, unsigned char>> errorSame;
    CalcDrawResultScore(d, teams, errorSame);
    if (!errorSame.empty()) {
        message += L"Matches between teams from same country:\n";
        for (auto &i : errorSame)
            message += TEAM_NAME(teams[i.first - 1]) + L" - " + TEAM_NAME(teams[i.second - 1]) + L"\n";
        message += L"\n";
    }
    vector<pair<unsigned char, pair<CountryIdType, unsigned char>>> errorOpponents;
    CalcDrawResultScore_TooManyOppsFromSameCountry(d, teams, errorOpponents);
    if (!errorOpponents.empty()) {
        message += L"Teams playing too many matches with teams from same country:\n";
        for (auto &i : errorOpponents)
            message += TEAM_NAME(teams[i.first - 1]) + L" will play with " + to_wstring(i.second.second) + L" teams from " + COUNTRY_NAME(i.second.first) + L"\n";
        message += L"\n";
    }
    size_t numMatchesInMatchday = teams.size() / 2;
    for (size_t m = 0; m < numMatchdays; m++) {
        message += L"Matchday " + to_wstring(m + 1) + L":\n";
        for (size_t i = 0; i < numMatchesInMatchday; i++) {
            size_t fixtureIndex = m * numMatchesInMatchday * 2 + i * 2;
            message += to_wstring(i + 1) + L". " + TEAM_NAME(teams[d.fixtures[fixtureIndex] - 1]) + L" - " + TEAM_NAME(teams[d.fixtures[fixtureIndex + 1] - 1]) + L"\n";
        }
        message += L"\n";
    }

#ifdef CL_DRAWING_TEST
    wcout << message;
    return elapsed_time_ms;
#else
    SafeLog::Write(message);
#endif
}

}

bool DrawUEFALeaguePhase(CDBCompetition *poolTeams, CDBCompetition *poolFixtures, UInt numPots, UInt numMatchdays) {
    using namespace cldraw;
    if (poolTeams->GetNumOfRegisteredTeams() != poolTeams->GetNumOfTeams())
        return false;
    DrawType drawType = DrawType::DrawCL;
    if (poolTeams->GetCompetitionType() == COMP_UEFA_CUP)
        drawType = DrawType::DrawEL;
    else if (poolTeams->GetCompetitionType() == COMP_CONFERENCE_LEAGUE)
        drawType = DrawType::DrawCO;
    vector<Team> teams(poolTeams->GetNumOfTeams());
    for (UInt t = 0; t < poolTeams->GetNumOfTeams(); t++)
        teams[t] = poolTeams->GetTeamID(t);

    DrawResult d;
    draw(drawType, numPots, numMatchdays, teams, d);

    CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(poolFixtures, 0xA0);
    memset(pTeamIDs, 0, poolFixtures->GetNumOfTeams() * 4);

    UInt numMatchesInMatchday = teams.size() / 2;
    for (UInt m = 0; m < numMatchdays; m++) {
        for (UInt i = 0; i < numMatchesInMatchday; i++) {
            UInt fixtureIndex = m * numMatchesInMatchday * 2 + i * 2;
            pTeamIDs[m * teams.size() + i] = teams[d.fixtures[fixtureIndex] - 1];
            pTeamIDs[m * teams.size() + i + numMatchesInMatchday] = teams[d.fixtures[fixtureIndex + 1] - 1];
        }
    }
    poolFixtures->SetNumOfRegisteredTeams(poolFixtures->GetNumOfTeams());
    // send e-mails
    for (UInt i = 0; i < poolTeams->GetNumOfRegisteredTeams(); i++) {
        CTeamIndex thisTeam = poolTeams->GetTeamID(i);
        CDBTeam *team = GetTeam(thisTeam);
        if (team && !team->IsManagedByAI()) {
            CEAMailData mailData;
            mailData.SetCompetition(poolTeams->GetCompID());
            Vector<CTeamIndex> opponents;
            for (UInt m = 0; m < numMatchdays; m++) {
                for (UInt mm = 0; mm < numMatchesInMatchday; mm++) {
                    CTeamIndex team1 = pTeamIDs[m * teams.size() + mm];
                    CTeamIndex team2 = pTeamIDs[m * teams.size() + mm + numMatchesInMatchday];
                    if (!team1.isNull() && !team2.isNull()) {
                        if (team1 == thisTeam)
                            opponents.push_back(team2);
                        else if (team2 == thisTeam)
                            opponents.push_back(team1);
                    }
                }
            }
            UInt maxMatchdays = numMatchdays;
            if (maxMatchdays > 8)
                maxMatchdays = 8;
            UInt numOpponents = Utils::Min(opponents.size(), maxMatchdays);
            for (UInt i = 0; i < numOpponents; i++)
                mailData.SetArrayValue(i, opponents[i].ToInt());
            team->SendMail(1917, mailData, 1); // In the next round you have been drawn against: _TEAM_LIST.
        }
    }
    return true;
}

UInt *GetUEFALeaguePhaseMatchdaysCompIDs(UInt compId, UInt &numCompIds) {
    static UInt compIdsCL[] = { 0xF909000A, 0xF909000B, 0xF909000C, 0xF909000D, 0xF909000E, 0xF909000F, 0xF9090010, 0xF9090011 };
    static UInt compIdsEL[] = { 0xF90A0008, 0xF90A0009, 0xF90A000A, 0xF90A000B, 0xF90A000C, 0xF90A000D, 0xF90A000E, 0xF90A000F };
    static UInt compIdsCO[] = { 0xF933000C, 0xF933000D, 0xF933000E, 0xF933000F, 0xF9330010, 0xF9330011 };
    static UInt compIdsYL[] = { 0xF9260003, 0xF9260004, 0xF9260005, 0xF9260006, 0xF9260007, 0xF9260008 };
    UInt *compIds = nullptr;
    if (compId == 0xF9090000) {
        compIds = compIdsCL;
        numCompIds = std::size(compIdsCL);
    }
    else if (compId == 0xF90A0000) {
        compIds = compIdsEL;
        numCompIds = std::size(compIdsEL);
    }
    else if (compId == 0xF9330000) {
        compIds = compIdsCO;
        numCompIds = std::size(compIdsCO);
    }
    else if (compId == 0xF9260000) {
        compIds = compIdsYL;
        numCompIds = std::size(compIdsYL);
    }
    else
        numCompIds = 0;
    return compIds;
}

Bool IsUEFALeaguePhaseCompID(CCompID const &compID) {
    return compID.ToInt() == 0xF9090008 || compID.ToInt() == 0xF90A0006 || compID.ToInt() == 0xF933000A || compID.ToInt() == 0xF9260001;
}

Bool IsUEFALeaguePhaseMatchdayCompID(CCompID const &compID) {
    if (compID.type == COMP_CHAMPIONSLEAGUE || compID.type == COMP_UEFA_CUP || compID.type == COMP_CONFERENCE_LEAGUE || compID.type == COMP_YOUTH_CHAMPIONSLEAGUE) {
        UInt compId = compID.ToInt();
        UInt numCompIds = 0;
        UInt *compIds = GetUEFALeaguePhaseMatchdaysCompIDs(compId & 0xFFFF0000, numCompIds);
        for (UInt i = 0; i < numCompIds; i++) {
            if (compId == compIds[i])
                return true;
        }
    }
    return false;
}

Int METHOD OnStatsCupFixturesResultsAddOneCompetition(void *t, DUMMY_ARG, UInt *baseCompId, UInt *compId, CDBCompetition *comp) {
    Int result = CallMethodAndReturn<Int, 0x702BC0>(t, baseCompId, compId, comp);
    if (*compId == 0xF9090011 || *compId == 0xF90A000F || *compId == 0xF9330011 || *compId == 0xF9260008) { // last League Phase matchday ID
        void *comboBox = *raw_ptr<void *>(t, 0x4D8);
        CDBCompetition *baseComp = GetCompetition(*baseCompId);
        CallVirtualMethod<83>(comboBox, Format(L"%s (%s)", baseComp->GetName(), GetTranslation("ID_LEAGUE_PHASE_TABLE")).c_str(), *compId & 0xFFFF0000, 0);
    }
    return result;
}

Vector<TeamLeaguePhaseInfo> SortUEFALeaguePhaseTable(UInt compId, CDBCompetition *comp) {
    Vector<TeamLeaguePhaseInfo> vecTeams;
    if (compId == 0xF9090000 || compId == 0xF90A0000 || compId == 0xF9330000 || compId == 0xF9260000) {
        Map<UInt, TeamLeaguePhaseInfo> teams;
        UInt numCompIds = 0;
        UInt *compIds = GetUEFALeaguePhaseMatchdaysCompIDs(compId, numCompIds);
        for (UInt c = 0; c < numCompIds; c++) {
            CDBCompetition *comp = GetCompetition(compIds[c]);
            if (comp && comp->GetDbType() == DB_ROUND && comp->IsLaunched()) {
                CDBRound *r = (CDBRound *)comp;
                for (UInt i = 0; i < r->GetNumOfPairs(); i++) {
                    RoundPair rp;
                    memset(&rp, 0, sizeof(RoundPair));
                    r->GetRoundPair(i, rp);
                    if (rp.AreTeamsValid()) {
                        TeamLeaguePhaseInfo &team1 = teams[rp.m_n1stTeam.ToInt()];
                        TeamLeaguePhaseInfo &team2 = teams[rp.m_n2ndTeam.ToInt()];
                        team1.goalsFor += rp.result1[0];
                        team1.goalsAgainst += rp.result2[0];
                        team2.goalsFor += rp.result2[0];
                        team2.goalsAgainst += rp.result1[0];
                        if (team1.goalsFor > team1.goalsAgainst) {
                            team1.wins += 1;
                            team2.losses += 1;
                            team1.points += 3;
                        }
                        else if (team1.goalsFor < team1.goalsAgainst) {
                            team2.wins += 1;
                            team1.losses += 1;
                            team2.points += 3;
                        }
                        else {
                            team1.draws += 1;
                            team2.draws += 1;
                            team1.points += 1;
                            team2.points += 1;
                        }
                    }
                }
            }
        }
        if (!teams.empty()) {
            for (auto &[k, v] : teams) {
                v.teamId = CTeamIndex::make(k);
                vecTeams.push_back(v);
            }
            std::sort(vecTeams.begin(), vecTeams.end(), [](TeamLeaguePhaseInfo const &a, TeamLeaguePhaseInfo const &b) {
                if (a.points > b.points)
                    return true;
                if (b.points > a.points)
                    return false;
                Int gd1 = a.goalsFor - a.goalsAgainst;
                Int gd2 = b.goalsFor - b.goalsAgainst;
                if (gd1 > gd2)
                    return true;
                if (gd2 > gd1)
                    return false;
                if (a.goalsFor > b.goalsFor)
                    return true;
                if (b.goalsFor > a.goalsFor)
                    return false;
                return true;
            });
        }
    }
    if (comp) {
        UInt numTeams = Utils::Min(comp->GetNumOfTeams(), vecTeams.size());
        CTeamIndex *pTeamIDs = *raw_ptr<CTeamIndex *>(comp, 0xA0);
        memset(pTeamIDs, 0, comp->GetNumOfTeams() * 4);
        CAssessmentTable *table = GetAssesmentTable();
        Float points = 0.0f;
        Float pointsIncrease1to8 = 0.25f;
        Float pointsIncrease9to24 = 0.25f;
        if (compId == 0xF9330000)
            pointsIncrease9to24 = 0.125f;
        for (UInt i = numTeams; i > 0; i--) {
            pTeamIDs[i - 1] = vecTeams[i - 1].teamId;
            if (table && compId != 0xF9260000) {
                if (i >= 1 && i <= 8)
                    points += pointsIncrease1to8;
                else if (i >= 9 && i <= 24)
                    points += pointsIncrease9to24;
                if (points > 0.0f) {
                    UChar countryId = GetTeamCountryId_LiechtensteinCheck(pTeamIDs[i - 1]);
                    if (countryId >= 1 && countryId <= 207) {
                        table->GetInfoForCountry(countryId)->AddPoints(points);
                        SafeLog::Write(Utils::Format(L"Added %g points for %s (team: %s)",
                            points, CountryName(countryId), TeamName(GetTeam(pTeamIDs[i - 1]))));
                    }
                }
            }
        }
        comp->SetNumOfRegisteredTeams(numTeams);
    }
    return vecTeams;
}

void METHOD OnStatsCupFixturesResultsChangeCompetition(void *t) {
    CallMethod<0x7024C0>(t);
    if (*raw_ptr<UInt>(t, 0x490) == 0)
        return;
    void *data = *raw_ptr<void *>(t, 0x19F0);
    UInt compId = *raw_ptr<UInt>(data, 0x24);
    void *listBox = raw_ptr<void *>(t, 0x1A00);
    Bool specialListBox = false;
    ::Warning(L"%X", compId);
    if (compId == 0xF9090000 || compId == 0xF90A0000 || compId == 0xF9330000 || compId == 0xF9260000) {
        specialListBox = true;
        CallMethod<0xD1AF40>(listBox); // CFMListBox::Clear()
        auto vecTeams = SortUEFALeaguePhaseTable(compId, nullptr);
        if (!vecTeams.empty()) {
            for (UInt i = 0; i < vecTeams.size(); i++) {
                Int lastRowIndex = CallMethodAndReturn<Int, 0xD18640>(listBox) - 1; // CFMListBox::GetMaxRows()
                if (CallMethodAndReturn<Int, 0xD18600>(listBox) >= lastRowIndex) // CFMListBox::GetRowsCount()
                    break;
                UInt teamId = vecTeams[i].teamId.ToInt();
                TeamLeaguePhaseInfo &info = vecTeams[i];
                UInt color = CallMethodAndReturn<UInt, 0xA8BD80>(data, teamId);
                CallMethod<0xD22BE0>(listBox, (Int64)(i + 1), color, 0); // CFMListBox::AddColumnText() - position
                CallMethod<0xD1E620>(listBox, &teamId); // CFMListBox::AddTeamWidget
                CallMethod<0xD1F060>(listBox, &teamId, color, 0); // CFMListBox::AddTeamName
                CallMethod<0xD22BE0>(listBox, (Int64)(info.wins + info.draws + info.losses), color, 0); // CFMListBox::AddColumnText() - games
                CallMethod<0xD22BE0>(listBox, (Int64)info.wins, color, 0); // CFMListBox::AddColumnText() - wins
                CallMethod<0xD22BE0>(listBox, (Int64)info.draws, color, 0); // CFMListBox::AddColumnText() - draws
                CallMethod<0xD22BE0>(listBox, (Int64)info.losses, color, 0); // CFMListBox::AddColumnText() - loses
                CallMethod<0xD22BE0>(listBox, (Int64)info.goalsFor, color, 0); // CFMListBox::AddColumnText() - goals scored
                CallMethod<0xD22BE0>(listBox, (Int64)info.goalsAgainst, color, 0); // CFMListBox::AddColumnText() - goals against
                CallMethod<0xD22BE0>(listBox, (Int64)(info.goalsFor - info.goalsAgainst), color, 0); // CFMListBox::AddColumnText() - goal diff
                CallMethod<0xD22BE0>(listBox, (Int64)info.points, color, 0); // CFMListBox::AddColumnText() - points
                CallMethod<0xD18920>(listBox, 0); // CFMListBox::NextRow()
            }
        }
    }
    CallVirtualMethod<20>(listBox, specialListBox);
    if (specialListBox) {
        CallVirtualMethod<20>(*raw_ptr<void *>(data, 0x4), 0);
        CallVirtualMethod<20>(*raw_ptr<void *>(data, 0x8), 0);
        CallVirtualMethod<20>(*raw_ptr<void *>(data, 0x10), 0);
    }
}

void *METHOD OnStatsCupFixturesResultsCtor(void *t, DUMMY_ARG, void *guiInstance, UInt flags) {
    CallMethod<0x6E3EC0>(t, guiInstance, flags);
    CallMethod<0xD1AC00>(raw_ptr<void *>(t, 0x1A00));
    return t;
}

void *METHOD OnStatsCupFixturesResultsDtor(void *t) {
    CallMethod<0xD182F0>(raw_ptr<void *>(t, 0x1A00));
    CallMethod<0x6E3A80>(t);
    return t;
}

void *METHOD OnStatsCupFixturesCreateUI(void *t) {
    CallMethod<0xD4F110>(t);
    void *listBox = raw_ptr<void *>(t, 0x1A00);
    CallMethod<0xD1EEE0>(listBox, t, "ListBox3"); // CFMListBox::Create()
    Call<0xD19660>(listBox, 9, 4, 4, 9, 9, 9, 9, 9, 9, 9, 9, 63);
    Call<0xD196A0>(listBox, 210, 210, 204, 210, 210, 210, 210, 210, 210, 210, 210, 228);
    CallVirtualMethod<20>(listBox, 0);
    return t;
}

UInt METHOD CupDrawMailMessage_GetNumOfTeams(CDBCompetition *comp) {
    return IsUEFALeaguePhaseMatchdayCompID(comp->GetCompID()) ? 0 : comp->GetNumOfTeams();
}

void MyTeamListHandler(Int callbackArg, CEAMailData const &mailData, WideChar *out) {
    if (IsUEFALeaguePhaseCompID(mailData.GetCompetition())) {
        *out = L'\0';
        UInt numTeams = 0;
        for (UInt i = 0; i < 8; ++i) {
            CTeamIndex teamID = CTeamIndex::make(mailData.GetArrayValue(i));
            if (!teamID.isNull()) {
                CDBTeam *team = GetTeam(teamID);
                if (team) {
                    if (numTeams != 0)
                        wcscat(out, L", ");
                    wcscat(out, team->GetClickableTeamName(teamID));
                    numTeams++;
                }
            }
        }
    }
    else
        Call<0x14F35B2>(callbackArg, &mailData, out);
}

Int METHOD CupDrawMailMessage_GetMailID(void *t) {
    Int mailID = CallMethodAndReturn<Int, 0x100E1E0>(t);
    if (mailID == 1917) {
        CEAMailData *mailData = raw_ptr<CEAMailData>(t, 0x18);
        if (IsUEFALeaguePhaseCompID(mailData->GetCompetition()))
            return 0;
    }
    return mailID;
}

void PatchUEFALeaguePhase(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x70403C, OnStatsCupFixturesResultsAddOneCompetition);
        patch::SetPointer(0x23F18D8, OnStatsCupFixturesResultsChangeCompetition);
        patch::SetUInt(0x703DA4 + 1, 0x1A00 + 0x704);
        patch::SetUInt(0x703DAB + 1, 0x1A00 + 0x704);
        patch::RedirectCall(0x703BA7, OnStatsCupFixturesResultsCtor);
        patch::RedirectCall(0x703D13, OnStatsCupFixturesResultsDtor);
        patch::RedirectCall(0x702135, OnStatsCupFixturesCreateUI);

        // disable "Cup Draw: _COMPETITION" e-mail message for CL/EL/CO League Phase matchdays
        patch::RedirectCall(0x1045264, CupDrawMailMessage_GetNumOfTeams);
        // update the _TEAM_LIST behavior
        patch::SetPointer(0x309EB78, MyTeamListHandler);
        // remove the deletion of "Cup Draw: _COMPETITION" e-mail message for CL/EL/CO League Phase matchdays
        //patch::RedirectCall(0x8CFE45, CupDrawMailMessage_GetMailID);
    }
}
