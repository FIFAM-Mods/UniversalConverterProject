#include "3dPatches.h"
#include "GfxCoreHook.h"
#include <dinput.h>
#include "shared.h"
#include "GameInterfaces.h"
#include "Utils.h"
#include "Settings.h"
#include "Exception.h"
#include "Translation.h"
#include "license_check/license_check.h"
#include "shared.h"

using namespace plugin;

void OnCreateInputDevice() {
    SafeLog::Write(L"Initializing devices");
    CallMethodDynGlobal(GfxCoreAddress(0x309150), patch::GetUInt(GfxCoreAddress(0x6696DC), false));
    CallDynGlobal(GfxCoreAddress(0x308040));
}

void OnSetupTeamManagementDatas() {
    auto SetHomeTeamControl = [](Bool enable) { patch::SetUInt(GfxCoreAddress(0x6FA2F8 + 0x64), enable ? 1 : 0, false); };
    auto SetAwayTeamControl = [](Bool enable) { patch::SetUInt(GfxCoreAddress(0x6FA2F8 + 0x94 + 0x64), enable ? 1 : 0, false); };

    Bool teamControl = Settings::GetInstance().getTeamControl();
    Int humanHome = gfxGetVarInt("HUMAN_HOME", 0);
    Int humanAway = gfxGetVarInt("HUMAN_AWAY", 0);
    if (teamControl && humanHome && !humanAway) {
        SetHomeTeamControl(true);
        SetAwayTeamControl(false);
    }
    else if (teamControl && !humanHome && humanAway) {
        SetHomeTeamControl(false);
        SetAwayTeamControl(true);
    }
    else {
        SetHomeTeamControl(false);
        SetAwayTeamControl(false);
    }
    CallDynGlobal(GfxCoreAddress(0xCAC70));
}

/*
void OnSetupManualPlayerSwitch() {
    CallDynGlobal(GfxCoreAddress(0x23B750));
    if (Settings::GetInstance().getTeamControl() && Settings::GetInstance().getManualPlayerSwitch()) {
        SafeLog::Write(L"Manual switch status: enabled");
        Int humanHome = gfxGetVarInt("HUMAN_HOME", 0);
        Int humanAway = gfxGetVarInt("HUMAN_AWAY", 0);
        if (humanHome && !humanAway) {
            patch::SetUInt(GfxCoreAddress(0xAE095C), 1, false);
            SafeLog::Write(L"Enabled manual switch for home team");
        }
        else if (!humanHome && humanAway) {
            patch::SetUInt(GfxCoreAddress(0xAE0960), 1, false);
            SafeLog::Write(L"Enabled manual switch for away team");
        }
    }
    else
        SafeLog::Write(L"Manual switch status: disabled");
}
*/

void OnSetupManualPlayerSwitch() {
    auto SetHomeTeamControl = [](Bool enable) { patch::SetUInt(GfxCoreAddress(0x6FA2F8 + 0x64), enable ? 1 : 0, false); };
    auto SetAwayTeamControl = [](Bool enable) { patch::SetUInt(GfxCoreAddress(0x6FA2F8 + 0x94 + 0x64), enable ? 1 : 0, false); };

    Bool doReset = false;

    if (Settings::GetInstance().getTeamControl() && Settings::GetInstance().getManualPlayerSwitch()) {
        Int humanHome = gfxGetVarInt("HUMAN_HOME", 0);
        Int humanAway = gfxGetVarInt("HUMAN_AWAY", 0);
        if (humanHome && !humanAway) {
            SetHomeTeamControl(true);
            SetAwayTeamControl(false);
            doReset = true;
        }
        else if (!humanHome && humanAway) {
            SetHomeTeamControl(false);
            SetAwayTeamControl(true);
            doReset = true;
        }
    }

    CallDynGlobal(GfxCoreAddress(0x23B750));

    if (doReset) {
        SetHomeTeamControl(false);
        SetAwayTeamControl(false);
    }
}

Int METHOD OnEnableManualSwitch(void *vars, DUMMY_ARG, char const *param, Int defVal) {
    // TODO
    return 0;
}

Int METHOD OnGet3dPlayerBasicLevel(void *io, DUMMY_ARG, Int type, Char const *attrName) {
    Int result = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, attrName);
    result = (Int)((Float)result * ((Float)Settings::GetInstance().getTeamControlDifficulty() / 100.0f));
    return result;
}

Int METHOD OnGet3dPlayerOverall(void *io, DUMMY_ARG, Int type, Char const *attrName) {
    Int result = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, attrName);
    result = (Int)((Float)result * ((Float)Settings::GetInstance().getTeamControlDifficulty() / 100.0f));
    return result;
}

Int METHOD OnGet3dPlayerAttribute(void *io, DUMMY_ARG, Int type, Char const *attrName) {
    Int result = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, attrName);
    result = (Int)((Float)result * ((Float)Settings::GetInstance().getTeamControlDifficulty() / 100.0f));
    return result;
}

void OnSetTeamDifficulty(unsigned int nTeam) {
    patch::SetUInt(GfxCoreAddress(0x6FA2F8 + 0x58), 3, false);
    patch::SetUInt(GfxCoreAddress(0x6FA2F8 + 0x94 + 0x58), 0, false);
}

BOOL __stdcall OnEnumerateDevices(DIDEVICEINSTANCE *dev, int *counter) {
    BOOL result = CallMethodAndReturnDynGlobal<bool>(GfxCoreAddress(0x308EA0), 0, dev, counter);
    SafeLog::Write(Utils::Format(L"Device %d: %d - %s - %s", *counter, dev->dwDevType, AtoW(dev->tszInstanceName).c_str(), AtoW(dev->tszProductName).c_str()));
    return result;
}

void *OnSetupController(int deviceId, int infoType) {
    void *info = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x309040), deviceId, infoType);
    if (info && *raw_ptr<UChar>(info) == 4) { // joystick
        SafeLog::Write(L"Setup joystick...");
        UInt deviceFlags = *raw_ptr<UInt>(info, 4);
        if (deviceFlags & 1) {
            SafeLog::Write(L"Joystick flag 1 ENABLED");
            CallDynGlobal(GfxCoreAddress(0x181A0), deviceId, 0); // RegisterInGameDevice2
            UInt *deviceFlagsData = (UInt *)patch::GetPointer(GfxCoreAddress(0x6696EC), false);
            if (deviceFlagsData) {
                SafeLog::Write(L"Device flags data AVAILABLE");
                if (deviceFlags & 2) {
                    SafeLog::Write(L"Joystick flag 2 ENABLED");
                    if (CallAndReturnDynGlobal<bool>(GfxCoreAddress(0x308460), deviceId)) {
                        SafeLog::Write(L"Unknown check PASSED");
                        Int devData[23];
                        memset(devData, 0, 23 * sizeof(Int));
                        devData[0] = 0;
                        devData[1] = 10000;
                        devData[13] = 1000000;
                        devData[14] = 100;
                        devData[15] = 3;
                        devData[17] = 0;
                        devData[16] = -1;
                        devData[18] = *raw_ptr<UChar>(info, 1) != 6 ? 0 : 90;
                        deviceFlagsData[deviceId] = CallAndReturnDynGlobal<Int>(GfxCoreAddress(0x308C70), deviceId, devData);
                        CallDynGlobal(GfxCoreAddress(0x308790), deviceFlagsData[deviceId]);
                        CallDynGlobal(GfxCoreAddress(0x308800), deviceFlagsData[deviceId]);
                    }
                    else
                        SafeLog::Write(L"Unknown check NOT passed");
                }
                else
                    SafeLog::Write(L"Joystick flag 2 NOT enabled");
            }
            else
                SafeLog::Write(L"Device flags data NOT available");
        }
        else
            SafeLog::Write(L"Joystick flag 1 NOT enabled");
    }
    return info;
}

//void *OnOpenStrangeFile_FM11(char *filename, char *mode) {
//    Error(filename);
//    return CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3FF29E), filename, mode);
//}
//
//template<UInt Addr>
//void CopyNotify(void *dst, void *src, UInt length) {
//    if (!dst || !src) {
//        Error(Utils::Format(L"NULL pointer when copying from %p to %p (size= %u)\n@ 0x%X", src, dst, length, Addr));
//    }
//    else
//        memcpy(dst, src, length);
//    SafeLog::Write(Utils::Format(L"Copying from %p to %p (size= %u)\n@ 0x%X", src, dst, length, Addr));
//}

unsigned int CrashFix_5e3f0_retAddr = 0;

void __declspec(naked) CrashFix_5e3f0() {
    __asm {
        test ebp, ebp
        jz RET_ZERO
        mov eax, [ebp + 4]
        jmp RET_TO_FUNC
    RET_ZERO:
        mov eax, 0
    RET_TO_FUNC:
        mov [esi + 0x18], eax
        jmp CrashFix_5e3f0_retAddr
    }
}

void *TTF_GetAllocator(void *f) {
    return *raw_ptr<void *>(f, 0x5E38);
}

unsigned int TTF_GetSize(void *f) {
    return *raw_ptr<unsigned int>(f, 0x5E38 + 0x1C);
}

void TTF_CreateFactory(char value) {
    SafeLog::Write(Utils::Format(L"CreateFactory: %d", value));
    CallDynGlobal(GfxCoreAddress(0x3D05A0), value);
}

void TTF_DestroyFactory(void *f) {
    SafeLog::Write(Utils::Format(L"DestroyFactory (factory %p)", f));
    CallDynGlobal(GfxCoreAddress(0x3D05C0), f);
}

void METHOD TTF_SetFontsData(void *f, DUMMY_ARG, void *fontInfos, unsigned int numFonts) {
    SafeLog::Write(Utils::Format(L"SetFontsData: %d (factory %p, allocator %p, size %u)", numFonts, f, TTF_GetAllocator(f), TTF_GetSize(f)));
    CallMethodDynGlobal(GfxCoreAddress(0x3D0030), f, fontInfos, numFonts);
}

template<int T>
const char *METHOD TTF_GetFontLineForFont(void *f, DUMMY_ARG, const char *fontTag) {
    /*
    static char *emptyStr = "";
    if (!f || !TTF_GetAllocator(f))
        SafeLog::Write(Utils::Format(L"GetFontLineForFont (%s): %s (factory %p, allocator %p, size %u)", (T? L"SetFontsData" : L"CreateFont"), Utils::AtoW(fontTag), f, TTF_GetAllocator(f), TTF_GetSize(f)));
    const char *result = CallMethodAndReturnDynGlobal<const char *>(GfxCoreAddress(0x3CF5B0), f, fontTag);
    if (!f || !TTF_GetAllocator(f))
        result = emptyStr;
    return result;
    */
    static char *emptyStr = "";
    const char *result = emptyStr;
    if (f && TTF_GetAllocator(f))
        result = CallMethodAndReturnDynGlobal<const char *>(GfxCoreAddress(0x3CF5B0), f, fontTag);
    return result;
}

void *TTF_AllocateAllocator(unsigned int size) {
    void *result = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3CF0B0), size);
    SafeLog::Write(Utils::Format(L"AllocateAllocator: %p", result));
    return result;
}

void MySetFontDescFromParamsLine(void *fontDesc, char const *fontLine) {
    CallDynGlobal(GfxCoreAddress(0x3DC5C0), fontDesc, fontLine);
    void *app = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x16450));
    if (app) {
        Float u = 0.0f, h = 0.0f;
        CallVirtualMethod<45>(app, &u, &h);
        if (h > 0.0f && h < 0.87f) {
            if (strstr(fontLine, "|Weight=400|")) {
                *raw_ptr<Int>(fontDesc, 0) = (Int)((Float)(*raw_ptr<Int>(fontDesc, 0)) * h);
            }
        }
    }
}

void MySetFontDescParam(void *desc, Char const *name, Char const *value) {
    if (GameLanguage() == L"rus" && !_stricmp(name, "Typeface")) {
        static Pair<Char const *, Char const *> replacements[] = {
            { "ApexDFL BookTabular", "FM08M" },
            { "ApexDFL HeavyTabular", "FM08B" },
            { "Champions-Regular", "FM08" },
            { "Champions-Light", "FM08" },
            { "Helvetica LT Std Cond Blk", "EA05_Body" }
        };
        for (UInt i = 0; i < std::size(replacements); i++) {
            if (!_stricmp(value, replacements[i].first)) {
                value = replacements[i].second;
                break;
            }
        }
    }
    CallDynGlobal(GfxCoreAddress(0x3DC2A0), desc, name, value);
}

wchar_t const *gBannersDir = L"data\\banners";

void __declspec(naked) GetBannersDir() {
    __asm {
        mov   esi, [esp + 0x10]
        push  gBannersDir
        push  esi
        mov   eax, 0x1493F2F
        call  eax
        add   esp, 8
        mov   eax, 0x4DA9F2
        jmp   eax
    }
}

int GfxCopyData(void *a, void *b, int length) {
    if (!a || !b)
        return 0;
    return CallAndReturnDynGlobal<int>(GfxCoreAddress(0x3183A0), a, b, length);
}

char const *GetAppTitle() {
    static std::string result = WtoA(GetFullAppName(false));
    return result.c_str();
}

char const *GetAppDocumentsDir() {
    static std::string result = WtoA(GetFMDocumentsFolderName());
    return result.c_str();
}

void LoadPlayerHeadKitTexture(UInt *out, Int, Int, Int) {
    *out = 0;
}

void METHOD unk23(void *t, DUMMY_ARG, Int) {

}

void METHOD MySetModelLights(void *t, DUMMY_ARG, void *l) {
    Float light[28];
    Float irradLight[40];
    for (auto &f : light)
        f = 1.0f;
    for (auto &f : irradLight)
        f = 1.0f;
    CallDynGlobal(GfxCoreAddress(0x39558D), *raw_ptr<void *>(t, 0x30), light, irradLight);
}

Bool gRenderPlayerHead = false;

void METHOD MyRenderFifaPlayer(void *t, DUMMY_ARG, void *d) {
    static Float gDefault = 0.86f;
    static Float gfCustom = 1.75f;
    if (gRenderPlayerHead)
        patch::SetPointer(GfxCoreAddress(0x39039A + 2), &gfCustom);
    else
        patch::SetPointer(GfxCoreAddress(0x39039A + 2), &gDefault);
    CallMethodDynGlobal(GfxCoreAddress(0x3902C8), t, d);
}

void METHOD MyGeneratePlayerPortrait(void *t, DUMMY_ARG, int a2) {
    gRenderPlayerHead = true;
    CallMethodDynGlobal(GfxCoreAddress(0x378E3D), t, a2);
    gRenderPlayerHead = false;
}

void *METHOD OnCreateFileIO(void *fileIO) {
    CallMethodDynGlobal(GfxCoreAddress(0x3C6820), fileIO);
    std::wstring zdataPath = FM::GetGameDir() + L"data\\assets\\";
    wchar_t const *paths[] = { zdataPath.c_str() };
    CallMethodDynGlobal(GfxCoreAddress(0x3C4CB0), fileIO, paths, 1);
    return fileIO;
}

void GetZdataFilename(char *filename) {
    static std::string zdataPath = "data\\assets\\";
    std::string testPath = zdataPath + filename;
    if (CallAndReturnDynGlobal<Bool32>(GfxCoreAddress(0x19E60), testPath.c_str()))
        strcpy(filename, testPath.c_str());
}

void OnGetResourceFilename_0(char *dst, char const *src, size_t len) {
    strncpy(dst, src, len);
    GetZdataFilename(dst);
}

void OnGetResourceFilename_1(char *dst, char const *format, size_t a1) {
    sprintf(dst, format, a1);
    GetZdataFilename(dst);
}

void OnGetResourceFilename_2(char *dst, char const *format, size_t a1, size_t a2) {
    sprintf(dst, format, a1, a2);
    GetZdataFilename(dst);
}

void OnGetResourceFilename_3(char *dst, char const *format, size_t a1, size_t a2, size_t a3) {
    sprintf(dst, format, a1, a2, a3);
    GetZdataFilename(dst);
}

void OnGetResourceFilename_4(char *dst, char const *format, size_t a1, size_t a2, size_t a3, size_t a4) {
    sprintf(dst, format, a1, a2, a3, a4);
    GetZdataFilename(dst);
}

void OnGetResourceFilename_5(char *dst, char const *format, size_t a1, size_t a2, size_t a3, size_t a4, size_t a5) {
    sprintf(dst, format, a1, a2, a3, a4, a5);
    GetZdataFilename(dst);
}

void *METHOD OnConstructAsyncLoader(void *a, DUMMY_ARG, int b, int c) {
    CallMethodDynGlobal(GfxCoreAddress(0x44CC10), a, b, c);
    *(unsigned int *)a = 2;
    return a;
}

char const *GetZdataFilenameStatic(char const *filename) {
    static std::string zdataPath = "data\\assets\\";
    std::string testPath = zdataPath + filename;
    static char staticFilename[1024];
    strcpy(staticFilename, testPath.c_str());
    return staticFilename;
}

char const *GetFilePathForAsynchLoader(char const *srcFilePath, Bool32 *exists, UInt *size) {
    if (strlen(srcFilePath) > 1 && srcFilePath[1] == ':')
        return srcFilePath;
    if (CallAndReturnDynGlobal<Bool32>(GfxCoreAddress(0x19E60), srcFilePath)) {
        if (exists)
            *exists = true;
        if (size)
            *size = CallAndReturnDynGlobal<UInt>(GfxCoreAddress(0x19E70), srcFilePath);
        return srcFilePath;
    }
    static std::string zdataPath = "data\\assets\\";
    std::string testPath = zdataPath + srcFilePath;
    static char staticFilename[1024];
    strcpy(staticFilename, testPath.c_str());
    if (CallAndReturnDynGlobal<Bool32>(GfxCoreAddress(0x19E60), staticFilename)) {
        if (exists)
            *exists = true;
        if (size)
            *size = CallAndReturnDynGlobal<UInt>(GfxCoreAddress(0x19E70), staticFilename);
        return staticFilename;
    }
    return srcFilePath;
}

Bool32 OnFileExistsCheck(char const *filepath) {
    Bool32 exists = false;
    GetFilePathForAsynchLoader(filepath, &exists, nullptr);
    if (exists)
        return true;
    return CallAndReturnDynGlobal<Bool32>(GfxCoreAddress(0x19E60), filepath);
}

UInt OnFileSizeGet(char const *filepath) {
    Bool32 exists = false;
    UInt size = 0;
    GetFilePathForAsynchLoader(filepath, &exists, &size);
    if (exists)
        return size;
    return CallAndReturnDynGlobal<UInt>(GfxCoreAddress(0x19E70), filepath);
}

void OnFileNameGet(char *filepath) {
    Bool32 exists = false;
    //Error("Input file: %s", filepath);
    char const *newPath = GetFilePathForAsynchLoader(filepath, &exists, nullptr);
    if (exists) {
        strcpy(filepath, newPath);
        //Error("Modified file: %s", filepath);
    }
    _strlwr(filepath);
}

wchar_t const * METHOD GetAbsPath(void *t, DUMMY_ARG, wchar_t *output_buf, wchar_t const *input) {
    auto result = CallMethodAndReturnDynGlobal<wchar_t const *>(GfxCoreAddress(0x3C4B80), t, output_buf, input);
    String i = input;
    if (i.starts_with(L"m228"))
        Error(result);
    return result;
}

void METHOD OnFileIoSetupPaths(void *fileIO, DUMMY_ARG, const wchar_t **paths, unsigned int numPaths) {
    Vector<const wchar_t *> myPaths;
    if (numPaths > 7)
        numPaths = 7;
    for (unsigned int i = 0; i < numPaths; i++)
        myPaths.push_back(paths[i]);
    static path zdataPath = FM::GetGameDir() + L"data\\assets\\";
    myPaths.push_back(L"data\\assets\\");
    CallMethodDynGlobal(GfxCoreAddress(0x3C4CB0), fileIO, myPaths.data(), myPaths.size());
}

int AsynchLoaderGlobalTextures_GetTexture(const char *filename) {
    char tmp[1024];
    strcpy(tmp, filename);
    GetZdataFilename(tmp);
    void *asyncLoader = *(void **)GfxCoreAddress(0xD4BA94);
    //Error("AsynchLoaderGlobalTextures_GetTexture %d: %s", *(UInt *)asyncLoader, tmp);
    return CallAndReturnDynGlobal<int>(GfxCoreAddress(0x459040), tmp);
}

void AsynchLoaderGlobalTextures_PreLoadTexture(const char *filename, int a, char b) {
    char tmp[1024];
    strcpy(tmp, filename);
    GetZdataFilename(tmp);
    void *asyncLoader = *(void **)GfxCoreAddress(0xD4BA94);
    //Error("AsynchLoaderGlobalTextures_PreLoadTexture %d: %s", *(UInt *)asyncLoader, tmp);
    CallDynGlobal(GfxCoreAddress(0x4590F0), tmp, a, b);
}

void LoadTextureFSHCB(void *mem, int a2, int a3, unsigned int *shapeUserData) {
    SafeLog::Write(Utils::AtoW((char *)(shapeUserData[3])));
    CallDynGlobal(GfxCoreAddress(0x4093B0), mem, a2, a3, shapeUserData);
}

UInt AssetInfo(StringA const &assetFilename, UInt inputAssetSize, Bool update) {
    static Vector<StringA> assetTypes = { };
    static Vector<UInt> memoryForAsset(assetTypes.size());
    for (size_t i = 0; i < assetTypes.size(); i++) {
        if (assetFilename.starts_with(assetTypes[i])) {
            if (update) {
                if (inputAssetSize > memoryForAsset[i])
                    memoryForAsset[i] = inputAssetSize;
            }
            return memoryForAsset[i];
        }
    }
    return inputAssetSize;
}

void RegisterAsset(StringA const &assetFilename, UInt inputAssetSize){
    AssetInfo(assetFilename, inputAssetSize, true);
}

UInt GetRequiredMemoryForAsset(StringA const &assetFilename, UInt inputAssetSize) {
    return AssetInfo(assetFilename, inputAssetSize, false);
}

void RegisterAssets() {
    // process zdataX.big
    for (UInt i = 0; i < 99; i++) {
        Path zdataPath = Path("data") / Utils::Format(L"zdata_%02d.big", i);
        if (exists(zdataPath)) {
            FILE *bigFile = _wfopen(zdataPath.c_str(), L"rb");
            if (bigFile) {
                UInt bigSignature = 0;
                if (fread(&bigSignature, 4, 1, bigFile) == 1 && bigSignature == 'FGIB') {
                    UInt bigFileSize = 0;
                    if (fread(&bigFileSize, 4, 1, bigFile) == 1 && bigFileSize > 16) {
                        UInt bigNumFiles = 0;
                        if (fread(&bigNumFiles, 4, 1, bigFile) == 1) {
                            bigNumFiles = _byteswap_ulong(bigNumFiles);
                            if (bigNumFiles > 0) {
                                UInt bigHeaderSize = 0;
                                if (fread(&bigHeaderSize, 4, 1, bigFile) == 1) {
                                    bigHeaderSize = _byteswap_ulong(bigHeaderSize);
                                    if (bigHeaderSize > 0) {
                                        for (UInt f = 0; f < bigNumFiles; f++) {
                                            UInt bigFileEntryOffset = 0;
                                            if (fread(&bigFileEntryOffset, 4, 1, bigFile) == 1) {
                                                bigFileEntryOffset = _byteswap_ulong(bigFileEntryOffset);
                                                UInt bigFileEntrySize = 0;
                                                if (fread(&bigFileEntrySize, 4, 1, bigFile) == 1) {
                                                    bigFileEntrySize = _byteswap_ulong(bigFileEntrySize);
                                                    StringA bigEntryFileName;
                                                    Bool failedToReadFileName = false;
                                                    while (true) {
                                                        Char c = 0;
                                                        if (fread(&c, 1, 1, bigFile) != 1) {
                                                            failedToReadFileName = true;
                                                            break;
                                                        }
                                                        if (c == 0)
                                                            break;
                                                        bigEntryFileName += c;
                                                    }
                                                    if (!failedToReadFileName) {
                                                        auto savedFilePos = ftell(bigFile);
                                                        if (fseek(bigFile, bigFileEntryOffset, SEEK_SET) == 0) {
                                                            Bool compressed = false;
                                                            UInt uncompressedSize = bigFileEntrySize;
                                                            if (bigFileEntrySize >= 5) {
                                                                unsigned char compressionType = 0;
                                                                if (fread(&compressionType, 1, 1, bigFile) == 1) {
                                                                    unsigned char compressionFlag = 0;
                                                                    if (fread(&compressionFlag, 1, 1, bigFile) == 1) {
                                                                        if (compressionFlag == 0xFB) {
                                                                            unsigned int compressionHeaderSize = 0;
                                                                            switch (compressionType & 0xFE)
                                                                            {
                                                                            case 0x10:
                                                                            case 0x18:
                                                                            case 0x1A:
                                                                            case 0x1E:
                                                                            case 0x30:
                                                                            case 0x32:
                                                                            case 0x34:
                                                                            case 0x46:
                                                                                compressionHeaderSize = 3;
                                                                                break;
                                                                            case 0x90:
                                                                            case 0x98:
                                                                            case 0x9A:
                                                                            case 0x9E:
                                                                            case 0xB0:
                                                                            case 0xB2:
                                                                            case 0xB4:
                                                                            case 0xC6:
                                                                                compressionHeaderSize = 4;
                                                                                break;
                                                                            }
                                                                            if (compressionHeaderSize > 0) {
                                                                                unsigned char compressionHeader[4] = { 0, 0, 0, 0 };
                                                                                if (fread(&compressionHeader, 1, compressionHeaderSize, bigFile) == compressionHeaderSize) {
                                                                                    compressed = true;
                                                                                    if (compressionHeaderSize == 3)
                                                                                        uncompressedSize = compressionHeader[2] | (compressionHeader[1] << 8) | (compressionHeader[0] << 16);
                                                                                    else
                                                                                        uncompressedSize = compressionHeader[3] | (compressionHeader[2] << 8) | (compressionHeader[1] << 16) | (compressionHeader[0] << 24);
                                                                                }
                                                                                else
                                                                                    break;
                                                                            }
                                                                        }
                                                                    }
                                                                    else
                                                                        break;
                                                                }
                                                                else
                                                                    break;
                                                            }
                                                            RegisterAsset(bigEntryFileName, uncompressedSize);
                                                            if (fseek(bigFile, savedFilePos, SEEK_SET) != 0)
                                                                break;
                                                        }
                                                        else
                                                            break;
                                                    }
                                                    else
                                                        break;
                                                }
                                                else
                                                    break;
                                            }
                                            else
                                                break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                fclose(bigFile);
            }
        }
    }
    // process data/assets folder

    // dynamic assets
    RegisterAsset("t13__", 0); // banners
}

int MyCheck(char const *a, char const *b, size_t c) {
    Error(a);
    //SafeLog::Write(Utils::Format(L"%s", Utils::AtoW(a)));
    return strncmp(a, b, c);
}

void Install3dPatches_FM13() {

    //patch::RedirectCall(GfxCoreAddress(0x409919), MyCheck);

    static Float gf0 = 0.0f;
    static Float gf1 = 1.0f;
    static Float gf2 = 2.0f;
    static Float gf3 = 3.0f;

   // patch::SetPointer(GfxCoreAddress(0x54CFE8), MyRenderFifaPlayer);
   // patch::SetPointer(GfxCoreAddress(0x5495DC), MyGeneratePlayerPortrait);

    // set 32-bit render mode for EAGL PC Rertrival
    if (!Settings::GetInstance().getNoHardwareAccelerationFix())
        patch::SetUInt(GfxCoreAddress(0x2D63C5 + 1), 32);

  //  patch::SetPointer(GfxCoreAddress(0x5504A8), OnFileIoSetupPaths);

    //patch::RedirectCall(GfxCoreAddress(0x2775F0 + 0x1C), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x277670 + 0x31), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x27CDC0 + 0x362), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x2829D0 + 0x46), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x282D20 + 0x31), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x283780 + 0xCF), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x284150 + 0x127), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x284870 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x2848B0 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x2848F0 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x284930 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x284970 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x2849B0 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x2849F0 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x284A30 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x284A70 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x284AB0 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x284AF0 + 0x1D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3C29B0 + 0x25), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3CB880 + 0x91), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3CBB20 + 0x27), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3CBB20 + 0xE2), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3CBC80 + 0x27), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3CBC80 + 0xE2), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3D50B0 + 0x8D), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3D6460 + 0x29), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3D66C0 + 0x2B), OnCreateFileIO);
    //patch::RedirectCall(GfxCoreAddress(0x3D9000 + 21), OnCreateFileIO);
    //
    //patch::RedirectCall(GfxCoreAddress(0x40866C), OnGetResourceFilename_0);
    //patch::RedirectCall(GfxCoreAddress(0x40867D), OnGetResourceFilename_1);
    //patch::RedirectCall(GfxCoreAddress(0x408694), OnGetResourceFilename_2);
    //patch::RedirectCall(GfxCoreAddress(0x4086B1), OnGetResourceFilename_3);
    //patch::RedirectCall(GfxCoreAddress(0x4086D4), OnGetResourceFilename_4);
    //patch::RedirectCall(GfxCoreAddress(0x4086FD), OnGetResourceFilename_5);
    //patch::RedirectCall(GfxCoreAddress(0x1EB910 + 0x39), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x1F0C20 + 0x347), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x1F0C20 + 0x365), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x1F0C20 + 0x383), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x1F0C20 + 0x3D2), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x1F0C20 + 0x3F0), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202430 + 0x2C), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202430 + 0x56), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202430 + 0x7F), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202430 + 0xA8), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202430 + 0xD4), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202430 + 0xFD), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202430 + 0x126), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202580 + 0x31), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202580 + 0x5B), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202580 + 0x84), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202580 + 0xAD), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202580 + 0xD9), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202580 + 0x102), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x202580 + 0x12B), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x20C510 + 0x50), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x20C510 + 0xAA), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x20C510 + 0xD3), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x20C510 + 0xFC), AsynchLoaderGlobalTextures_GetTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x39), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x57), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x81), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0xA2), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0xD4), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0xF1), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x10E), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x12E), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x14B), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x199), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x1B6), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x1D3), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x1F3), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x210), AsynchLoaderGlobalTextures_PreLoadTexture);
    //patch::RedirectCall(GfxCoreAddress(0x201D90 + 0x22D), AsynchLoaderGlobalTextures_PreLoadTexture);

    //patch::RedirectJump(GfxCoreAddress(0x4811AF), OnFileNameGet);
    //patch::RedirectCall(GfxCoreAddress(0x44C62C), OnFileExistsCheck);
    //patch::RedirectCall(GfxCoreAddress(0x44C76C), OnFileExistsCheck);
    //patch::RedirectCall(GfxCoreAddress(0x44D870), OnFileExistsCheck);
    //patch::RedirectCall(GfxCoreAddress(0x44DB5B), OnFileExistsCheck);
    //
    //patch::RedirectCall(GfxCoreAddress(0x44C85F), OnFileSizeGet);
    //
    //patch::RedirectCall(GfxCoreAddress(0x44D850), OnFileNameGet);
    //patch::RedirectCall(GfxCoreAddress(0x44D832), OnFileNameGet);
    //patch::RedirectCall(GfxCoreAddress(0x44D832), OnFileNameGet);

    // loadingType == 2 check
    //patch::RedirectCall(GfxCoreAddress(0x44D01D), OnConstructAsyncLoader);
    //patch::Nop(GfxCoreAddress(0x44DB54), 6);
    //patch::Nop(GfxCoreAddress(0x44C621), 6);
    //patch::Nop(GfxCoreAddress(0x44C761), 6);
    //patch::Nop(GfxCoreAddress(0x44D86D), 2);

    // banners
    //for (UInt i = 0; i < 8; i++) {
    //    patch::SetUInt(GfxCoreAddress(0x524198 + i * 24), 256);
    //    patch::SetUInt(GfxCoreAddress(0x524198 + 4 + i * 24), 128);
    //}
    //patch::SetPointer(0x4DAAF8, GetBannersDir);

    patch::RedirectCall(GfxCoreAddress(0x3CFE90), TTF_GetFontLineForFont<0>);
    patch::RedirectCall(GfxCoreAddress(0x3D00BD), TTF_GetFontLineForFont<1>);

    patch::RedirectJump(GfxCoreAddress(0x4789EB), ShowExceptionError_GfxCore);

 //   static Float NewDist = 8.3f;
 //   static Double NewAngleX = -1.0;
 //   static Float NewZ = -110.0f;
 //   static Float NewY = 101.4f;
 //
 //   patch::SetDouble(GfxCoreAddress(0x549610), 90.0f);
 //   patch::SetPointer(GfxCoreAddress(0x378D22 + 2), &NewDist);
 //   patch::SetPointer(GfxCoreAddress(0x378DAC + 2), &NewAngleX);
 //
 //   patch::SetPointer(GfxCoreAddress(0x378D39 + 2), &NewZ);
 //   patch::SetPointer(GfxCoreAddress(0x378D30 + 2), &NewY);
 //
 //   // player head scene - remove kit
 //   patch::SetUChar(GfxCoreAddress(0x379187 + 1), 0);
 //   patch::RedirectCall(GfxCoreAddress(0x379170), LoadPlayerHeadKitTexture);

    patch::SetUInt(GfxCoreAddress(0x20F780 + 4), 4); // 25
    patch::SetUInt(GfxCoreAddress(0x20F794 + 4), 0); // 4
    patch::SetUInt(GfxCoreAddress(0x20F7A8 + 4), 60); // 39
    patch::SetUInt(GfxCoreAddress(0x20F7BC + 4), 120); // 78

    patch::SetUInt(GfxCoreAddress(0x20F7D0 + 4), 64); // 64
    patch::SetUInt(GfxCoreAddress(0x20F7E4 + 4), 0); // 4
    patch::SetUInt(GfxCoreAddress(0x20F7F8 + 4), 60); // 39
    patch::SetUInt(GfxCoreAddress(0x20F80C + 4), 120); // 78

    patch::SetUInt(GfxCoreAddress(0x20F820 + 4), 34); // 44
    patch::SetUInt(GfxCoreAddress(0x20F834 + 4), 0); // 4
    patch::SetUInt(GfxCoreAddress(0x20F848 + 4), 60); // 39
    patch::SetUInt(GfxCoreAddress(0x20F85C + 4), 120); // 78

    // BE Scene
//    patch::SetPointer(GfxCoreAddress(0x1E85E5 + 6), "data/BEscene_ucp.cs");

    // fonts changes

    // disable original height correction for fonts
    patch::Nop(GfxCoreAddress(0x3DC3F3), 1);
    patch::SetUChar(GfxCoreAddress(0x3DC3F3 + 1), 0xE9);
    //
    patch::RedirectCall(GfxCoreAddress(0x3DCFE6), MySetFontDescFromParamsLine);

    patch::RedirectCall(GfxCoreAddress(0x7029), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x7119), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x81D1), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x83FF), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x19647), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x78B9), GetAppDocumentsDir);
    patch::RedirectCall(GfxCoreAddress(0x3B0B9), GetAppDocumentsDir);
    patch::RedirectJump(GfxCoreAddress(0x281AF0), GetAppDocumentsDir);

   // patch::SetChar(GfxCoreAddress(0x39DB17 + 3), 't');
   // patch::SetChar(GfxCoreAddress(0x39DB21 + 3), 'g');
   // patch::SetChar(GfxCoreAddress(0x39DB2B + 3), 'a');
   //
   // patch::SetChar(GfxCoreAddress(0x458FA4 + 3), 't');
   // patch::SetChar(GfxCoreAddress(0x458FA8 + 3), 'g');
   // patch::SetChar(GfxCoreAddress(0x458FAC + 3), 'a');
   //
   // patch::SetPointer(GfxCoreAddress(0x39DAEB + 1), ".dds");
   // patch::SetPointer(GfxCoreAddress(0x458F92 + 1), ".dds");
   // patch::SetPointer(GfxCoreAddress(0x201DFD + 1), "hair_cm_%d_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x202597 + 1), "hair_cm_%d_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x370684 + 1), "hair_cm_-%d_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x201DB5 + 1), "cm_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x202442 + 1), "cm_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x370666 + 1), "cm_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x201E1E + 1), "hair_cmb_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x2025C1 + 1), "hair_cmb_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x370717 + 1), "hair_cmb_-%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x201DD3 + 1), "cmbhair_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x202466 + 1), "cmbhair_%d.dds");
   // patch::SetPointer(GfxCoreAddress(0x3706FF + 1), "cmbhair_%d.dds");

    if (!Settings::GetInstance().getTeamControlDisabledAtGameStart()) {
        patch::RedirectCall(GfxCoreAddress(0x23E629), OnSetupManualPlayerSwitch);
        patch::RedirectCall(GfxCoreAddress(0x165C3), OnCreateInputDevice);
        patch::RedirectCall(GfxCoreAddress(0x18553), OnSetupController);
        // fix null-pointer memory copying
        patch::RedirectCall(GfxCoreAddress(0x2E5B5A), GfxCopyData);
    }
}
