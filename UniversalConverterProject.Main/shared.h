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
Bool &IsWomensDatabase();
Bool &IsFirstLaunch();
path GetDocumentsPath();
void SaveTestFile();

const Bool ENABLE_LOG = true;

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

    static void WriteToFile(Path const &fileName, String const &msg) {
        //if (ENABLE_LOG) {
            static Map<Path, bool> fileCreated;
            FILE *file = nullptr;
            if (!fileCreated.contains(fileName)) {
                file = _wfopen(fileName.c_str(), L"w,ccs=UTF-8");
                fileCreated[fileName] = true;
            }
            else
                file = _wfopen(fileName.c_str(), L"at,ccs=UTF-8");
            if (file) {
                fputws(msg.c_str(), file);
                fputws(L"\n", file);
                fclose(file);
            }
        //}
    }
};
