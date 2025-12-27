#pragma once
#include "plugin.h"
#include "FifamTypes.h"

void PatchExtendedPlayer(FM::Version v);
void SetPlayerJerseyName(void *player, wchar_t const *name);
wchar_t const *GetPlayerJerseyName(void *player);
void SetPlayerFifaID(void *player, UInt id);
UInt GetPlayerFifaID(void *player);
