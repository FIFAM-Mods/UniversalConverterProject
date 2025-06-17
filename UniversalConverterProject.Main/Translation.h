#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"

String &GameLanguage();
extern Bool IsRussianLanguage;
extern Bool IsUkrainianLanguage;
extern eCurrency DefaultCurrency;
extern Bool IsDefaultImperialUnits;

void ProcessStringUkrainianLanguage(String &str);
void ProcessStringUkrainianLanguageNoCase(String &str);
void PatchTranslation(FM::Version v);
