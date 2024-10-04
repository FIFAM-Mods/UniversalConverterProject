#include "WinHeader.h"
#include <ShlObj.h>
#include "shared.h"

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
    return L"2024";
}

String GetPatchVersion() {
    return L"1.1";
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
