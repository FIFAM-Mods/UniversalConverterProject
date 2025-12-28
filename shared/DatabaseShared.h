#pragma once
#include "plugin.h"
#include "FifamTypes.h"

struct DatabaseInfo {
    StringA id;
    StringA parentDatabaseId;
    Bool isWomenDatabase = false;
    Bool hasEditorDatabase = false;
    Bool isEditorDatabase = false;
    Int index = -1;

    void Clear();
};

Map<StringA, DatabaseInfo> &Databases();
DatabaseInfo *GetDatabaseInfo(StringA const &id);
Vector<DatabaseInfo> &DatabasesVec();
DatabaseInfo &CurrentDatabase();
void ReadDatabaseIDs(Bool editor);
String GetDatabaseFolder(unsigned int databaseIndex);
void SetKLFilePath_Shared(WideChar *out, WideChar const *in);
Bool IsWomenDatabase();
