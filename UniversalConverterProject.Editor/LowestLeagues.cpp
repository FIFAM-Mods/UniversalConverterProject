#include "LowestLeagues.h"

using namespace plugin;

class CDBCompetition;
class CComboBox;
class CCountry;
class CClub;

void METHOD SetupLowestLeagues_13(int _this) {
    if (*(DWORD *)(_this + 15560) && *(DWORD *)(_this + 15556)) {
        HWND *wnd = (HWND *)(_this + 3892);
        for (int i = 0; i < 3; i++) {
            SendMessageW(*wnd, 331, 0, 0);
            wchar_t *v4 = CallMethodAndReturn<wchar_t *, 0x5808E0>((void *)_this, "Common.NotAvailable", 0);
            LRESULT v5 = SendMessageW(*wnd, 323, 0, (LPARAM)v4);
            if (v5 != -1)
                SendMessageW(*wnd, 337, v5, 0);
            wnd += 21;
        }

        int numLeaguesOnLevels[32];
        int leagueIndices[32];
        memset(leagueIndices, 0, 128);
        memset(numLeaguesOnLevels, 0, 128);

        int numLevels = CallMethodAndReturn<int, 0x4DD550>(*(CCountry **)(_this + 15556), leagueIndices, numLeaguesOnLevels);
        if (numLevels > 0) {
            for (int i = 0; i < numLevels; i++) {
                if (numLeaguesOnLevels[i] > 1) {
                    for (int l = 0; l < numLeaguesOnLevels[i]; l++) {
                        CDBCompetition *comp = CallMethodAndReturn<CDBCompetition *, 0x4DD4D0>(*(CCountry **)(_this + 15556), i, l);
                        if (comp) {
                            int comboBox = _this + 3860;
                            for (int c = 0; c < 3; c++) {
                                unsigned int compId = CallMethodAndReturn<unsigned int, 0x4FC3B0>(comp);
                                const wchar_t *compName = CallMethodAndReturn<const wchar_t *, 0x4FEA90>(comp);
                                CallMethod<0x4138A0>(0, comboBox, compName, compId);
                                comboBox += 84;
                            }
                        }
                    }
                }
            }
        }
        int v14 = CallMethodAndReturn<int, 0x4C3000>(*(CClub **)(_this + 15560), 0);
        CallMethod<0x4138E0>(0, (CComboBox *)(_this + 3860), v14);
        int v15 = CallMethodAndReturn<int, 0x4C3000>(*(CClub **)(_this + 15560), 1);
        CallMethod<0x4138E0>(0, (CComboBox *)(_this + 3944), v15);
        int v16 = CallMethodAndReturn<int, 0x4C3000>(*(CClub **)(_this + 15560), 2);
        CallMethod<0x4138E0>(0, (CComboBox *)(_this + 4028), v16);
    }
}

void PatchLowestLeagues(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::RedirectJump(0x429D70, SetupLowestLeagues_13);
    }
}
