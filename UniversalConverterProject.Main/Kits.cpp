#include "IncreaseSalaryOwnedClub.h"
#include "GfxCoreHook.h"
#include "Utils.h"
#include "FifamTypes.h"
#include "shared.h"

using namespace plugin;
using namespace std::filesystem;

Char *gpUserKitName = nullptr;
Bool gUsedCustomCaptainArmband = false;
Bool gUsedCustomDefaultCaptainArmband = false;
UInt gTeamId = 0;
UInt gCurrCompId = 0;
String gKitTypeStr;
Bool gIsGoalkeeper = false;
Bool gHasGkKit = false;

#define KITS_DEBUG

void WriteToLog(String const &str) {
#ifdef KITS_DEBUG
    SafeLog::Write(str);
#endif
}

namespace gfx {
    class Buffer {
    public:
        void *data;
        UInt size;
        
        //Buffer(UInt Size) {
        //    CallMethodDynGlobal(GfxCoreAddress(0x37382C), Size);
        //}
        //
        //~Buffer() {
        //    void *pool = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3726EF));
        //    CallVirtualMethod<2>(pool, data);
        //}
    };

    struct RawImageDesc {
        UInt width;
        UInt height;
        UInt stride;
        Buffer buffer;

        //RawImageDesc(UInt Width, UInt Height, UInt Stride, UInt Size) : buffer(Size) {
        //    width = Width;
        //    height = Height;
        //    stride = Stride;
        //}
    };
}

Bool FmFileExists(Path const &filepath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3859EA));
    if (fmFs)
        return CallVirtualMethodAndReturn<Bool, 9>(fmFs, filepath.c_str());
    return false;
}

Bool FmFileImageExists(String const &filepathWithoutExtension, String &resultPath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3859EA));
    if (fmFs) {
        String fpstr = filepathWithoutExtension + L".tga";
        if (CallVirtualMethodAndReturn<Bool, 9>(fmFs, fpstr.c_str())) {
            resultPath = fpstr;
            return true;
        }
        else {
            fpstr = filepathWithoutExtension + L".kit";
            if (CallVirtualMethodAndReturn<Bool, 9>(fmFs, fpstr.c_str())) {
                resultPath = fpstr;
                return true;
            }
        }
    }
    resultPath.clear();
    return false;
}

Bool UserKitAvailable() {
    return !gKitTypeStr.empty() && (gCurrCompId != 0 || gTeamId != 0);
}

String GetUserTexturePath(String const &directory, UInt compId, UInt clubId, String const &kitTypeStr) {
    String result;
    String clubIdStr = Utils::Format(L"%08X", clubId);
    if (compId != 0) {
        String compIdStr = Utils::Format(compId < 0xFFFF ? L"%04X" : L"%08X", compId);
        if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr + kitTypeStr, result))
            return result;
        if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr, result))
            return result;
        if (FmFileImageExists(directory + L"\\" + compIdStr, result))
            return result;
        if (FmFileImageExists(directory + L"\\" + clubIdStr + kitTypeStr, result))
            return result;
        if (FmFileImageExists(directory + L"\\" + clubIdStr, result))
            return result;
    }
    else {
        if (FmFileImageExists(directory + L"\\" + clubIdStr + kitTypeStr, result))
            return result;
        if (FmFileImageExists(directory + L"\\" + clubIdStr, result))
            return result;
    }
    return String();
}

void ClearKitParams() {
    gpUserKitName = nullptr;
    gTeamId = 0;
    gCurrCompId = 0;
    gKitTypeStr.clear();
}

void METHOD OnGenerateKit(void *kitGen, DUMMY_ARG, void *kitDesc, void *kitParams, void *resMan) {
    ClearKitParams();
    if (kitDesc) {
        WideChar *pUserKitPath = *raw_ptr<WideChar *>(kitDesc, 0x18);
        if (pUserKitPath && *pUserKitPath) {
            Path userKitPath = pUserKitPath;
            String userKitFilename = userKitPath.stem().c_str();
            auto fileNameParts = Utils::Split(userKitFilename, L'_', false, false);
            if (fileNameParts.size() == 2) {
                gTeamId = Utils::SafeConvertInt<UInt>(fileNameParts[0], true);
                gKitTypeStr = L"_" + fileNameParts[1];
                WriteToLog(Utils::Format(L"OnGenerateKit: gTeamId=%08X, gKitTypeStr=%s", gTeamId, gKitTypeStr));
                void *match = *(void **)0x3124748;
                if (match) {
                    CallMethod<0xE80190>(match, &gCurrCompId);
                    if (gCurrCompId > 0xFFFF) {
                        UInt compRegion = (gCurrCompId >> 24) & 0xFF;
                        if (compRegion <= 0 || compRegion > 207)
                            gCurrCompId = (gCurrCompId >> 16) & 0xFFFF;
                    }
                    WriteToLog(Utils::Format(L"OnGenerateKit: gCurrCompId=%08X", gCurrCompId));
                }
            }
        }
    }
    CallMethodDynGlobal(GfxCoreAddress(0x383DEA), kitGen, kitDesc, kitParams, resMan);
    ClearKitParams();
}

void ApplyCaptainArmbandTexture(String const &customCaptainArmbandPath, void *shapeGen, void *dataDesc) {
    WriteToLog(Utils::Format(L"ReadCaptainArmband: customCaptainArmbandPath=%s", customCaptainArmbandPath));
    UInt acc;
    CallMethodDynGlobal(GfxCoreAddress(0x373B6E), &acc);
    gfx::RawImageDesc img;
    CallVirtualMethod<5>(acc, &img, customCaptainArmbandPath.c_str(), 256 * 64 * 4 + 256, 1, 0); // loadImage
    if (img.width == 256 && img.height == 64) {
        gfx::RawImageDesc *dstImg = *raw_ptr<gfx::RawImageDesc *>(dataDesc, 0x400);
        dstImg->width = 256;
        dstImg->height = 64;
        dstImg->stride = 256 * 4;
        CallMethodDynGlobal(GfxCoreAddress(0x38398C), shapeGen, dstImg, &img); // copyPixels
        gUsedCustomCaptainArmband = true;
    }
    CallVirtualMethod<8>(acc, &img); // deleteImage
    CallMethodDynGlobal(GfxCoreAddress(0x373BA9), &acc);
}

void METHOD ReadCaptainArmband(void *_this, DUMMY_ARG, void *dataDesc, const Char *imageName, const WideChar *fshName) {
    WriteToLog(L"ReadCaptainArmband");
    gUsedCustomCaptainArmband = false;
    if (UserKitAvailable()) {
        String customCaptainArmbandPath = GetUserTexturePath(L"data\\kitarmband", gCurrCompId, gTeamId, gKitTypeStr);
        if (!customCaptainArmbandPath.empty())
            ApplyCaptainArmbandTexture(customCaptainArmbandPath, _this, dataDesc);
    }
    if (!gUsedCustomCaptainArmband) {
        String defaultCaptainArmbandPath;
        if (FmFileImageExists(L"data\\kitarmband\\00000000", defaultCaptainArmbandPath)) {
            ApplyCaptainArmbandTexture(defaultCaptainArmbandPath, _this, dataDesc);
            if (gUsedCustomCaptainArmband)
                gUsedCustomDefaultCaptainArmband = true;
        }
    }
    if (!gUsedCustomCaptainArmband)
        CallMethodDynGlobal(GfxCoreAddress(0x383619), _this, dataDesc, imageName, fshName);
}

void METHOD ApplyCaptainArmbandColor(void *_this, DUMMY_ARG, void *dataDesc, UInt color) {
    WriteToLog(L"ApplyCaptainArmbandColor");
    if (!gUsedCustomCaptainArmband && !gUsedCustomDefaultCaptainArmband)
        CallMethodDynGlobal(GfxCoreAddress(0x383BA7), _this, dataDesc, color);
    gUsedCustomCaptainArmband = false;
}

void InstallKits_FM13() {

    // custom captain armband

    //patch::RedirectCall(GfxCoreAddress(0x3728EB), OnGenerateKit);
    //patch::RedirectCall(GfxCoreAddress(0x38FF20), OnGenerateKit);
    //patch::RedirectCall(GfxCoreAddress(0x384D6F), ReadCaptainArmband);
    //patch::RedirectCall(GfxCoreAddress(0x384DA5), ApplyCaptainArmbandColor);

    // gk kit (doesn't work)

    //patch::Nop(GfxCoreAddress(0x38FC05), 1);
    //patch::SetUChar(GfxCoreAddress(0x38FC05 + 1), 0xE9);

    WriteToLog(L"Kits installed");
}

void PatchKits(FM::Version v) {
    //if (v.id() == ID_FM_13_1030_RLD) {
    //
    //}
}
