#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"

String &GameLanguage();
extern Bool IsRussianLanguage;
extern Bool IsUkrainianLanguage;

void PatchTranslation(FM::Version v);
