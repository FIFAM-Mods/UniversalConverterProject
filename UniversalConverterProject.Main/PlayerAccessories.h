#pragma once
#include "plugin.h"

using namespace plugin;

void PatchPlayerAccessories(FM::Version v);
void InstallPlayerAccessoriesGfxPatches();
void SetupSleevesForPlayerModel(void *plmodel);
void SetPlayerAccessoryShirtStyle(void *p, int id, int color);
