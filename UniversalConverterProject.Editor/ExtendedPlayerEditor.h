#pragma once
#include "plugin-std.h"

struct PlayerExtension {
    wchar_t *jerseyName;
    unsigned int creator;
    unsigned int fifaId;
    int footballManagerId;
};

struct RefereeExtension {
    unsigned int creator;
    int footballManagerId;
};

struct ClubExtension {
    
};

PlayerExtension *GetPlayerExtension(void *player);
void PatchExtendedPlayer(FM::Version v);
