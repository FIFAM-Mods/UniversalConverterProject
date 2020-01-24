#include "Editor.h"
#include "FifamTypes.h"
#include "Utils.h"
#include "license_check/license_check.h"
#include "compression.h"
#include "Random.h"
#include "Compiler.h"
#include "FifamReadWrite.h"
#include "Competitions.h"

using namespace plugin;

//#define EDITOR_READ_TEXT
//#define EDITOR_WRITE_TEXT
//#define UPDATE_CLUB_BUDGETS

enum FmLanguage {
    English,
    German,
    French,
    Spanish,
    Italian,
    Polish,
    Russian
};

FmLanguage gCurrentLanguage;

int METHOD RetMinOne(void *, DUMMY_ARG, void *) {
    return -1;
}

int METHOD RetZero(void *, DUMMY_ARG, void *) {
    return 0;
}

int METHOD RetTrue(void *) {
    return 1;
}

Int gWritingStatus = -1;
Int gReadingStatus = -1;
void *gCountry = nullptr;
WideChar gCountryDatabasePath[MAX_PATH];
Bool gCountryRelativePath = false;
Vector<CompDesc> gCompiledComps;

template<Bool IsScript>
Bool METHOD OnWriteCountryData(void *country, DUMMY_ARG, void *file) {
    String countriesDirPath = Magic<'d','a','t','a','b','a','s','e','\\','c','o','u','n','t','r','i','e','s'>(2719544074);
    CreateDirectoryW(countriesDirPath.c_str(), 0);
    Bool result = CallMethodAndReturn<Bool, IsScript ? 0x4DD0F0 : 0x4E4F30>(country, file);
    WideChar *filePath = raw_ptr<WideChar>(file, 0x10);
    UInt countryId = *raw_ptr<UInt>(country, 0x1A4);
    String countryName = raw_ptr<WideChar>(country, 0x4 + 30 * 2);
    if (countryName.empty())
        countryName = Utils::Format(Magic<'_','U','N','N','A','M','E','D','_','%','d'>(3563223291), countryId);
    String newFilePath = Utils::Format(Magic<'d','a','t','a','b','a','s','e','\\','c','o','u','n','t','r','i','e','s','\\','%','s','.','u','c','p','d','b'>(726523196), countryName.c_str());
    wcscpy_s(filePath, 260, newFilePath.c_str());
    countryName.clear();
    newFilePath.clear();
    countriesDirPath.clear();
    gWritingStatus = IsScript ? 1 : 2;
    CallMethod<0x512F00>(file);
    gWritingStatus = -1;
    return result;
}

template<Bool IsScript>
Bool METHOD OnReadCountryData(void *file, DUMMY_ARG, WideChar const *filePath, Int encoding) {
    //Message(String(L"Loading ") + filePath);
    UInt countryId = *raw_ptr<UInt>(gCountry, 0x1A4);
    String countryName = raw_ptr<WideChar>(gCountry, 0x4 + 30 * 2);
    if (countryName.empty())
        countryName = Utils::Format(Magic<'_','U','N','N','A','M','E','D','_','%','d'>(3563223291), countryId);
    Bool readCustomScript = false;
    String newFilePath;
    if (IsScript) {
        String customScriptPath = Utils::Format(Magic<'%','s','\\','c','u','s','t','o','m','_','s','c','r','i','p','t','s','\\','%','s','.','u','c','p','s','c'>(1266327827), gCountryDatabasePath, countryName.c_str());
        std::error_code ec;
        if (exists(customScriptPath, ec)) {
            FifamReader scriptReader(customScriptPath, 13, false, false);
            if (scriptReader.Available()) {
                String fileContent;
                while (!scriptReader.IsEof()) {
                    fileContent += scriptReader.ReadFullLine();
                    fileContent.append(Magic<0xA>(3068980695));
                }
                String compiledScript, compiledFixture, compiledData;
                CompilerOptions options;
                options.checkBonuses = false;
                options.checkMatchdays = false;
                options.checkNames = false;
                options.checkTeams = false;
                Vector<String> definitions;
                gCompiledComps.clear();
                if (ScriptEngine::Compile(fileContent, compiledScript, compiledFixture, compiledData, gCompiledComps, 13, false, true, String(), options, definitions)) {
                    WideChar lpTempPathBuffer[MAX_PATH];
                    UInt dwRetVal = GetTempPathW(MAX_PATH, lpTempPathBuffer);
                    Path tempPath;
                    if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
                        tempPath = Utils::Format(Magic<'%','s','\\','t','e','m','p'>(888714063), gCountryDatabasePath);
                        create_directories(tempPath, ec);
                    }
                    else
                        tempPath = lpTempPathBuffer;
                    tempPath /= Magic<'C','o','u','n','t','r','y','S','c','r','i','p','t','X','.','s','a','v'>(292907193);
                    FifamWriter scriptWriter(tempPath, 13, GetFifamVersion(13), true);
                    if (scriptWriter.Available()) {
                        scriptWriter.Write(compiledScript);
                        readCustomScript = true;
                        newFilePath = tempPath.c_str();
                    }
                    else {
                        Error(Utils::Format(Magic<'U','n','a','b','l','e',' ','t','o',' ','c','r','e','a','t','e',' ','a',' ','t','e','m','p','o','r','a','r','y',' ','f','i','l','e',' ','f','o','r',' ','c','u','s','t','o','m',' ','s','c','r','i','p','t',' ','(','%','s',')'>(3791467810), customScriptPath.c_str()));
                    }
                }
                else {
                    Error(Utils::Format(Magic<'U','n','a','b','l','e',' ','t','o',' ','c','o','m','p','i','l','e',' ','c','u','s','t','o','m',' ','s','c','r','i','p','t',' ','(','%','s',')',':',0xA,'%','s'>(1493177856), customScriptPath.c_str(), compiledScript.c_str()));
                }
            }
            else {
                Error(Utils::Format(Magic<'U','n','a','b','l','e',' ','t','o',' ','o','p','e','n',' ','c','u','s','t','o','m',' ','s','c','r','i','p','t',' ','(','%','s',')'>(3807250146), customScriptPath.c_str()));
            }
        }
    }
    if (!readCustomScript) {
        if (gCountryRelativePath)
            newFilePath = Utils::Format(Magic<'%', 's', '\\', 'c', 'o', 'u', 'n', 't', 'r', 'i', 'e', 's', '\\', '%', 's', '.', 'u', 'c', 'p', 'd', 'b'>(192558862), gCountryDatabasePath, countryName.c_str());
        else
            newFilePath = Utils::Format(Magic<'%', 's', '\\', '%', 's', '.', 'u', 'c', 'p', 'd', 'b'>(4127187384), gCountryDatabasePath, countryName.c_str());
        gReadingStatus = IsScript ? 1 : 2;
    }
    else
        gReadingStatus = -1;
    countryName.clear();
    Bool result = CallMethodAndReturn<Bool, 0x5140D0>(file, newFilePath.c_str(), encoding);
    gReadingStatus = -1;
    newFilePath.clear();
    if (readCustomScript) {
        //Delete custom script file
        std::error_code ec;
        remove(newFilePath, ec);
    }
    return result;
}

Bool __stdcall OnWriteFile(LPCWSTR lpFileName, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
    Bool result = false;
    UChar *compressedData = nullptr;
    UInt compressedSize = 0;
    if (gWritingStatus == 1 || gWritingStatus == 2) {
        compressedSize = compress((void *)lpBuffer, nNumberOfBytesToWrite, NULL, false);
        compressedData = new UChar[compressedSize];
        compress((void *)lpBuffer, nNumberOfBytesToWrite, compressedData, true);
        UChar key[8] = { 'N', 4, 'X', 1, 7, 'E', 1, 'Y' };
        for (UInt i = 0; i < compressedSize; i++)
            compressedData[i] = compressedData[i] ^ key[i % (sizeof(key) / sizeof(char))];
        if (compressedSize > 1) {
            compressedData[0] = 'C';
            compressedData[1] = 'P';
        }
    }
    DWORD fileCreationFlag = CREATE_ALWAYS;
    DWORD fileAccess = GENERIC_WRITE;
    if (gWritingStatus == 2) {
        fileAccess = FILE_APPEND_DATA;
        fileCreationFlag = OPEN_ALWAYS;
    }
    HANDLE h = CreateFileW(lpFileName, fileAccess, FILE_SHARE_WRITE, NULL, fileCreationFlag, FILE_ATTRIBUTE_NORMAL, 0);
    if (h != (HANDLE)-1 && h != 0) {
        DWORD numBytesWritten = 0;
        lpFileName = 0;
        if (gWritingStatus == 1 || gWritingStatus == 2) {
            UChar compressedSizeEnc[4];
            memcpy(compressedSizeEnc, &compressedSize, 4);
            UChar key[4] = { 'A', 6, 3, 'U' };
            for (UInt i = 0; i < 4; i++)
                compressedSizeEnc[i] = compressedSizeEnc[i] ^ key[i % (sizeof(key) / sizeof(char))];
            if (WriteFile(h, compressedSizeEnc, 4, &numBytesWritten, NULL) && numBytesWritten == 4) {
                if (WriteFile(h, compressedData, compressedSize, &numBytesWritten, NULL) && numBytesWritten == compressedSize)
                    result = true;
            }
        }
        else {
            if (WriteFile(h, lpBuffer, nNumberOfBytesToWrite, &numBytesWritten, NULL) && numBytesWritten == nNumberOfBytesToWrite)
                result = true;
        }
        CloseHandle(h);
    }
    if (gWritingStatus != -1)
        gWritingStatus = -1;
    if (compressedData)
        delete[] compressedData;
    return result;
}

Bool __stdcall OnReadFile(LPCWSTR lpFileName, void **ppData, DWORD *pDataSize) {
    HANDLE h = CreateFileW(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == (HANDLE)-1 || h == 0)
        return false;
    Bool result = false;
    DWORD fileSize = GetFileSize(h, 0);
    *pDataSize = fileSize;
    if (fileSize >= 0) {
        *ppData = CallAndReturn<void *, 0x5B04AE>(fileSize);
        if (*ppData) {
            DWORD numReadBytes = 0;
            result = ReadFile(h, *ppData, fileSize, &numReadBytes, NULL) && numReadBytes == fileSize;
        }
    }
    CloseHandle(h);
    if (gReadingStatus == 1 || gReadingStatus == 2) {
        result = false;
        if (*ppData) {
            UChar *data = (UChar *)(*ppData);
            UInt compressedSize = 0;
            UChar *compressedData = nullptr;
            if (fileSize >= 4) {
                compressedSize = *raw_ptr<UInt>(data, 0);
                UChar compressedSizeDec[4];
                memcpy(compressedSizeDec , &compressedSize, 4);
                UChar key[4] = { 'A', 6, 3, 'U' };
                for (UInt i = 0; i < 4; i++)
                    compressedSizeDec[i] = compressedSizeDec[i] ^ key[i % (sizeof(key) / sizeof(char))];
                memcpy(&compressedSize, compressedSizeDec, 4);
                if (gReadingStatus == 1) {
                    compressedData = raw_ptr<UChar>(data, 4);
                }
                else if (gReadingStatus == 2 && fileSize >= (8 + compressedSize)) {
                    compressedData = raw_ptr<UChar>(data, 8 + compressedSize);
                    compressedSize = *raw_ptr<UInt>(data, 4 + compressedSize);
                    memcpy(compressedSizeDec, &compressedSize, 4);
                    UChar key[4] = { 'A', 6, 3, 'U' };
                    for (UInt i = 0; i < 4; i++)
                        compressedSizeDec[i] = compressedSizeDec[i] ^ key[i % (sizeof(key) / sizeof(char))];
                    memcpy(&compressedSize, compressedSizeDec, 4);
                }
            }
            //Message(L"CompressedSize: %d, CompressedData: %p", compressedSize, compressedData);
            if (compressedData) {
                if (compressedSize > 1) {
                    compressedData[0] = 0xDE;
                    compressedData[1] = 0xFF;
                }
                UChar key[8] = { 'N', 4, 'X', 1, 7, 'E', 1, 'Y' };
                for (UInt i = 0; i < compressedSize; i++)
                    compressedData[i] = compressedData[i] ^ key[i % (sizeof(key) / sizeof(char))];
                UInt decompressedSize = get_decompressed_size(compressedData);
                void *newData = CallAndReturn<void *, 0x5B04AE>(decompressedSize);
                if (newData) {
                    decompress(compressedData, newData);
                    Call<0x5B04A3>(*ppData);
                    *ppData = newData;
                    *pDataSize = decompressedSize;
                    result = true;
                }
            }
        }
    }
    if (!result) {
        if (*ppData) {
            Call<0x5B04A3>(*ppData);
            *ppData = 0;
        }
        *pDataSize = 0;
    }
    if (gReadingStatus != -1) {
        gReadingStatus = -1;
        //Message(L"Result: %d", result);
    }
    return result;
}

Bool METHOD OnLoadCountry(void *country, DUMMY_ARG, WideChar const *databaseFolderPath, void *file, UChar bRelativePath) {
    gCountry = country;
    gCompiledComps.clear();
    wcscpy_s(gCountryDatabasePath, MAX_PATH, databaseFolderPath);
    gCountryRelativePath = bRelativePath;
    Bool result = CallMethodAndReturn<Bool, 0x4EA300>(country, databaseFolderPath, file, bRelativePath);
    if (!gCompiledComps.empty()) {
        void *world = *raw_ptr<void *>(country, 0x0);
        if (world) {
            for (auto const &c : gCompiledComps) {
                void *comp = CallMethodAndReturn<void *, 0x4D3010>(world, c.id.ToInt());
                if (comp) {
                    for (UInt i = 0; i < 6; i++)
                        CallMethod<0x4FC690>(comp, i, c.name.c_str());
                }
            }
        }
        gCompiledComps.clear();
    }
    return result;
}

void METHOD OnCloseClubFile(void *file) {
    gWritingStatus = 1;
    CallMethod<0x512F00>(file);
    gWritingStatus = -1;
}

Bool METHOD OnOpenClubFile(void *file, DUMMY_ARG, WideChar const *filePath, Int encoding) {
    gReadingStatus = 1;
    Bool result = CallMethodAndReturn<Bool, 0x5140D0>(file, filePath, encoding);
    gReadingStatus = -1;
    return result;
}

Bool METHOD OnOpenFreeAgentsFile(void *file, DUMMY_ARG, WideChar const *filePath, Int encoding) {
    gReadingStatus = 1;
    Bool result = CallMethodAndReturn<Bool, 0x5140D0>(file, filePath, encoding);
    gReadingStatus = -1;
    return result;
}

void METHOD OnCloseFreeAgentsFile(void *file) {
    gWritingStatus = 1;
    CallMethod<0x512F00>(file);
    gWritingStatus = -1;
}

void *gLeagueCountry = nullptr;

UChar METHOD OnStoreLeagueCountry(void *country) {
    gLeagueCountry = country;
    return CallMethodAndReturn<UChar, 0x4DCE70>(country);
}

void OnGetLeagueFileName(WideChar *dest, WideChar const *format, UInt countryId) {
    if (gLeagueCountry) {
        UInt countryId = *raw_ptr<UInt>(gLeagueCountry, 0x1A4);
        String countryName = raw_ptr<WideChar>(gLeagueCountry, 0x4 + 30 * 2);
        if (countryName.empty())
            countryName = Utils::Format(Magic<'_','U','N','N','A','M','E','D','_','%','d'>(3563223291), countryId);
        String ext = Magic<'.','u','c','p','d','b'>(1997942472);
        wcscpy_s(dest, 64, (countryName + ext).c_str());
        countryName.clear();
        ext.clear();
        gLeagueCountry = nullptr;
        return;
    }
    swprintf(dest, format, countryId);
}

void __declspec(naked) MySetEnableMenu() {
    __asm {
        add esp, 4
        push 0
        push 0x80E4
        mov eax, 0x4641EA
        jmp eax
    }
}

void OnGetFreeAgentsFilePath(WideChar *dest, WideChar const *format, WideChar const *dbPath) {
    String freeAgentsFilePathFormat = Magic<'%','s','/','W','i','t','h','o','u','t','.','u','c','p','d','b'>(1068035151);
    swprintf(dest, freeAgentsFilePathFormat.c_str(), dbPath);
}

UInt __stdcall WasEULAShown(Int) {
    static Int counter = 0;
    counter++;
    return counter == 1;
}

void OnGetEditorEULAPath(WideChar *dest, WideChar const *format, WideChar const *loc) {
    if (gCurrentLanguage == FmLanguage::German)
        wcscpy(dest, Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','E','U','L','A','\\','d','e','.','r','t','f'>(2988468587).c_str());
    else if (gCurrentLanguage == FmLanguage::French)
        wcscpy(dest, Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','E','U','L','A','\\','e','n','.','r','t','f'>(2598556572).c_str());
    else if (gCurrentLanguage == FmLanguage::Italian)
        wcscpy(dest, Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','E','U','L','A','\\','e','n','.','r','t','f'>(2598556572).c_str());
    else if (gCurrentLanguage == FmLanguage::Spanish)
        wcscpy(dest, Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','E','U','L','A','\\','e','s','.','r','t','f'>(2550935338).c_str());
    else if (gCurrentLanguage == FmLanguage::Polish)
        wcscpy(dest, Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','E','U','L','A','\\','p','l','.','r','t','f'>(4019035623).c_str());
    else if (gCurrentLanguage == FmLanguage::Russian)
        wcscpy(dest, Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','E','U','L','A','\\','r','u','.','r','t','f'>(3539348041).c_str());
    else
        wcscpy(dest, Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','E','U','L','A','\\','e','n','.','r','t','f'>(2598556572).c_str());
}

void *gDlgMainMenu = nullptr;
Bool gReadingCustomScript = false;

Bool METHOD CanImportScript(void *) {
    if (gDlgMainMenu != nullptr) {
        UInt countryId = *raw_ptr<UInt>(gDlgMainMenu, 0x9688);
        return countryId >= 1 && countryId <= 207;
    }
    return false;
}

void *METHOD OnCreateMainMenu(void *dlg, DUMMY_ARG, Int unk) {
    gDlgMainMenu = dlg;
    return CallMethodAndReturn<void *, 0x460F10>(dlg, unk);
}

void METHOD OnDeleteMainMenu(void *dlg) {
    CallMethod<0x461310>(dlg);
    gDlgMainMenu = nullptr;
}

void METHOD OnCreateCountryRoot(void *country, DUMMY_ARG, Int arg) {
    if (!gReadingCustomScript)
        CallMethod<0x4DD9B0>(country, arg);
}

template <Bool Replace>
Int METHOD ImportScript(void *dlg) {
    Int result = CallMethodAndReturn<Int, 0x5C59A1>(dlg);
    if (result == 1) {
        Int buf[4] = { 0, 0, 0, 0 };
        wchar_t *scriptFileName = *CallMethodAndReturn<wchar_t **, 0x5C5AD0>(dlg, buf);
        if (scriptFileName) {
            void *world = CallAndReturn<void *, 0x4D3EC0>();
            UInt countryId = *raw_ptr<UInt>(gDlgMainMenu, 0x9688);
            if (countryId >= 1 && countryId <= 207) {
                void *country = CallMethodAndReturn<void *, 0x4C9DC0>(world, countryId);
                if (country) {
                    FifamReader scriptReader(scriptFileName, 13, false, false);
                    if (scriptReader.Available()) {
                        String fileContent;
                        while (!scriptReader.IsEof()) {
                            fileContent += scriptReader.ReadFullLine();
                            fileContent.append(Magic<0xA>(3068980695));
                        }
                        String compiledScript, compiledFixture, compiledData;
                        CompilerOptions options;
                        options.checkBonuses = false;
                        options.checkMatchdays = false;
                        options.checkNames = false;
                        options.checkTeams = false;
                        Vector<String> definitions;
                        gCompiledComps.clear();
                        if (ScriptEngine::Compile(fileContent, compiledScript, compiledFixture, compiledData, gCompiledComps, 13, false, true, String(), options, definitions)) {
                            WideChar lpTempPathBuffer[MAX_PATH];
                            UInt dwRetVal = GetTempPathW(MAX_PATH, lpTempPathBuffer);
                            if (dwRetVal > 0 && dwRetVal < MAX_PATH) {
                                Path tempPath = lpTempPathBuffer;
                                tempPath /= Magic<'C', 'o', 'u', 'n', 't', 'r', 'y', 'S', 'c', 'r', 'i', 'p', 't', 'X', '.', 's', 'a', 'v'>(292907193);
                                FifamWriter scriptWriter(tempPath, 13, GetFifamVersion(13), true);
                                if (scriptWriter.Available()) {
                                    scriptWriter.Write(compiledScript);
                                    scriptWriter.Close();
                                    UChar fmFile[0x250];
                                    CallMethod<0x512EA0>(fmFile);
                                    if (CallMethodAndReturn<Bool, 0x5140D0>(fmFile, tempPath.c_str(), 0)) {
                                        if (Replace) {
                                            for (UInt i = 0; i < 4; i++)
                                                Call<0x4FE9A0>(CallAndReturn<void *, 0x4FC170>(countryId, COMP_FA_CUP, i));
                                            for (UInt i = 0; i < 4; i++)
                                                Call<0x4FE9A0>(CallAndReturn<void *, 0x4FC170>(countryId, COMP_LE_CUP, i));
                                            for (UInt i = 0; i < 4; i++)
                                                Call<0x4FE9A0>(CallAndReturn<void *, 0x4FC170>(countryId, COMP_SUPERCUP, i));
                                            for (UInt i = 0; i < 32; i++)
                                                Call<0x4FE9A0>(CallAndReturn<void *, 0x4FC170>(countryId, COMP_LEAGUE, i));
                                            for (UInt i = 0; i < 32; i++)
                                                Call<0x4FE9A0>(CallAndReturn<void *, 0x4FC170>(countryId, COMP_RELEGATION, i));
                                            for (UInt i = 0; i < 32; i++)
                                                Call<0x4FE9A0>(CallAndReturn<void *, 0x4FC170>(countryId, COMP_POOL, i));
                                        }
                                        else {
                                            for (auto const &c : gCompiledComps)
                                                Call<0x4FE9A0>(CallAndReturn<void *, 0x4FC170>(countryId, c.id.mType.ToInt(), c.id.mIndex));
                                        }
                                        gReadingCustomScript = true;
                                        CallMethod<0x4DFAC0>(country, fmFile);
                                        gReadingCustomScript = false;
                                        if (!gCompiledComps.empty()) {
                                            struct LevelDesc {
                                                Bool poolPresent = false;
                                                String poolName;
                                                Vector<String> leagueNames;
                                            };
                                            Array<LevelDesc, 16> levels;
                                            UInt lastCompLevel = 0;
                                            for (auto const &c : gCompiledComps) {
                                                if (c.level < std::size(levels)) {
                                                    if (c.id.mType == FifamCompType::Pool) {
                                                        levels[c.level].poolPresent = true;
                                                        if (levels[c.level].poolName.empty())
                                                            levels[c.level].poolName = c.name;
                                                    }
                                                    else if (c.id.mType == FifamCompType::League)
                                                        levels[c.level].leagueNames.push_back(c.name);
                                                    if (c.level > lastCompLevel)
                                                        lastCompLevel = c.level;
                                                }
                                            }
                                            void *world = *raw_ptr<void *>(country, 0x0);
                                            if (world) {
                                                for (auto const &c : gCompiledComps) {
                                                    void *comp = CallMethodAndReturn<void *, 0x4D3010>(world, c.id.ToInt());
                                                    if (comp) {
                                                        for (UInt i = 0; i < 6; i++)
                                                            CallMethod<0x4FC690>(comp, i, c.name.c_str());
                                                    }
                                                }
                                            }
                                            for (UInt l = 0; l <= lastCompLevel; l++) {
                                                String levelName;
                                                WideChar *dstLevelName = raw_ptr<WideChar>(country, 0x234 + l * 29 * 6 * 2);
                                                if (!levels[l].poolName.empty())
                                                    levelName = levels[l].poolName;
                                                if (levels[l].poolPresent || dstLevelName[0] == L'\0') {
                                                    if (levels[l].leagueNames.size() == 1) {
                                                        if (!levels[l].leagueNames[0].empty())
                                                            levelName = levels[l].leagueNames[0];
                                                    }
                                                    else
                                                        levelName = Utils::Format(Magic<'L','e','a','g','u','e',' ','L','e','v','e','l',' ','%','d'>(2791402137), l + 1);
                                                }
                                                if (!levelName.empty()) {
                                                    for (UInt t = 0; t < 6; t++) {
                                                        wcsncpy(dstLevelName, levelName.c_str(), 29);
                                                        dstLevelName += 29;
                                                    }
                                                }
                                            }
                                            gCompiledComps.clear();
                                        }
                                        CallMethod<0x45E6B0>(gDlgMainMenu);
                                    }
                                    else
                                        Error(Magic<'U','a','b','l','e',' ','t','o',' ','o','p','e','n',' ','a',' ','f','i','l','e',' ','w','i','t','h',' ','g','e','n','e','r','a','t','e','d',' ','s','c','r','i','p','t',' ','c','o','d','e'>(67622265));
                                    CallMethod<0x514080>(fmFile);
                                }
                                else
                                    Error(Magic<'U','a','b','l','e',' ','t','o',' ','c','r','e','a','t','e',' ','a',' ','f','i','l','e',' ','f','o','r',' ','g','e','n','e','r','a','t','e','d',' ','s','c','r','i','p','t',' ','c','o','d','e'>(3293126749));
                            }
                            else
                                Error(Utils::Format(Magic<'U', 'n', 'a', 'b', 'l', 'e', ' ', 't', 'o', ' ', 'c', 'r', 'e', 'a', 't', 'e', ' ', 'a', ' ', 't', 'e', 'm', 'p', 'o', 'r', 'a', 'r', 'y', ' ', 'f', 'i', 'l', 'e', ' ', 'f', 'o', 'r', ' ', 'c', 'u', 's', 't', 'o', 'm', ' ', 's', 'c', 'r', 'i', 'p', 't', ' ', '(', '%', 's', ')'>(3791467810), scriptFileName));
                        }
                        else
                            Error(Utils::Format(Magic<'U', 'n', 'a', 'b', 'l', 'e', ' ', 't', 'o', ' ', 'c', 'o', 'm', 'p', 'i', 'l', 'e', ' ', 'c', 'u', 's', 't', 'o', 'm', ' ', 's', 'c', 'r', 'i', 'p', 't', ' ', '(', '%', 's', ')', ':', 0xA, '%', 's'>(1493177856), scriptFileName, compiledScript.c_str()));
                        gCompiledComps.clear();
                    }
                    else
                        Error(Utils::Format(Magic<'U', 'n', 'a', 'b', 'l', 'e', ' ', 't', 'o', ' ', 'o', 'p', 'e', 'n', ' ', 'c', 'u', 's', 't', 'o', 'm', ' ', 's', 'c', 'r', 'i', 'p', 't', ' ', '(', '%', 's', ')'>(3807250146), scriptFileName));
                }
            }
        }
    }
    return 0;
}

wchar_t *OnPlayerCommentFormat(const wchar_t *in, const wchar_t *what, const wchar_t *to, wchar_t *out, unsigned int maxLen) {
    return CallAndReturn<wchar_t *, 0x5781C0>(out, what, to, in, maxLen);
}

#ifdef UPDATE_CLUB_BUDGETS
UInt const minBudget = 10'000;

struct ClubBudget {
    Int cash = 0,
        salaries = 0,
        salariesLeft = 0,
        transfers = 0,
        transfersLeft = 0,
        infrastructure = 0,
        infrastructureLeft = 0,
        misc = 0,
        miscLeft = 0,
        reserve = 0;
};

Map<UInt, ClubBudget> budgets;
Map<UInt, Int> changedClubs;

void METHOD OnReadClubCapital(void *reader, DUMMY_ARG, Int *dst) {
    CallMethod<0x513500>(reader, dst);
    void *club = (void *)(UInt(dst) - 0xD7C);
    UInt uid = *raw_ptr<UInt>(club, 0xC);
    if (*dst == 0) {
        auto it = budgets.find(uid);
        if (it != budgets.end()) {
            auto const &b = (*it).second;
            Int total = 0;
            Int inc = 0;
            if (b.salariesLeft < 0)
                total += b.salariesLeft;
            if (b.transfersLeft < 0)
                total += b.transfersLeft;
            if (b.infrastructureLeft < 0)
                total += b.infrastructureLeft;
            if (b.miscLeft < 0)
                total += b.miscLeft;
            if (b.reserve < 0)
                total += b.reserve;
            if (total < 0)
                inc = minBudget + total * -1;
            else {
                total = b.salariesLeft + b.transfersLeft + b.infrastructureLeft + b.miscLeft + b.reserve;
                if (total < minBudget)
                    inc = minBudget - total;
            }
            if (inc != 0) {
                *dst = b.cash + inc;
                changedClubs[uid] = *dst;
            }
        }
    }
    if (*dst < 500'000) {
        *dst = 500'000 + Random::Get(1, 10) * 10'000;
        changedClubs[uid] = *dst;
    }
    //if (*dst != 0) {
    //    void *club = (void *)(UInt(dst) - 0xD7C);
    //    UInt uid = *raw_ptr<UInt>(club, 0xC);
    //
    //    auto it = budgets.find(uid);
    //    if (it != budgets.end()) {
    //        auto const &b = (*it).second;
    //        Int total = 0;
    //        Int inc = 0;
    //        if (b.salariesLeft < 0)
    //            total += b.salariesLeft;
    //        if (b.transfersLeft < 0)
    //            total += b.transfersLeft;
    //        if (b.infrastructureLeft < 0)
    //            total += b.infrastructureLeft;
    //        if (b.miscLeft < 0)
    //            total += b.miscLeft;
    //        if (b.reserve < 0)
    //            total += b.reserve;
    //        if (total < 0)
    //            inc = minBudget + total * -1;
    //        else {
    //            total = b.salariesLeft + b.transfersLeft + b.infrastructureLeft + b.miscLeft + b.reserve;
    //            if (total < minBudget)
    //                inc = minBudget - total;
    //        }
    //        if (inc != 0)
    //            *dst += inc;
    //    }
    //}
}

void METHOD OnReadClubTransferBudget(void *reader, DUMMY_ARG, Int *dst) {
    CallMethod<0x513500>(reader, dst);
    void *club = (void *)(UInt(dst) - 0xD7C);
    UInt uid = *raw_ptr<UInt>(club, 0xC);

    auto it = changedClubs.find(uid);
    if (it != changedClubs.end()) {
        Int newCash = (*it).second;
        if (newCash > 0)
            *dst = newCash / 2;
    }
}
#endif

template <UInt funcAddr>
void METHOD SetClubNameForCurrentLanguage(void *club, DUMMY_ARG, const WideChar *name) {
    for (UInt i = 0; i < 6; i++)
        CallMethod<funcAddr>(club, i, name);
}

void METHOD SetClubStadiumName(void *club, DUMMY_ARG, UInt lang, const WideChar *name) {
    for (UInt i = 0; i < 6; i++)
        CallMethod<0x4C2580>(club, i, name);
}

template <UInt nameOffset>
void METHOD SetClubFanNameForCurrentLanguage(void *club, DUMMY_ARG, UInt type, const WideChar *name) {
    if (type == 0 || type == 1) {
        for (UInt i = 0; i < 6; i++)
            wcsncpy((WideChar *)(((i + 6 * type) << 6) + UInt(club) + nameOffset), name, 31);
    }
}

void METHOD SetClubPlayerInTextForCurrentLanguage(void *club, DUMMY_ARG, const WideChar *name) {
    for (UInt i = 0; i < 6; i++)
        wcsncpy((WideChar *)((i << 6) + UInt(club) + 1468), name, 31);
}

const wchar_t *GetAppLocalizedName() {
    static auto name = Magic<'F', 'M'>(1823017569);
    return name.c_str();
}

Int METHOD OnDialogDoModal(void *dlg) {
    void *app = CallAndReturn<void *, 0x4BE1A0>();
    if (app) {
        PostMessageW(*raw_ptr<HWND>(app, 0x22D0), 0x118, 0, 0);
        //CallMethod<0x420E10>(wnd, 0x118, 0, 0);
    }
    return CallMethodAndReturn<Int, 0x5B0869>(dlg);
}

void PatchEditor(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        auto fileName = Magic<'.', '\\', 'u', 'c', 'p', '.', 'i', 'n', 'i'>(2278893143);
        auto mainSectionStr = Magic<'M', 'A', 'I', 'N'>(3621565930);
        auto namesForAllLanguagesStr = Magic<'E', 'D', 'I', 'T', 'O', 'R', '_', 'N', 'A', 'M', 'E', 'S', '_', 'F', 'O', 'R', '_', 'A', 'L', 'L', '_', 'L', 'A', 'N', 'G', 'U', 'A', 'G', 'E', 'S'>(2174602746);
        //auto useOriginalDocumentsFolder = Magic<'U','S','E','_','O','R','I','G','I','N','A','L','_','D','O','O','C','U','M','E','N','T','S','_','F','O','L','D','E','R'>(2892465454);
        //auto noEditorModalDialogsFix = Magic<'N','O','_','E','D','I','T','O','R','_','M','O','D','A','L','_','D','I','A','L','O','G','S','_','F','I','X'>(4148762047);
        Int bNamesForAllLanguages = GetPrivateProfileIntW(mainSectionStr.c_str(), namesForAllLanguagesStr.c_str(), 0, fileName.c_str());
        //Int bUseOriginalDocumentsFolder = GetPrivateProfileIntW(mainSectionStr.c_str(), useOriginalDocumentsFolder.c_str(), 0, fileName.c_str());
        //Int bNoEditorModalDialogsFix = GetPrivateProfileIntW(mainSectionStr.c_str(), noEditorModalDialogsFix.c_str(), 0, fileName.c_str());

        //if (bUseOriginalDocumentsFolder == 0)
            patch::SetPointer(0x6685BC, GetAppLocalizedName);

        //if (bNoEditorModalDialogsFix == 0)
            patch::RedirectCall(0x5B0D69, OnDialogDoModal);

        wchar_t textLang[MAX_PATH];
        GetPrivateProfileStringW(Magic<'O', 'P', 'T', 'I', 'O', 'N', 'S'>(1224534890).c_str(), Magic<'T', 'E', 'X', 'T', '_', 'L', 'A', 'N', 'G', 'U', 'A', 'G', 'E'>(3562105574).c_str(), Magic<'e', 'n', 'g'>(3703889367).c_str(), textLang, MAX_PATH, Magic<'.', '\\', 'l', 'o', 'c', 'a', 'l', 'e', '.', 'i', 'n', 'i'>(2393442148).c_str());
        String strLang = Utils::ToLower(textLang);
        if (strLang == Magic<'e', 'n', 'g'>(3703889367))
            gCurrentLanguage = FmLanguage::English;
        if (strLang == Magic<'g', 'e', 'r'>(3386226877))
            gCurrentLanguage = FmLanguage::German;
        if (strLang == Magic<'f', 'r', 'e'>(2691495628))
            gCurrentLanguage = FmLanguage::French;
        if (strLang == Magic<'i', 't', 'a'>(1732132876))
            gCurrentLanguage = FmLanguage::Italian;
        if (strLang == Magic<'s', 'p', 'a'>(1310785015))
            gCurrentLanguage = FmLanguage::Spanish;
        if (strLang == Magic<'p', 'o', 'l'>(2212475149))
            gCurrentLanguage = FmLanguage::Polish;
        if (strLang == Magic<'r', 'u', 's'>(3352863500))
            gCurrentLanguage = FmLanguage::Russian;

        patch::RedirectCall(0x4C13D5, RetMinOne);
        patch::RedirectCall(0x4C13EC, RetMinOne);
        patch::RedirectCall(0x4C142F, RetMinOne);
        patch::RedirectCall(0x4C14A5, RetMinOne);
        patch::RedirectCall(0x4C14C1, RetMinOne);
        patch::RedirectCall(0x4C14D8, RetMinOne);
        patch::RedirectCall(0x4C14EF, RetMinOne);
        patch::RedirectCall(0x4C176E, RetMinOne);
        patch::RedirectCall(0x4C1643, RetZero);
        patch::RedirectCall(0x4C172B, RetZero);
        patch::Nop(0x4C1751, 5);
        patch::SetUChar(0x555A10, 0xC2);
        patch::SetUChar(0x555A10 + 1, 0x04);
        patch::SetUChar(0x555A10 + 2, 0x00);
        patch::SetUChar(0x463F2A, 0xEB);
        
        //patch::Nop(0x460429, 1);
        //patch::SetUChar(0x460429 + 1, 0xE9);

        patch::RedirectCall(0x460422, CanImportScript);
        patch::RedirectCall(0x4604A8, ImportScript<true>);

        //patch::Nop(0x4605A9, 1);
        //patch::SetUChar(0x4605A9 + 1, 0xE9);

        patch::RedirectCall(0x4605A2, CanImportScript);
        patch::RedirectCall(0x460628, ImportScript<false>);
        patch::SetUChar(0x4605EC + 1, 1);

        patch::RedirectCall(0x4C17C3, OnCreateMainMenu);
        patch::RedirectCall(0x4C17F0, OnDeleteMainMenu);

        patch::RedirectCall(0x4DFB00, OnCreateCountryRoot);

        patch::SetUChar(0x4FB120, 0xC3);
        patch::Nop(0x4FAD4B, 2);
        patch::Nop(0x4FAD54, 5);

        patch::SetUChar(0x45EE21, 0xEB);

        static std::wstring newClbFormat = Magic<'u','c','p','c','l','b'>(1969560182);
        patch::SetPointer(0x4601E1 + 1, (void *)newClbFormat.c_str());
        patch::SetPointer(0x46033B + 1, (void *)newClbFormat.c_str());

        static std::wstring newSavFormat = Magic<'u','c','p','d','b'>(2349550184);
        patch::SetPointer(0x4536D2 + 1, (void *)newSavFormat.c_str());

    #ifndef EDITOR_WRITE_TEXT
        patch::RedirectCall(0x4E6B51, OnWriteCountryData<true>);
        patch::RedirectCall(0x4E6B8F, OnWriteCountryData<false>);
        patch::SetPointer(0x675338, OnWriteFile);
    #endif

    #ifndef EDITOR_WRITE_TEXT
        patch::RedirectJump(0x4E6AB5, (void *)0x4E6B0F);
    #endif

        patch::RedirectCall(0x451785, OnLoadCountry);
        patch::RedirectCall(0x4D4076, OnLoadCountry);
        
    #ifndef EDITOR_READ_TEXT
        patch::RedirectCall(0x4EA364, OnReadCountryData<true>);
        patch::RedirectCall(0x4EA3B2, OnReadCountryData<false>);
        patch::SetPointer(0x675334, OnReadFile);
    #endif

        patch::RedirectCall(0x4C7AEC, OnCloseClubFile);

        patch::RedirectCall(0x4C7B71, OnOpenClubFile);
        patch::RedirectCall(0x4C7BB6, OnOpenClubFile);

        patch::RedirectCall(0x453880, OnStoreLeagueCountry);
        patch::RedirectCall(0x453895, OnGetLeagueFileName);

        patch::SetUChar(0x64E614 + 13, 'C');

        patch::SetUChar(0x464167 + 1, 2);
        patch::RedirectJump(0x4641E5, MySetEnableMenu);
        patch::SetUChar(0x4641F0 + 1, 0);

        patch::Nop(0x4640E5, 6);

        patch::SetUChar(0x46412D, 0xEB);
        patch::SetUChar(0x4C1471, 0xEB);

        patch::SetUChar(0x4640F9, 0xEB);

    #ifndef EDITOR_READ_TEXT
        patch::RedirectCall(0x53ADD4, OnGetFreeAgentsFilePath);
        patch::RedirectCall(0x53B585, OnGetFreeAgentsFilePath);

        patch::RedirectCall(0x53B5AC, OnOpenFreeAgentsFile);
    #endif

    #ifndef EDITOR_WRITE_TEXT
        patch::RedirectCall(0x53AEF6, OnCloseFreeAgentsFile);
    #endif

        patch::RedirectCall(0x442B25, WasEULAShown); // EULA

        patch::RedirectCall(0x4427E9, OnGetEditorEULAPath);

        patch::RedirectCall(0x52769C, OnPlayerCommentFormat);

        // player bio additional info
        patch::RedirectCall(0x46E665, RetTrue);
        patch::RedirectCall(0x46E695, RetTrue);


        if (bNamesForAllLanguages != 0) {
            patch::RedirectCall(0x42A324, SetClubNameForCurrentLanguage<0x4C1CF0>);
            patch::RedirectCall(0x42A27A, SetClubNameForCurrentLanguage<0x4C1D70>);
            patch::RedirectCall(0x42A268, SetClubNameForCurrentLanguage<0x4C1E10>);
            patch::RedirectCall(0x42A348, SetClubNameForCurrentLanguage<0x4C1E80>);
            patch::RedirectCall(0x437D44, SetClubStadiumName);
            patch::RedirectCall(0x42A2B0, SetClubPlayerInTextForCurrentLanguage);
            patch::RedirectCall(0x42A2C4, SetClubFanNameForCurrentLanguage<1852>);
            patch::RedirectCall(0x42A2D8, SetClubFanNameForCurrentLanguage<1852>);
            patch::RedirectCall(0x42A2EC, SetClubFanNameForCurrentLanguage<2620>);
            patch::RedirectCall(0x42A300, SetClubFanNameForCurrentLanguage<2620>);
        }

    #ifdef UPDATE_CLUB_BUDGETS
        patch::RedirectCall(0x4C5563, OnReadClubCapital);
        patch::RedirectCall(0x4C5571, OnReadClubTransferBudget);

        FifamReader reader(L"D:\\Games\\FIFA Manager 13\\ClubBudgets.csv", 13);
        reader.SkipLine();
        while (!reader.IsEof()) {
            if (reader.EmptyLine())
                reader.SkipLine();
            else {
                String d;
                ClubBudget b;
                UInt clubId = 0;
                reader.ReadLine(d, d, Hexadecimal(clubId), d, b.cash, b.salaries, b.salariesLeft, b.transfers, b.transfersLeft, b.infrastructure, b.infrastructureLeft,
                    b.misc, b.miscLeft, b.reserve);
                if (clubId != 0)
                    budgets[clubId] = b;
            }
        }
    #endif
    }
}
