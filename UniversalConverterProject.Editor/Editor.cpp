#include "Editor.h"
#include "FifamTypes.h"
#include "Utils.h"
#include "license_check/license_check.h"
#include "Random.h"
#include "Compiler.h"
#include "FifamReadWrite.h"
#include "Competitions.h"

using namespace plugin;

//#define EDITOR_READ_TEXT
#define EDITOR_WRITE_TEXT
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
Vector<CompDesc> gCompiledComps;

Bool METHOD OnWriteCountryData(void *country, DUMMY_ARG, void *file) {
    Bool result = CallMethodAndReturn<Bool, 0x4E4F30>(country, file);
    gWritingStatus = 1;
    WideChar* filePath = raw_ptr<WideChar>(file, 0x10);
    static Path newPath;
    newPath = filePath;
    static std::wstring newSavFormat = Magic<'u', 'c', 'p', 's', 'a', 'v'>(144798475);
    newPath.replace_extension(newSavFormat);
    wcscpy_s(filePath, 260, newPath.c_str());
    CallMethod<0x512F00>(file);
    gWritingStatus = -1;
    return result;
}

Bool METHOD OnReadCountryData(void *file, DUMMY_ARG, WideChar const *filePath, Int encoding) {
    //Message(String(L"Loading ") + filePath);
    gReadingStatus = 1;
    static Path newPath;
    newPath = filePath;
    static std::wstring newSavFormat = Magic<'u', 'c', 'p', 's', 'a', 'v'>(144798475);
    newPath.replace_extension(newSavFormat);
    Bool result = CallMethodAndReturn<Bool, 0x5140D0>(file, newPath.c_str(), encoding);
    gReadingStatus = -1;
    return result;
}

Bool __stdcall OnWriteFile(LPCWSTR lpFileName, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
    Bool result = false;
    if (gWritingStatus == 1) {
        UChar key[8] = { 'N', 4, 'X', 1, 7, 'E', 1, 'Y' };
        UChar *data = (UChar *)lpBuffer;
        for (UInt i = 0; i < nNumberOfBytesToWrite; i++)
            data[i] = data[i] ^ key[i % (sizeof(key) / sizeof(char))];
    }
    HANDLE h = CreateFileW(lpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (h != (HANDLE)-1 && h != 0) {
        DWORD numBytesWritten = 0;
        if (WriteFile(h, lpBuffer, nNumberOfBytesToWrite, &numBytesWritten, NULL) && numBytesWritten == nNumberOfBytesToWrite)
            result = true;
        CloseHandle(h);
    }
    if (gWritingStatus != -1)
        gWritingStatus = -1;
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
    if (gReadingStatus == 1) {
        result = false;
        if (*ppData) {
            UChar *data = (UChar *)(*ppData);
            UChar key[8] = { 'N', 4, 'X', 1, 7, 'E', 1, 'Y' };
            for (UInt i = 0; i < fileSize; i++)
                data[i] = data[i] ^ key[i % (sizeof(key) / sizeof(char))];
            result = true;
        }
    }
    if (!result) {
        if (*ppData) {
            Call<0x5B04A3>(*ppData);
            *ppData = 0;
        }
        *pDataSize = 0;
    }
    if (gReadingStatus != -1)
        gReadingStatus = -1;
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
    String freeAgentsFilePathFormat = Magic<'%','s','/','W','i','t','h','o','u','t','.','u','c','p','s','a','v'>(1898339310);
    swprintf(dest, freeAgentsFilePathFormat.c_str(), dbPath);
}

UInt __stdcall WasEULAShown(Int) {
    static Int counter = 0;
    counter++;
    return counter == 1;
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
    return L"FM";
}

Int METHOD OnDialogDoModal(void *dlg) {
    void *app = CallAndReturn<void *, 0x4BE1A0>();
    if (app) {
        PostMessageW(*raw_ptr<HWND>(app, 0x22D0), 0x118, 0, 0);
        //CallMethod<0x420E10>(wnd, 0x118, 0, 0);
    }
    return CallMethodAndReturn<Int, 0x5B0869>(dlg);
}

unsigned int beardTypes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
unsigned int skinColors[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
unsigned int hairColors[] = { 0, 1, 2, 3, 4, 5, 6, 7, 9 };
unsigned int hairTypes[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162 };
unsigned int headTypes[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,500,501,502,503,504,505,506,507,508,509,510,511,512,513,514,515,516,517,518,519,520,1000,1001,1002,1003,1004,1005,1006,1007,1008,1009,1010,1011,1012,1013,1014,1015,1016,1017,1018,1500,1501,1502,1503,1504,1505,1506,1507,1508,1509,1510,1511,1512,1513,1514,1515,1516,1517,1518,1519,1520,1521,1522,1523,1524,1525,1526,2000,2001,2002,2003,2004,2005,2006,2007,2008,2009,2010,2012,2500,2501,2502,2503,2504,2505,2506,3000,3001,3002,3003,3004,3005,3500,3501,3502,3503,3504,3505,4000,4001,4002,4003,4500,4501,4502,5000,5001,5002,5003,521,522,523,524,525,526,527,528,529,530,531,532,533,534,535,536,537,538,539,540,541,542,543,544,545,546,547,548,549,550,551,552,553,554,555,556,557,558,559,560,561,562,1019,1020,1021,1022,1023,1024,1025,1026,1027,1527,1528,2011,2013,2014,2015,2016,2017,2019,2020,2021,2022,2023,2024,2025,2026,2027,2028,2029,2030,2507,2508,2509,2510,2511,2512,2513,2514,2515,2516,2517,2518,4525 };
unsigned int eyeColors[] = { 1,2,9,5,4,7,0,3,8,6 };

unsigned int METHOD GetAppearanceAssetCount(void *t, DUMMY_ARG, int type) {
    switch (type) {
    case 0:
        return std::size(headTypes);
    case 1:
        return  std::size(skinColors);
    case 2:
        return 3;
    case 3:
        return std::size(hairTypes);
    case 4:
        return std::size(hairColors);
    case 5:
        return std::size(eyeColors);
    case 6:
        return 2;
    case 7:
        return std::size(beardTypes);
    case 8:
        return 5;
    }
    return 0;
}

void *gPlayerPlayerFaceDialog = nullptr;

int METHOD OnGetPlayerSpecialFace(void *player) {
    gPlayerPlayerFaceDialog = player;
    return CallMethodAndReturn<int, 0x51C410>(player);
}

void StorePlayerEyeColorForStarhead(void *data) {
    *raw_ptr<UInt>(data, 0x74) = 2;
    if (gPlayerPlayerFaceDialog) {
        UChar eyeColor = CallMethodAndReturn<UChar, 0x51C3D0>(gPlayerPlayerFaceDialog);
        if (eyeColor < std::size(eyeColors))
        *raw_ptr<UInt>(data, 0x74) = eyeColors[eyeColor];
    }
}

void __declspec(naked) OnGetPlayerEyeColorForStarhead() {
    __asm push ebp
    __asm call StorePlayerEyeColorForStarhead
    __asm pop ebp
    __asm mov eax, 0x4B6B34
    __asm jmp eax
}

void __declspec(naked) EditorDateFix() {
    __asm {
        mov     eax, [esi + 0x228]
        push    eax
        mov     eax, 0x510D10 // IsLeapYear
        call    eax
        add     esp, 4
        mov     cl, al
        mov     eax, [ebp + 0]
        mov     ebx, 1
        mov     edx, 0x57F9A7
        jmp     edx
    }
}

Bool METHOD ShowErr(void *t, DUMMY_ARG, wchar_t const *filename) {

    Error("%d", *raw_ptr<UInt>(t, 0x14));
    return CallMethodAndReturn<Bool, 0x550B90>(t, filename);
}

void PatchEditor(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {

        //patch::RedirectCall(0x4D9868, ShowErr);
        
        //auto useOriginalDocumentsFolder = Magic<'U','S','E','_','O','R','I','G','I','N','A','L','_','D','O','O','C','U','M','E','N','T','S','_','F','O','L','D','E','R'>(2892465454);
        //auto noEditorModalDialogsFix = Magic<'N','O','_','E','D','I','T','O','R','_','M','O','D','A','L','_','D','I','A','L','O','G','S','_','F','I','X'>(4148762047);
        Int bNamesForAllLanguages = GetPrivateProfileIntW(L"MAIN", L"EDITOR_NAMES_FOR_ALL_LANGUAGES", 0, L".\\ucp.ini");
        //Int bUseOriginalDocumentsFolder = GetPrivateProfileIntW(mainSectionStr.c_str(), useOriginalDocumentsFolder.c_str(), 0, fileName.c_str());
        //Int bNoEditorModalDialogsFix = GetPrivateProfileIntW(mainSectionStr.c_str(), noEditorModalDialogsFix.c_str(), 0, fileName.c_str());

        //if (bUseOriginalDocumentsFolder == 0)
        patch::SetPointer(0x6685BC, GetAppLocalizedName);

    //if (bNoEditorModalDialogsFix == 0)
        patch::RedirectCall(0x5B0D69, OnDialogDoModal);

        wchar_t textLang[MAX_PATH];
        GetPrivateProfileStringW(L"OPTIONS", L"TEXT_LANGUAGE", L"eng", textLang, MAX_PATH, L".\\locale.ini");
        String strLang = Utils::ToLower(textLang);
        if (strLang == L"eng")
            gCurrentLanguage = FmLanguage::English;
        if (strLang == L"ger")
            gCurrentLanguage = FmLanguage::German;
        if (strLang == L"fre")
            gCurrentLanguage = FmLanguage::French;
        if (strLang == L"ita")
            gCurrentLanguage = FmLanguage::Italian;
        if (strLang == L"spa")
            gCurrentLanguage = FmLanguage::Spanish;
        if (strLang == L"pol")
            gCurrentLanguage = FmLanguage::Polish;
        if (strLang == L"rus")
            gCurrentLanguage = FmLanguage::Russian;

        /*
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
        */

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

        // disabele database update request
        patch::SetUChar(0x4C1471, 0xEB);

        // Editor name
        patch::SetUChar(0x64E614 + 13, 'C');

        /*
        patch::SetUChar(0x4FB120, 0xC3);
        patch::Nop(0x4FAD4B, 2);
        patch::Nop(0x4FAD54, 5);

        patch::SetUChar(0x45EE21, 0xEB);

        static std::wstring newClbFormat = Magic<'u', 'c', 'p', 'c', 'l', 'b'>(1969560182);
        patch::SetPointer(0x4601E1 + 1, (void *)newClbFormat.c_str());
        patch::SetPointer(0x46033B + 1, (void *)newClbFormat.c_str());

        static std::wstring newSavFormat = Magic<'u', 'c', 'p', 's', 'a', 'v'>(144798475);
        patch::SetPointer(0x4536D2 + 1, (void *)newSavFormat.c_str());

    #ifndef EDITOR_WRITE_TEXT
        patch::RedirectCall(0x4E6B8F, OnWriteCountryData);
        patch::SetPointer(0x675338, OnWriteFile);
    #endif

    #ifndef EDITOR_WRITE_TEXT
        //patch::RedirectJump(0x4E6AB5, (void *)0x4E6B0F);
    #endif

    #ifndef EDITOR_READ_TEXT
        patch::RedirectCall(0x4EA3B2, OnReadCountryData);
        patch::SetPointer(0x675334, OnReadFile);
    #endif

        patch::RedirectCall(0x4C7AEC, OnCloseClubFile);

        patch::RedirectCall(0x4C7B71, OnOpenClubFile);
        patch::RedirectCall(0x4C7BB6, OnOpenClubFile);

        patch::SetPointer(0x45388F + 1, L"CountryData%d.ucpsav");

        patch::SetUChar(0x464167 + 1, 2);
        patch::RedirectJump(0x4641E5, MySetEnableMenu);
        patch::SetUChar(0x4641F0 + 1, 0);

        patch::Nop(0x4640E5, 6);

        patch::SetUChar(0x46412D, 0xEB);
        patch::SetUChar(0x4C1471, 0xEB);

        patch::SetUChar(0x4640F9, 0xEB);

    #ifndef EDITOR_READ_TEXT
        patch::RedirectCall(0x53B585, OnGetFreeAgentsFilePath);
        patch::RedirectCall(0x53B5AC, OnOpenFreeAgentsFile);
    #endif

    #ifndef EDITOR_WRITE_TEXT
        patch::RedirectCall(0x53ADD4, OnGetFreeAgentsFilePath);
        patch::RedirectCall(0x53AEF6, OnCloseFreeAgentsFile);
    #endif
    */

        patch::RedirectCall(0x442B25, WasEULAShown); // EULA

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

        // Mutex names
        patch::SetUInt(0x540F07 + 1, 0);
        patch::SetUInt(0x54108D + 1, 0);

        // sideburns
        patch::SetUInt(0x681C14 + 4 * 1, 0); // sideburns 1

        // beard types
        patch::SetUInt(0x681C40, std::size(beardTypes));
        patch::SetPointer(0x418720 + 0xA8 + 2, beardTypes); // mov     ecx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0xC9 + 2, beardTypes); // mov     edx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0xF3 + 1, beardTypes); // mov     eax, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x11D + 2, beardTypes); // mov     ecx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x13E + 2, beardTypes); // mov     edx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x15C + 2, beardTypes); // mov     edx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x17A + 2, beardTypes); // mov     edx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x191 + 3, beardTypes); // mov     ecx, ds:gFifaBeardTypes[eax*4]
        patch::SetPointer(0x41FE10 + 0x110 + 3, beardTypes); // mov     edx, ds:gFifaBeardTypes[ecx*4]
        patch::SetPointer(0x486E50 + 0x17C + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[edx*4]
        patch::SetPointer(0x4B6AE0 + 0x11E + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[edx*4]
        patch::SetPointer(0x418720 + 0x48 + 1, beardTypes); // mov     eax, ds:gFifaBeardTypes+4
        patch::SetPointer(0x486DCC + 3, beardTypes); // mov     edx, ds:gFifaBeardTypes[eax*4]; jumptable 00486D5C case 7
        patch::SetPointer(0x6D0A84, beardTypes); // .data:006D0A84	dd offset gFifaBeardTypes
        patch::SetPointer(0x5A04CD + 1, &beardTypes[0]);
        patch::SetPointer(0x5A04C8 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A04DE + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A04F7 + 1, &beardTypes[0]);
        patch::SetPointer(0x5A054D + 1, &beardTypes[0]);
        patch::SetPointer(0x5A0548 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A055E + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A0575 + 1, &beardTypes[0]);
        patch::SetPointer(0x5A0570 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A058A + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A059C + 1, &beardTypes[0]);
        patch::SetPointer(0x59F19C + 1, &beardTypes[0]);
        patch::SetPointer(0x59F197 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x59F1A9 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x59F1B8 + 1, &beardTypes[0]);

        // skin colors
        patch::SetUInt(0x681C44, std::size(skinColors));
        patch::SetPointer(0x418720 + 0xC2 + 3, skinColors); // mov     ecx, ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x41FE10 + 0xB6 + 3, skinColors); // mov     edx, ds:gFifaSkinColors[ecx*4]
        patch::SetPointer(0x486E50 + 0xDA + 3, skinColors); // mov     eax, ds:gFifaSkinColors[edx*4]
        patch::SetPointer(0x4B6AE0 + 0x9D + 3, skinColors); // mov     eax, ds:gFifaSkinColors[edx*4]
        patch::SetPointer(0x418720 + 0x2A + 1, skinColors); // mov     eax, ds:gFifaSkinColors+4
        patch::SetPointer(0x486D72 + 3, skinColors); // mov     edx, ds:gFifaSkinColors[eax*4]; jumptable 00486D5C case 1
        patch::SetPointer(0x6D0A74, skinColors); // .data:006D0A74	dd offset gFifaSkinColors
        patch::SetPointer(0x59FB63 + 1, &skinColors[0]);
        patch::SetPointer(0x59FB5E + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FB74 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FB7F + 1, &skinColors[0]);
        patch::SetPointer(0x59FDE0 + 1, &skinColors[0]);
        patch::SetPointer(0x59FDDB + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FDF1 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FE07 + 1, &skinColors[0]);
        patch::SetPointer(0x59FE02 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FE18 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FE23 + 1, &skinColors[0]);
        patch::SetPointer(0x59F21C + 1, &skinColors[0]);
        patch::SetPointer(0x59F217 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59F229 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59F238 + 1, &skinColors[0]);

        // hair colors
        patch::SetUInt(0x681C50, std::size(hairColors));
        patch::SetPointer(0x418720 + 0x155 + 3, hairColors); // mov     ecx, ds:gFifaHairColors[eax*4]
        patch::SetPointer(0x41FE10 + 0xF2 + 3, hairColors); // mov     ecx, ds:gFifaHairColors[eax*4]
        patch::SetPointer(0x486E50 + 0x140 + 3, hairColors); // mov     edx, ds:gFifaHairColors[ecx*4]
        patch::SetPointer(0x4B6AE0 + 0xEE + 3, hairColors); // mov     edx, ds:gFifaHairColors[ecx*4]
        patch::SetPointer(0x418720 + 0x5B + 2, hairColors); // mov     ecx, ds:gFifaHairColors+4
        patch::SetPointer(0x486D9F + 3, hairColors); // mov     edx, ds:gFifaHairColors[eax*4]; jumptable 00486D5C case 4
        patch::SetPointer(0x6D0A8C, hairColors); // .data:006D0A8C	dd offset gFifaHairColors
        patch::SetPointer(0x5A0269 + 1, &hairColors[0]);
        patch::SetPointer(0x5A0264 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A027A + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0285 + 1, &hairColors[0]);
        patch::SetPointer(0x5A036F + 1, &hairColors[0]);
        patch::SetPointer(0x5A036A + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0380 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0397 + 1, &hairColors[0]);
        patch::SetPointer(0x5A0392 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A03AC + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A03B8 + 1, &hairColors[0]);
        patch::SetPointer(0x59F11C + 1, &hairColors[0]);
        patch::SetPointer(0x59F117 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x59F129 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x59F138 + 1, &hairColors[0]);

        // hair models
        patch::SetUInt(0x681C3C, std::size(hairTypes));
        patch::SetPointer(0x418720 + 0xCF + 1, hairTypes); // mov     eax, ds:gFifaHairModels
        patch::SetPointer(0x418720 + 0xF8 + 2, hairTypes); // mov     ecx, ds:gFifaHairModels
        patch::SetPointer(0x418720 + 0x137 + 3, hairTypes); // mov     ecx, ds:gFifaHairModels[eax*4]
        patch::SetPointer(0x41FE10 + 0xE3 + 3, hairTypes); // mov     edx, ds:gFifaHairModels[ecx*4]
        patch::SetPointer(0x486E50 + 0x12B + 3, hairTypes); // mov     eax, ds:gFifaHairModels[edx*4]
        patch::SetPointer(0x4B6AE0 + 0xDC + 3, hairTypes); // mov     eax, ds:gFifaHairModels[edx*4]
        patch::SetPointer(0x4188D0 + 2, hairTypes); // mov     edx, ds:gFifaHairModels
        patch::SetPointer(0x486D90 + 3, hairTypes); // mov     ecx, ds:gFifaHairModels[eax*4]; jumptable 00486D5C case 3
        patch::SetPointer(0x6D0A70, hairTypes); // .data:006D0A70	dd offset gFifaHairModels
        patch::SetPointer(0x59FFD1 + 1, &hairTypes[0]);
        patch::SetPointer(0x59FFCC + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x59FFE2 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x59FFED + 1, &hairTypes[0]);
        patch::SetPointer(0x5A0110 + 1, &hairTypes[0]);
        patch::SetPointer(0x5A010B + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x5A0121 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x5A0138 + 1, &hairTypes[0]);
        patch::SetPointer(0x5A0133 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x5A014D + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x5A0159 + 1, &hairTypes[0]);
        patch::SetPointer(0x59F0DC + 1, &hairTypes[0]);
        patch::SetPointer(0x59F0D7 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x59F0E9 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x59F0F8 + 1, &hairTypes[0]);

        // head models
        patch::SetUInt(0x681C38, std::size(headTypes));
        patch::SetPointer(0x418720 + 0xA1 + 3, headTypes); // mov     eax, ds:gFifaHeadModels[edx*4]
        patch::SetPointer(0x41FE10 + 0xAB + 3, headTypes); // mov     eax, ds:gFifaHeadModels[edx*4]
        patch::SetPointer(0x486E50 + 0xB9 + 3, headTypes); // mov     ecx, ds:gFifaHeadModels[eax*4]
        patch::SetPointer(0x4B6AE0 + 0x85 + 3, headTypes); // mov     ecx, ds:gFifaHeadModels[eax*4]
        patch::SetPointer(0x418720 + 0x2F + 2, headTypes); // mov     edx, ds:gFifaHeadModels+4
        patch::SetPointer(0x486D63 + 3, headTypes); // mov     ecx, ds:gFifaHeadModels[eax*4]; jumptable 00486D5C case 0
        patch::SetPointer(0x6D0A6C, headTypes); // .data:off_6D0A6C	dd offset gFifaHeadModels
        patch::SetPointer(0x59FAF1 + 1, &headTypes[0]);
        patch::SetPointer(0x59FAEC + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FB02 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FB0D + 1, &headTypes[0]);
        patch::SetPointer(0x59FD31 + 1, &headTypes[0]);
        patch::SetPointer(0x59FD2C + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FD42 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FD4D + 1, &headTypes[0]);
        patch::SetPointer(0x59FD6C + 1, &headTypes[0]);
        patch::SetPointer(0x59FD67 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FD7D + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FD88 + 1, &headTypes[0]);
        patch::SetPointer(0x59F09C + 1, &headTypes[0]);
        patch::SetPointer(0x59F097 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59F0A9 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59F0B8 + 1, &headTypes[0]);

        patch::SetUInt(0x681C58, std::size(eyeColors));
        patch::SetPointer(0x418720 + 0x51 + 2, eyeColors); // mov     edx, ds:gFifaEyeColors
        patch::SetPointer(0x418720 + 0x116 + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0x41FE10 + 0xD4 + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0x486E50 + 0x10A + 3, eyeColors); // mov     ecx, ds:gFifaEyeColors[eax*4]
        patch::SetPointer(0x4B6AE0 + 0xC4 + 3, eyeColors); // mov     ecx, ds:gFifaEyeColors[eax*4]
        patch::SetPointer(0x486DAE + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[eax*4]; jumptable 00486D5C case 5
        patch::SetPointer(0x6D0A80, eyeColors); // .data:006D0A80	dd offset gFifaEyeColors
        patch::SetPointer(0x59FC73 + 1, &eyeColors[0]);
        patch::SetPointer(0x59FC6E + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FC84 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FC8F + 1, &eyeColors[0]);
        patch::SetPointer(0x59FCAF + 1, &eyeColors[0]);
        patch::SetPointer(0x59FCAA + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FCC4 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FCD0 + 1, &eyeColors[0]);
        patch::SetPointer(0x59FF19 + 1, &eyeColors[0]);
        patch::SetPointer(0x59FF14 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FF2A + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FF40 + 1, &eyeColors[0]);
        patch::SetPointer(0x59FF3B + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FF51 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FF5C + 1, &eyeColors[0]);
        patch::SetPointer(0x59F29C + 1, &eyeColors[0]);
        patch::SetPointer(0x59F297 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59F2A9 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59F2B8 + 1, &eyeColors[0]);

        patch::RedirectJump(0x4FB290, GetAppearanceAssetCount);

        // starheads eye colors
        patch::RedirectCall(0x4B6B1D, OnGetPlayerSpecialFace);
        patch::RedirectJump(0x4B6B2D, OnGetPlayerEyeColorForStarhead);

        // fifa id
        patch::Nop(0x435A96, 3);

        patch::RedirectJump(0x57F99F, EditorDateFix);

        static WideChar const *databaseRestoreFiles[] = {
            L"database\\data\\CountryData*.sav",
            L"database\\script\\CountryScript*.sav",
            L"database\\AppearanceDefs.sav",
            L"database\\Assessment.sav",
            L"database\\Countries.sav",
            L"database\\Master.dat",
            L"database\\Rules.sav",
            L"database\\Without.sav",
            L"database\\PriorityClubs.txt",
            L"database\\FemaleNames.txt",
            L"database\\MaleNames.txt",
            L"database\\Surnames.txt",
            nullptr
        };

        static WideChar const *databaseRestoreFolders1[] = {
            L"script",
            L"database",
            L"database\\data",
            L"database\\script",
            nullptr
        };

        static WideChar const *databaseRestoreFolders2[] = {
            L"data",
            L"script",
            nullptr
        };

        patch::SetPointer(0x4FA87D + 2, databaseRestoreFiles);
        patch::SetPointer(0x4FA886 + 3, databaseRestoreFiles);
        patch::SetPointer(0x4FABBF + 3, databaseRestoreFiles);

        patch::SetPointer(0x4FAF20 + 2, databaseRestoreFolders1);
        patch::SetUChar(0x4FAF4B + 2, UChar((std::size(databaseRestoreFolders1) - 1) * 4));

        patch::SetPointer(0x4FAE90 + 2, databaseRestoreFolders2);
        patch::SetUChar(0x4FAEBB + 2, UChar((std::size(databaseRestoreFolders2) - 1) * 4));
    }
}
