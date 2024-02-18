#pragma once
#include "plugin-std.h"

extern bool IsWomensDatabase;

wchar_t const *GetText(char const *key);

void PatchEditor(FM::Version v);
