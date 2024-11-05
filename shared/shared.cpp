#include "WinHeader.h"
#include <ShlObj.h>
#include "shared.h"

const Bool ENABLE_LOG = true;
const Bool ENABLE_FILE_LOG = true;

String& GameLanguage() {
    static String gameLanguage;
    return gameLanguage;
}

String GetAppName() {
    if (GameLanguage() == L"ger")
        return L"Fussball Manager";
    if (GameLanguage() == L"fre")
        return L"LFP Manager";
    return L"FIFA Manager";
}

String GetPatchName() {
    return L"2025";
}

String GetPatchVersion() {
    return L"1.0";
}

String GetFullAppName(Bool upperCase) {
    String name = GetAppName() + L' ' + GetPatchName() + L' ' + GetPatchVersion();
    if (upperCase)
        return Utils::ToUpper(name);
    return name;
}

String GetPatchNameWithVersion(Bool upperCase) {
    String name = GetPatchName() + L' ' + GetPatchVersion();
    if (upperCase)
        return Utils::ToUpper(name);
    return name;
}

String GetFMDocumentsFolderName() {
    return L"FM";
}

path GetDocumentsPath() {
    wchar_t documentsDir[MAX_PATH];
    bool foundDocuments = SHGetSpecialFolderPathW(NULL, documentsDir, CSIDL_MYDOCUMENTS, FALSE);
    if (foundDocuments)
        return path(documentsDir) / GetFMDocumentsFolderName();
    return path();
}

void SaveTestFile() {
    path documentsPath = GetDocumentsPath();
    if (!documentsPath.empty()) {
        path testFileName = documentsPath / "Config" / "ucp-launched";
        if (!exists(testFileName)) {
            FILE *testFile = _wfopen(testFileName.c_str(), L"wb");
            if (testFile)
                fclose(testFile);
        }
    }
}

Bool& IsWomensDatabase() {
    static Bool isWomensDatabase = false;
    return isWomensDatabase;
}

Bool &IsFirstLaunch() {
    static Bool isFirstLaunch = true;
    return isFirstLaunch;
}

void SafeLog::Write(String const& msg) {
    if (ENABLE_LOG) {
        FILE* file = fopen("ucp_safe.log", "at,ccs=UTF-8");
        if (file) {
            fputws(msg.c_str(), file);
            fputws(L"\n", file);
            fclose(file);
        }
    }
}

void SafeLog::WriteToFile(Path const& fileName, String const& msg, String const& header) {
    if (ENABLE_FILE_LOG) {
        static Map<Path, bool> fileCreated;
        FILE* file = nullptr;
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
