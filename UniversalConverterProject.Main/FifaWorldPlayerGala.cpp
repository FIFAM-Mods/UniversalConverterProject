#include "FifaWorldPlayerGala.h"
#include "GameInterfaces.h"
#include "EuropeanGoldenShoe.h"
#include "FifamNation.h"
#include "shared.h"
#include <string>
#include <array>

using namespace plugin;

#define GAMEEVENT_BALLONDOR_CALC 49
#define GAMEEVENT_BALLONDOR_GALA 50

struct WorldPlayerGalaEntry {
    Int m_nPlayerId;
    Float m_fChance;
    Float m_fLastChange;
};

struct WorldPlayerGalaList {
    CDBPlayer **m_apPlayers;
    UInt m_nNumPlayers;
    UInt m_nCapacity;
    FmVec<WorldPlayerGalaEntry> m_list;
};

Vector<Pair<String, Float>> FifaNominantes = {
    /*  1 */ { L"ViniciusJunior12072000", 1.52f },
    /*  2 */ { L"Rodri22061996", 1.44f },
    /*  3 */ { L"BellinghamJu29062003", 1.34f }
};

Array<Int, 3> &GetBallonDOrCandidates() {
    static Array<Int, 3> ballonDOrCandidates = {};
    return ballonDOrCandidates;
}

void ClearBallonDOrCandidates() {
    for (UInt i = 0; i < 3; i++)
        GetBallonDOrCandidates()[i] = 0;
}

void SetGalaScreenType(CXgFMPanel *screen, UChar type) {
    *raw_ptr<UChar>(screen, 0x4AD) = type;
}

UChar GetGalaScreenType(CXgFMPanel *screen) {
    return *raw_ptr<UChar>(screen, 0x4AD);
}

void METHOD CollectWorldPlayerGalaNominantes(WorldPlayerGalaList *list) {
    // collect default
    CallMethod<0x1136190>(list);

    if (GetCurrentYear() == 2024) {
        Vector<CDBPlayer *> nominantesPlayers;
        for (UInt i = 0; i < 50; i++) {
            if (list->m_apPlayers[i])
                nominantesPlayers.emplace_back(list->m_apPlayers[i]);
            list->m_apPlayers[i] = 0;
        }
        // find all nominantes
        for (auto &n : FifaNominantes) {
            CDBPlayer *p = CallAndReturn<CDBPlayer *, 0xFAF750>(n.first.c_str());
            if (p) {
                auto it = std::find(nominantesPlayers.begin(), nominantesPlayers.end(), p);
                if (it == nominantesPlayers.end())
                    nominantesPlayers.emplace_back(p);
            }
        }
        // store final
        UInt finalSize = 50;
        if (nominantesPlayers.size() < finalSize)
            finalSize = nominantesPlayers.size();
        for (UInt i = 0; i < finalSize; i++)
            list->m_apPlayers[i] = nominantesPlayers[i];
    }
}

void METHOD CalcVotes(WorldPlayerGalaEntry *entry) {
    // call original
    CallMethod<0x1134EC0>(entry);

    if (GetCurrentYear() == 2024) {
        // check all nominantes
        for (auto &n : FifaNominantes) {
            CDBPlayer *p = CallAndReturn<CDBPlayer *, 0xFAF750>(n.first.c_str());
            if (p && entry->m_nPlayerId == *raw_ptr<unsigned int>(p, 0xD8)) {
                entry->m_fChance = 100.0f + 100.0f * n.second;
                break;
            }
        }
    }
}

Bool gbBallonDOrScreen = false;

WideChar const *METHOD FIFAWorldPlayerGalaGetTranslation2(void *, DUMMY_ARG, Char const *key) {
    return GetTranslation(gbBallonDOrScreen ? "IDS_NATIONALTEAM_GALA_17" : key);
}

WideChar const *METHOD FIFAWorldPlayerGalaGetTranslation5(void *, DUMMY_ARG, Char const *key) {
    return GetTranslation(gbBallonDOrScreen ? "IDS_NATIONALTEAM_GALA_20" : key);
}

WideChar const *METHOD FIFAWorldPlayerGalaGetTranslation1(void *, DUMMY_ARG, Char const *key) {
    return GetTranslation(gbBallonDOrScreen ? "IDS_NATIONALTEAM_GALA_15" : key);
}

void METHOD OnFIFAWorldPlayerGalaCreateUI(CXgFMPanel *screen) {
    Char const *screenName = *raw_ptr<Char const *>(screen, 0x28);
    gbBallonDOrScreen = screenName && strstr(screenName, "BallonDOr");
    CallMethod<0x894DD0>(screen);
    gbBallonDOrScreen = false;
    // TODO: remove this
//    static UChar data[0x54];
//    *raw_ptr<UChar *>(screen, 0x4A8) = data;
    //
}

UInt GetNumBallonDOrCandidates() {
    UInt counter = 0;
    for (UInt i = 0; i < 3; i++) {
        if (GetPlayer(GetBallonDOrCandidates()[i]))
            counter++;
    }
    return counter;
}

struct MatchesGoalsAssists {
    unsigned char matches = 0;
    unsigned char goals = 0;
    unsigned char assists = 0;
    unsigned char numMoTM = 0;
};

Float CalcBallonDOrPreviousYear(UChar position, UChar overall, MatchesGoalsAssists const &league,
    MatchesGoalsAssists const &cup, MatchesGoalsAssists const &continental, MatchesGoalsAssists const &international,
    Float averageMarkInLeagueMatches)
{
    const Float W_LEAGUE = 1.0f;
    const Float W_CUP = 0.8f;
    const Float W_CONTINENTAL = 1.5f;
    const Float W_INTERNATIONAL = 1.7f;
    float G_factor = 0.0f, A_factor = 0.0f, M_factor = 0.0f, T_factor = 0.0f;
    switch (position) {
    case TEAMPART_OFFENSE:   G_factor = 4; A_factor = 2; M_factor = 3; T_factor = 5; break;
    case TEAMPART_MIDFIELD:  G_factor = 2; A_factor = 3; M_factor = 4; T_factor = 5; break;
    case TEAMPART_DEFENSE:   G_factor = 1; A_factor = 2; M_factor = 5; T_factor = 6; break;
    case TEAMPART_GOALKEEPER:G_factor = 0.5f; A_factor = 1; M_factor = 6; T_factor = 7; break;
    default:                 G_factor = 1; A_factor = 1; M_factor = 1; T_factor = 1; break;
    }
    auto perf = [&](MatchesGoalsAssists const &m, float compWeight) -> float {
        return (m.goals * G_factor +
            m.assists * A_factor +
            m.matches * (M_factor * 0.2f) +
            m.numMoTM * T_factor)
            * compWeight;
        };
    Float score = 0.0f;
    score += perf(league, W_LEAGUE);
    score += perf(cup, W_CUP);
    score += perf(continental, W_CONTINENTAL);
    score += perf(international, W_INTERNATIONAL);
    score += averageMarkInLeagueMatches * 0.5f * M_factor;
    score *= (1.0f + (overall / 500.0f));
    return score;
}

void CalcBallonDOrCandidates() {
    struct BallonDOrCandidate {
        CDBPlayer *player;
        Float score;
    };
    Vector<BallonDOrCandidate> candidates;
    for (UInt countryId = 1; countryId <= 207; countryId++) {
        auto team = GetTeam(CTeamIndex::make(countryId, 0, 0xFFFF));
        if (team) {
            for (UInt i = 0; i < team->GetNumPlayers(); i++) {
                CDBPlayer *player = GetPlayer(team->GetPlayer(i));
                if (player) {
                    Float score = player->GetBasicLevel(player->GetBestPosition());
                    candidates.emplace_back(player, score);
                }
            }
        }
    }
    Utils::Sort(candidates, [](BallonDOrCandidate const &a, BallonDOrCandidate const &b) {
        return a.score > b.score;
    });
    if (candidates.size() > 50)
        candidates.resize(50);
    UInt year = GetCurrentYear() - 1;
    for (auto &entry : candidates) {
        auto player = entry.player;
        entry.score = 0.0f;
        // TODO: improve the formula for current season
        auto matchesGoals = GetPlayerMatchesGoalsList(player->GetID());
        if (matchesGoals && matchesGoals->GetNumEntries() > 0) {
            Float averageMark;
            UChar numAverageMarks = 0;
            MatchesGoalsAssists mga[5];
            for (UInt e = 0; e < matchesGoals->GetNumEntries(); e++) {
                if (matchesGoals->GetYear(e) == year) {
                    for (UInt c = 0; c < 5; c++) {
                        mga[c].matches = matchesGoals->GetNumMatches(e, c);
                        mga[c].goals = matchesGoals->GetNumGoals(e, c);
                        mga[c].assists = matchesGoals->GetNumAssists(e, c);
                        mga[c].numMoTM = matchesGoals->GetMOTM(e, c);
                    }
                    averageMark = matchesGoals->GetAverageMark(e);
                    numAverageMarks++;
                }
            }
            if (numAverageMarks > 0)
                averageMark /= (Float)numAverageMarks;

            entry.score = CalcBallonDOrPreviousYear(player->GetTeamPart(),
                player->GetBasicLevel(player->GetBestPosition()), mga[0], mga[2], mga[3], mga[4], averageMark);
        }
    }
    Utils::Sort(candidates, [](BallonDOrCandidate const &a, BallonDOrCandidate const &b) {
        return a.score > b.score;
    });
    ClearBallonDOrCandidates();
    for (UInt i = 0; i < candidates.size(); i++)
        GetBallonDOrCandidates()[i] = candidates[i].player->GetID();
    SafeLog::Write(L"CalcBallonDOrCandidates:");
    for (UInt i = 0; i < 3; i++)
        SafeLog::Write(PlayerName(GetBallonDOrCandidates()[i]));
}

void ValidateBallonDOrCandidates() {
    Vector<CDBPlayer *> finalCandidates;
    for (UInt i = 0; i < 3; i++) {
        CDBPlayer *player = GetPlayer(GetBallonDOrCandidates()[i]);
        if (player)
            finalCandidates.push_back(player);
    }
    SafeLog::Write(Utils::Format(L"ValidateBallonDOrCandidates: finalCandidates=%d", finalCandidates.size()));
    if (finalCandidates.size() == 3)
        return;
    struct BallonDOrCandidate {
        CDBPlayer *player;
        Float score;
    };
    Vector<BallonDOrCandidate> candidates;
    for (UInt countryId = 1; countryId <= 207; countryId++) {
        auto team = GetTeam(CTeamIndex::make(countryId, 0, 0xFFFF));
        if (team) {
            for (UInt i = 0; i < team->GetNumPlayers(); i++) {
                CDBPlayer *player = GetPlayer(team->GetPlayer(i));
                if (player && !Utils::Contains(finalCandidates, player)) {
                    Float score = player->GetBasicLevel(player->GetBestPosition());
                    candidates.emplace_back(player, score);
                }
            }
        }
    }
    Utils::Sort(candidates, [](BallonDOrCandidate const &a, BallonDOrCandidate const &b) {
        return a.score > b.score;
    });
    if (candidates.size() > 50)
        candidates.resize(50);
    UInt year = GetCurrentYear() - 1;
    for (auto &entry : candidates) {
        auto player = entry.player;
        entry.score = 0.0f;
        auto matchesGoals = GetPlayerMatchesGoalsList(player->GetID());
        if (matchesGoals && matchesGoals->GetNumEntries() > 0) {
            Float averageMark;
            UChar numAverageMarks = 0;
            MatchesGoalsAssists mga[5];
            for (UInt e = 0; e < matchesGoals->GetNumEntries(); e++) {
                if (matchesGoals->GetYear(e) == year) {
                    for (UInt c = 0; c < 5; c++) {
                        mga[c].matches = matchesGoals->GetNumMatches(e, c);
                        mga[c].goals = matchesGoals->GetNumGoals(e, c);
                        mga[c].assists = matchesGoals->GetNumAssists(e, c);
                        mga[c].numMoTM = matchesGoals->GetMOTM(e, c);
                    }
                    averageMark = matchesGoals->GetAverageMark(e);
                    numAverageMarks++;
                }
            }
            if (numAverageMarks > 0)
                averageMark /= (Float)numAverageMarks;
            
            entry.score = CalcBallonDOrPreviousYear(player->GetTeamPart(),
                player->GetBasicLevel(player->GetBestPosition()), mga[0], mga[2], mga[3], mga[4], averageMark);
        }
    }
    Utils::Sort(candidates, [](BallonDOrCandidate const &a, BallonDOrCandidate const &b) {
        return a.score > b.score;
    });
    UInt counter = 0;
    for (UInt i = finalCandidates.size(); i < 3; i++) {
        if (counter < candidates.size())
            finalCandidates.push_back(candidates[counter].player);
        else
            break;
        counter++;
    }
    ClearBallonDOrCandidates();
    for (UInt i = 0; i < finalCandidates.size(); i++)
        GetBallonDOrCandidates()[i] = finalCandidates[i]->GetID();
    SafeLog::Write(L"ValidateBallonDOrCandidates:");
    for (UInt i = 0; i < 3; i++)
        SafeLog::Write(PlayerName(GetBallonDOrCandidates()[i]));
}

void AwardBallonDOrWinner() {
    if (GetCurrentYear() == 2024 && GetCurrentYear() == GetStartingYear()) {
        WideChar const *Candidates[] = { L"Rodri22061996", L"ViniciusJunior12072000", L"BellinghamJu29062003" };
        CDBPlayer *winner = nullptr;
        for (UInt i = 0; i < 3; i++) {
            CDBPlayer *player = FindPlayerByStringID(Candidates[i]);
            GetBallonDOrCandidates()[i] = player ? player->GetID() : 0;
            if (i == 0)
                winner = player;
        }
        if (winner) {
            AddPlayerTrophy(PLAYERTROPHY_BALLONDOR, GetCurrentYear(), winner);
            if (GetNumBallonDOrCandidates() == 3 && GetBallonDOrCandidates()[0] == winner->GetID())
                GetNetwork().AddEvent(NETWORKEVENT_WORLD_PLAYER_GALA, 1, 0);
        }
        else
            AddPlayerTrophy(PLAYERTROPHY_BALLONDOR, GetCurrentYear(), L"Rodri", L"", Date(22, 6, 1996), FifamNation::Spain, 0x000E000E, false);
    }
    else {
        UInt numCandidates = GetNumBallonDOrCandidates();
        if (numCandidates > 0) {
            AddPlayerTrophy(PLAYERTROPHY_BALLONDOR, GetCurrentYear(), GetPlayer(GetBallonDOrCandidates()[0]));
            if (numCandidates == 3)
                GetNetwork().AddEvent(NETWORKEVENT_WORLD_PLAYER_GALA, 1, 0);
        }
    }
}

FmVec<WorldPlayerGalaEntry> *METHOD FIFAWorldPlayerGalaGetGetNominants(CDBGame *game, DUMMY_ARG, FmVec<WorldPlayerGalaEntry> *vec) {
    if (gbBallonDOrScreen) {
        const UInt NUM_NOMINANTS = 3;
        static WorldPlayerGalaEntry entries[NUM_NOMINANTS] = {};
        static Char vecBuf[sizeof(FmVec<WorldPlayerGalaEntry>)];
        FmVec<WorldPlayerGalaEntry> &dummyVec = *(FmVec<WorldPlayerGalaEntry> *)vecBuf;
        static FmVec<WorldPlayerGalaEntry>::Proxy dummyProxy;
        dummyProxy.myVec = &dummyVec;
        dummyVec.proxy = &dummyProxy;
        dummyVec.begin = &entries[0];
        dummyVec.end = dummyVec.end_buf = &entries[NUM_NOMINANTS];
        dummyVec._size = dummyVec.capacity = NUM_NOMINANTS;
        for (UInt i = 0; i < 3; i++)
            entries[i].m_nPlayerId = GetBallonDOrCandidates()[i];
        SafeLog::Write(L"Ballon D'Or nominants:");
        for (UInt i = 0; i < 3; i++)
            SafeLog::Write(PlayerName(GetBallonDOrCandidates()[i]));
        SafeLog::Write(L"Ballon D'Or nominants (dummyVec):");
        for (UInt i = 0; i < dummyVec.size(); i++)
            SafeLog::Write(PlayerName(dummyVec[i].m_nPlayerId));
        CallMethod<0x4F4B40>(vec, &dummyVec);
        SafeLog::Write(L"Ballon D'Or nominants (vec):");
        for (UInt i = 0; i < vec->size(); i++)
            SafeLog::Write(PlayerName((*vec)[i].m_nPlayerId));
    }
    else
        CallMethod<0xF5FAE0>(game, &vec);
    return vec;
}

Char const *GetFIFAWorldPlayerGalaScreenName(CNetworkEvent *evt) {
    return (evt->m_nShortID == 1) ? "Screens/10BallonDOrGala.xml" : "Screens/10NationalTeamFIFAWorldPlayerGala.xml";
}

void __declspec(naked) GetFIFAWorldPlayerGalaScreenName_Exe() {
    __asm {
        push ebp
        call GetFIFAWorldPlayerGalaScreenName
        pop ebp
        retn
    }
}

Bool METHOD OnGamePopEvent(CGameEvents *gameEvents, DUMMY_ARG, UInt eventType, CJDate date, CGameEvent &out) {
    Bool result = gameEvents->PopEvent(eventType, date, out);
    if (result) {
        switch (out.m_nEventId) {
        case GAMEEVENT_BALLONDOR_CALC:
            SafeLog::Write(Utils::Format(L"%s. Executing Ballon D'Or Calculation event", GetCurrentDate().ToStr()));
            CalcBallonDOrCandidates();
            break;
        case GAMEEVENT_BALLONDOR_GALA:
            SafeLog::Write(Utils::Format(L"%s. Executing Ballon D'Or Gala event", GetCurrentDate().ToStr()));
            ValidateBallonDOrCandidates();
            AwardBallonDOrWinner();
            break;
        }
    }
    return result;
}

CDBGame *OnGameStartNewSeason() {
    auto game = Game();
    GameEvents().AddEvent(0, CGameEvent(CJDate(game->GetCurrentYear(), 6, 29), 0, GAMEEVENT_BALLONDOR_CALC, 0, 0, 0, 0, 0, 0));
    SafeLog::Write(L"Registered Ballon D'Or Calculation event");
    GameEvents().AddEvent(0, CGameEvent(CJDate(game->GetCurrentYear(), 9, 22), 0, GAMEEVENT_BALLONDOR_GALA, 0, 0, 0, 0, 0, 0));
    SafeLog::Write(L"Registered Ballon D'Or Gala event");
    return game;
}

void PatchFifaWorldPlayerGala(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // collecting players
        patch::RedirectCall(0x1136598, CollectWorldPlayerGalaNominantes);
        // calc votes
        patch::RedirectCall(0x1136503, CalcVotes);
        patch::RedirectCall(0x1135501, CalcVotes);
        // voting date
        patch::SetUChar(0xF68B09 + 1, 25); // 10
        patch::SetUChar(0xF68B0E + 1, 11); // August
        // ceremony date
        patch::SetUChar(0xF68B52 + 1, 17); // 24
        patch::SetUChar(0xF68B57 + 1, 12); // September
        patch::SetUChar(0x1099432 + 1, 17); // 24
        patch::SetUChar(0x1099434 + 1, 12); // September

        patch::SetPointer(0x241A884, OnFIFAWorldPlayerGalaCreateUI);
        patch::RedirectCall(0x895590, FIFAWorldPlayerGalaGetTranslation2);
        patch::RedirectCall(0x8955D5, FIFAWorldPlayerGalaGetTranslation5);
        patch::RedirectCall(0x89561A, FIFAWorldPlayerGalaGetTranslation1);
        patch::RedirectCall(0x895659, FIFAWorldPlayerGalaGetGetNominants);
        patch::RedirectCall(0x4B32CD, GetFIFAWorldPlayerGalaScreenName_Exe);

        // new game events
        patch::RedirectCall(0xF6E1C0, OnGamePopEvent);
        patch::RedirectCall(0xF6EE38, OnGamePopEvent);
        patch::RedirectCall(0xF68BC0, OnGameStartNewSeason);
    }
}
