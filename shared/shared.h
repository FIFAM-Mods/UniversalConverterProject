#pragma once
#include "FifamTypes.h"
#include "Utils.h"

String& GameLanguage();
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
const Bool ENABLE_FILE_LOG = true;

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

    static void WriteToFile(Path const &fileName, String const &msg, String const &header = String()) {
        if (ENABLE_FILE_LOG) {
            static Map<Path, bool> fileCreated;
            FILE *file = nullptr;
            if (!Utils::Contains(fileCreated, fileName)) {
                file = _wfopen(fileName.c_str(), L"w,ccs=UTF-8");
                fileCreated[fileName] = true;
                if (!header.empty()) {
                    fputws(header.c_str(), file);
                    fputws(L"\n", file);
                }
            }
            else
                file = _wfopen(fileName.c_str(), L"at,ccs=UTF-8");
            if (file) {
                fputws(msg.c_str(), file);
                fputws(L"\n", file);
                fclose(file);
            }
        }
    }
};
