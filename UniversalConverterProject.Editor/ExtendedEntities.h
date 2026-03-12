#pragma once
#include "plugin.h"
#include "FifamTypes.h"

struct PlayerExtension {
    WideChar *jerseyName;
    UInt creator;
    UInt fifaId;
    Int footballManagerId;
    UInt tmDeId;
    Int cityOfBirth;
    Bool isFemale;
};

struct RefereeExtension {
    UInt creator;
    Int footballManagerId;
    UInt tmDeId;
    Bool isFemale;
};

struct ClubExtension {
    Int footballManagerId;
    UInt tmDeId[5];
    UInt tmDeReserveId[5];
    UInt tmDeOtherIds[10];
    Int cityId;
};

static_assert(sizeof(ClubExtension) <= 320, "ClubExtension size can't be greater than 320");

struct LeagueRegion {
    Int regionID;
    UChar priority;
    UChar direction;
};

struct LeagueExtension {
    UChar order;
    UChar numberOfRegions;
    LeagueRegion *regions;
};

PlayerExtension *GetPlayerExtension(void *player);
ClubExtension *GetClubExtension(void *club);
void PatchExtendedPlayer(FM::Version v);
LeagueExtension *GetLeagueExtension(void *league);
