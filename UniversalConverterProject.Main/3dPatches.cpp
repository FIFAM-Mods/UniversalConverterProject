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
    return file_size(filename);

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
    void *device = *(void **)GfxCoreAddress(DeviceAddr);
    //device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    //device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    //device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
    //Error("OnSetFifaTexture: %d", samplerIndex);
    __asm {
        push    1
        push    1
        push    samplerIndex
        mov     eax, device
        mov     eax, [eax]
        push    device
        call    dword ptr[eax + 0x114]
    }
    __asm {
        push    1
        push    2
        push    samplerIndex
        mov     eax, device
        mov     eax, [eax]
        push    device
        call    dword ptr[eax + 0x114]
    }
    return CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(OriginalAddr), t, samplerIndex);
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

void Install3dPatches_FM13() {

    //patch::RedirectJump(GfxCoreAddress(0x20D21D), OnGenerateCrowdTex);

    //patch::RedirectCall(GfxCoreAddress(0x40A355), OnLoadToMem);
    //patch::RedirectCall(GfxCoreAddress(0x40AEBD), OnAllocate);


    //patch::RedirectCall(GfxCoreAddress(0xA95BD), OnAllocateMemory);

    //patch::RedirectCall(GfxCoreAddress(0xA95BD), OnDeleteMemory);

    //patch::RedirectCall(GfxCoreAddress(0x2E237C), OnRenderModel);

    //patch::RedirectCall(GfxCoreAddress(0x2E2449), OnExecuteRenderMethod);
    //patch::RedirectCall(GfxCoreAddress(0x2E23CB), OnExecuteRenderMethod);

    ///patch::SetUInt(GfxCoreAddress(0x1621F + 6), 0x30000000);

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

    // banners
    //for (UInt i = 0; i < 8; i++) {
    //    patch::SetUInt(GfxCoreAddress(0x524198 + i * 24), 256);
    //    patch::SetUInt(GfxCoreAddress(0x524198 + 4 + i * 24), 128);
    //}
    //patch::SetPointer(0x4DAAF8, GetBannersDir);

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

    patch::SetPointer(GfxCoreAddress(0x37C45C + 1), "Coordinate4::*::UVOffset0");
    patch::SetPointer(GfxCoreAddress(0x37C4C6 + 1), "Coordinate4::*::UVOffset1");

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
    patch::RedirectCall(GfxCoreAddress(0x458DF6), OnSetFifaTexture<0xDB21D0, 0x458C20>);
    patch::RedirectCall(GfxCoreAddress(0x458E05), OnSetFifaTexture<0xDB21D0, 0x458C20>);

    // commentary
    //patch::RedirectCall(GfxCoreAddress(0x2361B3), GetPlayerCommentaryID);

    // DO NOT ENABLE
    //patch::SetUChar(GfxCoreAddress(0x646048), 0);
    //patch::SetUChar(GfxCoreAddress(0x23144E + 6), 0);

    //patch::RedirectJump(GfxCoreAddress(0x19E60), CachedExists);
    //patch::RedirectJump(GfxCoreAddress(0x19E70), CachedSize);

    if (Settings::GetInstance().getEnableStadiumsPatches()) {
        patch::RedirectCall(GfxCoreAddress(0x42156d), OnSkyModel);
    }

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
}
