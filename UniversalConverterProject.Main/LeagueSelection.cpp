#include "LeagueSelection.h"
#include "GameInterfaces.h"
#include "FifamCompType.h"
#include "FifamCompRegion.h"
#include "DatabaseOptions.h"
#include "Translation.h"
#include "FifamNation.h"

using namespace plugin;

enum RecommendedLeagueLevel {
    LEVEL_1 = 0,
    LEVEL_2 = 1,
    LEVEL_3 = 2,
    LEVEL_4 = 3,
    LEVEL_5 = 4
};

struct RecommendedLeague {
    UChar countryId;
    UChar level;

    RecommendedLeague(UChar _countryId, UChar _level) {
        countryId = _countryId;
        level = _level;
    }

    RecommendedLeague(FifamNation _countryId, RecommendedLeagueLevel _level) {
        countryId = _countryId.ToInt();
        level = _level;
    }
};

struct RecommendedLeagues {
    RecommendedLeague leagues[8];
};

void SetPreferredLeagues(Vector<RecommendedLeague> const &leagues) {
    const UInt RecommendedLeaguesAddr = 0x23BDE10;
    const UInt EntriesCount = 8 * 6;
    const UInt MaxEntries = EntriesCount - 1;
    injector::MemoryFill(RecommendedLeaguesAddr, 0, EntriesCount * 2, true);
    for (UInt i = 0; i < Utils::Min(leagues.size(), MaxEntries); i++)
        patch::Set(RecommendedLeaguesAddr + i * 2, leagues[i]);
}

void DecidePreferredLeagues() {
    if (!IsWomenDatabase()) {
        if (GameLanguage() == L"eng") {
            if (DefaultCurrency == CURRENCY_GBP) {
                SetPreferredLeagues({
                    { (UChar)FifamNation::England,     LEVEL_4 },
                    { (UChar)FifamNation::Scotland,    LEVEL_2 },
                    { (UChar)FifamNation::Spain,       LEVEL_2 },
                    { (UChar)FifamNation::Italy,       LEVEL_2 },
                    { (UChar)FifamNation::Germany,     LEVEL_2 },
                    { (UChar)FifamNation::France,      LEVEL_2 },
                    { (UChar)FifamNation::Portugal,    LEVEL_1 },
                    { (UChar)FifamNation::Netherlands, LEVEL_1 },
                    { (UChar)FifamNation::Brazil,      LEVEL_1 },
                    { (UChar)FifamNation::Argentina,   LEVEL_1 }
                });
            }
            else if (DefaultCurrency == CURRENCY_USD) {
                SetPreferredLeagues({
                    { (UChar)FifamNation::United_States, LEVEL_2 },
                    { (UChar)FifamNation::England,     LEVEL_2 },
                    { (UChar)FifamNation::Spain,       LEVEL_2 },
                    { (UChar)FifamNation::Italy,       LEVEL_2 },
                    { (UChar)FifamNation::Germany,     LEVEL_2 },
                    { (UChar)FifamNation::France,      LEVEL_2 },
                    { (UChar)FifamNation::Portugal,    LEVEL_1 },
                    { (UChar)FifamNation::Netherlands, LEVEL_1 },
                    { (UChar)FifamNation::Mexico,      LEVEL_1 },
                    { (UChar)FifamNation::Brazil,      LEVEL_1 },
                    { (UChar)FifamNation::Argentina,   LEVEL_1 }
                });
            }
            else {
                SetPreferredLeagues({
                    { (UChar)FifamNation::England,     LEVEL_3 },
                    { (UChar)FifamNation::Spain,       LEVEL_2 },
                    { (UChar)FifamNation::Italy,       LEVEL_2 },
                    { (UChar)FifamNation::Germany,     LEVEL_2 },
                    { (UChar)FifamNation::France,      LEVEL_2 },
                    { (UChar)FifamNation::Portugal,    LEVEL_1 },
                    { (UChar)FifamNation::Netherlands, LEVEL_1 },
                    { (UChar)FifamNation::Brazil,      LEVEL_1 },
                    { (UChar)FifamNation::Argentina,   LEVEL_1 }
                });
            }
        }
        else if (GameLanguage() == L"ger") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Germany,     LEVEL_3 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"fre") {
            SetPreferredLeagues({
                { (UChar)FifamNation::France,      LEVEL_3 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"spa") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Spain,       LEVEL_3 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"ita") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Italy,       LEVEL_3 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"pol") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Poland,      LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"ukr") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Ukraine,     LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"cze") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Czech_Republic, LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"hun") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Hungary,     LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"por") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Portugal,    LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Brazil,      LEVEL_2 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"tur") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Turkey,      LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"chi") {
            SetPreferredLeagues({
                { (UChar)FifamNation::China_PR,    LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"kor") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Korea_Republic, LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
        else {
            SetPreferredLeagues({
                { (UChar)FifamNation::England,     LEVEL_3 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Netherlands, LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 }
            });
        }
    }
    else {
        if (GameLanguage() == L"eng") {
            if (DefaultCurrency == CURRENCY_GBP) {
                SetPreferredLeagues({
                    { (UChar)FifamNation::England,     LEVEL_3 },
                    { (UChar)FifamNation::Scotland,    LEVEL_1 },
                    { (UChar)FifamNation::Wales,       LEVEL_1 },
                    { (UChar)FifamNation::Spain,       LEVEL_2 },
                    { (UChar)FifamNation::Italy,       LEVEL_2 },
                    { (UChar)FifamNation::Germany,     LEVEL_2 },
                    { (UChar)FifamNation::France,      LEVEL_2 },
                    { (UChar)FifamNation::Portugal,    LEVEL_1 },
                    { (UChar)FifamNation::Sweden,      LEVEL_1 },
                    { (UChar)FifamNation::Brazil,      LEVEL_1 },
                    { (UChar)FifamNation::Argentina,   LEVEL_1 },
                    { (UChar)FifamNation::United_States, LEVEL_1 }
                });
            }
            else if (DefaultCurrency == CURRENCY_USD) {
                SetPreferredLeagues({
                    { (UChar)FifamNation::United_States, LEVEL_2 },
                    { (UChar)FifamNation::Mexico,      LEVEL_1 },
                    { (UChar)FifamNation::Canada,      LEVEL_1 },
                    { (UChar)FifamNation::England,     LEVEL_2 },
                    { (UChar)FifamNation::Spain,       LEVEL_2 },
                    { (UChar)FifamNation::Italy,       LEVEL_2 },
                    { (UChar)FifamNation::Germany,     LEVEL_2 },
                    { (UChar)FifamNation::France,      LEVEL_1 },
                    { (UChar)FifamNation::Portugal,    LEVEL_1 },
                    { (UChar)FifamNation::Sweden,      LEVEL_1 },
                    { (UChar)FifamNation::Brazil,      LEVEL_1 },
                    { (UChar)FifamNation::Argentina,   LEVEL_1 }
                });
            }
            else {
                SetPreferredLeagues({
                    { (UChar)FifamNation::England,     LEVEL_2 },
                    { (UChar)FifamNation::Spain,       LEVEL_2 },
                    { (UChar)FifamNation::Italy,       LEVEL_2 },
                    { (UChar)FifamNation::Germany,     LEVEL_2 },
                    { (UChar)FifamNation::France,      LEVEL_2 },
                    { (UChar)FifamNation::Portugal,    LEVEL_1 },
                    { (UChar)FifamNation::Sweden,      LEVEL_1 },
                    { (UChar)FifamNation::Brazil,      LEVEL_1 },
                    { (UChar)FifamNation::Argentina,   LEVEL_1 },
                    { (UChar)FifamNation::United_States, LEVEL_1 }
                });
            }
        }
        else if (GameLanguage() == L"ger") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Austria,     LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"fre") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Belgium,     LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"spa") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Spain,       LEVEL_3 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"ita") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"pol") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Poland,      LEVEL_1 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"ukr") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Ukraine,     LEVEL_1 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"cze") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Czech_Republic, LEVEL_1 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"por") {
            SetPreferredLeagues({
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else if (GameLanguage() == L"chi") {
            SetPreferredLeagues({
                { (UChar)FifamNation::China_PR,    LEVEL_1 },
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::Japan,       LEVEL_1 },
                { (UChar)FifamNation::Australia,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
        else {
            SetPreferredLeagues({
                { (UChar)FifamNation::England,     LEVEL_2 },
                { (UChar)FifamNation::Spain,       LEVEL_2 },
                { (UChar)FifamNation::Italy,       LEVEL_2 },
                { (UChar)FifamNation::Germany,     LEVEL_2 },
                { (UChar)FifamNation::France,      LEVEL_2 },
                { (UChar)FifamNation::Portugal,    LEVEL_1 },
                { (UChar)FifamNation::Sweden,      LEVEL_1 },
                { (UChar)FifamNation::Brazil,      LEVEL_1 },
                { (UChar)FifamNation::Argentina,   LEVEL_1 },
                { (UChar)FifamNation::United_States, LEVEL_1 }
            });
        }
    }
}

Int LeagueSelectionRet0() {
    return 0;
}

void PatchLeagueSelection(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // enable 'IGNORE_LEAGUE_LIMITATION'
        patch::Nop(0x5424D3, 2);
        patch::SetUInt(0x3067B20, 3);

        patch::RedirectCall(0x541D58, LeagueSelectionRet0);
        patch::RedirectCall(0x542C1C, LeagueSelectionRet0);

        // league limitations
        patch::SetUChar(0x542369 + 2, 32);
        patch::SetUChar(0x54237A + 2, 64);
        patch::SetUChar(0x542391 + 2, 127);
        patch::SetUChar(0x5422FD + 2, 16);
        patch::SetUChar(0x54231C + 2, 32);
        patch::SetUChar(0x542336 + 2, 48);
        patch::SetUInt(0x3067B00, 1536 * 2);
    }
}
