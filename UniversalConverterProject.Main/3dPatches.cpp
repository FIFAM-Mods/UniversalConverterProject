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
#include "d3dx9.h"
#include "FifamReadWrite.h"
#include "Utils.h"

using namespace plugin;

void OnCreateInputDevice() {
    SafeLog::Write(L"Initializing devices");
    CallMethodDynGlobal(GfxCoreAddress(0x309150), patch::GetUInt(GfxCoreAddress(0x6696DC), false));
    CallDynGlobal(GfxCoreAddress(0x308040));
}

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

void *TTF_GetAllocator(void *f) {
    return *raw_ptr<void *>(f, 0x5E38);
}

unsigned int TTF_GetSize(void *f) {
    return *raw_ptr<unsigned int>(f, 0x5E38 + 0x1C);
}

template<int T>
const char *METHOD TTF_GetFontLineForFont(void *f, DUMMY_ARG, const char *fontTag) {
    static char *emptyStr = "";
    const char *result = emptyStr;
    if (f && TTF_GetAllocator(f))
        result = CallMethodAndReturnDynGlobal<const char *>(GfxCoreAddress(0x3CF5B0), f, fontTag);
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

//const WideChar *gBannersDir = L"data\\banners";
//
//void __declspec(naked) GetBannersDir() {
//    __asm {
//        mov   esi, [esp + 0x10]
//        push  gBannersDir
//        push  esi
//        mov   eax, 0x1493F2F
//        call  eax
//        add   esp, 8
//        mov   eax, 0x4DA9F2
//        jmp   eax
//    }
//}

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

Bool32 CachedExists(char const *filename) {
    return exists(filename);
    
    //static Set<StringA> cache;
    //StringA filenameStr = filename;
    //if (cache.contains(filenameStr))
    //    return true;
    //Bool32 exists = CallAndReturnDynGlobal<Bool32>(GfxCoreAddress(0x19E60), filename);
    //if (exists != 0)
    //    cache.insert(filenameStr);
    //return exists;
}

UInt CachedSize(char const *filename) {
    return UInt(file_size(filename));

    //static Map<StringA, UInt> cache;
    //StringA filenameStr = filename;
    //auto it = cache.find(filenameStr);
    //if (it != cache.end())
    //    return (*it).second;
    //UInt size = CallAndReturnDynGlobal<UInt>(GfxCoreAddress(0x19E70), filename);
    //cache[filenameStr] = size;
    //return size;
}

char const *GetFilePathForAsynchLoader(char const *srcFilePath, Bool32 *exists, UInt *size) {
    if (strlen(srcFilePath) > 1 && srcFilePath[1] == ':')
        return srcFilePath;
    //if (CallAndReturnDynGlobal<Bool32>(GfxCoreAddress(0x19E60), srcFilePath)) {
    //    if (exists)
    //        *exists = true;
    //    if (size)
    //        *size = CallAndReturnDynGlobal<UInt>(GfxCoreAddress(0x19E70), srcFilePath);
    //    return srcFilePath;
    //}
    static std::string zdataPath = "data\\assets\\";
    std::string testPath = zdataPath + srcFilePath;
    static char staticFilename[1024];
    strcpy(staticFilename, testPath.c_str());
    if (CachedExists(staticFilename)) {
        if (exists)
            *exists = true;
        if (size)
            *size = CachedSize(staticFilename);
        return staticFilename;
    }
    return srcFilePath;
}

void OnFileNameGet(char *filepath) {
    Bool32 exists = false;
    char const *newPath = GetFilePathForAsynchLoader(filepath, &exists, nullptr);
    if (exists)
        strcpy(filepath, newPath);
    _strlwr(filepath);
}

void METHOD OnFileIoSetupPaths(void *fileIO, DUMMY_ARG, const wchar_t **paths, unsigned int numPaths) {
    Vector<const wchar_t *> myPaths;
    if (numPaths > 7)
        numPaths = 7;
    for (unsigned int i = 0; i < numPaths; i++)
        myPaths.push_back(paths[i]);
    myPaths.push_back(L"data\\assets\\");
    CallMethodDynGlobal(GfxCoreAddress(0x3C4CB0), fileIO, myPaths.data(), myPaths.size());
}

StringA modelName;

void OnRenderModel(void *a, void *b) {
    void *model = (void *)(unsigned int(a) - 0x4C);
    modelName = *(char **)(unsigned int(model) + 0xB4);
    CallDynGlobal(GfxCoreAddress(0x2E2A00), a, b);
}

void METHOD OnExecuteRenderMethod(void *rm, DUMMY_ARG, void *descriptor) {
    if (!rm) {
        Error("Empty renderMethod in %s", modelName.c_str());
    }
    else
        CallMethodDynGlobal(GfxCoreAddress(0x2E43C0), rm, descriptor);
}

void *OnAllocateMemory(const char *name, size_t size, size_t alignment) {
    auto mem = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x30D130), name, size, alignment);
    if (!mem)
        Error("Unable to allocate memory for %s, size %d", name, size);
    SafeLog::WriteToFile("memory.csv", Utils::Format(L"ALLOC,%X,%s,%d,%d", mem, Utils::AtoW(name), size, alignment));
    return mem;
}

void OnDeleteMemory(void *mem) {
    CallDynGlobal<void *>(GfxCoreAddress(0x30D230), mem);
    SafeLog::WriteToFile("memory.csv", Utils::Format(L"DELETE,%X,,0,0", mem));
}

int METHOD OnLoadToPool(void *t, DUMMY_ARG, const char *memPoolName, const char *fileName, int flags, int alignment, void *callback, void *callbackData, int arg_18) {
    Error("%s %s", memPoolName, fileName);
    return CallMethodAndReturnDynGlobal<int>(GfxCoreAddress(0x44D7F0), t, memPoolName, fileName, flags, alignment, callback, callbackData, arg_18);
}

int METHOD OnLoadToMem(void *t, DUMMY_ARG, const char *fileName, void *dst, int dstSize, int flags, void *callback, void *callbackData) {
    Error("%s %d", fileName, dstSize);
    return CallMethodAndReturnDynGlobal<int>(GfxCoreAddress(0x44DAD0), t, fileName, dst, dstSize, flags, callback, callbackData);
}

void * OnAllocate(const char *name, int size, int alignment) {
    Error("%s %s %s", name, size, alignment);
    return CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x30D130), name, size, alignment);
}

void OnGenerateCrowdTex() {
    //CallDynGlobal(GfxCoreAddress(0x457D20));
    auto se = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x4100B0));
    auto c = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x40EBC0), se);
    for (unsigned int i = 0; i < 13; i++) {
        void *crowdTex = *raw_ptr<void *>(c, i * 44);
        if (crowdTex) {
            void *texObj = *raw_ptr<void *>(crowdTex, 36);
            IDirect3DTexture9 *tex = *raw_ptr<IDirect3DTexture9 *>(texObj, 24);
            D3DXSaveTextureToFile(Utils::Format("crowd_%d.png", i).c_str(), D3DXIFF_PNG, tex, NULL);
        }
    }
}

Bool METHOD OnCheckOption(void *t, DUMMY_ARG, const Char *name) {
    return gfxGetVarInt(name, 1) == 1;
}

Bool MySetAALevel(void *frameBuffer, UInt level) {
    Bool set = false;
    if (level == 0)
        set = true;
    else {
        set = CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x2D4EB0), frameBuffer, 0, 
            patch::GetUInt(GfxCoreAddress(0x661734)), patch::GetUInt(GfxCoreAddress(0x661738)), patch::GetUInt(GfxCoreAddress(0x661740)),
            level, patch::GetUInt(GfxCoreAddress(0x66173C)) != 0);
    }
    if (set) {
        CallMethodDynGlobal(GfxCoreAddress(0x2D3CD0), frameBuffer, level);
        //Error(L"Enabled AA level: %d", level);
    }
    return set;
}

Bool METHOD OnSetAALevel(void *t, DUMMY_ARG, UInt level) {
    if (!MySetAALevel(t, 4)) {
        if (!MySetAALevel(t, 2))
            MySetAALevel(t, 0);
    }
    return true;
}

struct PlayerCommentaryEntry {
    String firstName, lastName, commonName;
    UInt fifaId = 0;
};

Map<Int, PlayerCommentaryEntry> GetPlayerCommentaryEntries() {
    static Map<Int, PlayerCommentaryEntry> playerCommentaryEntries;
    return playerCommentaryEntries;
}

void ReadPlayerCommentaryConfig() {
    wchar_t commentsLanguageStr[MAX_PATH];
    GetPrivateProfileStringW(L"OPTIONS", L"TEXT_LANGUAGE_OVERRIDE", L"eng", commentsLanguageStr, MAX_PATH, L".\\locale.ini");
    FifamReader reader(L"plugins\\ucp\\commentary_" + String(commentsLanguageStr), 14);
    if (reader.Available()) {
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                PlayerCommentaryEntry entry;
                Int id = 0;
                reader.ReadLine(id, entry.firstName, entry.lastName, entry.commonName, entry.fifaId);
                if (id != 0)
                    GetPlayerCommentaryEntries()[id] = entry;
            }
            else
                reader.SkipLine();
        }
    }
}

Int GetCommentaryID(String const &firstName, String const &lastName, String const &commonName, UInt fifaId) {
    for (auto const &[id, entry] : GetPlayerCommentaryEntries()) {
        if (entry.firstName.empty() || entry.firstName == firstName) {
            if (entry.lastName.empty() || entry.lastName == firstName) {
                if (entry.commonName.empty() || entry.commonName == firstName) {
                    if (entry.fifaId == 0 || entry.fifaId == fifaId) {
                        return id;
                    }
                }
            }
        }
    }
    return 0;
}

String SystemStringGet(void *s) {
    String result;
    if (*raw_ptr<Char *>(s, 0x48))
        result = Utils::AtoW(*raw_ptr<Char *>(s, 0x48));
    return result;
}

Int * METHOD GetPlayerCommentaryID(void *t, DUMMY_ARG, UInt playerIndex) {
    static Int result[2] = { 0, 0 };
    void *player = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x95020), t, playerIndex);
    if (player) {
        UInt fifaId = *raw_ptr<UInt>(player, 0x4);
        if (fifaId != 0)
            result[1] = fifaId;
        else {
            String firstName = SystemStringGet(raw_ptr<void *>(player, 0x8));
            String lastName = SystemStringGet(raw_ptr<void *>(player, 0x54));
            String commonName = SystemStringGet(raw_ptr<void *>(player, 0xEC));
            UInt fifaId = *raw_ptr<UInt>(player, 0x4);
            //Error(L"%s, %s, %s, %d", firstName.c_str(), lastName.c_str(), commonName.c_str(), fifaId);
            Int id = GetCommentaryID(firstName, lastName, commonName, fifaId);
            if (id != 0)
                result[1] = id;
        }
    }
    return result;
}

void METHOD OnSetupLightingData(float *data) {

    CallMethodDynGlobal(GfxCoreAddress(0x1EE3F0), data);

    static unsigned int r = 0x93;
    static unsigned int g = 0xd0;
    static unsigned int b = 0x12;
    static float i = 1.0f;

    if (KeyPressed(VK_TAB)) {
        r = 0x93;
        g = 0xd0;
        b = 0x12;
        i = 1.0f;
    }

    bool plus = KeyPressed(VK_ADD);
    bool minus = !plus && KeyPressed(VK_SUBTRACT);

    if (plus || minus) {
        if (KeyPressed(VK_NUMPAD1)) {
            if (plus) {
                r += 1;
                if (r > 255)
                    r = 255;
            }
            else {
                r -= 1;
                if (r == 0)
                    r = 1;
            }
        }
        if (KeyPressed(VK_NUMPAD2)) {
            if (plus) {
                g += 1;
                if (g > 255)
                    g = 255;
            }
            else {
                g -= 1;
                if (g == 0)
                    g = 1;
            }
        }
        if (KeyPressed(VK_NUMPAD3)) {
            if (plus) {
                b += 1;
                if (b > 255)
                    b = 255;
            }
            else {
                b -= 1;
                if (b == 0)
                    b = 1;
            }
        }
        if (KeyPressed(VK_NUMPAD0)) {
            if (plus) {
                i += 0.01f;
                if (i > 2.0f)
                    i = 2.0f;
            }
            else {
                i -= 0.01f;
                if (i <= 0.0f)
                    i = 0.0f;
            }
        }
        FILE *f = fopen("colors.txt", "wt");
        fprintf(f, "%d %d %d - %g", r, g, b, i);
        fclose(f);

        RGBAReal *clr1 = raw_ptr<RGBAReal>(data, 0x14);
        RGBAReal *clr2 = raw_ptr<RGBAReal>(data, 0x24);
        RGBAReal *brt = raw_ptr<RGBAReal>(data, 0x44);

        clr1->SetRGB(r, g, b);
        clr2->SetRGB(r, g, b);
        brt->Set(i, i, i, brt->a);
    }
}

template<UInt DeviceAddr, UInt OriginalAddr>
Bool METHOD OnSetFifaTexture(void *t, DUMMY_ARG, int samplerIndex) {
    IDirect3DDevice9 *device = *(IDirect3DDevice9 **)GfxCoreAddress(DeviceAddr);
    device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    return CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(OriginalAddr), t, samplerIndex);
}

Vector<Tuple<int, D3DSAMPLERSTATETYPE, DWORD>> fifaTexSamplersToRestore;

void OnSetFifaTexture2(void *t, int samplerIndex) {

    IDirect3DDevice9 *device = *(IDirect3DDevice9 **)GfxCoreAddress(0xDB21D0);
    //static DWORD oldValue = 0;

    //device->GetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, &oldValue);
    //fifaTexSamplersToRestore.emplace_back(samplerIndex, D3DSAMP_ADDRESSU, oldValue);
    device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);

    //device->GetSamplerState(samplerIndex, D3DSAMP_ADDRESSV, &oldValue);
    //fifaTexSamplersToRestore.emplace_back(samplerIndex, D3DSAMP_ADDRESSV, oldValue);
    device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    CallDynGlobal(GfxCoreAddress(0x458DE0), t, samplerIndex);
}

void METHOD OnRenderFifaPlayerInMatch(void *t, DUMMY_ARG, void *m) {
    //CallMethodDynGlobal(GfxCoreAddress(0x2E2230), t, m);
    IDirect3DDevice9 *device = *(IDirect3DDevice9 **)GfxCoreAddress(0xDB21D0);
    for (auto const &e : fifaTexSamplersToRestore)
        device->SetSamplerState(get<0>(e), get<1>(e), get<2>(e));
    fifaTexSamplersToRestore.clear();
}

void OnRenderRenderSlots(void *s) {
    IDirect3DDevice9 *device = *(IDirect3DDevice9 **)GfxCoreAddress(0xDB21D0);
    for (UInt i = 0; i < 8; i++) {
        device->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        device->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    CallDynGlobal(GfxCoreAddress(0x408B60), s);
}

wchar_t const *gSkyDir = L"data\\stadium\\generator\\sky";

void __declspec(naked) OnSkyModel() {
    static unsigned int gRetAddr = 0x10421575;
    __asm {
        cmp[ebp + 0x28], 3
        je Night_ID
        movzx edx, byte ptr ss : [ebp + 0x1A31] // Weather ID
        je Weather_ID
    Night_ID:
        mov edx, 5
    Weather_ID:
        PUSH EDX
        MOV EAX, gSkyDir
        mov ecx, gRetAddr
        push ecx
        retn
    }
}

unsigned int gCurrentPlayerLoadPlayerId1 = 0;
unsigned int gCurrentPlayerLoadPlayerId2 = 0;
unsigned int gCurrentPlayerLoadPlayerId3 = 0;
unsigned int gCurrentPlayerLoadPlayerId4 = 0;

void METHOD OnLoadFifaPlayer1(void *player, DUMMY_ARG, void *playerInfo, void *resource, bool loadBody, void *resMan) {
    gCurrentPlayerLoadPlayerId1 = *raw_ptr<unsigned int>(playerInfo, 4);
    CallMethodDynGlobal(GfxCoreAddress(0x39142B), player, playerInfo, resource, loadBody, resMan);
    gCurrentPlayerLoadPlayerId1 = 0;
}

int METHOD OnGetPlayerEffTexture1(void *t, DUMMY_ARG, char const *name, bool cached, bool cube) {
    if (gCurrentPlayerLoadPlayerId1 != 0) {
        auto customEffTexPath = "data\\assets\\eff_" + Utils::Format("%d", gCurrentPlayerLoadPlayerId1) + ".dds";
        if (exists(customEffTexPath))
            return CallMethodAndReturnDynGlobal<int>(GfxCoreAddress(0x39DEFA), t, customEffTexPath.c_str(), false, cube);
    }
    return CallMethodAndReturnDynGlobal<int>(GfxCoreAddress(0x39DEFA), t, name, cached, cube);
}

void OnLoadFifaPlayer2(void *playerInfo, unsigned int playerId) {
    gCurrentPlayerLoadPlayerId2 = playerId;
    CallDynGlobal(GfxCoreAddress(0x202430), playerInfo, playerId);
    gCurrentPlayerLoadPlayerId2 = 0;
}

int OnGetPlayerEffTexture2(char const *name) {
    if (gCurrentPlayerLoadPlayerId2 != 0) {
        auto customEffTexPath = "data\\assets\\eff_" + Utils::Format("%d", gCurrentPlayerLoadPlayerId2) + ".dds";
        if (exists(customEffTexPath))
            return CallAndReturnDynGlobal<int>(GfxCoreAddress(0x459040), customEffTexPath.c_str());
    }
    return CallAndReturnDynGlobal<int>(GfxCoreAddress(0x459040), name);
}

void OnPreLoadFifaPlayerTex(char const *name, int a, bool b) {
    bool loadDefaultEff = false;
    for (int i = 0; i < 26; i++) {
        void *playerDesc = (void *)(GfxCoreAddress(0xABEDE0 + i * 4832));
        unsigned int fifaId = *raw_ptr<unsigned int>(playerDesc, 0x1044);
        unsigned int useGenHead = *raw_ptr<unsigned int>(playerDesc, 0x1058);
        if (!useGenHead && fifaId < 500'000) {
            auto customEffTexPath = "data\\assets\\eff_" + Utils::Format("%d", fifaId) + ".dds";
            if (exists(customEffTexPath))
                CallDynGlobal(GfxCoreAddress(0x4590F0), customEffTexPath.c_str(), a, b);
            else
                loadDefaultEff = true;
        }
        else
            loadDefaultEff = true;
    }
    if (loadDefaultEff)
        CallDynGlobal(GfxCoreAddress(0x4590F0), name, a, b);
}

char const *EVENT_NAMES[] = {
"INPLAY",
"FOUL",
"OFFSIDE",
"ENDHALF_AUDIO_WHISTLE",
"ENDHALF",
"SUBSTARTMASTER",
"SUBSETUP",
"SUBEXCHANGE",
"SUBENDMASTER",
"RESTARTINIT",
"TARGETKICKER",
"TARGETCURSOR",
"SETPLAY_TARGETPLAYERSELECTED",
"SETPLAY_TARGETRECEIVINGBALL",
"SETPLAY_NONTARGETRECEIVINGBALL",
"INJURY",
"USERINTERRUPT",
"SHOOTOUTNOGOAL",
"SHOOTOUTSCOREWIN",
"SHOOTOUTMISSWIN",
"SHOOTOUT_SETUP",
"SHOTMISSED",
"SCRIPT_READY",
"SCRIPT_ENDING",
"SCRIPT_ENDED",
"SCRIPT_PURGE_READY",
"SCRIPT_CAMERA_SWITCH",
"SCRIPT_EFFECT",
"COLLISION",
"STARTMATCH",
"STARTHALF",
"GAINPOSSESSION",
"LOSSPOSSESSION",
"CHOREO_NEARMISS",
"CHOREO_GOALIETIPOUT",
"CHOREO_CORNER",
"CHOREO_HOME_CORNER",
"CHOREO_AWAY_CORNER",
"CHOREO_GOALKICK",
"CHOREO_THROWIN",
"CHOREO_SCOREGOAL",
"CHOREO_SHOOTOUTGOAL",
"CHOREO_SHOOTOUTMISS",
"CHOREO_CHECKCORNER",
"CHOREO_TICKER_START",
"CHOREO_TICKER_STOP",
"CHOREO_PLAYER_EJECTED",
"OOP_SCRIPT_RUNNING",
"OOP_SCRIPT_OVER",
"REFCALL_ENDHALF",
"REFCALL_PLAY",
"REFCALL_SHOOTOUTPLAY",
"REFCALL_GOALKICK",
"REFCALL_CORNER",
"REFCALL_THROWIN",
"REFCALL_GOAL",
"REFCALL_CHECK_OFFSIDE",
"REFCALL_CALL_OFFSIDE",
"LINESMAN_SIGNAL_OFFSIDE",
"REFCALL_EVALUATE_FOUL",
"REFCALL_FOUL",
"BALLOUT",
"BALLOUT_THROWIN",
"BALLOUT_CORNER",
"BALLOUT_GOALKICK",
"BALLHITPOST",
"BALLHITCROSSBAR",
"BALLHITWALL",
"BALLBOUNCE",
"BALLHITNET",
"BALLHITNETSFX",
"BALLSPLASH",
"ONETIMER",
"JUGGLE",
"JUGGLESHOT",
"JUGGLEPASS",
"JUGGLING",
"JUGGLETOUCH",
"BALLHITPLAYER",
"HEADED_ON",
"HEADED_BACK",
"HEADED_SIDE",
"2PLAYERHEADER",
"CHESTTRAP",
"TRAPBALL",
"DEKE",
"KICK",
"KICK_CLEARANCE",
"SAVE",
"SAVE_ANIM",
"GKSAVE_MISS",
"GOALIEDROPKICK",
"KICKOFF",
"PENALTYREADY",
"PENALTYKICKTAKEN",
"TURNOVER",
"ALLOWCAMSWITCH",
"DELAYCAMSWITCH",
"FREEKICKBALLID",
"THROWINREADY",
"THROWINTAKEN",
"THROWINSTART",
"FREEKICKSTARTING",
"FREEKICKSTART",
"FREEKICKTAKEN",
"CORNERSTART",
"CORNERTAKEN",
"GOALKICKSTARTING",
"GOALKICKTAKEN",
"FREEKICK_TAKE_TIME",
"STOPPAGETIME",
"RESTARTWHISTLE",
"BREAKAWAY",
"HOME_BREAKAWAY",
"AWAY_BREAKAWAY",
"USERRUN",
"FOULCALLED",
"DEFERFOUL",
"NOFOUL",
"NOFOULCALLED",
"BADREFCALL",
"STARTTACKLE",
"TACKLETYPE",
"TACKLESEVERITY",
"TACKLEBEHIND",
"TACKLERESULT",
"PLAYERFALL",
"PLAYERSTUMBLE",
"AIRCOLLISION",
"TACKLESFX",
"GOALSCORED",
"HOME_GOALSCORED",
"AWAY_GOALSCORED",
"ILLEGALGOAL",
"SUBSTITUTION",
"BALLHITKEEPER",
"SHOTBLOCKED",
"KICKBLOCKED",
"FORMINGWALL",
"KNOCKEDOVERBYBALL",
"FIRSTTOUCH",
"START_PASS",
"RECEIVE_PASS",
"REPLAY_START",
"REPLAY_END",
"SHOW_CARD",
"DISCONNECTED",
"OUT_OF_SYNC",
"OOP_CALLED",
"OOP_CROWD",
"AUTO_REPLAY",
"AUTO_REPLAY_END",
"FRIENDLY_TIE_MENU",
"SCRIPT_SKIPPED",
"GENERIC",
"ALLOW_ALL_BUTTONS_INTERRUPT",
"SCREEN_TRANSITION",
"SCRIPT_AUDIO_LOAD_READY",
"SCRIPT_MOTION_LOAD_READY",
"CAMERA_REINITIALIZED",
"COACH_MENU_START",
"FOULDELAY",
"CONTROLLER_CHANGED",
"MOMENTZOOM_CAMERAZOOMSTART",
"REPLAY_SPEECH_GOAL",
"REPLAY_SPEECH_OFFSIDE",
"REPLAY_SPEECH_MISS",
"REPLAY_SPEECH_SAVE",
"MOMENTZOOM_MISS_PLAYER",
"MOMENTZOOM_GOAL_PLAYER",
"MOMENTZOOM_FOUL_PLAYER",
"MOMENTZOOM_MISS_PLAYER_COMING",
"MOMENTZOOM_GOAL_PLAYER_COMING",
"MOMENTZOOM_MISS_PLAYER_ABORTED",
"MOMENTZOOM_GOAL_PLAYER_ABORTED",
"MOMENTZOOM_PLAYER_READY",
"CAMERA_SCHEMECHANGED",
"SETCAMERA_STARTPENALTYKICK",
"SETCAMERA_STARTFREEKICK",
"SETCAMERA_FREEKICK_WAITFORCONTACT",
"STARTSNIPERMODE",
"ENDSNIPERMODE",
"AUDIO_COLOUR_SPEECH",
"SPECIAL_EFFECT",
"TELESTRATOR_START",
"HIGHLIGHTS_PLAYING",
"PAUSE_MENU_START",
"PAUSE_MENU_END",
"SCRIPT_TAG",
"PAUSEMENU_ENDHIGHLIGHTS",
"PAUSEMENU_STARTHIGHLIGHTS",
"POSTGAME_STARTHIGHLIGHTS",
"AUDIO_BEGIN_DUCKER",
"AUDIO_END_DUCKER",
"MENUSFX",
"HOME_WIN",
"AWAY_WIN",
"DEBUG_MOMENTZOOM_MISS",
"DEBUG_MOMENTZOOM_GOAL",
"POPUP_STARTED",
"POPUP_CLOSED",
"SPLASHSCREEN_STARTED",
"MOMENT_START",
"MOMENT_END",
"GOALIE_FUMBLE_BALL",
"MANAGER_SENT_OFF_HOME",
"MANAGER_SENT_OFF_AWAY"
};

int OnPostEvent(unsigned int eventId, unsigned char *data) {
    //if (eventId < std::size(EVENT_NAMES)) {
    //    if (eventId == 4) {
    //        patch::SetInt(GfxCoreAddress(0xADDAE0), 1);
    //        CallDynGlobal(GfxCoreAddress(0x1E79D0));
    //    }
    //    String result = Utils::AtoW(EVENT_NAMES[eventId]);
    //    SafeLog::WriteToFile("Events.txt", result);
    //}
    return CallAndReturnDynGlobal<int>(GfxCoreAddress(0x9C9A0), eventId, data);
}

struct ChoreoTask {
    void *callback;
    unsigned int arg1;
    unsigned int arg2;
    unsigned int arg3;
    char name[40];
};

struct ChoreoEvent {
    ChoreoTask *tasks;
    unsigned int numTasks;
    char name[36];
};

void DumpChoreoEvents() {
    FILE *f = fopen("ChoreoEvents.h", "wt");
    ChoreoEvent *events = (ChoreoEvent *)GfxCoreAddress(0x5F6F08);
    fputs("enum eChoreEvents {\n", f);
    for (int i = 0; i < 84; i++) {
        fprintf(f, "    CHOREOEVENT_%s = %d", events[i].name, i);
        if (i != 83)
            fputs(",", f);
        fputs("\n", f);
    }
    fputs("};\n\n", f);
    for (int i = 0; i < 84; i++) {
        fprintf(f, "%s\n", events[i].name);
        for (unsigned int t = 0; t < events[i].numTasks; t++)
            fprintf(f, "    %s(%d, %d, %d)\n", events[i].tasks[t].name, events[i].tasks[t].arg1, events[i].tasks[t].arg2, events[i].tasks[t].arg3);
        fputs("\n", f);
    }
    fclose(f);
}

unsigned int FshNewHash(char const *str) {
    unsigned int hash = 0;
    while (*str) {
        hash += *str;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        str++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

void OnAllocRAM(size_t memSize, size_t a2) {
    Error("RAM: %X", memSize);
    CallDynGlobal(GfxCoreAddress(0x1E00C0), 0x1C000000, a2);
}

void __cdecl LoadFshFifaM(const char *poolName, int numShapes, void *shapeStore, unsigned char bReload, int maxTextures, int *outNumTextures, void *a7, int a8) {
    String str = Format(L"  LoadFshFifaM: %s Reload: %d NumFsh: %d FshDatas:", Utils::AtoW(poolName).c_str(), bReload, numShapes);
    for (Int i = 0; i < numShapes; i++) {
        void *texData = *raw_ptr<void *>(shapeStore, i * 4);
        char texName[5];
        texName[0] = 0;
        texName[4] = 0;
        if (texData)
            strncpy(texName, (char const *)texData, 4);
        str += Format(L" %08X (%s)", texData, Utils::AtoW(texName).c_str());
    }
    SafeLog::WriteToFile("fsh.txt", str);
    CallDynGlobal(GfxCoreAddress(0x458180), poolName, numShapes, shapeStore, bReload, maxTextures, outNumTextures, a7, a8);
}

void FshLoadCB(unsigned __int8 *data, int dataSize, int requestId, void *userData) {
    void *texBlock = *raw_ptr<void *>(userData, 4);
    const char *texBlockName = *raw_ptr<const char *>(texBlock, 0x10);
    void *tex = (void *)(*raw_ptr<UInt>(texBlock, 4) + 12 * *raw_ptr<UInt>(userData, 0));
    const char *name = *raw_ptr<const char *>(tex, 0);
    String str = Format(L"FshLoadCB: %s TexBlock: %s %08X Size: %d RequestId: %d ", Utils::AtoW(name).c_str(), Utils::AtoW(texBlockName).c_str(), data, dataSize, requestId);
    SafeLog::WriteToFile("fsh.txt", str);
    CallDynGlobal(GfxCoreAddress(0x4093B0), data, dataSize, requestId, userData);
}

void MyError(Int, char const *format, char const *message) {
    Error(message);
}

bool METHOD OnMotionMgrStartScript(void *m, DUMMY_ARG, int a) {
    bool result = CallMethodAndReturnDynGlobal<bool>(GfxCoreAddress(0x222710), m, a);
    if (result)
        Error("StartScript: %s", CallMethodAndReturnDynGlobal<char const *>(GfxCoreAddress(0x21F3C0), m));
    return result;
}

BOOL OnAiPlayerProcessControl(void *player, int a2, int a3) {
    return false;
}

BOOL OnAiPlayerProcessControl2(void *player, int a2) {
    return false;
}

int OnAiPlayerPerformAction(void *player, void *actions, int a3, char a4) {
    return 0;
}

struct CamSettings {
    Bool TCM_CAM_ENABLED = false;
    Float TCM_CAM_HEIGHT = 45.7f;
    Float TCM_CAM_DEPTH_DIST = 111.0f;
    Float TCM_CAM_LENSE_ANGLE = 14.4f;
    Float TCM_CAM_HORIZONTAL_FOLLOWING = 1.0f;
    Float TCM_CAM_HORIZONTAL_BOUNDS = 0.77f;
    Float TCM_CAM_DEPTH_BOUNDS = 3.5f;
    Float TCM_CAM_NEAR_CLIP = 86.0f;
    Float TCM_CAM_TARGET_HORIZONTAL_BOUNDS = 0.77f;
    Float TCM_CAM_TARGET_DEPTH_BOUNDS = 0.37f;
};

CamSettings &GetCamSettings(size_t camId) {
    static CamSettings camSettings[10];
    if (camId >= 1 && camId <= 10)
        return camSettings[camId - 1];
    return camSettings[0];
}

Int GetCameraSchemeIds(Int &outMatchCameraId) {
    static Int camSchemeToId[] = { -1, 2, 4, 1, -1, 3, -1, 5, -1, 7, 6, 8, 9, 10, -1, -1 };
    Int scheme = CallAndReturnDynGlobal<Int>(GfxCoreAddress(0x40350));
    outMatchCameraId = -1;
    if (scheme >= 0 && scheme < std::size(camSchemeToId) && camSchemeToId[scheme] != -1 && GetCamSettings(camSchemeToId[scheme]).TCM_CAM_ENABLED)
        outMatchCameraId = camSchemeToId[scheme];
    return scheme;
}

template<Int resultId>
Int GetCameraSchemeId() {
    static Int camSchemeToId[] = { -1, 2, 4, 1, -1, 3, -1, 5, -1, 7, 6, 8, 9, 10, -1, -1 };
    Int matchCameraId = -1;
    Int scheme = GetCameraSchemeIds(matchCameraId);
    if (matchCameraId != -1)
        return resultId;
    return scheme;
}

void METHOD GetCameraParameters(void *tcmCam, DUMMY_ARG, Int a2, Int a3, Int a4, float *a5, float *a6, float *a7, float *a8, float *a9, float *a10, float *a11) {
    Int matchCameraId = -1;
    Int scheme = GetCameraSchemeIds(matchCameraId);
    Float saved_TCM_CAM_HEIGHT = 0.0f;
    Float saved_TCM_CAM_DEPTH_DIST = 0.0f;
    Float saved_TCM_CAM_LENSE_ANGLE = 0.0f;
    Float saved_TCM_CAM_HORIZONTAL_FOLLOWING = 0.0f;
    Float saved_TCM_CAM_HORIZONTAL_BOUNDS = 0.0f;
    Float saved_TCM_CAM_DEPTH_BOUNDS = 0.0f;
    Float saved_TCM_CAM_TARGET_HORIZONTAL_BOUNDS = 0.0f;
    Float saved_TCM_CAM_TARGET_DEPTH_BOUNDS = 0.0f;
    Float saved_TCM_CAM_NEAR_CLIP = 0.0f;
    if (matchCameraId != -1) {
        auto &camSettings = GetCamSettings(matchCameraId);
        saved_TCM_CAM_HEIGHT = *raw_ptr<Float>(tcmCam, 0x08);
        saved_TCM_CAM_DEPTH_DIST = *raw_ptr<Float>(tcmCam, 0x0C);
        saved_TCM_CAM_LENSE_ANGLE = *raw_ptr<Float>(tcmCam, 0x10);
        saved_TCM_CAM_HORIZONTAL_FOLLOWING = *raw_ptr<Float>(tcmCam, 0x14);
        saved_TCM_CAM_HORIZONTAL_BOUNDS = *raw_ptr<Float>(tcmCam, 0x18);
        saved_TCM_CAM_DEPTH_BOUNDS = *raw_ptr<Float>(tcmCam, 0x1C);
        saved_TCM_CAM_TARGET_HORIZONTAL_BOUNDS = *raw_ptr<Float>(tcmCam, 0x20);
        saved_TCM_CAM_TARGET_DEPTH_BOUNDS = *raw_ptr<Float>(tcmCam, 0x24);
        saved_TCM_CAM_NEAR_CLIP = *raw_ptr<Float>(tcmCam, 0x28);
        *raw_ptr<Float>(tcmCam, 0x08) = camSettings.TCM_CAM_HEIGHT * 52.0f;
        *raw_ptr<Float>(tcmCam, 0x0C) = camSettings.TCM_CAM_DEPTH_DIST * 52.0f;
        *raw_ptr<Float>(tcmCam, 0x10) = camSettings.TCM_CAM_LENSE_ANGLE / 180.0f * 3.141499996185303f;
        *raw_ptr<Float>(tcmCam, 0x14) = camSettings.TCM_CAM_HORIZONTAL_FOLLOWING;
        *raw_ptr<Float>(tcmCam, 0x18) = camSettings.TCM_CAM_HORIZONTAL_BOUNDS;
        *raw_ptr<Float>(tcmCam, 0x1C) = camSettings.TCM_CAM_DEPTH_BOUNDS;
        *raw_ptr<Float>(tcmCam, 0x20) = camSettings.TCM_CAM_TARGET_HORIZONTAL_BOUNDS;
        *raw_ptr<Float>(tcmCam, 0x24) = camSettings.TCM_CAM_TARGET_DEPTH_BOUNDS;
        *raw_ptr<Float>(tcmCam, 0x28) = camSettings.TCM_CAM_NEAR_CLIP * 52.0f;
    }
    CallMethodDynGlobal(GfxCoreAddress(0x24BD50), tcmCam, 1, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    if (matchCameraId != -1) {
        *raw_ptr<Float>(tcmCam, 0x08) = saved_TCM_CAM_HEIGHT;
        *raw_ptr<Float>(tcmCam, 0x0C) = saved_TCM_CAM_DEPTH_DIST;
        *raw_ptr<Float>(tcmCam, 0x10) = saved_TCM_CAM_LENSE_ANGLE;
        *raw_ptr<Float>(tcmCam, 0x14) = saved_TCM_CAM_HORIZONTAL_FOLLOWING;
        *raw_ptr<Float>(tcmCam, 0x18) = saved_TCM_CAM_HORIZONTAL_BOUNDS;
        *raw_ptr<Float>(tcmCam, 0x1C) = saved_TCM_CAM_DEPTH_BOUNDS;
        *raw_ptr<Float>(tcmCam, 0x20) = saved_TCM_CAM_TARGET_HORIZONTAL_BOUNDS;
        *raw_ptr<Float>(tcmCam, 0x24) = saved_TCM_CAM_TARGET_DEPTH_BOUNDS;
        *raw_ptr<Float>(tcmCam, 0x28) = saved_TCM_CAM_NEAR_CLIP;
    }
}

void Install3dPatches_FM13() {

    /*
    WideChar const *camSettingsFilename = L".\\plugins\\ucp\\camera.ini";
    auto ReadIniFloat = [](WideChar const *section, WideChar const *key, float &outValue, WideChar const *filename) {
        WideChar resultStr[64];
        GetPrivateProfileStringW(section, key, L"NA", resultStr, 64, filename);
        if (wcscmp(resultStr, L"NA"))
            outValue = Utils::SafeConvertFloat(resultStr);
    };
    for (UInt i = 1; i <= 10; i++) {
        auto camName = Utils::Format(L"CAMERA_%d", i);
        Int camUsed = GetPrivateProfileIntW(camName.c_str(), L"TCM_CAM_ENABLE", 0, camSettingsFilename);
        if (camUsed != 0) {
            auto &camSettings = GetCamSettings(i);
            camSettings.TCM_CAM_ENABLED = true;
            ReadIniFloat(camName.c_str(), L"TCM_CAM_HEIGHT", camSettings.TCM_CAM_HEIGHT, camSettingsFilename);
            ReadIniFloat(camName.c_str(), L"TCM_CAM_DEPTH_DIST", camSettings.TCM_CAM_DEPTH_DIST, camSettingsFilename);
            ReadIniFloat(camName.c_str(), L"TCM_CAM_LENSE_ANGLE", camSettings.TCM_CAM_LENSE_ANGLE, camSettingsFilename);
            ReadIniFloat(camName.c_str(), L"TCM_CAM_HORIZONTAL_FOLLOWING", camSettings.TCM_CAM_HORIZONTAL_FOLLOWING, camSettingsFilename);
            ReadIniFloat(camName.c_str(), L"TCM_CAM_HORIZONTAL_BOUNDS", camSettings.TCM_CAM_HORIZONTAL_BOUNDS, camSettingsFilename);
            ReadIniFloat(camName.c_str(), L"TCM_CAM_DEPTH_BOUNDS", camSettings.TCM_CAM_DEPTH_BOUNDS, camSettingsFilename);
            ReadIniFloat(camName.c_str(), L"TCM_CAM_NEAR_CLIP", camSettings.TCM_CAM_NEAR_CLIP, camSettingsFilename);
            ReadIniFloat(camName.c_str(), L"TCM_CAM_TARGET_HORIZONTAL_BOUNDS", camSettings.TCM_CAM_TARGET_HORIZONTAL_BOUNDS, camSettingsFilename);
            ReadIniFloat(camName.c_str(), L"TCM_CAM_TARGET_DEPTH_BOUNDS", camSettings.TCM_CAM_TARGET_DEPTH_BOUNDS, camSettingsFilename);
        }
    }

    patch::RedirectCall(GfxCoreAddress(0x4663F), GetCameraSchemeId<1>);
    patch::RedirectCall(GfxCoreAddress(0x466CE), GetCameraSchemeId<3>);
    patch::RedirectCall(GfxCoreAddress(0x466E7), GetCameraSchemeId<1>);
    patch::RedirectCall(GfxCoreAddress(0x4673B), GetCameraParameters);
    */

    //patch::RedirectJump(GfxCoreAddress(0xD0520), OnAiPlayerProcessControl);
    //patch::RedirectCall(GfxCoreAddress(0xD0656), OnAiPlayerProcessControl2);
    //patch::RedirectCall(GfxCoreAddress(0xD0688), OnAiPlayerProcessControl2);
   // patch::RedirectCall(GfxCoreAddress(0xCFC43), OnAiPlayerPerformAction); // Cross
   // patch::RedirectCall(GfxCoreAddress(0xCFC10), OnAiPlayerPerformAction); // Shot
   // patch::RedirectCall(GfxCoreAddress(0xCFBDF), OnAiPlayerPerformAction); // Clearance
   // patch::RedirectCall(GfxCoreAddress(0xCFA7F), OnAiPlayerPerformAction); // Keep Away
   // patch::RedirectCall(GfxCoreAddress(0xCFAA0), OnAiPlayerPerformAction); // Lob Pass
   // patch::RedirectCall(GfxCoreAddress(0xCFABA), OnAiPlayerPerformAction); // Through Pass
   // patch::RedirectCall(GfxCoreAddress(0xCFB44), OnAiPlayerPerformAction); // Dribble 1
   // patch::RedirectCall(GfxCoreAddress(0xCFB5E), OnAiPlayerPerformAction); // Dribble 2

    //patch::RedirectCall(GfxCoreAddress(0x21CC6B), OnMotionMgrStartScript);
    //DumpChoreoEvents();
    //patch::RedirectCall(GfxCoreAddress(0x0018BA0 + 0x61), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0035260 + 0x17), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0035260 + 0x31), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00352A0 + 0x20), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00352A0 + 0x38), OnPostEvent);
    //patch::RedirectJump(GfxCoreAddress(0x0035350 + 0x1AC), OnPostEvent);
    //patch::RedirectJump(GfxCoreAddress(0x0035350 + 0x1EA), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0035350 + 0x211), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x003F360 + 0x42), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x003F360 + 0x64), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x003F410 + 0x107), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x003B840 + 0x3F8D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0043420 + 0x5E3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0046A20 + 0x12B4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0053140 + 0x19), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0055BE0 + 0x445), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0056C50 + 0x365), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0056C50 + 0x7A1), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0056C50 + 0x7B6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0057C30 + 0xC5), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0058180 + 0x130), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0058D60 + 0x6E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0058D60 + 0xC9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0058D60 + 0xE3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0058D60 + 0x142), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0058D60 + 0x15C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005A3A0 + 0x38D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005A3A0 + 0x3B6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005AF10 + 0x158), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005AF10 + 0x182), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005AF10 + 0x229), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B200 + 0xD6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B200 + 0x12D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B350 + 0x26), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B350 + 0x11F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B7C0 + 0xDD), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B7C0 + 0x100), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B7C0 + 0x147), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B7C0 + 0x1B4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005B7C0 + 0x290), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005BA90 + 0x2D1), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005BA90 + 0x2E7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005BDF0 + 0x81), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005BDF0 + 0x9E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005BDF0 + 0xB7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005BDF0 + 0xCD), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005BDF0 + 0xE3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005BFC0 + 0x12), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005C030 + 0x120), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005C030 + 0x2BE), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005CBB0 + 0x78), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005CC40 + 0x64), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x005DA10 + 0x36), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x009CAC0 + 0x23), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x009CAF0 + 0x12), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00BFD00 + 0xA4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00BFD00 + 0xB6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00E1660 + 0x5A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00E6660 + 0x12F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00E6660 + 0x150), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00E6660 + 0x1C3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EDD80 + 0x162), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EDD80 + 0x1B4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EDF49), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EDD80 + 0x36E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EE100 + 0xDA), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EE100 + 0xF6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EE340 + 0x2DC), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EE630 + 0x29C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EE630 + 0x340), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EE630 + 0x376), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00EE630 + 0x3A0), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00F0430 + 0x2AD), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00F0430 + 0x38A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00F0430 + 0x396), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00F0430 + 0x43F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00F0430 + 0x49E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00F0430 + 0x514), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00F0430 + 0x5F3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00F0430 + 0x64E), OnPostEvent);
    //patch::RedirectJump(GfxCoreAddress(0x00FAF20 + 0x1D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x00FF340 + 0x279), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0110D40 + 0x180), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01121C0 + 0xC09), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01121C0 + 0xC12), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01168D0 + 0x772), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01168D0 + 0xA40), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01168D0 + 0xA4C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01168D0 + 0xACA), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0118430 + 0x61), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0118430 + 0x72), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0119220 + 0x1D2), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0119410 + 0x114), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x011A3E0 + 0xB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x011A7E0 + 0x552), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x011A7E0 + 0x599), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x011A7E0 + 0x5B2), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0127D00 + 0x131), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012A860 + 0x1FB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012A860 + 0x286), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012A860 + 0x2A2), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012A860 + 0x2BB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012CEA0 + 0x11D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012CEA0 + 0x125), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012CEA0 + 0x2D9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012CEA0 + 0x2F0), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D2D9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D1F0 + 0xFA), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D1F0 + 0x110), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D1F0 + 0x121), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D3C8), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D320 + 0xDB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D47C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D538), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D410 + 0x13D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D410 + 0x14B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D570 + 0x3E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D570 + 0x77), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D640 + 0x47), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012D640 + 0x61), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012DF20 + 0x3D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012E0A0 + 0x16E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012E290 + 0x46), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012E2F0 + 0xBB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012E3C0 + 0x2C9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012EC00 + 0x4E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012EE50 + 0x257), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012EE50 + 0x503), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012EE50 + 0x578), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012EE50 + 0x919), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012EE50 + 0x969), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012EE50 + 0x9A6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012EE50 + 0x9E4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012F852), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x012F8C0 + 0x16C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130257), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130270 + 0xD1), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130270 + 0x10F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130390 + 0x1BB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130390 + 0x200), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130390 + 0x27F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130830 + 0xA4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130980 + 0xAE), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130980 + 0xBA), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130980 + 0xF0), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130980 + 0xFF), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0130980 + 0x17C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01310B0 + 0x19), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01310E0 + 0x1E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131140 + 0x32), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131230 + 0x4B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01312B0 + 0xEA), OnPostEvent);
    //patch::RedirectJump(GfxCoreAddress(0x01313B0 + 0x373), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01317C0 + 0xE7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01318C0 + 0xCA), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01318C0 + 0x112), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01318C0 + 0x12E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131A35), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131A50 + 0x324), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131A50 + 0x3F5), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131E80 + 0x87), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131E80 + 0x90), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131F20 + 0x6B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0131F20 + 0x28E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01321C0 + 0x113), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01321C0 + 0x12F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01321C0 + 0x362), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01321C0 + 0x48D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0134BA0 + 0x1FC), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0134BA0 + 0x2C9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0134BA0 + 0x529), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0134BA0 + 0x616), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0134BA0 + 0x62E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0134BA0 + 0x68F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0135260 + 0x122), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01371E0 + 0x22A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0137450 + 0x348), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0137450 + 0x3D5), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0137450 + 0x575), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0137450 + 0x590), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0137450 + 0x81F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0137450 + 0x82B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0137DA0 + 0x24C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0139F60 + 0x1DB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0139F60 + 0x2F1), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0139F60 + 0x381), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0139F60 + 0x59F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0139F60 + 0x697), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0139F60 + 0x6A6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x013A660 + 0x1C1), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01431A0 + 0x1C7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01447B0 + 0x1F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0144BD0 + 0x40D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0145542), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01454A0 + 0xD6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0146530 + 0x4C0), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0146530 + 0x4C9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0147410 + 0x1C6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0147410 + 0x2F6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0147410 + 0x487), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0147410 + 0x92E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0147D55), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0147410 + 0x96A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0148460 + 0x248), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0148460 + 0x254), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x014FF70 + 0xE), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0158030 + 0x3B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0158030 + 0x6D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0158030 + 0x1B7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0158200 + 0xFB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0158200 + 0x13E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01600A0 + 0x71B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01600A0 + 0x731), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0160D40 + 0x674), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D450 + 0x24), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D450 + 0x38), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D450 + 0x4E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D4D0 + 0x2C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D4D0 + 0x47), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D4D0 + 0x63), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D4D0 + 0x6C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D560 + 0x31), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D560 + 0x45), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D560 + 0x4E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D5C0 + 0xC3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D5C0 + 0x117), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D5C0 + 0x12B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D5C0 + 0x137), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x016D710 + 0x8F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01785D0 + 0x32), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01785D0 + 0x211), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01788B0 + 0x4A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01788B0 + 0xC7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01788B0 + 0x10E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01956C0 + 0x3B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5B40 + 0x2C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5BB0 + 0x56), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5C10 + 0xE2), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5C10 + 0x11E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5D80 + 0x13), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5D80 + 0x4F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5DE0 + 0x4E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5E60 + 0x8C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01A5F00 + 0x58), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01E5E00 + 0x142), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x01E6DC0 + 0xC7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02051A0 + 0x33), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0211A80 + 0xB9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021DC60 + 0x20), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021DC60 + 0x44), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1A0B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x19EE), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x19D1), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x19B4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1961), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1944), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1927), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x190A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x18ED), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x18D0), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x18B3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1896), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1879), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x185C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x183F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1822), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1805), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x17E8), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x17CB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x17AE), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1791), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1774), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1757), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x173A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x171D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1700), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x16E3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x16C6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x16A9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x168C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x166F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1652), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1635), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1618), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x15FB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x15DE), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x15C1), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x15A4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1587), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x156A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x154D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1530), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1513), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x14F6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x14D9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x14BC), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x149F), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1482), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1465), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1448), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x142B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x140E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x13F1), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x13D4), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x13B7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x139A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1380), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1363), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1346), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1329), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x130C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x12EF), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x12D2), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x12B5), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1298), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x127B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x125A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1239), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x1218), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 - 0x11F7), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021ECE0 + 0x12), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021ECE0 + 0x29), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021ECE0 + 0x40), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021ECE0 + 0x57), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021ECE0 + 0x6E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021ECE0 + 0x85), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021ECE0 + 0x9C), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021ECE0 + 0xB3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021F840 + 0x13), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x021FDB0 + 0x7A), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0222570 + 0x7D), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0222570 + 0xA0), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0222570 + 0xC6), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0235150 + 0xCB), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02354B0 + 0x30), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02355A0 + 0x36), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0257C40 + 0x8B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0257CE0 + 0x1DD), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0257EF0 + 0x205), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0258130 + 0x276), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02583D0 + 0x70), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02586F0 + 0x161), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0258860 + 0x4E), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02588C0 + 0xB3), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02588C0 + 0xC2), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02588C0 + 0xD8), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x02588C0 + 0xE9), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x027AB70 - 0x1C7BC), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x027CA50 - 0x1E689), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0260210 + 0x16B), OnPostEvent);
    //patch::RedirectCall(GfxCoreAddress(0x0284D10 + 0x5C), OnPostEvent);

    //patch::RedirectCall(GfxCoreAddress(0x458935), LoadFshFifaM);
    //patch::RedirectCall(GfxCoreAddress(0x458980), LoadFshFifaM);
    //patch::SetPointer(GfxCoreAddress(0x4096B1 + 1), FshLoadCB);
    //patch::SetPointer(GfxCoreAddress(0x4096E6 + 1), FshLoadCB);

    patch::RedirectCall(GfxCoreAddress(0x3791BE), OnLoadFifaPlayer1);
    patch::RedirectCall(GfxCoreAddress(0x379769), OnLoadFifaPlayer1);
    patch::RedirectCall(GfxCoreAddress(0x37A2C9), OnLoadFifaPlayer1);
    patch::RedirectCall(GfxCoreAddress(0x3A05D6), OnLoadFifaPlayer1);
    patch::RedirectCall(GfxCoreAddress(0x392423), OnGetPlayerEffTexture1);

    patch::RedirectCall(GfxCoreAddress(0x20272C), OnLoadFifaPlayer2);
    patch::RedirectCall(GfxCoreAddress(0x204253), OnLoadFifaPlayer2);
    patch::RedirectCall(GfxCoreAddress(0x2024AF), OnGetPlayerEffTexture2);

    patch::RedirectCall(GfxCoreAddress(0x201F46), OnPreLoadFifaPlayerTex);

    //patch::RedirectJump(GfxCoreAddress(0x20D21D), OnGenerateCrowdTex);

    //patch::RedirectCall(GfxCoreAddress(0x40A355), OnLoadToMem);
    //patch::RedirectCall(GfxCoreAddress(0x40AEBD), OnAllocate);


    //patch::RedirectCall(GfxCoreAddress(0xA95BD), OnAllocateMemory);

    //patch::RedirectCall(GfxCoreAddress(0xA95BD), OnDeleteMemory);

    //patch::RedirectCall(GfxCoreAddress(0x2E237C), OnRenderModel);

    //patch::RedirectCall(GfxCoreAddress(0x2E2449), OnExecuteRenderMethod);
    //patch::RedirectCall(GfxCoreAddress(0x2E23CB), OnExecuteRenderMethod);

    //patch::SetUInt(GfxCoreAddress(0x1621F + 6), 0x30000000);

    ReadPlayerCommentaryConfig();

    static Float gf0 = 0.0f;
    static Float gf1 = 1.0f;
    static Float gf2 = 2.0f;
    static Float gf3 = 3.0f;

    patch::SetPointer(GfxCoreAddress(0x54CFE8), MyRenderFifaPlayer);
    patch::SetPointer(GfxCoreAddress(0x5495DC), MyGeneratePlayerPortrait);

    // set 32-bit render mode for EAGL PC Rertrival
    if (!Settings::GetInstance().getNoHardwareAccelerationFix()) {
        patch::SetUInt(GfxCoreAddress(0x2D63C5 + 1), 32);
        //Error("Enabled Hardware Acceleration Fix");
    }

    patch::SetPointer(GfxCoreAddress(0x5504A8), OnFileIoSetupPaths);

    patch::RedirectJump(GfxCoreAddress(0x4811AF), OnFileNameGet);

    // loadingType == 2 check
    patch::Nop(GfxCoreAddress(0x44DB54), 6);
    patch::Nop(GfxCoreAddress(0x44C621), 6);
    patch::Nop(GfxCoreAddress(0x44C761), 6);
    patch::Nop(GfxCoreAddress(0x44D86D), 2);

    // old fonts glyph buffer fix
    ///patch::RedirectCall(GfxCoreAddress(0x3CFE90), TTF_GetFontLineForFont<0>);
    ///patch::RedirectCall(GfxCoreAddress(0x3D00BD), TTF_GetFontLineForFont<1>);
    patch::SetUInt(GfxCoreAddress(0x3DAE47 + 2), 6'000);

    patch::RedirectJump(GfxCoreAddress(0x4789EB), ShowExceptionError_GfxCore);

    static Float NewDist = 8.3f;
    static Double NewAngleX = -1.0;
    static Float NewZ = -110.0f;
    static Float NewY = 101.4f;
 
    patch::SetDouble(GfxCoreAddress(0x549610), 90.0f);
    patch::SetPointer(GfxCoreAddress(0x378D22 + 2), &NewDist);
    patch::SetPointer(GfxCoreAddress(0x378DAC + 2), &NewAngleX);
 
    patch::SetPointer(GfxCoreAddress(0x378D39 + 2), &NewZ);
    patch::SetPointer(GfxCoreAddress(0x378D30 + 2), &NewY);
 
    // player head scene - remove kit
    patch::SetUChar(GfxCoreAddress(0x379187 + 1), 0);
    patch::RedirectCall(GfxCoreAddress(0x379170), LoadPlayerHeadKitTexture);

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

    patch::SetChar(GfxCoreAddress(0x39DB17 + 3), 't');
    patch::SetChar(GfxCoreAddress(0x39DB21 + 3), 'g');
    patch::SetChar(GfxCoreAddress(0x39DB2B + 3), 'a');
   
    patch::SetChar(GfxCoreAddress(0x458FA4 + 3), 't');
    patch::SetChar(GfxCoreAddress(0x458FA8 + 3), 'g');
    patch::SetChar(GfxCoreAddress(0x458FAC + 3), 'a');
   
    patch::SetPointer(GfxCoreAddress(0x39DAEB + 1), ".dds");
    patch::SetPointer(GfxCoreAddress(0x458F92 + 1), ".dds");
    patch::SetPointer(GfxCoreAddress(0x201DFD + 1), "hair_cm_%d_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x202597 + 1), "hair_cm_%d_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x370684 + 1), "hair_cm_-%d_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x201DB5 + 1), "cm_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x202442 + 1), "cm_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x370666 + 1), "cm_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x201E1E + 1), "hair_cmb_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x2025C1 + 1), "hair_cmb_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x370717 + 1), "hair_cmb_-%d.dds");
    patch::SetPointer(GfxCoreAddress(0x201DD3 + 1), "cmbhair_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x202466 + 1), "cmbhair_%d.dds");
    patch::SetPointer(GfxCoreAddress(0x3706FF + 1), "cmbhair_%d.dds");

    //patch::SetPointer(GfxCoreAddress(0x37C45C + 1), "Coordinate4::*::UVOffset0");
    //patch::SetPointer(GfxCoreAddress(0x37C4C6 + 1), "Coordinate4::*::UVOffset1");

    if (Settings::GetInstance().getCrowdResolution() != 1024 && Settings::GetInstance().getCrowdResolution() >= 2) {
        patch::SetUInt(GfxCoreAddress(0x412FF6 + 1), Settings::GetInstance().getCrowdResolution() / 4);
        patch::SetUInt(GfxCoreAddress(0x412FFB + 1), Settings::GetInstance().getCrowdResolution());
    }

    if (!Settings::GetInstance().getTeamControlDisabledAtGameStart()) {
        patch::RedirectCall(GfxCoreAddress(0x23E629), OnSetupManualPlayerSwitch);
        patch::RedirectCall(GfxCoreAddress(0x165C3), OnCreateInputDevice);
        patch::RedirectCall(GfxCoreAddress(0x18553), OnSetupController);
        patch::RedirectCall(GfxCoreAddress(0x2E5B5A), GfxCopyData); // fix null-pointer memory copying
    }

    // fix for some options
    patch::RedirectCall(GfxCoreAddress(0x4553BB), OnCheckOption);
    patch::RedirectCall(GfxCoreAddress(0x4553E0), OnCheckOption);

    // for for AA
    patch::RedirectCall(GfxCoreAddress(0x4553CF), OnSetAALevel);

    // Pitch color configurator
    //patch::RedirectJump(GfxCoreAddress(0x1EEA88), OnSetupLightingData);
    //patch::RedirectCall(GfxCoreAddress(0x1EEA95), OnSetupLightingData);
    //patch::RedirectCall(GfxCoreAddress(0x1EEAD8), OnSetupLightingData);

    // fix for hair textures rendering
    patch::RedirectCall(GfxCoreAddress(0x39DEDD), OnSetFifaTexture<0xC0F1E4, 0x39DC49>);
    patch::RedirectCall(GfxCoreAddress(0x39DEF1), OnSetFifaTexture<0xC0F1E4, 0x39DC49>);
    patch::RedirectJump(GfxCoreAddress(0x2155EC), OnRenderRenderSlots);
    //patch::RedirectCall(GfxCoreAddress(0x454743), OnSetFifaTexture2);
    //patch::RedirectCall(GfxCoreAddress(0x45483D), OnRenderFifaPlayerInMatch);

    // commentary
    //patch::RedirectCall(GfxCoreAddress(0x2361B3), GetPlayerCommentaryID);

    // DO NOT ENABLE
    //patch::SetUChar(GfxCoreAddress(0x646048), 0);
    //patch::SetUChar(GfxCoreAddress(0x23144E + 6), 0);

    //patch::RedirectJump(GfxCoreAddress(0x19E60), CachedExists);
    //patch::RedirectJump(GfxCoreAddress(0x19E70), CachedSize);

    if (Settings::GetInstance().getEnableCommentaryPatches()) {
        patch::SetUInt(GfxCoreAddress(0x231a0f + 1), GfxCoreAddress(0x51d0a8));
        patch::SetUInt(GfxCoreAddress(0x231a14 + 2), GfxCoreAddress(0x51d0ac));
        patch::SetUInt(GfxCoreAddress(0x231a1a + 2), GfxCoreAddress(0x51d0b0));
        patch::SetPointer(GfxCoreAddress(0x235c9f + 1), "Commentary\\pbp.evt");
        patch::SetPointer(GfxCoreAddress(0x235de4 + 1), "Commentary\\hdr.big");
        patch::SetUInt(GfxCoreAddress(0x235e4d + 1), 17);
        patch::SetPointer(GfxCoreAddress(0x235e52 + 1), "data\\audio\\Commentary\\dat.big");
        patch::SetUInt(GfxCoreAddress(0x235e82 + 1), 18);
        patch::SetPointer(GfxCoreAddress(0x235e87 + 1), "data\\audio\\Commentary\\dat.big");
    }

    // Shape writer
    patch::RedirectJump(GfxCoreAddress(0x38708F), FshNewHash);
    patch::SetUInt(GfxCoreAddress(0x382FC0 + 6), 0x400000); // file size limit
    patch::SetUInt(GfxCoreAddress(0x383058 + 6), 0x400000); // file size limit

    // Ball Near/Far Clip
    static Float BallHDFar = 0.95f; // 4.5
    static Float BallLDNear = 1.0f; // 4.7
    patch::SetPointer(GfxCoreAddress(0x24104A + 2), &BallHDFar);
    patch::SetPointer(GfxCoreAddress(0x241056 + 2), &BallLDNear);

    // eye colors for starheads
    patch::Nop(GfxCoreAddress(0x92259), 2);

    //Float Unk1 = 576.0f * 2.0f;
    //patch::SetPointer(GfxCoreAddress(0x12CB79 + 2), &Unk1);

   /// patch::SetUInt(GfxCoreAddress(0x1621A + 1), 0x6000000 * 4);
   //patch::SetUInt(GfxCoreAddress(0x1621F + 6), 0x18000000 * 2);
   //patch::SetUInt(GfxCoreAddress(0x16233 + 1), 0x300000);
   /// patch::SetUInt(GfxCoreAddress(0x49E72E + 1), 0x4000000 * 4);
   /// patch::SetUInt(GfxCoreAddress(0x49E741 + 1), 0x4000000 * 4);
    //
    //patch::SetUInt(GfxCoreAddress(0x3BAE8C + 1), 0x10000000 * 2);
    //patch::SetUInt(GfxCoreAddress(0x3C1E0E + 1), 256);
    //patch::SetUChar(GfxCoreAddress(0x3C1E18 + 1), 0xEB);
    //patch::RedirectCall(GfxCoreAddress(0x316BF5), MyError);

    //patch::RedirectCall(GfxCoreAddress(0x16239), OnAllocRAM);
   /// const unsigned int kitW = 1024;
   /// const unsigned int kitH = 2048;
   ///
   /// patch::SetUInt(GfxCoreAddress(0x383F9C + 6), kitW);
   /// patch::SetUInt(GfxCoreAddress(0x383FA6 + 6), kitH);
   /// patch::SetUInt(GfxCoreAddress(0x383FB0 + 1), kitW);
   /// patch::SetUInt(GfxCoreAddress(0x383F88 + 1), kitW * 2);
   /// patch::SetUInt(GfxCoreAddress(0x38403A + 1), kitW * kitH * 4 + 0x100);
   ///
   /// patch::SetUInt(GfxCoreAddress(0x38405D + 6), kitW);
   /// patch::SetUInt(GfxCoreAddress(0x384069 + 6), kitH);
   /// patch::SetUInt(GfxCoreAddress(0x3840D9 + 1), kitH);
   /// patch::SetUInt(GfxCoreAddress(0x3840DE + 1), kitW);
}
