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
    /*  1 */ { L"MessiLi24061987", 1.52f },
    /*  2 */ { L"MbappeKy20121998", 1.44f },
    /*  3 */ { L"BenzemaKa19121987", 1.34f }
};

unsigned short GetCurrentYear2() {
    CDBGame *game = CallAndReturn<CDBGame *, 0xF61410>();
    return CallMethodAndReturn<unsigned short, 0xF498C0>(game);
}

void METHOD CollectWorldPlayerGalaNominantes(WorldPlayerGalaList *list) {
    // collect default
    CallMethod<0x1136190>(list);
    
    if (GetCurrentYear2() == 2022) {
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

    if (GetCurrentYear2() == 2022) {
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
        patch::SetUChar(0xF68B09 + 1, 25); // 10
        patch::SetUChar(0xF68B0E + 1, 11); // August
        // ceremony date
        patch::SetUChar(0xF68B52 + 1, 17); // 24
        patch::SetUChar(0xF68B57 + 1, 12); // September

        patch::SetUChar(0x1099432 + 1, 17); // 24
        patch::SetUChar(0x1099434 + 1, 12); // September
    }
}
