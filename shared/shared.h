#pragma once
#include "FifamTypes.h"
#include "Utils.h"
#include "WinHeader.h"

String& GameLanguage();
String GetAppName();
String GetPatchName();
String GetPatchVersion();
WideChar const *GetPatchTestVersion();
String GetFullAppName(Bool upperCase = false);
String GetPatchNameWithVersion(Bool upperCase = false);
Int GetPatchUpdateNumber();
Int GetPatchHotfixNumber();
Int GetPatchVersionNumber();
String GetFMDocumentsFolderName();
Bool &IsWomensDatabase();
Bool &IsFirstLaunch();
path GetDocumentsPath();
void SaveTestFile();
String GetIniOption(String const &group, String const &key, String const &defaultValue, Path const &filePath);

const UInt NUM_LANGUAGES = 105 + 1;

class CriticalSection {
    CRITICAL_SECTION cs;
public:
    CriticalSection() { InitializeCriticalSection(&cs); }
    ~CriticalSection() { DeleteCriticalSection(&cs); }

    void Enter() { EnterCriticalSection(&cs); }
    void Leave() { LeaveCriticalSection(&cs); }
};

class CriticalSectionLock {
    CriticalSection &c;
public:
    CriticalSectionLock(CriticalSection &c_) : c(c_) { c.Enter(); }
    ~CriticalSectionLock() { c.Leave(); }
};

class SafeLog {
public:
    struct LogFile {
        Bool created = false;
        CriticalSection cs;
    };
private:
    static CriticalSection &cs();
    static Map<Path, LogFile> &logFiles();
public:
    static void Clear();
    static void Write(String const& msg);
    static void WriteToFile(Path const& fileName, String const& msg, String const& header = String());
};
