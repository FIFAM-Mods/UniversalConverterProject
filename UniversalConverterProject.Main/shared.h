#pragma once
#include "FifamTypes.h"

wchar_t const *GetMainDatabaseName();
wchar_t const *GetWorldCupDatabaseName();
wchar_t const *GetEditorDatabaseName();
char const *GetMainMenuScreenName();
char const *GetDatabaseScreenName();
String GetAppName();
String GetPatchName();
String GetPatchVersion();
String GetFullAppName(Bool upperCase = false);
String GetPatchNameWithVersion(Bool upperCase = false);
String GetFMDocumentsFolderName();

const Bool ENABLE_LOG = false;

class SafeLog {
public:
    static void Write(String const &msg) {
        if (ENABLE_LOG) {
            FILE *file = fopen("ucp_safe.log", "at,ccs=UTF-8");
            if (file) {
                fputws(msg.c_str(), file);
                fputws(L"\n", file);
                fclose(file);
            }
        }
    }
};
