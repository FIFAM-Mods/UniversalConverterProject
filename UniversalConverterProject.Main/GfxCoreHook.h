#pragma once
#include "plugin.h"

using namespace plugin;

unsigned int GfxCoreAddress(unsigned int addr);
void PatchGfxCoreHook(FM::Version v);
