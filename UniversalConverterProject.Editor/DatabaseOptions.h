#pragma once
#include "plugin-std.h"

extern wchar_t DatabaseID[256];
extern wchar_t DatabaseFolderName[256];

void PatchDatabaseOptions(FM::Version v);
