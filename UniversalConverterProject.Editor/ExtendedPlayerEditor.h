#pragma once
#include "plugin.h"
#include "FifamTypes.h"

struct PlayerExtension {
    WideChar *jerseyName;
    UInt creator;
    UInt fifaId;
    Int footballManagerId;
    //Bool isFemale;
};

struct RefereeExtension {
    UInt creator;
    Int footballManagerId;
};

struct ClubExtension {
    
};

PlayerExtension *GetPlayerExtension(void *player);
void PatchExtendedPlayer(FM::Version v);
