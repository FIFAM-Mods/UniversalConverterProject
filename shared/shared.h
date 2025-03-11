#pragma once
#include "FifamTypes.h"
#include "Utils.h"

String& GameLanguage();
String GetAppName();
String GetPatchName();
String GetPatchVersion();
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

class SafeLog {
public:
    static void Clear();
    static void Write(String const& msg);
    static void WriteToFile(Path const& fileName, String const& msg, String const& header = String());
};
