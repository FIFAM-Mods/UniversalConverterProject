#include "FifaWorldPlayerGala.h"
#include "GameInterfaces.h"
#include "EuropeanGoldenShoe.h"
#include "FifamNation.h"
#include <string>
#include <array>

using namespace plugin;

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
    /*  1 */ { L"MessiLi24061987", 1.52f },
    /*  2 */ { L"MbappeKy20121998", 1.44f },
    /*  3 */ { L"BenzemaKa19121987", 1.34f }
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

    if (GetCurrentYear() == 2022) {
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

    if (GetCurrentYear() == 2022) {
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
    static UChar data[0x54];
    *raw_ptr<UChar *>(screen, 0x4A8) = data;
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

UInt CalcBallonDOrCandidates(Bool currentSeason) {
    Vector<CDBPlayer *> finalCandidates;
    if (!currentSeason) {
        for (UInt i = 0; i < 3; i++) {
            CDBPlayer *player = GetPlayer(GetBallonDOrCandidates()[i]);
            if (player)
                finalCandidates.push_back(player);
        }
        if (finalCandidates.size() == 3)
            return 3;
    }
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
                    Float score = player->GetLevel(player->GetMainPosition());
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
    for (auto &entry : candidates) {
        entry.score = 0.0f;
        auto matchesGoals = GetPlayerMatchesGoalsList(entry.player->GetID());
        if (matchesGoals && matchesGoals->GetNumEntries() > 0) {
            UShort entryId = matchesGoals->GetNumEntries() > 1 ? matchesGoals->GetNumEntries() - (currentSeason ? 1 : 2) : 0;

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
    return finalCandidates.size();
}

void METHOD CalcBallonDOrCandidates(CDBGame *game, DUMMY_ARG, UChar) {
    CalcBallonDOrCandidates(true);
}

void METHOD AwardBallonDOrWinner(CDBGame *game, DUMMY_ARG, UChar) {
    if (GetCurrentYear() == GetStartingYear()) {
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
            if (CalcBallonDOrCandidates(false) == 3 && GetBallonDOrCandidates()[0] == winner->GetID())
                GetNetwork().AddEvent(NETWORKEVENT_WORLD_PLAYER_GALA, 1, 0);
        }
        else
            AddPlayerTrophy(PLAYERTROPHY_BALLONDOR, GetCurrentYear(), L"Rodri", L"", Date(22, 6, 1996), FifamNation::Spain, 0x000E000E, false);
    }
    else {
        UInt numCandidates = CalcBallonDOrCandidates(false);
        if (numCandidates > 0) {
            AddPlayerTrophy(PLAYERTROPHY_BALLONDOR, GetCurrentYear(), GetPlayer(GetBallonDOrCandidates()[0]));
            if (numCandidates == 3)
                GetNetwork().AddEvent(NETWORKEVENT_WORLD_PLAYER_GALA, 1, 0);
        }
    }
    ClearBallonDOrCandidates();
}

FmVec<WorldPlayerGalaEntry> *METHOD FIFAWorldPlayerGalaGetGetNominants(CDBGame *game, DUMMY_ARG, FmVec<WorldPlayerGalaEntry> *vec) {
    //

    WideChar const *Candidates[] = { L"Rodri22061996", L"ViniciusJunior12072000", L"BellinghamJu29062003" };
    CDBPlayer *winner = nullptr;
    for (UInt i = 0; i < 3; i++) {
        CDBPlayer *player = FindPlayerByStringID(Candidates[i]);
        GetBallonDOrCandidates()[i] = player ? player->GetID() : 0;
        if (i == 0)
            winner = player;
    }

    //
    if (gbBallonDOrScreen) {
        const UInt NUM_NOMINANTS = 3;
        static WorldPlayerGalaEntry entries[NUM_NOMINANTS] = {};
        static FmVec<WorldPlayerGalaEntry> dummyVec = {};
        static FmVec<WorldPlayerGalaEntry>::Proxy dummyProxy;
        dummyProxy.myVec = &dummyVec;
        dummyVec.proxy = &dummyProxy;
        dummyVec.begin = &entries[0];
        dummyVec.end = dummyVec.end_buf = &entries[NUM_NOMINANTS];
        dummyVec._size = dummyVec.capacity = NUM_NOMINANTS;
        //ValidateBallonDOrCandidates();
        for (UInt i = 0; i < 3; i++)
            entries[i].m_nPlayerId = GetBallonDOrCandidates()[i];
        CallMethod<0x4F4B40>(vec, &dummyVec);
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
    }
}
