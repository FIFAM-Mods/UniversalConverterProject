#include "EuropeanGoldenShoe.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "FifamReadWrite.h"
#include "FifamContinent.h"
#include "FifamNames.h"
#include "Utils.h"
#include "InterfaceTheme.h"
#include "DebugPrint.h"
#include "shared.h"
#include "TextFileTable.h"
#include "FifaWorldPlayerGala.h"
#include "FifamBeg.h"
#include "Competitions.h"

using namespace plugin;

enum CompWinConfigFile { COMPWINCONFIG_WC, COMPWINCONFIG_EC };

struct GoldenShoeWinner {
    UShort year;
    NameDesc name;
    UChar countryId;
    UChar goals;
    UShort points;
    CJDate birthdate;
    UInt empicsId;
    UInt clubId;
};

struct MyHistoricalPlayerEntry {
    UShort year;
    NameDesc name;
    CTeamIndex teamID;
    UInt clubId;
    UChar age;
    UChar position;
    Char _pad12[2];
    UInt empicsId;
    UInt playerId;
    CJDate birthdate;
    UChar countryId;
    Char _pad21[3];
};

static_assert(sizeof(MyHistoricalPlayerEntry) == 0x24, "Failed");

const UInt PLAYER_STATS_GS_AND_BD_OFFSET = 0x7B;
const UInt LIST_OFFSET_BALLONDOR = 0xA570;
const UInt LIST_OFFSET_BESTEUROPEAN = 0xA554;
const UInt LIST_OFFSET_FIFAWORLD = 0xA58C;

void DumpPlayerList(FmList<HistoricalPlayerEntry> &lst, Path const &filename) {
    TextFileTable table;
    table.AddRow("year", "name", "team", "country");
    for (auto &i : lst)
        table.AddRow(i.year, i.name.ToName(), TeamName(i.teamID), CountryName(i.countryId));
    table.WriteUnicodeText(filename);
}

void DumpPlayerList(Path const &filename, UInt ListOffset) {
    DumpPlayerList(*raw_ptr<FmList<HistoricalPlayerEntry>>(CDBGame::GetInstance(), ListOffset), filename);
}

void PlayerStatsSetGoldenShoeWins(CPlayerStats *stats, UChar wins) {
    UChar value = *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET);
    if (wins > 15)
        wins = 15;
    *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET) = (value & 0xF) | (wins << 4);
}

UChar PlayerStatsGetGoldenShoeWins(CPlayerStats *stats) {
    UChar value = *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET);
    UChar wins = (value >> 4) & 0xF;
    return wins;
}

void PlayerStatsSetBallonDOrWins(CPlayerStats *stats, UChar wins) {
    UChar value = *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET);
    if (wins > 15)
        wins = 15;
    *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET) = (wins & 0xF) | (value & 0xF0);
}

UChar PlayerStatsGetBallonDOrWins(CPlayerStats *stats) {
    UChar value = *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET);
    UChar wins = value & 0xF;
    return wins;
}

void PlayerStatsInitAdditionalStats(CPlayerStats *stats) {
    *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET) = 0;
}

static_assert(sizeof(GoldenShoeWinner) == 0x18, "Failed");

Vector<GoldenShoeWinner> &GoldenShoeWinners() {
    static Vector<GoldenShoeWinner> vec;
    return vec;
}

void ClearGoldenShoeWinners() {
    GoldenShoeWinners().clear();
}

void ReadGoldenShoeConfiig() {
    ClearGoldenShoeWinners();
    FifamReader r(L"fmdata\\historic\\EuropeanGoldenShoe.txt");
    if (r.Available()) {
        r.SkipLine();
        while (!r.IsEof()) {
            if (!r.EmptyLine()) {
                String firstName, lastName, clubName, pointsStr;
                UInt clubId = 0;
                UShort year = 0, goals = 0;
                UChar countryId = 0;
                Date birthdate;
                r.ReadLineWithSeparator(L'\t', year, lastName, firstName, birthdate, countryId, Hexadecimal(clubId), clubName, goals, pointsStr);
                if (year > 0 && year <= GetStartingYear()) {
                    GoldenShoeWinner winner;
                    winner.year = year;
                    if (goals > 256)
                        goals = 256;
                    winner.goals = (UChar)goals;
                    winner.points = 0;
                    if (!pointsStr.empty() && pointsStr != L"-") {
                        Float points = Utils::SafeConvertFloat(pointsStr);
                        if (points > 300.0f)
                            points = 300.0f;
                        winner.points = (UShort)(points * 100.0f);
                    }
                    winner.countryId = countryId;
                    winner.birthdate.Set(birthdate.year, birthdate.month, birthdate.day);
                    winner.name.languageId = GetCountryFirstLanguage(countryId);
                    String pseudonym;
                    if (firstName.empty())
                        pseudonym = lastName;
                    String playerStringID = FifamNames::GetPersonStringId(13, firstName, lastName, pseudonym, birthdate, 0);
                    CDBPlayer *player = FindPlayerByStringID(playerStringID.c_str());
                    if (player) {
                        winner.name = player->GetNameDesc();
                        winner.empicsId = player->GetEmpicsID();
                        PlayerStatsSetGoldenShoeWins(player->GetStats(), PlayerStatsGetGoldenShoeWins(player->GetStats()) + 1);
                    }
                    else {
                        if (firstName.empty()) {
                            winner.name.firstName = GetNamePools()->AddName(winner.name.languageId, 2, lastName.c_str());
                            winner.name.lastName = 0;
                            winner.name.flags = 2;
                        }
                        else {
                            winner.name.firstName = GetNamePools()->AddName(winner.name.languageId, 1, firstName.c_str());
                            winner.name.lastName = GetNamePools()->AddName(winner.name.languageId, 4, lastName.c_str());
                            winner.name.flags = 0;
                        }
                        winner.empicsId = 0;
                    }
                    winner.clubId = clubId;
                    GoldenShoeWinners().push_back(winner);
                }
            }
            else
                r.SkipLine();
        }
    }
}

void AddPlayerTrophy(ePlayerTrophy type, UShort year, String lastName, String firstName, Date birthdate,
    UChar countryId, UInt clubId, Bool searchForRealPlayer)
{
    MyHistoricalPlayerEntry winner;
    memset(&winner, 0, sizeof(MyHistoricalPlayerEntry));
    winner.year = year;
    winner.countryId = countryId;
    winner.name.languageId = GetCountryFirstLanguage(countryId);
    String pseudonym;
    if (firstName.empty())
        pseudonym = lastName;
    CDBPlayer *player = nullptr;
    if (searchForRealPlayer) {
        String playerStringID = FifamNames::GetPersonStringId(13, firstName, lastName, pseudonym, birthdate, 0);
        player = FindPlayerByStringID(playerStringID.c_str());
    }
    if (player) {
        winner.name = player->GetNameDesc();
        winner.playerId = player->GetID();
        winner.empicsId = player->GetEmpicsID();
        if (type == PLAYERTROPHY_BALLONDOR)
            PlayerStatsSetBallonDOrWins(player->GetStats(), PlayerStatsGetBallonDOrWins(player->GetStats()) + 1);
        else if (type == PLAYERTROPHY_BESTINEUROPE)
            player->GetStats()->SetNumEuropeanPlayerOfTheYear(player->GetStats()->GetNumEuropeanPlayerOfTheYear() + 1);
    }
    else {
        if (firstName.empty()) {
            winner.name.firstName = GetNamePools()->AddName(winner.name.languageId, 2, lastName.c_str());
            winner.name.lastName = 0;
            winner.name.flags = 2;
        }
        else {
            winner.name.firstName = GetNamePools()->AddName(winner.name.languageId, 1, firstName.c_str());
            winner.name.lastName = GetNamePools()->AddName(winner.name.languageId, 4, lastName.c_str());
            winner.name.flags = 0;
        }
    }
    auto team = GetTeamByUniqueID(clubId);
    if (team)
        winner.teamID = team->GetTeamID();
    winner.clubId = clubId;
    winner.birthdate.Set(birthdate.year, birthdate.month, birthdate.day);

    if (type == PLAYERTROPHY_BALLONDOR)
        CallMethod<0xF5C020>(raw_ptr<FmList<HistoricalPlayerEntry>>(CDBGame::GetInstance(), LIST_OFFSET_BALLONDOR), &winner);
    else if (type == PLAYERTROPHY_BESTINEUROPE)
        CallMethod<0xF5C020>(raw_ptr<FmList<HistoricalPlayerEntry>>(CDBGame::GetInstance(), LIST_OFFSET_BESTEUROPEAN), &winner);
}

void AddPlayerTrophy(ePlayerTrophy type, UShort year, CDBPlayer *player) {
    MyHistoricalPlayerEntry winner;
    memset(&winner, 0, sizeof(MyHistoricalPlayerEntry));
    winner.year = year;
    winner.countryId = player->GetNationality();
    winner.name = player->GetNameDesc();
    winner.playerId = player->GetID();
    winner.empicsId = player->GetEmpicsID();
    if (type == PLAYERTROPHY_BALLONDOR)
        PlayerStatsSetBallonDOrWins(player->GetStats(), PlayerStatsGetBallonDOrWins(player->GetStats()) + 1);
    else if (type == PLAYERTROPHY_BESTINEUROPE)
        player->GetStats()->SetNumEuropeanPlayerOfTheYear(player->GetStats()->GetNumEuropeanPlayerOfTheYear() + 1);
    winner.teamID = player->GetCurrentTeam();
    if (!winner.teamID.isNull()) {
        auto team = GetTeam(winner.teamID);
        if (team)
            winner.clubId = team->GetTeamUniqueID();
    }
    winner.birthdate = player->GetBirthdate();
    if (type == PLAYERTROPHY_BALLONDOR)
        CallMethod<0xF5C020>(raw_ptr<FmList<HistoricalPlayerEntry>>(CDBGame::GetInstance(), LIST_OFFSET_BALLONDOR), &winner);
    else if (type == PLAYERTROPHY_BESTINEUROPE)
        CallMethod<0xF5C020>(raw_ptr<FmList<HistoricalPlayerEntry>>(CDBGame::GetInstance(), LIST_OFFSET_BESTEUROPEAN), &winner);
}

void ReadPlayerTrophyConfiig(Path const &filename, ePlayerTrophy type) {
    FifamReader r(filename);
    if (r.Available()) {
        r.SkipLine();
        while (!r.IsEof()) {
            if (!r.EmptyLine()) {
                String firstName, lastName, clubName;
                UInt clubId = 0;
                UShort year = 0;
                UChar countryId = 0;
                Date birthdate;
                r.ReadLineWithSeparator(L'\t', year, lastName, firstName, birthdate, countryId, Hexadecimal(clubId), clubName);
                if (year > 0 && year < GetStartingYear())
                    AddPlayerTrophy(type, year, lastName, firstName, birthdate, countryId, clubId);
            }
            else
                r.SkipLine();
        }
    }
}

void ReadPlayerCompWinConfiig(Path const &filename, CompWinConfigFile type) {
    FifamReader r(filename);
    if (r.Available()) {
        r.SkipLine();
        while (!r.IsEof()) {
            if (!r.EmptyLine()) {
                String firstName, lastName;
                UShort year = 0;
                Date birthdate;
                r.ReadLineWithSeparator(L'\t', year, lastName, firstName, birthdate);
                if (year > 0 && year <= GetStartingYear()) {
                    String pseudonym;
                    if (firstName.empty())
                        pseudonym = lastName;
                    String playerStringID = FifamNames::GetPersonStringId(13, firstName, lastName, pseudonym, birthdate, 0);
                    CDBPlayer *player = FindPlayerByStringID(playerStringID.c_str());
                    if (player) {
                        if (type == COMPWINCONFIG_WC)
                            player->GetStats()->AddWorldCupWin();
                        else if (type == COMPWINCONFIG_EC)
                            player->GetStats()->AddEuroCupWin();
                    }
                }
            }
            else
                r.SkipLine();
        }
    }
}

void METHOD OnReadFifaWorldPlayers(CDBGame *game) {
    CallMethod<0xF67230>(game);
    ReadPlayerTrophyConfiig(L"fmdata\\historic\\BallonDOr.txt", PLAYERTROPHY_BALLONDOR);
    ReadPlayerTrophyConfiig(L"fmdata\\historic\\BestPlayerInEurope.txt", PLAYERTROPHY_BESTINEUROPE);
    ReadPlayerCompWinConfiig(L"fmdata\\historic\\WorldCupWinners.txt", COMPWINCONFIG_WC);
    ReadPlayerCompWinConfiig(L"fmdata\\historic\\EuroCupWinners.txt", COMPWINCONFIG_WC);
    ReadGoldenShoeConfiig();
}

struct GoldenBootPlayerInfo {
    UInt playerId = 0;
    Float points = 0.0f;
    UShort minutesPlayed = 0;
    UChar goals = 0;
    UChar assists = 0;
    UChar penalties = 0;
    Vector<CTeamIndex> teams;
};

Bool GoldenBootPlayerInfoSorter(GoldenBootPlayerInfo const &a, GoldenBootPlayerInfo const &b) {
    if (a.points > b.points) return true;
    if (b.points > a.points) return false;
    if (a.minutesPlayed < b.minutesPlayed) return true;
    if (b.minutesPlayed < a.minutesPlayed) return false;
    if (a.assists > b.assists) return true;
    if (b.assists > a.assists) return false;
    if (a.penalties < b.penalties) return true;
    if (b.penalties < a.penalties) return false;
    return false;
}

Float GetGoalsMultiplier(UChar countryId) {
    Int pos = GetAssesmentTable()->GetCountryPosition(countryId);
    if (pos >= 0) {
        pos += 1;
        if (pos >= 22)
            return 1.0f;
        if (pos >= 6)
            return 1.5f;
        return 2.0f;
    }
    return 0.0f;
}

Float GetGoalsMultiplierForPlayerFromNonPlayableCountry(UInt playerId, CJDate const &seasonStartDate) {
    Float currentMP = 0.0f;
    CDBPlayerCareerList *cl = (CDBPlayerCareerList *)GetObjectByID(GetIDForObject(2, playerId));
    if (cl) {
        for (UInt e = 0; e < cl->GetNumEntries(); e++) {
            if (cl->GetEndDate(e).IsNull() || cl->GetEndDate(e) > seasonStartDate) {
                CTeamIndex teamID = cl->GetTeamID(e);
                if (teamID.isNull()
                    || GetCountry(teamID.countryId)->GetContinent() != FifamContinent::Europe
                    || Game()->IsCountryPlayable(teamID.countryId))
                {
                    return 0.0f;
                }
                CDBTeam *team = GetTeam(teamID);
                if (!team || team->GetFirstTeamDivision() != 0)
                    return 0.0f;
                Float mp = GetGoalsMultiplier(teamID.countryId);
                if (mp == 0.0f)
                    return 0.0f;
                if (currentMP == 0.0f)
                    currentMP = mp;
                else if (currentMP != mp)
                    return 0.0f;
            }
        }
    }
    return currentMP;
}

Bool GetPlayerEuropeanTeamsThisSeason(UInt playerId, CJDate const &seasonStartDate, Vector<CTeamIndex> &teams) {
    teams.clear();
    CDBPlayerCareerList *cl = (CDBPlayerCareerList *)GetObjectByID(GetIDForObject(2, playerId));
    if (cl) {
        Set<UInt> teamIDs;
        for (UInt e = 0; e < cl->GetNumEntries(); e++) {
            if (cl->GetEndDate(e).IsNull() || cl->GetEndDate(e) > seasonStartDate) {
                CTeamIndex teamID = cl->GetTeamID(e);
                if (!teamID.isNull() && GetCountry(teamID.countryId)->GetContinent() == FifamContinent::Europe) {
                    auto team = GetTeam(teamID);
                    if (team && team->GetFirstTeamDivision() == 0)
                        teamIDs.insert(teamID.firstTeam().ToInt());
                }
            }
        }
        for (auto const &teamID : teamIDs)
            teams.push_back(CTeamIndex::make(teamID));
    }
    return !teams.empty();
}

Float LimitTopPlayersList(Map<UInt, Float> &topList) {
    Vector<Pair<UInt, Float>> players(topList.begin(), topList.end());
    Utils::Sort(players, [](Pair<UInt, Float> const &a, Pair<UInt, Float> const &b) {
        return a.second > b.second;
    });
    if (players.size() <= 100) {
        topList = Map<UInt, Float>(players.begin(), players.end());
        return players.empty() ? 0.0f : players.back().second;
    }
    Float threshold = players[99].second;
    UInt count = 100;
    while (count < players.size() && players[count].second == threshold)
        ++count;
    players.resize(count > 500 ? 500 : count);
    topList = Map<UInt, Float>(players.begin(), players.end());
    return threshold;
}

struct PlayerAppInfo {
    UChar minuteIn = 0, minuteOut = 0;
    Bool onTheBench = false, homeTeam = false;
};

Pair<UInt, Vector<GoldenBootPlayerInfo>> CalcGoldenBootWinner() {
    // Phase 1. Find all goalscorers in playable countries
    Map<UInt, Float> playersWithGoals;
    for (UInt countryId = 1; countryId <= 207; countryId++) {
        auto country = GetCountry(countryId);
        if (country->GetContinent() == FifamContinent::Europe) {
            CCompID leagueID = CCompID::Make(countryId, COMP_LEAGUE, 0);
            auto firstLeague = GetLeague(leagueID);
            if (firstLeague && Game()->IsCountryPlayable(countryId)) {
                Vector<CDBLeague *> leaguesToCheck;
                leaguesToCheck.push_back(firstLeague);
                if (IsCompetitionLeagueSplit(leagueID)) {
                    for (UInt i = 0; i < 2; i++) {
                        auto relegation = GetLeague(CCompID::Make(countryId, COMP_RELEGATION, i));
                        if (relegation)
                            leaguesToCheck.push_back(relegation);
                    }
                }
                for (auto league : leaguesToCheck) {
                    auto root = league->GetRoot();
                    Float mp = GetGoalsMultiplier(countryId);
                    for (UInt matchday = 0; matchday < league->GetNumMatchdays(); matchday++) {
                        for (UInt matchIndex = 0; matchIndex < league->GetMatchesInMatchday(); matchIndex++) {
                            CMatch match;
                            league->GetMatch(matchday, matchIndex, match);
                            if (match.CheckFlag(FifamBeg::_1stPlayed)) {
                                CDBMatchEventEntry event;
                                Int eventIndex = match.GetMatchEventsStartIndex();
                                root->GetMatchEvent(eventIndex, event);
                                while (event.GetEventType() != MET_END) {
                                    if (event.GetEventType() == MET_GOAL && event.GetMinute() != 250 && event.GetPlayerInitiator())
                                        playersWithGoals[event.GetPlayerInitiator()] += mp;
                                    root->GetMatchEvent(++eventIndex, event);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    Float thresholdPoints = LimitTopPlayersList(playersWithGoals);
    // Phase 2. Find all goalscorers in non-playable countries
    CJDate seasonStartDate = Game()->GetCurrentSeasonStartDate();
    for (UInt countryId = 1; countryId <= 207; countryId++) {
        auto country = GetCountry(countryId);
        if (country->GetContinent() == FifamContinent::Europe) {
            auto league = GetLeague(countryId, COMP_LEAGUE, 0);
            if (league) {
                for (UInt t = 0; t < league->GetNumOfTeams(); t++) {
                    auto team = GetTeam(league->GetTeamID(t));
                    if (team) {
                        for (UInt p = 0; p < team->GetNumPlayers(); p++) {
                            auto playerId = team->GetPlayer(p);
                            if (playerId && !Utils::Contains(playersWithGoals, playerId)) {
                                auto player = GetPlayer(playerId);
                                if (player) {
                                    CDBMatchesGoalsLeagueList *mgl = (CDBMatchesGoalsLeagueList *)GetObjectByID(GetIDForObject(3, player->GetID()));
                                    if (mgl) {
                                        UChar totalGoals = 0;
                                        for (Int e = 0; e < mgl->GetNumEntries(); e++) {
                                            if (mgl->GetYear(e) == GetCurrentSeasonStartYear())
                                                totalGoals += mgl->GetNumGoals(e, MG_COMP_LEAGUE);
                                        }
                                        if (totalGoals > 0) {
                                            Float mp = GetGoalsMultiplierForPlayerFromNonPlayableCountry(playerId, seasonStartDate);
                                            if (mp > 0.0f) {
                                                Float points = totalGoals * mp;
                                                if (points >= thresholdPoints)
                                                    playersWithGoals[playerId] = points;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    thresholdPoints = LimitTopPlayersList(playersWithGoals);
    // Phase 3. Find additional info for top goalscorers (goals, assists, minutes played, penalties)
    Map<UInt, GoldenBootPlayerInfo> candidates;
    for (UInt countryId = 1; countryId <= 207; countryId++) {
        auto country = GetCountry(countryId);
        if (country->GetContinent() == FifamContinent::Europe) {
            CCompID leagueID = CCompID::Make(countryId, COMP_LEAGUE, 0);
            auto firstLeague = GetLeague(leagueID);
            if (firstLeague && Game()->IsCountryPlayable(countryId)) {
                Vector<CDBLeague *> leaguesToCheck;
                leaguesToCheck.push_back(firstLeague);
                if (IsCompetitionLeagueSplit(leagueID)) {
                    for (UInt i = 0; i < 2; i++) {
                        auto relegation = GetLeague(CCompID::Make(countryId, COMP_RELEGATION, i));
                        if (relegation)
                            leaguesToCheck.push_back(relegation);
                    }
                }
                for (auto league : leaguesToCheck) {
                    auto root = league->GetRoot();
                    Float mp = GetGoalsMultiplier(countryId);
                    for (UInt matchday = 0; matchday < league->GetNumMatchdays(); matchday++) {
                        for (UInt matchIndex = 0; matchIndex < league->GetMatchesInMatchday(); matchIndex++) {
                            CMatch match;
                            league->GetMatch(matchday, matchIndex, match);
                            if (match.CheckFlag(FifamBeg::_1stPlayed)) {
                                CDBMatchEventEntry event;
                                CTeamIndex teams[2]{};
                                league->GetFixtureTeams(matchday, matchIndex, teams[0], teams[1]);
                                if (!teams[0].isNull() && !teams[1].isNull()) {
                                    Map<UInt, PlayerAppInfo> appInfo;
                                    Int eventIndex = match.GetMatchEventsStartIndex();
                                    root->GetMatchEvent(eventIndex, event);
                                    while (event.GetEventType() != MET_END) {
                                        if (event.GetEventType() == MET_PLAYER_APPEARANCE) {
                                            UInt playerIndex = 0;
                                            while (event.GetEventType() == MET_PLAYER_APPEARANCE) {
                                                for (UInt i = 0; i < 3; ++i) {
                                                    UInt appPlayer = event.GetValue(i);
                                                    if (appPlayer && Utils::Contains(playersWithGoals, appPlayer)) {
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
                                        else if (event.GetEventType() == MET_GOAL) {
                                            if (event.GetMinute() != 250) {
                                                for (UInt p = 0; p < 2; p++) {
                                                    Bool assist = p == 1;
                                                    UInt playerId = 0;
                                                    if (assist)
                                                        playerId = (event.GetPlayerInitiator() == event.GetPlayerAffected()) ? 0 : event.GetPlayerAffected();
                                                    else
                                                        playerId = event.GetPlayerInitiator();
                                                    if (playerId && Utils::Contains(playersWithGoals, playerId)) {
                                                        auto &c = candidates[playerId];
                                                        if (assist)
                                                            c.assists += 1;
                                                        else {
                                                            c.goals += 1;
                                                            c.points += mp;
                                                            if (event.GetReason1() == GOAL_PENALTY)
                                                                c.penalties += 1;
                                                        }
                                                        c.playerId = playerId;
                                                    }
                                                }
                                            }
                                        }
                                        else if (event.GetEventType() == MET_YELRED_CARD || event.GetEventType() == MET_RED_CARD || event.GetEventType() == MET_SUBST) {
                                            UInt outPlayer = event.GetValue(0);
                                            if (outPlayer && Utils::Contains(playersWithGoals, outPlayer))
                                                appInfo[outPlayer].minuteOut = event.GetMinute();
                                            if (event.GetEventType() == MET_SUBST) {
                                                UInt inPlayer = event.GetValue(1);
                                                if (inPlayer && Utils::Contains(playersWithGoals, inPlayer))
                                                    appInfo[inPlayer].minuteIn = event.GetMinute();
                                            }
                                        }
                                        root->GetMatchEvent(++eventIndex, event);
                                    }
                                    for (auto const &[playerId, app] : appInfo) {
                                        if (!app.onTheBench || app.minuteIn) {
                                            auto &c = candidates[playerId];
                                            c.minutesPlayed += Utils::Clamp(
                                                app.minuteOut ? app.minuteOut - app.minuteIn : event.GetMinute() - app.minuteIn,
                                                1, 120);
                                            c.playerId = playerId;
                                            CTeamIndex team = app.homeTeam ? teams[0] : teams[1];
                                            if (!Utils::Contains(c.teams, team))
                                                c.teams.push_back(team);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Phase 4. Find additional info for top goalscorers (goals, assists, minutes played, penalties)
    for (UInt countryId = 1; countryId <= 207; countryId++) {
        auto country = GetCountry(countryId);
        if (country->GetContinent() == FifamContinent::Europe) {
            auto league = GetLeague(countryId, COMP_LEAGUE, 0);
            if (league && !Game()->IsCountryPlayable(countryId)) {
                for (UInt t = 0; t < league->GetNumOfTeams(); t++) {
                    auto team = GetTeam(league->GetTeamID(t));
                    if (team) {
                        for (UInt p = 0; p < team->GetNumPlayers(); p++) {
                            auto playerId = team->GetPlayer(p);
                            if (playerId && Utils::Contains(playersWithGoals, playerId) && !Utils::Contains(candidates, playerId)) {
                                auto player = GetPlayer(playerId);
                                if (player) {
                                    CDBMatchesGoalsLeagueList *mgl = (CDBMatchesGoalsLeagueList *)GetObjectByID(GetIDForObject(3, player->GetID()));
                                    if (mgl) {
                                        Float mp = GetGoalsMultiplierForPlayerFromNonPlayableCountry(playerId, seasonStartDate);
                                        if (mp > 0.0f) {
                                            GoldenBootPlayerInfo info;
                                            for (Int e = 0; e < mgl->GetNumEntries(); e++) {
                                                if (mgl->GetYear(e) == GetCurrentSeasonStartYear()) {
                                                    info.minutesPlayed += mgl->GetNumMatches(e, MG_COMP_LEAGUE) * 90;
                                                    info.goals += mgl->GetNumGoals(e, MG_COMP_LEAGUE);
                                                    info.assists += mgl->GetNumAssists(e, MG_COMP_LEAGUE);
                                                }
                                            }
                                            if (info.goals > 0 && GetPlayerEuropeanTeamsThisSeason(playerId, seasonStartDate, info.teams)) {
                                                info.points = info.goals * mp;
                                                info.playerId = playerId;
                                                candidates[playerId] = info;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Phase 5. Find the winner
    playersWithGoals.clear();
    Pair<UInt, Vector<GoldenBootPlayerInfo>> result;
    result.first = 0;
    auto &vec = result.second;
    vec.resize(candidates.size());
    UInt counter = 0;
    for (auto const &[playerId, info] : candidates)
        vec[counter++] = info;
    candidates.clear();
    if (!vec.empty()) {
        Utils::Sort(vec, GoldenBootPlayerInfoSorter);
        if (vec.size() > 100)
            vec.resize(100);
        const auto &top = vec[0];
        for (const auto &p : vec) {
            if (p.points == top.points && p.minutesPlayed == top.minutesPlayed && p.assists == top.assists && p.penalties == top.penalties)
                result.first++;
            else
                break;
        }
    }
    return result;
}

UInt METHOD OnLoadGameData_LoadGoldenShoeWinners(void *save) {
    ClearGoldenShoeWinners();
    ClearBallonDOrCandidates();
    if (SaveGameLoadGetVersion(save) >= 47) {
        UInt count = SaveGameReadSize(save);
        GoldenShoeWinners().resize(count);
        for (UInt i = 0; i < count; i++)
            SaveGameReadData(save, &GoldenShoeWinners()[i], sizeof(GoldenShoeWinner));
        for (UInt i = 0; i < 3; i++)
            GetBallonDOrCandidates()[i] = SaveGameReadInt32(save);
    }
    return SaveGameReadSize(save);
}

void METHOD OnSaveGameData_SaveGoldenShoeWinners(void *save, DUMMY_ARG, UInt size) {
    SaveGameWriteSize(save, GoldenShoeWinners().size());
    for (UInt i = 0; i < GoldenShoeWinners().size(); i++)
        SaveGameWriteData(save, &GoldenShoeWinners()[i], sizeof(GoldenShoeWinner));
    for (UInt i = 0; i < 3; i++)
        SaveGameWriteInt32(save, GetBallonDOrCandidates()[i]);
    SaveGameWriteSize(save, size);
}

void METHOD OnInitGame(CDBGame *game) {
    ClearGoldenShoeWinners();
    ClearBallonDOrCandidates();
    CallMethod<0xF598E0>(game);
}

void METHOD OnDestroyGame(CDBGame *game) {
    ClearGoldenShoeWinners();
    ClearBallonDOrCandidates();
    CallMethod<0xF5BD40>(game);
}

class StatsEuropeanGoldenShoe {
public:
    void *vtable;
    UChar data[0x4CC - 4];
    UChar listBox[0x704];
    void *tbScreenDesc;
    void *tbScreenName;

    static void METHOD CreateUI(StatsEuropeanGoldenShoe *t) {
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        CallMethod<0x6E2340>(t, 1);
        CallMethod<0xD4F110>(t); // CXgFMPanel::CreateUI()
        listBox->Create((CXgFMPanel *)t, "Lb_ForeignPlayers");
        t->tbScreenDesc = CreateTextBox(t, "Tb_ForeignPlayers");
        t->tbScreenName = CreateTextBox(t, "Tb_ScreenName");
        CFMListBox::InitColumnTypes(listBox, 0, 2, 6, 4, 37, 9, 12, 58, 63);
        CFMListBox::InitColumnFormatting(listBox, 210, 210, 204, 204, 204, 210, 210, 210, 228);
        FillTable(t);
    }

    static void METHOD FillTable(StatsEuropeanGoldenShoe *t) {
        auto winners = GoldenShoeWinners();
        Utils::Sort(winners, [](GoldenShoeWinner const &a, GoldenShoeWinner const &b) {
            return a.year <= b.year;
        });
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        listBox->Clear();
        UInt color = CallMethodAndReturn<UInt, 0x6E22E0>(t); // CStatsBaseScr::GetTextColor()
        Map<String, UInt> totalWins;
        for (UInt i = 0; i < winners.size(); i++) {
            auto &w = winners[i];
            Int lastRowIndex = listBox->GetMaxRows() - 1;
            if (listBox->GetNumRows() >= lastRowIndex)
                break;
            String season = Utils::Format(L"%04d/", w.year - 1) + Utils::Format(L"%04d", w.year).substr(2);
            listBox->AddColumnString(season.c_str(), color, 0);
            listBox->AddCountryFlag(w.countryId, 0);
            CDBPlayer *player = FindPlayerByStringID(w.name.ToPlayerStringID(w.birthdate, 0).c_str());
            if (player)
                listBox->AddColumnInt(player->GetID(), color, 0);
            else
                listBox->AddColumnString(w.name.ToName(), color, 0);
            CDBTeam *team = GetTeamByUniqueID(w.clubId);
            if (team)
                listBox->AddTeamName(team->GetTeamID(), color, 0);
            else {
                auto teamName = GetTranslationIfPresent(Utils::Format("IDS_TEAMNAME_%08X", w.clubId).c_str());
                if (teamName)
                    listBox->AddColumnString(teamName, color, 0);
                else
                    listBox->AddColumnString(GetTranslation("DLG_NA"), color, 0);
            }
            CTeamIndex teamUID = CTeamIndex::make(w.clubId);
            if (!teamUID.isNull())
                listBox->AddCompetition(CCompID::Make(teamUID.countryId, COMP_LEAGUE, 0), color, 0);
            else
                listBox->AddColumnString(GetTranslation("DLG_NA"), color, 0);
            listBox->AddColumnInt(w.goals, color, 0);
            listBox->AddColumnFloat((Float)w.points / 100.0f, color, 0);
            listBox->NextRow(0);
            if (w.points == 0)
                listBox->GetCellTextBox(i, 6)->SetText(L"-");
            String playerStringID = w.name.ToPlayerStringID(w.birthdate, 0);
            if (Utils::Contains(totalWins, playerStringID))
                totalWins[playerStringID]++;
            else
                totalWins[playerStringID] = 1;
            String playerName = listBox->GetCellTextBox(i, 2)->GetText();
            if (Utils::EndsWith(playerName, L" (C)"))
                playerName = playerName.substr(0, playerName.size() - 4);
            if (totalWins[playerStringID] > 1)
                playerName += Utils::Format(L" (x%d)", totalWins[playerStringID]);
            listBox->GetCellTextBox(i, 2)->SetText(playerName.c_str());
        }
        listBox->Sort(0, true);
    }

    StatsEuropeanGoldenShoe(void *guiInstance);
};

UInt VtableStatsEuropeanGoldenShoe[] = {
    0x02532AD4,
    0x006E4750,0x00D378B0,0x004F2030,0x006E49B0,
    (UInt)&StatsEuropeanGoldenShoe::CreateUI, // 0x006E4680,
    0x004F2090,0x00D2AE00,0x00D2AE20,0x004F20A0,0x004060B0,0x004F23A0,0x004F2130,0x004F23D0,0x004F23C0,0x004F23B0,0x004F3370,0x004F2460,0x004F1FE0,0x004F2020,0x004060C0,0x00D2AC60,0x006E4760,0x00D2ADC0,0x00D4A7A0,0x00D2AFB0,0x00D2AFD0,0x00D2AFC0,0x00D2AFE0,0x00D2AFF0,0x00D2B000,0x00D2B010,0x00D2B020,0x00D2B030,0x00D2B040,0x00D2B060,0x00D2B050,0x00D2B070,0x00D2B080,0x00D2B090,0x00D2B0A0,0x00D2B0B0,0x00D2B0C0,0x00D2B0D0,0x00D2B0E0,0x00D2B0F0,0x00D2B100,0x006D8000,0x00D2B110,0x00D2B120,0x00D2B130,0x00D2B140,0x00D38B40,0x00D38BD0,0x00D38C60,0x00D38CF0,0x004060D0,0x00D2B150,0x00D2B160,0x00D2B170,0x00D2ADD0,0x00D2ADE0,0x00D2ADF0,0x00D37A70,0x00D32790,0x00D327D0,0x00D42CB0,0x00D32230,0x00D32390,0x00D2AF60,0x00D2AF90,0x00D39F10,0x00D3A6A0,0x00D39FA0,0x00D3A030,0x00D3A0C0,0x00D3A150,0x00D3A300,0x00D3A4E0,0x00D3A750,0x00D3AB20,0x00D3B3D0,0x00D3B5D0,0x00D3ADF0,0x00D3B140,0x004060E0,0x004060F0,0x006E4770,0x006E47D0,0x006E2300,0x006E4140,0x006E4830,0x006E4860,0x006E25E0,
    (UInt)&StatsEuropeanGoldenShoe::FillTable // 0x006E4B50
};

StatsEuropeanGoldenShoe::StatsEuropeanGoldenShoe(void *guiInstance) {
    CallMethod<0x6E3EC0>(this, guiInstance, 0); // CStatsBaseScr::CStatsBaseScr
    vtable = &VtableStatsEuropeanGoldenShoe[1];
    CallMethod<0xD1AC00>(listBox); // CFMListBox::CFMListBox()
}

class StatsEuropeanGoldenShoeWrapper : public CStatsBaseScrWrapper {
public:
    StatsEuropeanGoldenShoeWrapper(WideChar const *name) {
        CallMethod<0x14978B3>(m_name, name);
        m_nType = 1;
    }

    virtual void *CreateScreen(void *parentInstance) override {
        Char const *screenName = "Screens/10StatsEuropeanGoldenBoot.xml";
        WideChar screenNameW[260];
        void *pDummyPool = *(void **)0x309BBEC;
        void *pGuiFrame = *(void **)0x30C8940;
        void *pGuiStringDb = *(void **)0x3121C04;
        Call<0x1493F12>(screenNameW, 260, screenName); // MultibyteToWide()
        void *instance = CallMethodAndReturn<void *, 0x4EDF40>(pGuiFrame, screenNameW, pGuiStringDb, parentInstance, 0, 0); // CGuiFrame::LoadScreen()
        void *scr = CallVirtualMethodAndReturn<void *, 1>(pDummyPool, sizeof(StatsEuropeanGoldenShoe)); // pDummyPool->Allocate()
        Call<0x1573400>(scr, 0, sizeof(StatsEuropeanGoldenShoe)); // CMemoryMgr::Fill()
        StatsEuropeanGoldenShoe *s = new (scr) StatsEuropeanGoldenShoe(instance);
        CallMethod<0x4F20E0>(s, instance); // CXgPanel::SetName()
        CallVirtualMethod<0x11>(instance, s); // screenGui->SetMessageProc()
        CallVirtualMethod<4>(s); // s->CreateUI()
        return s;
    }
};

class StatsGoldenShoeCurrent {
public:
    void *vtable;
    UChar data[0x4CC - 4];
    UChar listBox[0x704];
    void *tbScreenDesc;
    void *tbScreenName;

    static void METHOD CreateUI(StatsEuropeanGoldenShoe *t) {
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        CallMethod<0x6E2340>(t, 1);
        CallMethod<0xD4F110>(t); // CXgFMPanel::CreateUI()
        listBox->Create((CXgFMPanel *)t, "Lb_ForeignPlayers");
        t->tbScreenDesc = CreateTextBox(t, "Tb_ForeignPlayers");
        t->tbScreenName = CreateTextBox(t, "Tb_ScreenName");
                                               // Pos       Flag      Player      Club      League         Goals     Assists   Minutes   Factor     Points
        CFMListBox::InitColumnTypes(listBox,      LBT_INT,  LBT_FLAG, LBT_PLAYER, LBT_CLUB, LBT_COMP_NAME, LBT_INT,  LBT_INT,  LBT_INT,  LBT_FLOAT, LBT_FLOAT, LBT_END);
        CFMListBox::InitColumnFormatting(listBox, LBF_NONE, LBF_NONE, LBF_NAME,   LBF_NAME, LBF_NAME,      LBF_NONE, LBF_NONE, LBF_NONE, LBF_FLOAT, LBF_FLOAT, LBF_END);
        FillTable(t);
    }

    static void METHOD FillTable(StatsEuropeanGoldenShoe *t) {
        CFMListBox *LbStandings = (CFMListBox *)t->listBox;
        LbStandings->Clear();
        UInt color = CallMethodAndReturn<UInt, 0x6E22E0>(t); // CStatsBaseScr::GetTextColor()
        auto players = CalcGoldenBootWinner();

        UInt counter = 0;
        for (UInt i = 0; i < players.second.size(); i++) {
            auto const &p = players.second[i];
            auto player = GetPlayer(p.playerId);
            if (player) {
                LbStandings->AddColumnInt(counter + 1, color);
                LbStandings->AddCountryFlag(player->GetNationality());
                LbStandings->AddColumnInt(p.playerId, color);
                CTeamIndex teamID = p.teams.empty() ? CTeamIndex::null() : p.teams[p.teams.size() - 1];
                LbStandings->AddTeamName(teamID, color);
                CCompID compID = CCompID::Make(teamID.countryId, COMP_LEAGUE, 0);
                LbStandings->AddCompetition(compID, color);
                LbStandings->AddColumnInt(p.goals, color);
                LbStandings->AddColumnInt(p.assists, color);
                LbStandings->AddColumnInt(p.minutesPlayed, color);
                LbStandings->AddColumnFloat(GetGoalsMultiplier(teamID.countryId), color);
                LbStandings->AddColumnFloat(p.points, color);
                LbStandings->NextRow();
                String playerName = LbStandings->GetCellTextBox(counter, 2)->GetText();
                if (Utils::EndsWith(playerName, L" (C)")) {
                    playerName = playerName.substr(0, playerName.size() - 4);
                    LbStandings->GetCellTextBox(counter, 2)->SetText(playerName.c_str());
                }
                counter++;
            }
        }
        CallMethod<0x6E2750>(t, 2, LbStandings, 1); // CStatsBaseScr::NoDataMessage()
    }

    StatsGoldenShoeCurrent(void *guiInstance);
};

UInt VtableStatsGoldenShoeCurrent[] = {
    0x02532AD4,
    0x006E4750,0x00D378B0,0x004F2030,0x006E49B0,
    (UInt)&StatsGoldenShoeCurrent::CreateUI, // 0x006E4680,
    0x004F2090,0x00D2AE00,0x00D2AE20,0x004F20A0,0x004060B0,0x004F23A0,0x004F2130,0x004F23D0,0x004F23C0,0x004F23B0,0x004F3370,0x004F2460,0x004F1FE0,0x004F2020,0x004060C0,0x00D2AC60,0x006E4760,0x00D2ADC0,0x00D4A7A0,0x00D2AFB0,0x00D2AFD0,0x00D2AFC0,0x00D2AFE0,0x00D2AFF0,0x00D2B000,0x00D2B010,0x00D2B020,0x00D2B030,0x00D2B040,0x00D2B060,0x00D2B050,0x00D2B070,0x00D2B080,0x00D2B090,0x00D2B0A0,0x00D2B0B0,0x00D2B0C0,0x00D2B0D0,0x00D2B0E0,0x00D2B0F0,0x00D2B100,0x006D8000,0x00D2B110,0x00D2B120,0x00D2B130,0x00D2B140,0x00D38B40,0x00D38BD0,0x00D38C60,0x00D38CF0,0x004060D0,0x00D2B150,0x00D2B160,0x00D2B170,0x00D2ADD0,0x00D2ADE0,0x00D2ADF0,0x00D37A70,0x00D32790,0x00D327D0,0x00D42CB0,0x00D32230,0x00D32390,0x00D2AF60,0x00D2AF90,0x00D39F10,0x00D3A6A0,0x00D39FA0,0x00D3A030,0x00D3A0C0,0x00D3A150,0x00D3A300,0x00D3A4E0,0x00D3A750,0x00D3AB20,0x00D3B3D0,0x00D3B5D0,0x00D3ADF0,0x00D3B140,0x004060E0,0x004060F0,0x006E4770,0x006E47D0,0x006E2300,0x006E4140,0x006E4830,0x006E4860,0x006E25E0,
    (UInt)&StatsGoldenShoeCurrent::FillTable // 0x006E4B50
};

StatsGoldenShoeCurrent::StatsGoldenShoeCurrent(void *guiInstance) {
    CallMethod<0x6E3EC0>(this, guiInstance, 0); // CStatsBaseScr::CStatsBaseScr
    vtable = &VtableStatsGoldenShoeCurrent[1];
    CallMethod<0xD1AC00>(listBox); // CFMListBox::CFMListBox()
}

class StatsGoldenShoeCurrentWrapper : public CStatsBaseScrWrapper {
public:
    StatsGoldenShoeCurrentWrapper(WideChar const *name) {
        CallMethod<0x14978B3>(m_name, name);
        m_nType = 1;
    }

    virtual void *CreateScreen(void *parentInstance) override {
        Char const *screenName = "Screens/10StatsEuropeanGoldenBootCurrent.xml";
        WideChar screenNameW[260];
        void *pDummyPool = *(void **)0x309BBEC;
        void *pGuiFrame = *(void **)0x30C8940;
        void *pGuiStringDb = *(void **)0x3121C04;
        Call<0x1493F12>(screenNameW, 260, screenName); // MultibyteToWide()
        void *instance = CallMethodAndReturn<void *, 0x4EDF40>(pGuiFrame, screenNameW, pGuiStringDb, parentInstance, 0, 0); // CGuiFrame::LoadScreen()
        void *scr = CallVirtualMethodAndReturn<void *, 1>(pDummyPool, sizeof(StatsGoldenShoeCurrent)); // pDummyPool->Allocate()
        Call<0x1573400>(scr, 0, sizeof(StatsGoldenShoeCurrent)); // CMemoryMgr::Fill()
        StatsGoldenShoeCurrent *s = new (scr) StatsGoldenShoeCurrent(instance);
        CallMethod<0x4F20E0>(s, instance); // CXgPanel::SetName()
        CallVirtualMethod<0x11>(instance, s); // screenGui->SetMessageProc()
        CallVirtualMethod<4>(s); // s->CreateUI()
        return s;
    }
};

template<UInt ListOffset>
class StatsPlayerTrophyWinners {
public:
    void *vtable;
    UChar data[0x4CC - 4];
    UChar listBox[0x704];
    CXgTextBox *tbScreenDesc;
    CXgTextBox *tbScreenName;

    static void METHOD CreateUI(StatsPlayerTrophyWinners *t) {
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        CallMethod<0x6E2340>(t, 1);
        CallMethod<0xD4F110>(t); // CXgFMPanel::CreateUI()
        listBox->Create((CXgFMPanel *)t, "Lb_ForeignPlayers");
        t->tbScreenDesc = (CXgTextBox *)CreateTextBox(t, "Tb_ForeignPlayers");
        t->tbScreenName = (CXgTextBox *)CreateTextBox(t, "Tb_ScreenName");
        if (ListOffset == LIST_OFFSET_BALLONDOR) {
            t->tbScreenDesc->SetText(GetTranslation("IDS_BALLONDOR"));
            t->tbScreenName->SetText(GetTranslation("IDS_BALLONDOR"));
        }
        else if (ListOffset == LIST_OFFSET_BESTEUROPEAN) {
            t->tbScreenDesc->SetText(GetTranslation("IDS_PLAYER_INFO_ALL_TIME_3"));
            t->tbScreenName->SetText(GetTranslation("IDS_PLAYER_INFO_ALL_TIME_3"));
        }
                                               // Pos       Flag      Player      Badge           Club     
        CFMListBox::InitColumnTypes(listBox,      0,        LBT_FLAG, LBT_PLAYER, LBT_CLUB_BADGE, LBT_CLUB, LBT_END);
        CFMListBox::InitColumnFormatting(listBox, LBF_NONE, LBF_NONE, LBF_NAME,   LBF_NONE,       LBF_NAME, LBF_END);
        FillTable(t);
    }

    static void METHOD FillTable(StatsPlayerTrophyWinners *t) {
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        listBox->Clear();
        UInt color = CallMethodAndReturn<UInt, 0x6E22E0>(t); // CStatsBaseScr::GetTextColor()
        auto &ballonDOrWinners = *raw_ptr<FmList<MyHistoricalPlayerEntry>>(CDBGame::GetInstance(), ListOffset);
        UInt counter = 0;
        for (auto &w : ballonDOrWinners) {
            Int lastRowIndex = listBox->GetMaxRows() - 1;
            if (listBox->GetNumRows() >= lastRowIndex)
                break;
            listBox->AddColumnString(Utils::Format(L"%4d", w.year).c_str(), color);
            listBox->AddCountryFlag(w.countryId, 0);
            CDBPlayer *player = FindPlayerByStringID(w.name.ToPlayerStringID(w.birthdate, 0).c_str());
            if (player)
                listBox->AddColumnInt(player->GetID(), color, 0);
            else
                listBox->AddColumnString(w.name.ToName(), color, 0);
            CDBTeam *team = GetTeamByUniqueID(w.clubId);
            if (team) {
                listBox->AddTeamWidget(team->GetTeamID());
                listBox->AddTeamName(team->GetTeamID(), color, 0);
            }
            else {
                auto teamName = GetTranslationIfPresent(Utils::Format("IDS_TEAMNAME_%08X", w.clubId).c_str());
                if (teamName)
                    listBox->AddColumnString(teamName, color, 0);
                else
                    listBox->AddColumnString(GetTranslation("DLG_NA"), color, 0);
            }
            listBox->NextRow(0);
            String playerName = listBox->GetCellTextBox(counter, 2)->GetText();
            if (Utils::EndsWith(playerName, L" (C)"))
                playerName = playerName.substr(0, playerName.size() - 4);
            listBox->GetCellTextBox(counter, 2)->SetText(playerName.c_str());
            counter++;
        }
    }

    StatsPlayerTrophyWinners(void *guiInstance);
};

UInt VtableStatsBallonDOr[] = {
    0x02532AD4,
    0x006E4750,0x00D378B0,0x004F2030,0x006E49B0,
    (UInt)&StatsPlayerTrophyWinners<LIST_OFFSET_BALLONDOR>::CreateUI, // 0x006E4680,
    0x004F2090,0x00D2AE00,0x00D2AE20,0x004F20A0,0x004060B0,0x004F23A0,0x004F2130,0x004F23D0,0x004F23C0,0x004F23B0,0x004F3370,0x004F2460,0x004F1FE0,0x004F2020,0x004060C0,0x00D2AC60,0x006E4760,0x00D2ADC0,0x00D4A7A0,0x00D2AFB0,0x00D2AFD0,0x00D2AFC0,0x00D2AFE0,0x00D2AFF0,0x00D2B000,0x00D2B010,0x00D2B020,0x00D2B030,0x00D2B040,0x00D2B060,0x00D2B050,0x00D2B070,0x00D2B080,0x00D2B090,0x00D2B0A0,0x00D2B0B0,0x00D2B0C0,0x00D2B0D0,0x00D2B0E0,0x00D2B0F0,0x00D2B100,0x006D8000,0x00D2B110,0x00D2B120,0x00D2B130,0x00D2B140,0x00D38B40,0x00D38BD0,0x00D38C60,0x00D38CF0,0x004060D0,0x00D2B150,0x00D2B160,0x00D2B170,0x00D2ADD0,0x00D2ADE0,0x00D2ADF0,0x00D37A70,0x00D32790,0x00D327D0,0x00D42CB0,0x00D32230,0x00D32390,0x00D2AF60,0x00D2AF90,0x00D39F10,0x00D3A6A0,0x00D39FA0,0x00D3A030,0x00D3A0C0,0x00D3A150,0x00D3A300,0x00D3A4E0,0x00D3A750,0x00D3AB20,0x00D3B3D0,0x00D3B5D0,0x00D3ADF0,0x00D3B140,0x004060E0,0x004060F0,0x006E4770,0x006E47D0,0x006E2300,0x006E4140,0x006E4830,0x006E4860,0x006E25E0,
    (UInt)&StatsPlayerTrophyWinners<LIST_OFFSET_BALLONDOR>::FillTable // 0x006E4B50
};

StatsPlayerTrophyWinners<LIST_OFFSET_BALLONDOR>::StatsPlayerTrophyWinners(void *guiInstance) {
    CallMethod<0x6E3EC0>(this, guiInstance, 0); // CStatsBaseScr::CStatsBaseScr
    vtable = &VtableStatsBallonDOr[1];
    CallMethod<0xD1AC00>(listBox); // CFMListBox::CFMListBox()
}

UInt VtableStatsBestEuropean[] = {
    0x02532AD4,
    0x006E4750,0x00D378B0,0x004F2030,0x006E49B0,
    (UInt)&StatsPlayerTrophyWinners<LIST_OFFSET_BESTEUROPEAN>::CreateUI, // 0x006E4680,
    0x004F2090,0x00D2AE00,0x00D2AE20,0x004F20A0,0x004060B0,0x004F23A0,0x004F2130,0x004F23D0,0x004F23C0,0x004F23B0,0x004F3370,0x004F2460,0x004F1FE0,0x004F2020,0x004060C0,0x00D2AC60,0x006E4760,0x00D2ADC0,0x00D4A7A0,0x00D2AFB0,0x00D2AFD0,0x00D2AFC0,0x00D2AFE0,0x00D2AFF0,0x00D2B000,0x00D2B010,0x00D2B020,0x00D2B030,0x00D2B040,0x00D2B060,0x00D2B050,0x00D2B070,0x00D2B080,0x00D2B090,0x00D2B0A0,0x00D2B0B0,0x00D2B0C0,0x00D2B0D0,0x00D2B0E0,0x00D2B0F0,0x00D2B100,0x006D8000,0x00D2B110,0x00D2B120,0x00D2B130,0x00D2B140,0x00D38B40,0x00D38BD0,0x00D38C60,0x00D38CF0,0x004060D0,0x00D2B150,0x00D2B160,0x00D2B170,0x00D2ADD0,0x00D2ADE0,0x00D2ADF0,0x00D37A70,0x00D32790,0x00D327D0,0x00D42CB0,0x00D32230,0x00D32390,0x00D2AF60,0x00D2AF90,0x00D39F10,0x00D3A6A0,0x00D39FA0,0x00D3A030,0x00D3A0C0,0x00D3A150,0x00D3A300,0x00D3A4E0,0x00D3A750,0x00D3AB20,0x00D3B3D0,0x00D3B5D0,0x00D3ADF0,0x00D3B140,0x004060E0,0x004060F0,0x006E4770,0x006E47D0,0x006E2300,0x006E4140,0x006E4830,0x006E4860,0x006E25E0,
    (UInt)&StatsPlayerTrophyWinners<LIST_OFFSET_BESTEUROPEAN>::FillTable // 0x006E4B50
};

StatsPlayerTrophyWinners<LIST_OFFSET_BESTEUROPEAN>::StatsPlayerTrophyWinners(void *guiInstance) {
    CallMethod<0x6E3EC0>(this, guiInstance, 0); // CStatsBaseScr::CStatsBaseScr
    vtable = &VtableStatsBestEuropean[1];
    CallMethod<0xD1AC00>(listBox); // CFMListBox::CFMListBox()
}

template<UInt ListOffset>
class StatsPlayerTrophyWinnersWrapper : public CStatsBaseScrWrapper {
public:
    StatsPlayerTrophyWinnersWrapper(WideChar const *name) {
        CallMethod<0x14978B3>(m_name, name);
        m_nType = 1;
    }

    virtual void *CreateScreen(void *parentInstance) override {
        Char const *screenName = "Screens/10StatsPlayerTrophyWinners.xml";
        WideChar screenNameW[260];
        void *pDummyPool = *(void **)0x309BBEC;
        void *pGuiFrame = *(void **)0x30C8940;
        void *pGuiStringDb = *(void **)0x3121C04;
        Call<0x1493F12>(screenNameW, 260, screenName); // MultibyteToWide()
        void *instance = CallMethodAndReturn<void *, 0x4EDF40>(pGuiFrame, screenNameW, pGuiStringDb, parentInstance, 0, 0); // CGuiFrame::LoadScreen()
        void *scr = CallVirtualMethodAndReturn<void *, 1>(pDummyPool, sizeof(StatsPlayerTrophyWinners<ListOffset>)); // pDummyPool->Allocate()
        Call<0x1573400>(scr, 0, sizeof(StatsPlayerTrophyWinners<ListOffset>)); // CMemoryMgr::Fill()
        StatsPlayerTrophyWinners<ListOffset> *s = new (scr) StatsPlayerTrophyWinners<ListOffset>(instance);
        CallMethod<0x4F20E0>(s, instance); // CXgPanel::SetName()
        CallVirtualMethod<0x11>(instance, s); // screenGui->SetMessageProc()
        CallVirtualMethod<4>(s); // s->CreateUI()
        return s;
    }
};

void METHOD CreateStatsPlayerTrophiesWrapper(void *vec, DUMMY_ARG, void *data) {
    CallMethod<0x736690>(vec, data);
    void *mem = nullptr;

    mem = CallAndReturn<void *, 0x15738F3>(sizeof(StatsPlayerTrophyWinnersWrapper<LIST_OFFSET_BALLONDOR>));
    auto w1 = new (mem) StatsPlayerTrophyWinnersWrapper<LIST_OFFSET_BALLONDOR>(GetTranslation("IDS_BALLONDOR"));
    CallMethod<0x736690>(vec, &w1);

    mem = CallAndReturn<void *, 0x15738F3>(sizeof(StatsPlayerTrophyWinnersWrapper<LIST_OFFSET_BESTEUROPEAN>));
    auto w2 = new (mem) StatsPlayerTrophyWinnersWrapper<LIST_OFFSET_BESTEUROPEAN>(GetTranslation("IDS_PLAYER_INFO_ALL_TIME_3"));
    CallMethod<0x736690>(vec, &w2);

    mem = CallAndReturn<void *, 0x15738F3>(sizeof(StatsEuropeanGoldenShoeWrapper));
    auto w3 = new (mem) StatsEuropeanGoldenShoeWrapper(GetTranslation("IDS_GOLDENBOOT"));
    CallMethod<0x736690>(vec, &w3);

    mem = CallAndReturn<void *, 0x15738F3>(sizeof(StatsGoldenShoeCurrentWrapper));
    auto w4 = new (mem) StatsGoldenShoeCurrentWrapper(GetTranslation("IDS_GOLDENBOOT_CURRENT"));
    CallMethod<0x736690>(vec, &w4);
}

const UInt NewspaperGoldenBootDefaultSize = 0xC20;

struct NewspaperGoldenBootExtension {
    CXgTextBox *TbT1;
    CXgTextBox *TbClub[2];
    CXgTextBox *TbPlayer[2];
    CXgTextBox *TbFlag[2];
    CXgImage *ImgGoldenBoot1;
    CXgImage *ImgGoldenBoot2;
    CXgTextBox *TbWinnerText;
    CXgTextBox *TbPicByPlayer;
    CXgTextBox *TbPicByClub;
    CXgTextBox *TbPicByNation;
    CXgTextBox *TbPicByLeague;
    UChar LbByPlayer[0x704];
    UChar LbByClub[0x704];
    UChar LbByNation[0x704];
    UChar LbByLeague[0x704];
};

void METHOD NewspaperGoldenBootFill(CXgFMPanel *screen, DUMMY_ARG, void *data) {
    auto ext = raw_ptr<NewspaperGoldenBootExtension>(screen, NewspaperGoldenBootDefaultSize);
    auto LbByPlayer = (CFMListBox *)ext->LbByPlayer;
    auto LbByClub = (CFMListBox *)ext->LbByClub;
    auto LbByNation = (CFMListBox *)ext->LbByNation;
    auto LbByLeague = (CFMListBox *)ext->LbByLeague;

    *raw_ptr<void *>(screen, 0xC1C) = data; // m_pNetworkEventData = data;
    CFMListBox *LbStandings = raw_ptr<CFMListBox>(screen, 0x4AC);
    LbStandings->Clear();                      // Pos       Flag      Player      Flag      Club      Goals     Assists   Minutes   Factor     Points
    CFMListBox::InitColumnTypes(LbStandings,      LBT_INT,  LBT_FLAG, LBT_PLAYER, LBT_FLAG, LBT_CLUB, LBT_INT,  LBT_INT,  LBT_INT,  LBT_FLOAT, LBT_FLOAT, LBT_END);
    CFMListBox::InitColumnFormatting(LbStandings, LBF_NONE, LBF_NONE, LBF_NONE,   LBF_NONE, LBF_NAME, LBF_NONE, LBF_NONE, LBF_NONE, LBF_FLOAT, LBF_FLOAT, LBF_END);
    CXgTextBox *TbDate = *raw_ptr<CXgTextBox *>(screen, 0xBB0);
    FmString str;
    CJDate currentDate = CDBGame::GetInstance()->GetCurrentDate();
    CallMethod<0x14954E3>(&currentDate, &str, true);
    TbDate->SetText(str.data());
    ext->TbT1->SetText(Utils::Format(GetTranslation("IDS_GOLDENBOOT_TITLE"), GetCurrentSeasonStartYear(), GetCurrentSeasonEndYear()).c_str());
    auto players = CalcGoldenBootWinner();

    UInt color = GetGuiColor(COL_TEXT_STANDARD);
    UInt counter = 0;
    for (UInt i = 0; i < players.second.size(); i++) {
        auto const &p = players.second[i];
        auto player = GetPlayer(p.playerId);
        if (player) {
            LbStandings->AddColumnInt(counter + 1, color);
            LbStandings->AddCountryFlag(player->GetNationality());
            LbStandings->AddColumnInt(p.playerId, color);
            CTeamIndex teamID = p.teams.empty() ? CTeamIndex::null() : p.teams[p.teams.size() - 1];
            LbStandings->AddCountryFlag(teamID.countryId);
            LbStandings->AddTeamName(teamID, color);
            LbStandings->AddColumnInt(p.goals, color);
            LbStandings->AddColumnInt(p.assists, color);
            LbStandings->AddColumnInt(p.minutesPlayed, color);
            LbStandings->AddColumnFloat(GetGoalsMultiplier(teamID.countryId), color);
            LbStandings->AddColumnFloat(p.points, color);
            LbStandings->NextRow();
            String playerName = LbStandings->GetCellTextBox(counter, 2)->GetText();
            if (Utils::EndsWith(playerName, L" (C)")) {
                playerName = playerName.substr(0, playerName.size() - 4);
                LbStandings->GetCellTextBox(counter, 2)->SetText(playerName.c_str());
            }
            counter++;
        }
    }
    ext->ImgGoldenBoot1->SetVisible(players.first <= 1);
    ext->ImgGoldenBoot2->SetVisible(players.first >= 2);
    CEAMailData formatter;
    for (UInt i = 0; i < Utils::Min(players.first, 3u); i++) {
        auto const &info = players.second[i];
        CTeamIndex teamID = info.teams.empty() ? CTeamIndex::null() : info.teams[info.teams.size() - 1];
        if (i < 1)
            formatter.SetArrayValue(i, info.goals);
        if (i < 2) {
            ext->TbPlayer[i]->SetVisible(false);
            ext->TbClub[i]->SetVisible(false);
            ext->TbFlag[i]->SetVisible(false);
            CDBPlayer *player = GetPlayer(info.playerId);
            if (player) {
                
                screen->SetPlayerPortrait(ext->TbPlayer[i], info.playerId, false);
                ext->TbPlayer[i]->SetVisible(true);
                if (!players.second[0].teams.empty()) {
                    screen->SetTeamBadge(ext->TbClub[i], teamID);
                    ext->TbClub[i]->SetVisible(true);
                }
                screen->SetCountryFlag(ext->TbFlag[i], player->GetNationality());
                ext->TbFlag[i]->SetVisible(true);
            }
        }
        formatter.SetTeam(i, teamID);
        formatter.SetPlayer(i, info.playerId);
    }
    WideChar winnerText[2048];
    CEAMailData::Format(winnerText, std::size(winnerText),
        GetTranslation(Utils::Format("IDS_GOLDENBOOT_TEXT_%d", players.first > 4 ? 4 : players.first).c_str()), formatter);
    ext->TbWinnerText->SetText(winnerText);

    struct WinByPlayer {
        UInt playerId = 0;
        NameDesc nameDesc;
        CJDate birthdate;
        UChar countryId = 0;
        UShort count = 0;
        UShort firstYear = 0;
    };
    Map<String, WinByPlayer> winsByPlayer;
    for (auto &w : GoldenShoeWinners()) {
        String playerStringID = w.name.ToPlayerStringID(w.birthdate, 0);
        auto &e = winsByPlayer[playerStringID];
        e.nameDesc = w.name;
        e.birthdate = w.birthdate;
        e.countryId = w.countryId;
        e.count++;
        if (e.firstYear == 0)
            e.firstYear = w.year;
    }
    Vector<WinByPlayer> vecWinsByPlayer;
    for (auto const &[id, w] : winsByPlayer)
        vecWinsByPlayer.push_back(w);
    Utils::Sort(vecWinsByPlayer, [](WinByPlayer const &a, WinByPlayer const &b) {
        if (a.count > b.count) return true;
        if (b.count > a.count) return false;
        if (a.firstYear < b.firstYear) return true;
        if (b.firstYear < a.firstYear) return false;
        return true;
    });
    if (vecWinsByPlayer.size() > 30)
        vecWinsByPlayer.resize(30);
    LbByPlayer->Clear();
    counter = 0;
    for (auto &w : vecWinsByPlayer) {
        LbByPlayer->AddColumnInt(counter + 1, color);
        LbByPlayer->AddCountryFlag(w.countryId);
        CDBPlayer *player = FindPlayerByStringID(w.nameDesc.ToPlayerStringID(w.birthdate, 0).c_str());
        if (player)
            LbByPlayer->AddColumnInt(player->GetID(), color, 0);
        else
            LbByPlayer->AddColumnString(w.nameDesc.ToName(), color, 0);
        LbByPlayer->AddColumnInt(w.count, color);
        LbByPlayer->NextRow();
        String playerName = LbByPlayer->GetCellTextBox(counter, 2)->GetText();
        if (Utils::EndsWith(playerName, L" (C)")) {
            playerName = playerName.substr(0, playerName.size() - 4);
            LbByPlayer->GetCellTextBox(counter, 2)->SetText(playerName.c_str());
        }
        counter++;
    }
    if (!vecWinsByPlayer.empty()) {
        String playerStringID = vecWinsByPlayer[0].nameDesc.ToPlayerStringID(vecWinsByPlayer[0].birthdate, 0);
        CDBPlayer *player = FindPlayerByStringID(playerStringID.c_str());
        if (player)
            screen->SetPlayerPortrait(ext->TbPicByPlayer, player->GetID(), false);
        else
            SetImageFilename(ext->TbPicByPlayer, (L"portraits\\club\\160x160\\" + playerStringID + L".png").c_str(), 4, 4);
    }

    struct WinByClub {
        UInt clubUID = 0;
        UShort count = 0;
        UShort firstYear = 0;
    };
    Map<UInt, WinByClub> winsByClub;
    for (auto const &w : GoldenShoeWinners()) {
        auto &e = winsByClub[w.clubId];
        e.clubUID = w.clubId;
        e.count++;
        if (e.firstYear == 0)
            e.firstYear = w.year;
    }
    Vector<WinByClub> vecWinsByClub;
    for (auto const &[id, w] : winsByClub)
        vecWinsByClub.push_back(w);
    Utils::Sort(vecWinsByClub, [](WinByClub const &a, WinByClub const &b) {
        if (a.count > b.count) return true;
        if (b.count > a.count) return false;
        if (a.firstYear < b.firstYear) return true;
        if (b.firstYear < a.firstYear) return false;
        return true;
    });
    if (vecWinsByClub.size() > 30)
        vecWinsByClub.resize(30);
    LbByClub->Clear();
    counter = 0;
    for (auto const &w : vecWinsByClub) {
        auto team = GetTeamByUniqueID(w.clubUID);
        if (team) {
            LbByClub->AddColumnInt(counter + 1, color);
            LbByClub->AddTeamWidget(team->GetTeamID());
            LbByClub->AddTeamName(team->GetTeamID(), color);
            LbByClub->AddColumnInt(w.count, color);
            LbByClub->NextRow();
            counter++;
        }
        else {
            auto teamName = GetTranslationIfPresent(Utils::Format("IDS_TEAMNAME_%08X", w.clubUID).c_str());
            if (teamName) {
                LbByClub->AddColumnInt(counter + 1, color);
                LbByClub->AddColumnString(L"", color);
                LbByClub->AddColumnString(teamName, color);
                LbByClub->AddColumnInt(w.count, color);
                LbByClub->NextRow();
                counter++;
            }
        }
    }
    if (!vecWinsByClub.empty()) {
        auto team = GetTeamByUniqueID(vecWinsByClub[0].clubUID);
        if (team)
            screen->SetTeamBadge(ext->TbPicByClub, team->GetTeamID());
    }

    struct WinByNation {
        UChar countryId = 0;
        UShort count = 0;
        UShort firstYear = 0;
    };
    Map<UInt, WinByNation> winsByNation;
    for (auto const &w : GoldenShoeWinners()) {
        auto &e = winsByNation[w.countryId];
        e.countryId = w.countryId;
        e.count++;
        if (e.firstYear == 0)
            e.firstYear = w.year;
    }
    Vector<WinByNation> vecWinsByNation;
    for (auto const &[id, w] : winsByNation)
        vecWinsByNation.push_back(w);
    Utils::Sort(vecWinsByNation, [](WinByNation const &a, WinByNation const &b) {
        if (a.count > b.count) return true;
        if (b.count > a.count) return false;
        if (a.firstYear < b.firstYear) return true;
        if (b.firstYear < a.firstYear) return false;
        return true;
    });
    if (vecWinsByNation.size() > 30)
        vecWinsByNation.resize(30);
    LbByNation->Clear();
    counter = 0;
    for (auto const &w : vecWinsByNation) {
        LbByNation->AddColumnInt(counter + 1, color);
        LbByNation->AddCountryFlag(w.countryId);
        LbByNation->AddColumnInt(w.countryId, color);
        LbByNation->AddColumnInt(w.count, color);
        LbByNation->NextRow();
        counter++;
    }
    if (!vecWinsByNation.empty())
        screen->SetCountryFlag(ext->TbPicByNation, vecWinsByNation[0].countryId);

    struct WinByLeague {
        UChar countryId = 0;
        UShort count = 0;
        UShort firstYear = 0;
    };
    Map<UInt, WinByLeague> winsByLeague;
    for (auto const &w : GoldenShoeWinners()) {
        UChar countryId = (w.clubId >> 16) & 0xFF;
        auto &e = winsByLeague[countryId];
        e.countryId = countryId;
        e.count++;
        if (e.firstYear == 0)
            e.firstYear = w.year;
    }
    Vector<WinByLeague> vecWinsByLeague;
    for (auto const &[id, w] : winsByLeague)
        vecWinsByLeague.push_back(w);
    Utils::Sort(vecWinsByLeague, [](WinByLeague const &a, WinByLeague const &b) {
        if (a.count > b.count) return true;
        if (b.count > a.count) return false;
        if (a.firstYear < b.firstYear) return true;
        if (b.firstYear < a.firstYear) return false;
        return true;
    });
    if (vecWinsByLeague.size() > 30)
        vecWinsByLeague.resize(30);
    LbByLeague->Clear();
    counter = 0;
    for (auto const &w : vecWinsByLeague) {
        LbByLeague->AddColumnInt(counter + 1, color);
        CCompID compID = CCompID::Make(w.countryId, COMP_LEAGUE, 0);
        LbByLeague->AddCompetition(compID, color);
        LbByLeague->AddCompetition(compID, color);
        LbByLeague->AddColumnInt(w.count, color);
        LbByLeague->NextRow();
        counter++;
    }
    if (!vecWinsByLeague.empty())
        screen->SetCompetitionBadge(ext->TbPicByLeague, CCompID::Make(vecWinsByLeague[0].countryId, COMP_LEAGUE, 0), 1);
}

void METHOD OnNewspaperGoldenBootCreateUI(CXgFMPanel *screen) {
    CallMethod<0xA79080>(screen);
    auto ext = raw_ptr<NewspaperGoldenBootExtension>(screen, NewspaperGoldenBootDefaultSize);
    ext->TbT1 = screen->GetTextBox("_t1");
    ext->TbClub[0] = screen->GetTextBox("TbClub1");
    ext->TbClub[1] = screen->GetTextBox("TbClub2");
    ext->TbPlayer[0] = screen->GetTextBox("TbPlayer1");
    ext->TbPlayer[1] = screen->GetTextBox("TbPlayer2");
    ext->TbFlag[0] = screen->GetTextBox("TbFlag1");
    ext->TbFlag[1] = screen->GetTextBox("TbFlag2");
    ext->ImgGoldenBoot1 = screen->GetImage("ImgGoldenBoot1");
    ext->ImgGoldenBoot2 = screen->GetImage("ImgGoldenBoot2");
    ext->TbWinnerText = screen->GetTextBox("TbWinnerText");
    ext->TbPicByPlayer = screen->GetTextBox("TbPicByPlayer");
    ext->TbPicByClub = screen->GetTextBox("TbPicByClub");
    ext->TbPicByNation = screen->GetTextBox("TbPicByNation");
    ext->TbPicByLeague = screen->GetTextBox("TbPicByLeague");
    auto LbByPlayer = (CFMListBox *)ext->LbByPlayer;
    auto LbByClub = (CFMListBox *)ext->LbByClub;
    auto LbByNation = (CFMListBox *)ext->LbByNation;
    auto LbByLeague = (CFMListBox *)ext->LbByLeague;
    LbByPlayer->Create(screen, "LbByPlayer"); // Pos       Flag      Player      Wins     
    CFMListBox::InitColumnTypes(LbByPlayer, LBT_INT, LBT_FLAG, LBT_PLAYER, LBT_INT, LBT_END);
    CFMListBox::InitColumnFormatting(LbByPlayer, LBF_NONE, LBF_NONE, LBF_NAME, LBF_NONE, LBF_END);
    LbByClub->Create(screen, "LbByClub");   // Pos       Badge           Club      Wins     
    CFMListBox::InitColumnTypes(LbByClub, LBT_INT, LBT_CLUB_BADGE, LBT_CLUB, LBT_INT, LBT_END);
    CFMListBox::InitColumnFormatting(LbByClub, LBF_NONE, LBF_NONE, LBF_NAME, LBF_NONE, LBF_END);
    LbByNation->Create(screen, "LbByNation"); // Pos       Flag      Country      Wins     
    CFMListBox::InitColumnTypes(LbByNation, LBT_INT, LBT_FLAG, LBT_COUNTRY, LBT_INT, LBT_END);
    CFMListBox::InitColumnFormatting(LbByNation, LBF_NONE, LBF_NONE, LBF_NAME, LBF_NONE, LBF_END);
    LbByLeague->Create(screen, "LbByLeague"); // Pos       Logo           League         Wins     
    CFMListBox::InitColumnTypes(LbByLeague, LBT_INT, LBT_COMP_LOGO, LBT_COMP_NAME, LBT_INT, LBT_END);
    CFMListBox::InitColumnFormatting(LbByLeague, LBF_NONE, LBF_NONE, LBF_NAME, LBF_NONE, LBF_END);

    // TODO: remove this
    static char data[0x54];
    NewspaperGoldenBootFill(screen, 0, data);
}

CXgFMPanel *METHOD OnNewspaperGoldenBootConstruct(CXgFMPanel *screen, DUMMY_ARG, void *guiInstance) {
    CallMethod<0xD527C0>(screen, guiInstance);
    auto ext = raw_ptr<NewspaperGoldenBootExtension>(screen, NewspaperGoldenBootDefaultSize);
    CallMethod<0xD1AC00>(ext->LbByPlayer); // CFMListBox::CFMListBox()
    CallMethod<0xD1AC00>(ext->LbByClub); // CFMListBox::CFMListBox()
    CallMethod<0xD1AC00>(ext->LbByNation); // CFMListBox::CFMListBox()
    CallMethod<0xD1AC00>(ext->LbByLeague); // CFMListBox::CFMListBox()
    return screen;
}

CXgFMPanel *METHOD OnNewspaperGoldenBootDestruct(CXgFMPanel *screen) {
    auto ext = raw_ptr<NewspaperGoldenBootExtension>(screen, NewspaperGoldenBootDefaultSize);
    CallMethod<0xD182F0>(ext->LbByPlayer); // CFMListBox::~CFMListBox()
    CallMethod<0xD182F0>(ext->LbByClub); // CFMListBox::~CFMListBox()
    CallMethod<0xD182F0>(ext->LbByNation); // CFMListBox::~CFMListBox()
    CallMethod<0xD182F0>(ext->LbByLeague); // CFMListBox::~CFMListBox()
    CallMethod<0xD54220>(screen);
    return screen;
}

Bool CanDisplayGoldenBootNewspaperScreen() {
    UInt numPlayableCountriesInEurope = 0;
    for (UInt i = 1; i <= 207; i++) {
        if (CDBGame::GetInstance()->IsCountryPlayable(i) && GetCountry(i)->GetContinent() == FifamContinent::Europe)
            numPlayableCountriesInEurope++;
    }
    return numPlayableCountriesInEurope >= 0; // TODO: change to >0
}

void METHOD OnShowGoldenBootScreen(void *t, DUMMY_ARG, UShort screenId, Int employeeId, Int a3) {
    auto players = CalcGoldenBootWinner();
    if (players.first > 0) {
        auto year = GetCurrentSeasonEndYear();
        for (UInt i = 0; i < players.first; i++) {
            CDBPlayer *player = GetPlayer(players.second[i].playerId);
            if (player) {
                GoldenShoeWinner winner;
                winner.year = year;
                winner.goals = players.second[i].goals;
                Float points = players.second[i].points;
                if (points > 300.0f)
                    points = 300.0f;
                winner.points = (UShort)(points * 100.0f);
                winner.countryId = player->GetNationality();
                winner.birthdate = player->GetBirthdate();
                winner.name = player->GetNameDesc();
                winner.empicsId = player->GetEmpicsID();
                UInt clubId = 0;
                if (!players.second[i].teams.empty()) {
                    auto team = GetTeam(players.second[i].teams[players.second[i].teams.size() - 1]);
                    if (team)
                        clubId = team->GetTeamUniqueID();
                }
                winner.clubId = clubId;
                PlayerStatsSetGoldenShoeWins(player->GetStats(), PlayerStatsGetGoldenShoeWins(player->GetStats()) + 1);
                GoldenShoeWinners().push_back(winner);
            }
        }
    }
    if (CanDisplayGoldenBootNewspaperScreen())
        CallMethod<0x1120440>(t, screenId, employeeId, a3);
}

void METHOD OnProcessGoldenBootScreen(void *t) {
    if (CanDisplayGoldenBootNewspaperScreen())
        CallMethod<0x111F730>(t);
}

UInt METHOD OnNewspaperGoldenBootGetDayOfWeek(CJDate *date) {
    if (date->GetDays() == 1 && date->GetMonth() == 1)
        date->Set(GetCurrentYear() + 1, 6, 1);
    return date->GetDayOfWeek();
}

const UInt PlayerInfoCareerDefaultSize = 0x15A8;
const UInt MAX_TROPHIES = 7;

struct PlayerInfoCareerExtended {
    CXgTextBox *TbTrophy[MAX_TROPHIES];
    CXgTextBox *TbTrophyWins[MAX_TROPHIES];
    CXgImage *ImgTrophyWins[MAX_TROPHIES];
};

void METHOD OnPlayerInfoCareerCreateUI(CXgFMPanel *screen) {
    CallMethod<0x5D6030>(screen);
    auto ext = raw_ptr<PlayerInfoCareerExtended>(screen, PlayerInfoCareerDefaultSize);
    for (UInt i = 0; i < MAX_TROPHIES; i++) {
        ext->TbTrophy[i] = screen->GetTextBox(Utils::Format("TbTrophy%d", i + 1).c_str());
        ext->TbTrophyWins[i] = screen->GetTextBox(Utils::Format("TbTrophyWins%d", i + 1).c_str());
        ext->ImgTrophyWins[i] = screen->GetImage(Utils::Format("ImgTrophyWins%d", i + 1).c_str());
    }
}

Int METHOD OnPlayerInfoCareerFill(CXgFMPanel *screen) {
    UInt playerId = CallMethodAndReturn<Int, 0x5DE220>(screen); // CPlayerInfoPanel::GetCurrentPlayerId
    CDBPlayer *player = GetPlayer(playerId);
    if (player) {
        auto ext = raw_ptr<PlayerInfoCareerExtended>(screen, PlayerInfoCareerDefaultSize);
        for (UInt i = 0; i < MAX_TROPHIES; i++) {
            SetImageFilename(ext->TbTrophy[i], L"");
            ext->TbTrophy[i]->SetVisible(false);
            ext->TbTrophyWins[i]->SetVisible(false);
            ext->ImgTrophyWins[i]->SetVisible(false);
        }
        UInt counter = 0;
        UInt numBallonDOr = Utils::Clamp(PlayerStatsGetBallonDOrWins(player->GetStats()), 0, 99);
        if (numBallonDOr) {
            SetImageFilename(ext->TbTrophy[counter], L"art_fm\\lib\\Trophies\\64x64\\BallonDOr.tga");
            ext->TbTrophy[counter]->SetTooltip(GetTranslation("IDS_BALLONDOR"));
            ext->TbTrophyWins[counter++]->SetText(Utils::Format(L"%d", numBallonDOr).c_str());
        }
        UInt numFIFATheBest = Utils::Clamp(player->GetStats()->GetNumFifaWorldPlayerAwards(), 0, 99);
        if (numFIFATheBest) {
            SetImageFilename(ext->TbTrophy[counter], L"art_fm\\lib\\Trophies\\64x64\\TheBestTrophy.tga");
            ext->TbTrophy[counter]->SetTooltip(GetTranslation("IDS_PLAYER_INFO_ALL_TIME_4"));
            ext->TbTrophyWins[counter++]->SetText(Utils::Format(L"%d", numFIFATheBest).c_str());
        }
        UInt numEuropeanBest = Utils::Clamp(player->GetStats()->GetNumEuropeanPlayerOfTheYear(), 0, 99);
        if (numEuropeanBest) {
            SetImageFilename(ext->TbTrophy[counter], L"art_fm\\lib\\Trophies\\64x64\\BestPlayerInEurope.tga");
            ext->TbTrophy[counter]->SetTooltip(GetTranslation("IDS_PLAYER_INFO_ALL_TIME_3"));
            ext->TbTrophyWins[counter++]->SetText(Utils::Format(L"%d", numEuropeanBest).c_str());
        }
        UInt numGoldenBoot = Utils::Clamp(PlayerStatsGetGoldenShoeWins(player->GetStats()), 0, 99);
        if (numGoldenBoot) {
            SetImageFilename(ext->TbTrophy[counter], L"art_fm\\lib\\Trophies\\64x64\\GoldenBoot.tga");
            ext->TbTrophy[counter]->SetTooltip(GetTranslation("IDS_GOLDENBOOT"));
            ext->TbTrophyWins[counter++]->SetText(Utils::Format(L"%d", numGoldenBoot).c_str());
        }
        UInt numPlayerOfTheYear = Utils::Clamp(player->GetStats()->GetNumPlayerOfTheYear(), 0, 99);
        if (numPlayerOfTheYear) {
            SetImageFilename(ext->TbTrophy[counter], L"art_fm\\lib\\Trophies\\64x64\\PlayerOfTheYear.tga");
            ext->TbTrophy[counter]->SetTooltip(GetTranslation("IDS_PLAYER_INFO_ALL_TIME_2"));
            ext->TbTrophyWins[counter++]->SetText(Utils::Format(L"%d", numPlayerOfTheYear).c_str());
        }
        UInt numPlayerOfTheMonth = Utils::Clamp(player->GetStats()->GetNumPlayerOfTheMonth(), 0, 99);
        if (numPlayerOfTheMonth) {
            SetImageFilename(ext->TbTrophy[counter], L"art_fm\\lib\\Trophies\\64x64\\PlayerOfTheMonth.tga");
            ext->TbTrophy[counter]->SetTooltip(GetTranslation("IDS_PLAYER_INFO_ALL_TIME_1"));
            ext->TbTrophyWins[counter++]->SetText(Utils::Format(L"%d", numPlayerOfTheMonth).c_str());
        }
        UInt numWC = Utils::Clamp(player->GetStats()->GetNumWCWins(), 0, 99);
        if (numWC) {
            SetImageFilename(ext->TbTrophy[counter], L"art_fm\\lib\\Trophies\\64x64\\WorldCup.tga");
            ext->TbTrophy[counter]->SetTooltip(GetTranslation("ID_TOURNAMENT_WORLDCUP"));
            ext->TbTrophyWins[counter++]->SetText(Utils::Format(L"%d", numWC).c_str());
        }
        for (UInt i = 0; i < counter; i++) {
            ext->TbTrophy[i]->SetVisible(true);
            ext->TbTrophyWins[i]->SetVisible(true);
            ext->ImgTrophyWins[i]->SetVisible(true);
        }
    }
    return playerId;
}

UInt METHOD OnGetNumPlayerOfTheYear(CPlayerStats *stats) {
    PlayerStatsSetBallonDOrWins(stats, PlayerStatsGetBallonDOrWins(stats) + 1);
    return PlayerStatsGetBallonDOrWins(stats);
}

void METHOD OnPlayerStatsInit(CPlayerStats *stats) {
    CallMethod<0x1005410>(stats);
    PlayerStatsInitAdditionalStats(stats);
}

void METHOD OnPlayerStatsSave(CPlayerStats *stats) {
    CallMethod<0x10054B0>(stats);
    void *save = *(void **)0x3179DD4;
    SaveGameWriteInt8(save, *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET));
}

void METHOD OnPlayerStatsLoad(CPlayerStats *stats) {
    CallMethod<0x10057F0>(stats);
    void *save = *(void **)0x3179DD8;
    if (SaveGameLoadGetVersion(save) >= 47) {
        SaveGameReadInt8(save, *raw_ptr<UChar>(stats, PLAYER_STATS_GS_AND_BD_OFFSET));
    }
}

CPlayerStats *METHOD PlayerStatsCopy(CPlayerStats *t, DUMMY_ARG, CPlayerStats *other) {
    memcpy(t, other, 128);
    return t;
}

void PatchEuropeanGoldenShoe(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x108F56C, OnReadFifaWorldPlayers); // CDBGame::ReadFifaWorldPlayers
        patch::RedirectCall(0xF5E178, OnLoadGameData_LoadGoldenShoeWinners);
        patch::RedirectCall(0xF5705B, OnSaveGameData_SaveGoldenShoeWinners);
        patch::RedirectCall(0xF5B106, OnInitGame);
        patch::RedirectCall(0x108D840, OnInitGame);
        patch::RedirectCall(0xF5FED2, OnDestroyGame);
        patch::RedirectCall(0x73865A, CreateStatsPlayerTrophiesWrapper);
        // CNewspaperGoldenBoot
        patch::SetUInt(0x4A07D4 + 1, NewspaperGoldenBootDefaultSize + sizeof(NewspaperGoldenBootExtension));
        patch::SetUInt(0x4A07DB + 1, NewspaperGoldenBootDefaultSize + sizeof(NewspaperGoldenBootExtension));
        patch::SetPointer(0x244E794, OnNewspaperGoldenBootCreateUI);
        patch::RedirectJump(0xA7BBF0, NewspaperGoldenBootFill);
        patch::RedirectCall(0xA78FD2, OnNewspaperGoldenBootConstruct);
        patch::RedirectCall(0xA79050, OnNewspaperGoldenBootDestruct);
        patch::Nop(0xF68DC1, 2); // always execute NewspaperGoldenBoot event
        patch::RedirectCall(0xF68DC7, OnNewspaperGoldenBootGetDayOfWeek); // set date to 01.06 if there are no playable European leagues
        patch::RedirectCall(0xF6EC22, OnShowGoldenBootScreen);
        patch::RedirectCall(0xF6EC2C, OnProcessGoldenBootScreen);
        // CPlayerInfoCareer
        patch::SetUInt(0x5C99D4 + 1, PlayerInfoCareerDefaultSize + sizeof(PlayerInfoCareerExtended));
        patch::SetUInt(0x5C99DB + 1, PlayerInfoCareerDefaultSize + sizeof(PlayerInfoCareerExtended));
        patch::SetPointer(0x23D0814, OnPlayerInfoCareerCreateUI);
        patch::RedirectCall(0x5D7250, OnPlayerInfoCareerFill);
        // player of the year (CDBGame)
        patch::RedirectCall(0xF64FF4, OnGetNumPlayerOfTheYear);
        // additional stats in CPlayerStats
        patch::RedirectCall(0x1006425, OnPlayerStatsInit);
        patch::RedirectCall(0x100644B, OnPlayerStatsInit);
        patch::RedirectCall(0xFC3E28, OnPlayerStatsSave);
        patch::RedirectCall(0xFC4AA8, OnPlayerStatsLoad);
        patch::RedirectJump(0xA9B040, PlayerStatsCopy);
        patch::RedirectJump(0x850230, PlayerStatsCopy);
    }
}
