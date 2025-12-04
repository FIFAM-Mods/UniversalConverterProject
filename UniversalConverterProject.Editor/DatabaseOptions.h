#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"

struct DatabaseInfo {
    StringA id;
    String title;
    String description;
    StringA parentDatabaseId;
    Bool isWomenDatabase = false;
    Int index = -1;
};

extern wchar_t DatabaseID[256];
extern wchar_t DatabaseFolderName[256];
Map<StringA, DatabaseInfo> &Databases();
void PatchDatabaseOptions(FM::Version v);
