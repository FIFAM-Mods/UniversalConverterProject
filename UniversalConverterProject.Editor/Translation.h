#pragma once
#include "plugin.h"
#include "FifamTypes.h"

extern Int CurrentLanguageId;
String &GameLanguage();

void LoadCustomCountryNames(Path const &dbFolder);
void PatchTranslation(FM::Version v);
