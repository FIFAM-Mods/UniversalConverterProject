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

template<Bool ExistsCheck>
Int METHOD OnGetFileZdataId(void *fat, DUMMY_ARG, char *filename) {
    std::string p = std::string("data\\zdata\\") + filename;
    if (exists(p)) {
        *raw_ptr<UChar>(fat, 0x14) = 1;
        if (ExistsCheck)
            return 1;
        strcpy_s(filename, 260, p.c_str());
        return 0;
    }
    return CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x44EBE0), fat, filename);
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
    static Float gfCustom = 2.0f;
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

void Install3dPatches_FM13() {

    //patch::RedirectCall(GfxCoreAddress(0x3907AC), unk23);
    //patch::RedirectJump(GfxCoreAddress(0x3902C8), unk23);

    static Float gf0 = 0.0f;
    static Float gf1 = 1.0f;
    static Float gf2 = 2.0f;
    static Float gf3 = 3.0f;

    //patch::SetPointer(GfxCoreAddress(0x390498 + 2), &gf2);
    //patch::SetPointer(GfxCoreAddress(0x3904A4 + 2), &gf2);
    //patch::SetPointer(GfxCoreAddress(0x3904B0 + 2), &gf2);

    //patch::SetPointer(GfxCoreAddress(0x39041E + 2), &gf1);
    //patch::SetPointer(GfxCoreAddress(0x390427 + 2), &gf1);

    ///patch::SetPointer(GfxCoreAddress(0x3904C8 + 2), &gf2);

    //patch::RedirectCall(GfxCoreAddress(0x3907AC), MySetModelLights);

    //patch::SetPointer(GfxCoreAddress(0x39037F + 2), &gf0);
    //patch::SetPointer(GfxCoreAddress(0x390388 + 2), &gf3);
    //patch::SetPointer(GfxCoreAddress(0x390391 + 2), &gf3);

    patch::SetPointer(GfxCoreAddress(0x54CFE8), MyRenderFifaPlayer);
    patch::SetPointer(GfxCoreAddress(0x5495DC), MyGeneratePlayerPortrait);

    // set 32-bit render mode for EAGL PC Rertrival
    if (!Settings::GetInstance().getNoHardwareAccelerationFix())
        patch::SetUInt(GfxCoreAddress(0x2D63C5 + 1), 32);

    patch::RedirectCall(GfxCoreAddress(0x44C6E0), OnGetFileZdataId<true>); // file exists
    patch::RedirectCall(GfxCoreAddress(0x44C81F), OnGetFileZdataId<false>); // file size
    patch::RedirectCall(GfxCoreAddress(0x44D860), OnGetFileZdataId<false>); // file load 1
    patch::RedirectCall(GfxCoreAddress(0x44DB45), OnGetFileZdataId<false>); // file load 2

    // banners
    //for (UInt i = 0; i < 8; i++) {
    //    patch::SetUInt(GfxCoreAddress(0x524198 + i * 24), 256);
    //    patch::SetUInt(GfxCoreAddress(0x524198 + 4 + i * 24), 128);
    //}
    //patch::SetPointer(0x4DAAF8, GetBannersDir);

    patch::RedirectCall(GfxCoreAddress(0x3CFE90), TTF_GetFontLineForFont<0>);
    patch::RedirectCall(GfxCoreAddress(0x3D00BD), TTF_GetFontLineForFont<1>);

    //patch::Nop(GfxCoreAddress(0x315FE8), 8);
    //patch::Nop(GfxCoreAddress(0x315ED9), 8);
    //patch::Nop(GfxCoreAddress(0x2D5816), 7);
    //patch::SetUChar(GfxCoreAddress(0x6DF0), 0xC3);
    //
    //patch::Nop(GfxCoreAddress(0x182CD8), 1);
    //patch::Nop(GfxCoreAddress(0x182CE1), 6);
    //
    //patch::Nop(GfxCoreAddress(0x39CE88), 7);
    //patch::Nop(GfxCoreAddress(0x4552C8), 10);
    //patch::SetUChar(GfxCoreAddress(0x4507A1), 0xEB);

    //CrashFix_5e3f0_retAddr = GfxCoreAddress(0x5E3F6);
    //patch::RedirectJump(GfxCoreAddress(0x5E3F0), CrashFix_5e3f0);

    //patch::SetUChar(GfxCoreAddress(0x184B80), 0xC3);

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

    // Jersey number texture size
    //const unsigned int newSize = 256; // default 128x128
    //
    //patch::SetUInt(GfxCoreAddress(0x2022EE + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x2022F3 + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x202338 + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x20233D + 1), newSize);
    
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

    // player name

    //patch::SetUChar(GfxCoreAddress(0x204789 + 1), 40);

    // BE Scene
    patch::SetPointer(GfxCoreAddress(0x1E85E5 + 6), "data/BEscene_ucp.cs");

    // accs

    //gSetAcc = patch::RedirectCall(GfxCoreAddress(0x23D175), set_acc);
    //patch::SetUInt(GfxCoreAddress(0x281F15 + 4), 5);
    //patch::SetUInt(GfxCoreAddress(0x3B50E + 1), 120);
    //patch::SetUInt(GfxCoreAddress(0x3B519 + 1), 120);

    //patch::SetUInt(GfxCoreAddress(0x23CE18 + 1), 5);

    // fonts changes

    // disable original height correction for fonts
    patch::Nop(GfxCoreAddress(0x3DC3F3), 1);
    patch::SetUChar(GfxCoreAddress(0x3DC3F3 + 1), 0xE9);
    //
    patch::RedirectCall(GfxCoreAddress(0x3DCFE6), MySetFontDescFromParamsLine);
    //patch::RedirectCall(GfxCoreAddress(0x3DC677), MySetFontDescParam); removed

    // team control
    //return;
    //if (Settings::GetInstance().getTeamControl()) {
        //patch::RedirectCall(GfxCoreAddress(0x11AB6E), OnSetupTeamManagementDatas);

    patch::RedirectCall(GfxCoreAddress(0x7029), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x7119), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x81D1), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x83FF), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x19647), GetAppTitle);
    patch::RedirectCall(GfxCoreAddress(0x78B9), GetAppDocumentsDir);
    patch::RedirectCall(GfxCoreAddress(0x3B0B9), GetAppDocumentsDir);
    patch::RedirectJump(GfxCoreAddress(0x281AF0), GetAppDocumentsDir);

    if (!Settings::GetInstance().getTeamControlDisabledAtGameStart()) {

        patch::RedirectCall(GfxCoreAddress(0x23E629), OnSetupManualPlayerSwitch);

        //patch::SetUChar(GfxCoreAddress(0x23B7E3), 0xB0);
        //patch::SetUChar(GfxCoreAddress(0x23B7E3 + 1), 0x01);
        //patch::SetUChar(GfxCoreAddress(0x23B7E3 + 2), 0x90);
        //patch::RedirectCall(GfxCoreAddress(0x23B891), OnEnableManualSwitch);


        patch::RedirectCall(GfxCoreAddress(0x165C3), OnCreateInputDevice);


        ///patch::RedirectJump(GfxCoreAddress(0x17CC90), OnSetTeamDifficulty);

        patch::RedirectCall(GfxCoreAddress(0x18553), OnSetupController);

        //patch::RedirectCall(GfxCoreAddress(0x9240B), OnGet3dPlayerAttribute);
        //patch::RedirectCall(GfxCoreAddress(0x923E5), OnGet3dPlayerBasicLevel);
        //patch::RedirectCall(GfxCoreAddress(0x9242B), OnGet3dPlayerOverall);

        // fix null-pointer memory copying
        patch::RedirectCall(GfxCoreAddress(0x2E5B5A), GfxCopyData);

    //}
    }
    
}

void Install3dPatches_FM11() {
    ///patch::SetFloat(GfxCoreAddress(0x4C3CE8), 3.15f);
    ///patch::SetPointer(GfxCoreAddress(0x487844 + 2), &NewDist);
    //patch::SetPointer(GfxCoreAddress(0x378DAC + 2), &NewAngleX);

    // Jersey number texture size
    //const unsigned int newSize = 256; // default 128x128
    //
    //patch::SetUInt(GfxCoreAddress(0x2022EE + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x2022F3 + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x202338 + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x20233D + 1), newSize);

    ///patch::SetUInt(GfxCoreAddress(0x1FAD50 + 4), 4); // 25
    ///patch::SetUInt(GfxCoreAddress(0x1FAD64 + 4), 0); // 4
    ///patch::SetUInt(GfxCoreAddress(0x1FAD78 + 4), 60); // 39
    ///patch::SetUInt(GfxCoreAddress(0x1FAD8C + 4), 120); // 78
    ///
    ///patch::SetUInt(GfxCoreAddress(0x1FADA0 + 4), 64); // 64
    ///patch::SetUInt(GfxCoreAddress(0x1FADB4 + 4), 0); // 4
    ///patch::SetUInt(GfxCoreAddress(0x1FADC8 + 4), 60); // 39
    ///patch::SetUInt(GfxCoreAddress(0x1FADDC + 4), 120); // 78
    ///
    ///patch::SetUInt(GfxCoreAddress(0x1FADF0 + 4), 34); // 44
    ///patch::SetUInt(GfxCoreAddress(0x1FAE04 + 4), 0); // 4
    ///patch::SetUInt(GfxCoreAddress(0x1FAE18 + 4), 60); // 39
    ///patch::SetUInt(GfxCoreAddress(0x1FAE2C + 4), 120); // 78

    // player name

    //patch::SetUChar(GfxCoreAddress(0x204789 + 1), 40);

    // BE Scene
    ///patch::SetPointer(GfxCoreAddress(0x1D5AD3 + 6), "data/BEscene_ucp.cs");

    // accs

    //gSetAcc = patch::RedirectCall(GfxCoreAddress(0x23D175), set_acc);
    //patch::SetUInt(GfxCoreAddress(0x281F15 + 4), 5);
    //patch::SetUInt(GfxCoreAddress(0x3B50E + 1), 120);
    //patch::SetUInt(GfxCoreAddress(0x3B519 + 1), 120);

    //patch::SetUInt(GfxCoreAddress(0x23CE18 + 1), 5);
}
