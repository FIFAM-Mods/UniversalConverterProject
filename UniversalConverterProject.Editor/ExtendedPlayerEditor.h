#pragma once
#include "plugin-std.h"

struct PlayerExtension {
    wchar_t *jerseyName;
    int footballManagerId;
};

struct ClubExtension {
    
};

PlayerExtension *GetPlayerExtension(void *player);
void PatchExtendedPlayer(FM::Version v);
