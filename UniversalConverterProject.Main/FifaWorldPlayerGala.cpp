#include "FifaWorldPlayerGala.h"
#include <string>
#include <array>

using namespace plugin;

class CDBPlayer;
class CDBGame;

template<typename T>
struct FmVec {
    T *data;
    unsigned int size;
    unsigned int capacity;
    T *begin;
    T *end;
    T *end_buf;
};

struct WorldPlayerGalaEntry {
    int m_nPlayerId;
    float m_fChance;
    float m_fLastChange;
};

struct WorldPlayerGalaList {
    CDBPlayer **m_apPlayers;
    unsigned int m_nNumPlayers;
    unsigned int m_nCapacity;
    FmVec<WorldPlayerGalaEntry> m_list;
};

std::vector<std::pair<std::wstring, float>> FifaNominantes = {
    /*  1 */ { L"ModricLu09091985", 29.05f },
    /*  2 */ { L"CristianoRonaldo05021985", 19.08f },
    /*  3 */ { L"SalahMo15061992", 11.23f },
    /*  4 */ { L"MbappeKy20121998", 10.52f },
    /*  5 */ { L"MessiLi24061987", 9.81f },
    /*  6 */ { L"GriezmannAn21031991", 6.69f },
    /*  7 */ { L"HazardEd07011991", 5.65f },
    /*  8 */ { L"DeBruyneKe28061991", 3.54f },
    /*  9 */ { L"VaraneRa25041993", 3.45f },
    /* 10 */ { L"KaneHa28071993", 0.98f }
};

unsigned short GetCurrentYear2() {
    CDBGame *game = CallAndReturn<CDBGame *, 0xF61410>();
    return CallMethodAndReturn<unsigned short, 0xF498C0>(game);
}

void METHOD CollectWorldPlayerGalaNominantes(WorldPlayerGalaList *list) {
    // collect default
    CallMethod<0x1136190>(list);
    
    if (GetCurrentYear2() == 2018) {
        std::vector<CDBPlayer *> nominantesPlayers;
        for (unsigned int i = 0; i < 50; i++) {
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
        unsigned int finalSize = 50;
        if (nominantesPlayers.size() < finalSize)
            finalSize = nominantesPlayers.size();
        for (unsigned int i = 0; i < finalSize; i++)
            list->m_apPlayers[i] = nominantesPlayers[i];
    }
}

void METHOD CalcVotes(WorldPlayerGalaEntry *entry) {
    // call original
    CallMethod<0x1134EC0>(entry);

    if (GetCurrentYear2() == 2018) {
        // check all nominantes
        for (auto &n : FifaNominantes) {
            CDBPlayer *p = CallAndReturn<CDBPlayer *, 0xFAF750>(n.first.c_str());
            if (p && entry->m_nPlayerId == *raw_ptr<unsigned int>(p, 0xD8)) {
                entry->m_fChance = 100.0f + 100.0f * n.second;
                break;
            }
        }
    }

    //FILE *f = fopen("GalaVotes.txt", "at");
    //CDBPlayer *player = CallAndReturn<CDBPlayer *, 0xF97C70>(entry->m_nPlayerId);
    //wchar_t playerName[256];
    //CallMethod<0x13255C0>(0, player, playerName, false);
    //fwprintf(f, L"%s\t%f\t%f\n", playerName, entry->m_fChance, entry->m_fLastChange);
    //fclose(f);
}

void PatchFifaWorldPlayerGala(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // collecting players
        patch::RedirectCall(0x1136598, CollectWorldPlayerGalaNominantes);
        // calc votes
        patch::RedirectCall(0x1136503, CalcVotes);
        patch::RedirectCall(0x1135501, CalcVotes);
        // voting date
        patch::SetUChar(0xF68B09 + 1, 10); // 10
        patch::SetUChar(0xF68B0E + 1, 8); // August
        // ceremony date
        patch::SetUChar(0xF68B52 + 1, 24); // 24
        patch::SetUChar(0xF68B57 + 1, 9); // September
    }
}
