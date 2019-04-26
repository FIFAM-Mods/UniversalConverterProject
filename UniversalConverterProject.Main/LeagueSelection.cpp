#include "LeagueSelection.h"

using namespace plugin;

struct RecommendedLeague {
    unsigned char countryId;
    unsigned char level;
};

struct RecommendedLeagues {
    RecommendedLeague leagues[8];
};

#define GAME_VER_ENG 0
#define GAME_VER_GER 1
#define GAME_VER_FRA 2
#define GAME_VER_SPA 3
#define GAME_VER_ITA 4
#define GAME_VER_POL 5

void PatchLeagueSelection(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // remove special places for Germany, Spain and Scotland
        patch::RedirectJump(0xF8E60A, (void *)0xF8E6DF);
        // enable 'IGNORE_LEAGUE_LIMITATION'
        patch::Nop(0x5424D3, 2);
        
        // league selection
        RecommendedLeagues *ptr = (RecommendedLeagues *)0x23BDE10;

        DWORD oldProtect;
        VirtualProtect(ptr, 2 * 8 * 6, PAGE_READWRITE, &oldProtect);

        for (auto i = 0; i < 6; i++) {
            for (auto j = 0; j < 8; j++) {
                ptr[i].leagues[j].countryId = 0;
                ptr[i].leagues[j].level = 0;
            }
        }

        ptr[GAME_VER_ENG].leagues[0].countryId = 14; // England
        ptr[GAME_VER_ENG].leagues[0].level = 1;
        ptr[GAME_VER_ENG].leagues[1].countryId = 45; // Spain
        ptr[GAME_VER_ENG].leagues[1].level = 0;
        ptr[GAME_VER_ENG].leagues[2].countryId = 27; // Italy
        ptr[GAME_VER_ENG].leagues[2].level = 0;
        ptr[GAME_VER_ENG].leagues[3].countryId = 21; // Germany
        ptr[GAME_VER_ENG].leagues[3].level = 0;
        ptr[GAME_VER_ENG].leagues[4].countryId = 18; // France
        ptr[GAME_VER_ENG].leagues[4].level = 0;

        ptr[GAME_VER_GER].leagues[0].countryId = 21; // Germany
        ptr[GAME_VER_GER].leagues[0].level = 2;
        ptr[GAME_VER_GER].leagues[1].countryId = 14; // England
        ptr[GAME_VER_GER].leagues[1].level = 0;
        ptr[GAME_VER_GER].leagues[2].countryId = 45; // Spain
        ptr[GAME_VER_GER].leagues[2].level = 0;
        ptr[GAME_VER_GER].leagues[3].countryId = 27; // Italy
        ptr[GAME_VER_GER].leagues[3].level = 0;
        ptr[GAME_VER_GER].leagues[4].countryId = 18; // France
        ptr[GAME_VER_GER].leagues[4].level = 0;

        ptr[GAME_VER_FRA].leagues[0].countryId = 18; // France
        ptr[GAME_VER_FRA].leagues[0].level = 2;
        ptr[GAME_VER_FRA].leagues[1].countryId = 14; // England
        ptr[GAME_VER_FRA].leagues[1].level = 0;
        ptr[GAME_VER_FRA].leagues[2].countryId = 45; // Spain
        ptr[GAME_VER_FRA].leagues[2].level = 0;
        ptr[GAME_VER_FRA].leagues[3].countryId = 27; // Italy
        ptr[GAME_VER_FRA].leagues[3].level = 0;
        ptr[GAME_VER_FRA].leagues[4].countryId = 21; // Germany
        ptr[GAME_VER_FRA].leagues[4].level = 0;

        ptr[GAME_VER_SPA].leagues[0].countryId = 45; // Spain
        ptr[GAME_VER_SPA].leagues[0].level = 2;
        ptr[GAME_VER_SPA].leagues[1].countryId = 14; // England
        ptr[GAME_VER_SPA].leagues[1].level = 0;
        ptr[GAME_VER_SPA].leagues[2].countryId = 27; // Italy
        ptr[GAME_VER_SPA].leagues[2].level = 0;
        ptr[GAME_VER_SPA].leagues[3].countryId = 21; // Germany
        ptr[GAME_VER_SPA].leagues[3].level = 0;
        ptr[GAME_VER_SPA].leagues[4].countryId = 18; // France
        ptr[GAME_VER_SPA].leagues[4].level = 0;

        ptr[GAME_VER_ITA].leagues[0].countryId = 27; // Italy
        ptr[GAME_VER_ITA].leagues[0].level = 2;
        ptr[GAME_VER_ITA].leagues[1].countryId = 14; // England
        ptr[GAME_VER_ITA].leagues[1].level = 0;
        ptr[GAME_VER_ITA].leagues[2].countryId = 45; // Spain
        ptr[GAME_VER_ITA].leagues[2].level = 0;
        ptr[GAME_VER_ITA].leagues[3].countryId = 21; // Germany
        ptr[GAME_VER_ITA].leagues[3].level = 0;
        ptr[GAME_VER_ITA].leagues[4].countryId = 18; // France
        ptr[GAME_VER_ITA].leagues[4].level = 0;

        ptr[GAME_VER_POL].leagues[0].countryId = 37; // Poland
        ptr[GAME_VER_POL].leagues[0].level = 1;
        ptr[GAME_VER_POL].leagues[1].countryId = 14; // England
        ptr[GAME_VER_POL].leagues[1].level = 0;
        ptr[GAME_VER_POL].leagues[2].countryId = 45; // Spain
        ptr[GAME_VER_POL].leagues[2].level = 0;
        ptr[GAME_VER_POL].leagues[3].countryId = 27; // Italy
        ptr[GAME_VER_POL].leagues[3].level = 0;
        ptr[GAME_VER_POL].leagues[4].countryId = 21; // Germany
        ptr[GAME_VER_POL].leagues[4].level = 0;
        ptr[GAME_VER_POL].leagues[5].countryId = 18; // France
        ptr[GAME_VER_POL].leagues[5].level = 0;

        VirtualProtect(ptr, 2 * 8 * 6, oldProtect, &oldProtect);
    }
    else if (v.id() == ID_FM_11_1003) {
        // remove special places for Germany, Spain and Scotland
        patch::SetUChar(0xDFC231, 0xEB);
        patch::SetUChar(0xDFC241, 0xEB);
        patch::Nop(0xDFC1FA, 2);
        patch::SetUChar(0xDFC201, 0xEB);
        patch::RedirectJump(0xF63A7A, (void *)0xF63B00);

        // enable 'IGNORE_LEAGUE_LIMITATION'
        patch::Nop(0x4E1531, 2);

        // league selection
        RecommendedLeagues *ptr = (RecommendedLeagues *)0x12742E0;

        DWORD oldProtect;
        VirtualProtect(ptr, 2 * 8 * 6, PAGE_READWRITE, &oldProtect);

        for (auto i = 0; i < 6; i++) {
            for (auto j = 0; j < 8; j++) {
                ptr[i].leagues[j].countryId = 0;
                ptr[i].leagues[j].level = 0;
            }
        }

        ptr[GAME_VER_ENG].leagues[0].countryId = 14; // England
        ptr[GAME_VER_ENG].leagues[0].level = 1;
        ptr[GAME_VER_ENG].leagues[1].countryId = 45; // Spain
        ptr[GAME_VER_ENG].leagues[1].level = 0;
        ptr[GAME_VER_ENG].leagues[2].countryId = 27; // Italy
        ptr[GAME_VER_ENG].leagues[2].level = 0;
        ptr[GAME_VER_ENG].leagues[3].countryId = 21; // Germany
        ptr[GAME_VER_ENG].leagues[3].level = 0;
        ptr[GAME_VER_ENG].leagues[4].countryId = 18; // France
        ptr[GAME_VER_ENG].leagues[4].level = 0;

        ptr[GAME_VER_GER].leagues[0].countryId = 21; // Germany
        ptr[GAME_VER_GER].leagues[0].level = 2;
        ptr[GAME_VER_GER].leagues[1].countryId = 14; // England
        ptr[GAME_VER_GER].leagues[1].level = 0;
        ptr[GAME_VER_GER].leagues[2].countryId = 45; // Spain
        ptr[GAME_VER_GER].leagues[2].level = 0;
        ptr[GAME_VER_GER].leagues[3].countryId = 27; // Italy
        ptr[GAME_VER_GER].leagues[3].level = 0;
        ptr[GAME_VER_GER].leagues[4].countryId = 18; // France
        ptr[GAME_VER_GER].leagues[4].level = 0;

        ptr[GAME_VER_FRA].leagues[0].countryId = 18; // France
        ptr[GAME_VER_FRA].leagues[0].level = 2;
        ptr[GAME_VER_FRA].leagues[1].countryId = 14; // England
        ptr[GAME_VER_FRA].leagues[1].level = 0;
        ptr[GAME_VER_FRA].leagues[2].countryId = 45; // Spain
        ptr[GAME_VER_FRA].leagues[2].level = 0;
        ptr[GAME_VER_FRA].leagues[3].countryId = 27; // Italy
        ptr[GAME_VER_FRA].leagues[3].level = 0;
        ptr[GAME_VER_FRA].leagues[4].countryId = 21; // Germany
        ptr[GAME_VER_FRA].leagues[4].level = 0;

        ptr[GAME_VER_SPA].leagues[0].countryId = 45; // Spain
        ptr[GAME_VER_SPA].leagues[0].level = 2;
        ptr[GAME_VER_SPA].leagues[1].countryId = 14; // England
        ptr[GAME_VER_SPA].leagues[1].level = 0;
        ptr[GAME_VER_SPA].leagues[2].countryId = 27; // Italy
        ptr[GAME_VER_SPA].leagues[2].level = 0;
        ptr[GAME_VER_SPA].leagues[3].countryId = 21; // Germany
        ptr[GAME_VER_SPA].leagues[3].level = 0;
        ptr[GAME_VER_SPA].leagues[4].countryId = 18; // France
        ptr[GAME_VER_SPA].leagues[4].level = 0;

        ptr[GAME_VER_ITA].leagues[0].countryId = 27; // Italy
        ptr[GAME_VER_ITA].leagues[0].level = 2;
        ptr[GAME_VER_ITA].leagues[1].countryId = 14; // England
        ptr[GAME_VER_ITA].leagues[1].level = 0;
        ptr[GAME_VER_ITA].leagues[2].countryId = 45; // Spain
        ptr[GAME_VER_ITA].leagues[2].level = 0;
        ptr[GAME_VER_ITA].leagues[3].countryId = 21; // Germany
        ptr[GAME_VER_ITA].leagues[3].level = 0;
        ptr[GAME_VER_ITA].leagues[4].countryId = 18; // France
        ptr[GAME_VER_ITA].leagues[4].level = 0;

        ptr[GAME_VER_POL].leagues[0].countryId = 37; // Poland
        ptr[GAME_VER_POL].leagues[0].level = 1;
        ptr[GAME_VER_POL].leagues[1].countryId = 14; // England
        ptr[GAME_VER_POL].leagues[1].level = 0;
        ptr[GAME_VER_POL].leagues[2].countryId = 45; // Spain
        ptr[GAME_VER_POL].leagues[2].level = 0;
        ptr[GAME_VER_POL].leagues[3].countryId = 27; // Italy
        ptr[GAME_VER_POL].leagues[3].level = 0;
        ptr[GAME_VER_POL].leagues[4].countryId = 21; // Germany
        ptr[GAME_VER_POL].leagues[4].level = 0;
        ptr[GAME_VER_POL].leagues[5].countryId = 18; // France
        ptr[GAME_VER_POL].leagues[5].level = 0;

        VirtualProtect(ptr, 2 * 8 * 6, oldProtect, &oldProtect);
    }
}
