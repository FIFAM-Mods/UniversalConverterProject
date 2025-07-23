#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"
#include "Date.h"

enum ePlayerTrophy {
    PLAYERTROPHY_BALLONDOR,
    PLAYERTROPHY_FIFATHEBEST,
    PLAYERTROPHY_BESTINEUROPE,
    PLAYERTROPHY_GOLDENBOOT,
    PLAYERTROPHY_PLAYEROFTHEMONTH
};

class CDBPlayer;

void AddPlayerTrophy(ePlayerTrophy type, UShort year, String lastName, String firstName, Date birthdate,
    UChar countryId, UInt clubId, Bool searchForRealPlayer = true);
void AddPlayerTrophy(ePlayerTrophy type, UShort year, CDBPlayer *player);
void PatchEuropeanGoldenShoe(FM::Version v);
