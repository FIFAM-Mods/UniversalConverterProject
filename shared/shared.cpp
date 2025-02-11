#include "WinHeader.h"
#include <ShlObj.h>
#include "shared.h"
#include "UcpSettings.h"

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

Int GetPatchUpdateNumber() {
    return 0;
}

WideChar const *GetPatchTestVersion() {
    return L"February 11 Test";
}

String GetPatchVersion() {
    if (GetPatchTestVersion())        // TODO:
        return GetPatchTestVersion(); // remove this
    return Utils::Format(L"1.%u", GetPatchUpdateNumber());
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

Path &SafeLogPath() {
    static Path safeLogPath = "ucp_safe.log";
    return safeLogPath;
}

void SafeLog::Clear() {
    if (Settings::GetInstance().EnableMainLog) {
        std::error_code ec;
        if (exists(SafeLogPath(), ec))
            remove(SafeLogPath(), ec);
    }
}

void SafeLog::Write(String const& msg) {
    if (Settings::GetInstance().EnableMainLog) {
        FILE* file = _wfopen(SafeLogPath().c_str(), L"at,ccs=UTF-8");
        if (file) {
            fputws(msg.c_str(), file);
            fputws(L"\n", file);
            fclose(file);
        }
    }
}

void SafeLog::WriteToFile(Path const& fileName, String const& msg, String const& header) {
    if (Settings::GetInstance().EnableAllLogFiles) {
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
