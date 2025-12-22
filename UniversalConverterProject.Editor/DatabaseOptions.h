#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"
#include "DatabaseShared.h"

extern wchar_t DatabaseID[256];
extern wchar_t DatabaseFolderName[256];
void PatchDatabaseOptions(FM::Version v);
