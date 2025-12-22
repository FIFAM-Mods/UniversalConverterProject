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
    return L"2026";
}

Int GetPatchUpdateNumber() {
    return 0;
}

Int GetPatchHotfixNumber() {
    return 0;
}

Int GetPatchVersionNumber() {
    return GetPatchUpdateNumber() * 10 + GetPatchHotfixNumber();
}

WideChar const *GetPatchTestVersion() {
    return L"Test 21122025";
}

String GetPatchVersion() {
    if (GetPatchTestVersion())
        return GetPatchTestVersion();
    return Utils::Format(L"1.%u.%u", GetPatchUpdateNumber(), GetPatchHotfixNumber());
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

String GetIniOption(String const &group, String const &key, String const &defaultValue, Path const &filePath) {
    WideChar buf[MAX_PATH];
    GetPrivateProfileStringW(group.c_str(), key.c_str(), defaultValue.c_str(), buf, MAX_PATH, filePath.c_str());
    return Utils::ToLower(buf);
}

Bool &IsFirstLaunch() {
    static Bool isFirstLaunch = true;
    return isFirstLaunch;
}

Path &SafeLogPath() {
    static Path safeLogPath = "ucp_safe.log";
    return safeLogPath;
}

CriticalSection &SafeLog::cs() {
    static CriticalSection criticalSection;
    return criticalSection;
}

Map<Path, SafeLog::LogFile> &SafeLog::logFiles() {
    static Map<Path, SafeLog::LogFile> lf;
    return lf;
}

void SafeLog::Clear() {
    if (Settings::GetInstance().EnableMainLog) {
        CriticalSectionLock lock(cs());
        std::error_code ec;
        if (exists(SafeLogPath(), ec))
            remove(SafeLogPath(), ec);
    }
}

void SafeLog::Write(const String &msg) {
    if (Settings::GetInstance().EnableMainLog) {
        CriticalSectionLock lock(cs());
        FILE *file = _wfopen(SafeLogPath().c_str(), L"at,ccs=UTF-8");
        if (file) {
            fputws(msg.c_str(), file);
            fputws(L"\n", file);
            fclose(file);
        }
    }
}

void SafeLog::WriteToFile(Path const& fileName, String const& msg, String const& header) {
    if (Settings::GetInstance().EnableAllLogFiles) {
        auto &lf = logFiles()[fileName];
        CriticalSectionLock lock(lf.cs);
        FILE* file = nullptr;
        if (!lf.created) {
            file = _wfopen(fileName.c_str(), L"w,ccs=UTF-8");
            if (file) {
                if (!header.empty()) {
                    fputws(header.c_str(), file);
                    fputws(L"\n", file);
                }
                lf.created = true;
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
