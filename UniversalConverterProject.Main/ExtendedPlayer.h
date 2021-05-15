#pragma once
#include "plugin-std.h"

void PatchExtendedPlayer(FM::Version v);
void SetPlayerJerseyName(void *player, wchar_t const *name);
wchar_t const *GetPlayerJerseyName(void *player);
