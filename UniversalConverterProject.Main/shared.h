#pragma once
#include "FifamTypes.h"

wchar_t const *GetMainDatabaseName();
wchar_t const *GetWorldCupDatabaseName();
wchar_t const *GetEditorDatabaseName();
char const *GetMainMenuScreenName();
char const *GetDatabaseScreenName();

class SafeLog {
public:
    static void Write(String const &msg) {
        FILE *file = fopen("ucp_safe.log", "at");
        if (file) {
            fputws(msg.c_str(), file);
            fputws(L"\n", file);
            fclose(file);
        }
    }
};
