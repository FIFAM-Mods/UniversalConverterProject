#pragma once
#include "plugin.h"
#include "FifamTypes.h"

String &GameLanguage();

void LoadCustomCountryNames(Path const &dbFolder);
void PatchTranslation(FM::Version v);
