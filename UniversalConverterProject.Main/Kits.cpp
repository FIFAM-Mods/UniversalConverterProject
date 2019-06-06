#include "IncreaseSalaryOwnedClub.h"
#include "GfxCoreHook.h"
#include "Utils.h"
#include "FifamTypes.h"
#include "FifamReadWrite.h"
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
Bool gWritingKitNumbers = false;

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

Bool FmFileExists_FM11(Path const &filepath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3E5349));
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

Bool FmFileImageExists_FM11(String const &filepathWithoutExtension, String &resultPath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3E5349));
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

String GetUserKitNumberTexturePath(String const &directory, UInt compId, UInt clubId, String const &kitTypeStr, Int colorId) {
    String result;
    if (clubId > 0) {
        String clubIdStr = Utils::Format(L"%08X", clubId);
        if (compId != 0) {
            String compIdStr = Utils::Format(compId < 0xFFFF ? L"%04X_%d" : L"%08X_%d", compId, colorId);
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
    }
    else if (compId > 0) {
        String compIdStr = Utils::Format(compId < 0xFFFF ? L"%04X_%d" : L"%08X_%d", compId, colorId);
        if (FmFileImageExists(directory + L"\\" + compIdStr, result))
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

void METHOD OnGenerateKit_FM11(void *kitGen, DUMMY_ARG, void *kitDesc, void *kitParams, void *resMan) {
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
                void *match = *(void **)0x1516C08;
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
    CallMethodDynGlobal(GfxCoreAddress(0x3E615C), kitGen, kitDesc, kitParams, resMan);
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

void ApplyCaptainArmbandTexture_FM11(String const &customCaptainArmbandPath, void *shapeGen, void *dataDesc) {
    WriteToLog(Utils::Format(L"ReadCaptainArmband: customCaptainArmbandPath=%s", customCaptainArmbandPath));
    UInt acc;
    CallMethodDynGlobal(GfxCoreAddress(0x3E2E7E), &acc);
    gfx::RawImageDesc img;
    CallVirtualMethod<5>(acc, &img, customCaptainArmbandPath.c_str(), 256 * 64 * 4 + 256, 1, 0); // loadImage
    if (img.width == 256 && img.height == 64) {
        gfx::RawImageDesc *dstImg = *raw_ptr<gfx::RawImageDesc *>(dataDesc, 0x400);
        dstImg->width = 256;
        dstImg->height = 64;
        dstImg->stride = 256 * 4;
        CallMethodDynGlobal(GfxCoreAddress(0x3E5908), shapeGen, dstImg, &img); // copyPixels
        gUsedCustomCaptainArmband = true;
    }
    CallVirtualMethod<8>(acc, &img); // deleteImage
    CallMethodDynGlobal(GfxCoreAddress(0x3E2E7E), &acc);
}

void METHOD ReadCaptainArmband(void *_this, DUMMY_ARG, void *dataDesc, const Char * imageName, const WideChar * fshName) {
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

void METHOD ReadCaptainArmband_FM11(void *_this, DUMMY_ARG, void *dataDesc, const Char * imageName, const WideChar * fshName) {
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
        CallMethodDynGlobal(GfxCoreAddress(0x3E54AB), _this, dataDesc, imageName, fshName);
}

void METHOD ApplyCaptainArmbandColor(void *_this, DUMMY_ARG, void *dataDesc, UInt color) {
    WriteToLog(L"ApplyCaptainArmbandColor");
    if (!gUsedCustomCaptainArmband && !gUsedCustomDefaultCaptainArmband)
        CallMethodDynGlobal(GfxCoreAddress(0x383BA7), _this, dataDesc, color);
    gUsedCustomCaptainArmband = false;
}

void METHOD ApplyCaptainArmbandColor_FM11(void *_this, DUMMY_ARG, void *dataDesc, UInt color) {
    WriteToLog(L"ApplyCaptainArmbandColor");
    if (!gUsedCustomCaptainArmband && !gUsedCustomDefaultCaptainArmband)
        CallMethodDynGlobal(GfxCoreAddress(0x3E5AC0), _this, dataDesc, color);
    gUsedCustomCaptainArmband = false;
}

// Kits config

struct DefaultKitRenderData {
    UInt nShirtType;
    UInt nShortsType;
    UInt nSocksType;
    UInt nShirtColorPart1;
    UInt nShirtColorPart2;
    UInt nShirtColorPart3;
    UInt nShortsColorPart1;
    UInt nShortsColorPart2;
    UInt nShortsColorPart3;
    UInt nSocksColorPart1;
    UInt nSocksColorPart2;
    UInt nArmbandColor;
    UInt nShirtBadgePosition;
    UInt nShirtNumberColor;
    UInt nShirtNameColor;
    UInt nNamePlacement;
    Int field_40;
    WideChar field_44[260];
    WideChar szBadgePath[260];
    WideChar szUserKitPath[260];
};

struct team_kit_desc {
    unsigned char collar : 4;
    unsigned char nameplacement : 2;
    unsigned char namecase : 2;

    unsigned char jerseynumbercolor : 4; // 15 - unset
    unsigned char shortsnumbercolor : 4; // 15 - unset

    unsigned char jerseynamecolor : 7; // 127 - unset
    unsigned char frontnumber : 1;
    
    unsigned char shortsnumberhotspotid : 4; // 15 - unset
    unsigned char canusecompbadges : 1;
    unsigned char used : 1;

    team_kit_desc() {
        used = false;
    }
};

Map<UInt, team_kit_desc[4]> &GetTeamKitsMap() {
    static Map<UInt, team_kit_desc[4]> teamKitsMap;
    return teamKitsMap;
}

team_kit_desc *GetTeamKitInfo(UInt teamid, UInt kittype) {
    auto it = GetTeamKitsMap().find(teamid);
    if (it != GetTeamKitsMap().end()) {
        return &(*it).second[kittype];
    }
    return nullptr;
}

void SetVarInt(char const *varName, Int value) {
    CallVirtualMethod<1>(*(void **)0x30ABBC4, varName, value);
}

int gResultKits[2] = { 0, 0 };

void OnGetKitsFor3dMatch(DefaultKitRenderData *data, UInt *resultKits) {
    Call<0x440A00>(data, resultKits);
    gResultKits[0] = resultKits[0];
    gResultKits[1] = resultKits[1];

}

void *METHOD OnSetupKitForTeam1(void *team) {
    void *kit = CallMethodAndReturn<void *, 0xED3D60>(team);
    UChar color = CallMethodAndReturn<UChar, 0xFFCC50>(kit, gResultKits[0] ? 1 : 0);
    SetVarInt("HOME_KITTYPE", color);
    return kit;
}

void *METHOD OnSetupKitForTeam2(void *team) {
    void *kit = CallMethodAndReturn<void *, 0xED3D60>(team);
    UChar color = CallMethodAndReturn<UChar, 0xFFCC50>(kit, gResultKits[1] ? 1 : 0);
    SetVarInt("AWAY_KITTYPE", color);
    return kit;
}

void METHOD OnSetupKitForTeams_Highlights(WideChar *dst, WideChar *src) {
    Call<0x44B2C0>(dst, src);
    void *match = *(void **)0x3124748;
    UChar homeColor = 0;
    UChar awayColor = 0;
    void *gfxCoreInterface = *reinterpret_cast<void **>(0x30ABBD0);
    void *aiInterface = CallVirtualMethodAndReturn<void *, 7>(gfxCoreInterface);
    void *matchData = CallVirtualMethodAndReturn<void *, 65>(aiInterface);
    void *kitsData = raw_ptr<void *>(matchData, 0x30);
    UInt homeTeamKitId = *raw_ptr<UInt>(kitsData, 0xED8);
    UInt awayTeamKitId = *raw_ptr<UInt>(kitsData, 0xEDC);
    if (match) {
        void *homeTeam = CallMethodAndReturn<void *, 0xE7FD10>(match, true);
        if (homeTeam) {
            void *kit = CallMethodAndReturn<void *, 0xED3D60>(homeTeam);
            homeColor = CallMethodAndReturn<UChar, 0xFFCC50>(kit, homeTeamKitId ? 1 : 0);
        }
        void *awayTeam = CallMethodAndReturn<void *, 0xE7FD10>(match, false);
        if (homeTeam) {
            void *kit = CallMethodAndReturn<void *, 0xED3D60>(homeTeam);
            awayColor = CallMethodAndReturn<UChar, 0xFFCC50>(kit, awayTeamKitId ? 1 : 0);
        }
    }
    SetVarInt("HOME_KITTYPE", homeColor);
    SetVarInt("AWAY_KITTYPE", awayColor);
}

Int gfxGetVarInt(char const *varName, Int defaultValue) {
    void *expVars = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x239120));
    return CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x239370), expVars, varName, defaultValue);
}

int gfxOnSetupKitParametersTeam1(void *desc, int a2, int a3, int kitType) {
    int result = CallAndReturnDynGlobal<int>(GfxCoreAddress(0x23F190), desc, a2, a3, kitType);
    int used = gfxGetVarInt("HOME_KIT_CONFIG", 0);
    if (used == 1) {
        *raw_ptr<Int>(desc, 0x40) = gfxGetVarInt("HOME_COLLAR", 0);
        *raw_ptr<Int>(desc, 0x54) = gfxGetVarInt("HOME_NAMEPLACEMENT", 0);
        *raw_ptr<Int>(desc, 0x4C) = gfxGetVarInt("HOME_FRONTNUMBER", 0);
    }
    return result;
}

int gfxOnSetupKitParametersTeam2(void *desc, int a2, int a3, int kitType) {
    int result = CallAndReturnDynGlobal<int>(GfxCoreAddress(0x23F190), desc, a2, a3, kitType);
    int used = gfxGetVarInt("AWAY_KIT_CONFIG", 0);
    if (used == 1) {
        *raw_ptr<Int>(desc, 0x40) = gfxGetVarInt("AWAY_COLLAR", 0);
        *raw_ptr<Int>(desc, 0x54) = gfxGetVarInt("AWAY_NAMEPLACEMENT", 0);
        *raw_ptr<Int>(desc, 0x4C) = gfxGetVarInt("AWAY_FRONTNUMBER", 0);
    }
    return result;
}

void ReadKitsFile() {
    FifamReader reader(L"plugins\\ucp\\kits.csv", 14);
    if (reader.Available()) {
        reader.SkipLine();
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                String d;
                UInt teamid = 0;
                UChar kittype = 0;
                UChar collar = 0;
                UChar nameplacement = 0;
                Bool frontnumber = false;
                Int jerseynumbercolor = -1;
                Int jerseynamecolor = -1;
                Int shortsnumbercolor = -1;
                Int shortsnumberhotspotid = -1;
                UChar namecase = 0;
                Bool canusecompbadges = false;
                reader.ReadLine(d, d, d, Hexadecimal(teamid), kittype, collar, nameplacement, frontnumber,
                    jerseynumbercolor, jerseynamecolor, shortsnumbercolor, shortsnumberhotspotid, namecase, canusecompbadges);
                if (teamid != 0 && kittype <= 3) {
                    auto &kits = GetTeamKitsMap()[teamid];
                    auto &kitDesc = kits[kittype];
                    kitDesc.used = true;
                    if (collar > 8)
                        collar = 0;
                    kitDesc.collar = collar;
                    if (nameplacement > 2)
                        nameplacement = 0;
                    kitDesc.nameplacement = nameplacement;
                    kitDesc.frontnumber = frontnumber;
                    if (jerseynumbercolor < 1 || jerseynumbercolor > 6)
                        jerseynumbercolor = 15;
                    kitDesc.jerseynumbercolor = jerseynumbercolor;
                    if (jerseynamecolor < 1 || jerseynamecolor > 64)
                        jerseynamecolor = 127;
                    if (jerseynamecolor == 127 && jerseynumbercolor != 15) {
                        switch (jerseynumbercolor) {
                        case 1:
                            jerseynamecolor = 1;
                            break;
                        case 2:
                            jerseynamecolor = 2;
                            break;
                        case 3:
                            jerseynamecolor = 5;
                            break;
                        case 4:
                            jerseynamecolor = 19;
                            break;
                        case 5:
                            jerseynamecolor = 47;
                            break;
                        case 6:
                            jerseynamecolor = 59;
                            break;
                        }
                    }
                    kitDesc.jerseynamecolor = jerseynamecolor;
                    if (shortsnumbercolor < 1 || shortsnumbercolor > 6)
                        shortsnumbercolor = 15;
                    kitDesc.shortsnumbercolor = shortsnumbercolor;
                    if (shortsnumberhotspotid < 0 || shortsnumberhotspotid > 15)
                        shortsnumberhotspotid = 15;
                    kitDesc.shortsnumberhotspotid = shortsnumberhotspotid;
                    if (namecase > 2)
                        namecase = 0;
                    kitDesc.namecase = namecase;
                    kitDesc.canusecompbadges = canusecompbadges;
                }
            }
            else
                reader.SkipLine();
        }
    }
}

wchar_t const *gDynamicTextures[] = {
    L"GenKitHome.big",
    L"GenKitAway.big",
    L"BannerHome.big",
    L"BannerAway.big",
    L"Adboards.big",
    L"GenKitGkHome.big",
    L"GenKitGkAway.big",
    L"JNumHome.big",
    L"JNumGkHome.big",
    L"JNumAway.big",
    L"JNumGkAway.big"
};

struct DynTexAdditionalData {
    Int homeTeamGkKitFifaId;
    Int awayTeamGkKitFifaId;
    Int homeTeamJerseyNumFifaId;
    Int homeTeamGkJerseyNumFifaId;
    Int awayTeamJerseyNumFifaId;
    Int awayTeamGkJerseyNumFifaId;
    team_kit_desc homeTeamKitDesc;
    team_kit_desc homeTeamGkKitDesc;
    team_kit_desc awayTeamKitDesc;
    team_kit_desc awayTeamGkKitDesc;
};

const size_t numDynTextures = std::size(gDynamicTextures);
const size_t dynTexOffset = 0x3A8;
const size_t dynTexCountOffset = dynTexOffset + numDynTextures * 4;
const size_t dynTexAdditionalDataOffset = dynTexCountOffset + 4;
const size_t dynTexNewSize = dynTexAdditionalDataOffset + sizeof(DynTexAdditionalData);

struct PlayerAppearanceRenderData {
    UInt nHairStyle;
    UInt field_4;
    UInt field_8;
    UInt nFace;
    UInt field_10;
    UInt field_14;
    UInt field_18;
    UInt field_1C;
    UInt field_20;
};

struct PlayerRenderData {
    UInt nFlags;
    Int nFifaFaceId;
    PlayerAppearanceRenderData appearance;
    Int nHeight;
    Int nFifaKitId;
    Bool bHasUserKit;
    UChar pad[3];
};

struct KitFileDesc {
    UInt nKitType;
    Int nFifaId;
    WideChar const *pBigFileName;
    DefaultKitRenderData *pData;
    Int field_10;
    WideChar const *pszBadgePath;
    WideChar const *pszUserKitPath;
};

struct FmStreamHandle {
    void *pStream;
    void *pFileSystem;
};

UChar GetFifaKitNumberIdFromFifamId(UChar id) {
    static const UChar fifaIds[] = { 1, 2, 5, 4, 6, 3 };
    if (id > 0 && id <= std::size(fifaIds))
        return fifaIds[id - 1];
    return fifaIds[0];
}

void SetupKitNumberForKit(UChar kitType, UInt kitNumberFifaId, team_kit_desc &kitDesc) {
    if (kitNumberFifaId > 0)
        *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * kitType + 0x5C) = kitNumberFifaId;
    else if (kitDesc.used && kitDesc.jerseynumbercolor != 15)
        * raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * kitType + 0x5C) = GetFifaKitNumberIdFromFifamId(kitDesc.jerseynumbercolor);
}

void METHOD OnSetupDynamicTexturesFor3dMatch(void *dynTextures, DUMMY_ARG, Bool localizeKits) {
    DynTexAdditionalData *additionalData = raw_ptr<DynTexAdditionalData>(dynTextures, dynTexAdditionalDataOffset);
    if (additionalData->homeTeamGkKitFifaId > 0)
        *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * 1 + 0x2C) = additionalData->homeTeamGkKitFifaId;
    if (additionalData->awayTeamGkKitFifaId > 0)
        *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * 3 + 0x2C) = additionalData->awayTeamGkKitFifaId;

    SetupKitNumberForKit(0, additionalData->homeTeamJerseyNumFifaId, additionalData->homeTeamKitDesc);
    SetupKitNumberForKit(1, additionalData->homeTeamGkJerseyNumFifaId, additionalData->homeTeamGkKitDesc);
    SetupKitNumberForKit(2, additionalData->awayTeamJerseyNumFifaId, additionalData->awayTeamKitDesc);
    SetupKitNumberForKit(3, additionalData->awayTeamGkJerseyNumFifaId, additionalData->awayTeamGkKitDesc);

    for (UInt side = 0; side < 2; side++) {
        for (UInt type = 0; type < 2; type++) {
            team_kit_desc &desc = side ?
                (type ? additionalData->awayTeamGkKitDesc : additionalData->awayTeamKitDesc) :
                (type ? additionalData->homeTeamGkKitDesc : additionalData->homeTeamKitDesc);
            UInt kitid = side * 2 + type;
            Bool customNameColor = false;
            if (desc.used) {
                *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * kitid + 0x40) = desc.collar;
                *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * kitid + 0x54) = desc.nameplacement;
                *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * kitid + 0x4C) = desc.frontnumber;
                if (desc.jerseynamecolor != 127) {
                    *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * kitid + 0x10) = desc.jerseynamecolor;
                    customNameColor = true;
                }
            }
            if (!customNameColor && type == 1) {
                Bool setBlackNameColor = false;
                if (side == 0)
                    setBlackNameColor = additionalData->homeTeamGkJerseyNumFifaId > 0;
                else
                    setBlackNameColor = additionalData->awayTeamGkJerseyNumFifaId > 0;
                if (setBlackNameColor)
                    * raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * kitid + 0x10) = 2;
            }
        }
    }

    CallMethodDynGlobal(GfxCoreAddress(0x27CDC0), dynTextures, localizeKits);
}

Bool GetCustomKitTeamAndType(WideChar *in, UInt &teamId, UChar &kitType) {
    if (in && in[0]) {
        Path kitPath = in;
        String kitName = kitPath.stem().c_str();
        if (kitName.size() > 2 && kitName[kitName.size() - 2] == L'_') {
            UInt id = Utils::SafeConvertInt<UInt>(kitName.substr(0, kitName.size() - 2), true);
            if (id > 0) {
                Int type = -1;
                WideChar ckit = kitName[kitName.size() - 1];
                if (ckit == L'h')
                    type = 0;
                else if (ckit == L'a')
                    type = 1;
                else if (ckit == L'g')
                    type = 2;
                else if (ckit == L't')
                    type = 3;
                if (type >= 0) {
                    teamId = id;
                    kitType = type;
                    return true;
                }
            }
        }
    }
    return false;
}

Bool GetCustomGkKitPathFromKitPath(WideChar *in, Path &out, UInt &teamId, UChar &kitType) {
    UInt id;
    UChar type;
    if (GetCustomKitTeamAndType(in, id, type) && (type == 0 || type == 1 || type == 3)) {
        Path kitPath = in;
        teamId = id;
        kitType = type;
        out = kitPath.parent_path() / (Utils::Format(L"%08X_g", id) + kitPath.extension().c_str());
        return true;
    }
    return false;
}

void METHOD OnGenerateDynamicTextures(void *dynTextures, DUMMY_ARG, void *genData) {
    DynTexAdditionalData *additionalData = raw_ptr<DynTexAdditionalData>(dynTextures, dynTexAdditionalDataOffset);
    additionalData->homeTeamGkKitFifaId = 0;
    additionalData->awayTeamGkKitFifaId = 0;
    additionalData->homeTeamJerseyNumFifaId = 0;
    additionalData->homeTeamGkJerseyNumFifaId = 0;
    additionalData->awayTeamJerseyNumFifaId = 0;
    additionalData->awayTeamGkJerseyNumFifaId = 0;
    additionalData->homeTeamKitDesc.used = false;
    additionalData->homeTeamGkKitDesc.used = false;
    additionalData->awayTeamKitDesc.used = false;
    additionalData->awayTeamGkKitDesc.used = false;

    CallMethodDynGlobal(GfxCoreAddress(0x27D180), dynTextures, genData);

    UInt homeTeamId = 0;
    UInt awayTeamId = 0;
    UChar homeTeamKitType = 0;
    UChar awayTeamKitType = 0;

    WideChar bigFileName[260];
    PlayerRenderData plyrData;
    plyrData.nHeight = 180;
    plyrData.nFifaKitId = -1;
    plyrData.bHasUserKit = true;
    plyrData.nFlags = 1;
    for (UInt i = 0; i < 2; i++) {
        DefaultKitRenderData *kitData = *raw_ptr<DefaultKitRenderData *>(genData, (i == 0)? 0x10 : 0x18);
        Path gkPath;
        if (kitData && GetCustomGkKitPathFromKitPath(kitData->szUserKitPath, gkPath, (i == 0) ? homeTeamId : awayTeamId, (i == 0) ? homeTeamKitType : awayTeamKitType)) {
            if (FmFileExists(gkPath)) {
                KitFileDesc kitDesc;
                swprintf(bigFileName, L"%s\\%s", raw_ptr<WideChar>(dynTextures, 0x150), (i == 0) ? L"GenKitGkHome.big" : L"GenKitGkAway.big");
                kitDesc.pBigFileName = bigFileName;
                kitDesc.nFifaId = 201010 + i;
                kitDesc.pData = kitData;
                kitDesc.pszBadgePath = nullptr;
                kitDesc.field_10 = *raw_ptr<Int>(kitData, 0x44);
                kitDesc.pszUserKitPath = gkPath.c_str();
                kitDesc.nKitType = 2;
                CallVirtualMethod<1>(*(void **)(GfxCoreAddress(0xC07ED4)), &kitDesc, &plyrData, 0);
                Int * pNumTextures = raw_ptr<Int>(dynTextures, dynTexCountOffset);
                if (*pNumTextures < numDynTextures) {
                    WriteToLog(Utils::Format(L"OK: numTextures=%d", *pNumTextures));
                    Char *pathMb = CallAndReturnDynGlobal<Char *>(GfxCoreAddress(0x3A1707), kitDesc.pBigFileName);
                    WriteToLog(Utils::Format(L"path: %s", AtoW(pathMb)));
                    if (CallAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3129F0), pathMb, 0, 100, raw_ptr<Int>(dynTextures, dynTexOffset + *pNumTextures * 4)))
                        * pNumTextures += 1;
                }
                else {
                    WriteToLog(Utils::Format(L"FAILED: numTextures=%d", *pNumTextures));
                }
                if (i == 0)
                    additionalData->homeTeamGkKitFifaId = kitDesc.nFifaId;
                else
                    additionalData->awayTeamGkKitFifaId = kitDesc.nFifaId;
            }
        }
    }

    // comp id
    UInt compId = 0;
    void *match = *(void **)0x3124748;
    if (match) {
        CallMethod<0xE80190>(match, &compId);
        if (compId > 0xFFFF) {
            UInt compRegion = (compId >> 24) & 0xFF;
            if (compRegion <= 0 || compRegion > 207)
                compId = (compId >> 16) & 0xFFFF;
        }
    }

    for (UInt i = 0; i < 2; i++) {
        UInt teamId = (i == 0) ? homeTeamId : awayTeamId;
        UChar kitType = (i == 0) ? homeTeamKitType : awayTeamKitType;
        UChar kitNumberColor = gfxGetVarInt((i == 0) ? "HOME_KITTYPE" : "AWAY_KITTYPE", 1);
        for (int k = 0; k < 2; k++) {
            team_kit_desc *desc = nullptr;
            if (teamId > 0)
                desc = GetTeamKitInfo(teamId, (k == 0) ? kitType : 2);
            if (desc && desc->used) {
                if (i == 0) {
                    if (k == 0)
                        additionalData->homeTeamKitDesc = *desc;
                    else
                        additionalData->homeTeamGkKitDesc = *desc;
                }
                else {
                    if (k == 0)
                        additionalData->awayTeamKitDesc = *desc;
                    else
                        additionalData->awayTeamGkKitDesc = *desc;
                }

            }
            Int colorId = 1;
            if (desc && desc->used && desc->jerseynumbercolor != 15)
                colorId = desc->jerseynumbercolor;
            else {
                if (k == 0 && kitType != 3) {
                    if (kitNumberColor >= 1 && kitNumberColor <= 6)
                        colorId = kitNumberColor;
                }
                else
                    colorId = 2;
            }
            String kitTypeStr;
            if (k == 0) {
                if (kitType == 1)
                    kitTypeStr = L"_a";
                else if (kitType == 3)
                    kitTypeStr = L"_t";
                else
                    kitTypeStr = L"_h";
            }
            else
                kitTypeStr = L"_g";
            String numpath = GetUserKitNumberTexturePath(L"data\\kitnumbers\\jersey", compId, teamId, kitTypeStr, colorId);
            WriteToLog(numpath);
            if (!numpath.empty() && FmFileExists(numpath)) {
                UInt acc;
                CallMethodDynGlobal(GfxCoreAddress(0x373B6E), &acc);
                gfx::RawImageDesc img;
                CallVirtualMethod<5>(acc, &img, numpath.c_str(), 1024 * 128 * 4 + 256, 1, 0); // loadImage
                WriteToLog(Utils::Format(L"%dx%d", img.width, img.height));
                if (img.width == 1024 && img.height == 128) {
                    void *fs = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x3859EA));
                    WideChar *numbigname = nullptr;
                    UInt numid = 0;
                    if (i == 0) {
                        if (k == 0) {
                            numbigname = L"JNumHome.big";
                            numid = 201020;
                            additionalData->homeTeamJerseyNumFifaId = numid;
                        }
                        else {
                            numbigname = L"JNumGkHome.big";
                            numid = 201030;
                            additionalData->homeTeamGkJerseyNumFifaId = numid;
                        }
                    }
                    else {
                        if (k == 0) {
                            numbigname = L"JNumAway.big";
                            numid = 201021;
                            additionalData->awayTeamJerseyNumFifaId = numid;
                        }
                        else {
                            numbigname = L"JNumGkAway.big";
                            numid = 201031;
                            additionalData->awayTeamGkJerseyNumFifaId = numid;
                        }
                    }
                    swprintf(bigFileName, L"%s\\%s", raw_ptr<WideChar>(dynTextures, 0x150), numbigname);
                    FmStreamHandle fileHandle;
                    fileHandle.pStream = CallVirtualMethodAndReturn<void *, 3>(fs, bigFileName, 1);
                    fileHandle.pFileSystem = fs;
                    Char numfshname[128];
                    sprintf(numfshname, "t70__%d.fsh", numid);
                    gWritingKitNumbers = true;
                    Char shapeWriter[0x500];
                    CallMethodDynGlobal(GfxCoreAddress(0x387018), shapeWriter, fileHandle.pStream, 1, numfshname, 1);
                    CallMethodDynGlobal(GfxCoreAddress(0x3876A5), shapeWriter, &img, "JNum", 3);
                    CallMethodDynGlobal(GfxCoreAddress(0x38710C), shapeWriter);
                    CallMethodDynGlobal(GfxCoreAddress(0x373730), &fileHandle);
                    gWritingKitNumbers = false;
                    Int *pNumTextures = raw_ptr<Int>(dynTextures, dynTexCountOffset);
                    if (*pNumTextures < numDynTextures) {
                        WriteToLog(Utils::Format(L"OK: numTextures=%d", *pNumTextures));
                        Char *pathMb = CallAndReturnDynGlobal<Char *>(GfxCoreAddress(0x3A1707), bigFileName);
                        WriteToLog(Utils::Format(L"path: %s", AtoW(pathMb)));
                        if (CallAndReturnDynGlobal<Bool>(GfxCoreAddress(0x3129F0), pathMb, 0, 100, raw_ptr<Int>(dynTextures, dynTexOffset + *pNumTextures * 4)))
                            * pNumTextures += 1;
                    }
                    else
                        WriteToLog(Utils::Format(L"FAILED: numTextures=%d", *pNumTextures));
                }
                CallVirtualMethod<8>(acc, &img); // deleteImage
                CallMethodDynGlobal(GfxCoreAddress(0x373BA9), &acc);
            }
        }
    }

    OnSetupDynamicTexturesFor3dMatch(dynTextures, 0, false);
}

void METHOD SetGkKitFileName(PlayerRenderData *data, DUMMY_ARG, Char *dst, Char const *format, Int genKitId) {
    if (data->nFifaKitId >= 200000)
        sprintf(dst, format, data->nFifaKitId);
    else
        sprintf(dst, format, genKitId);
}

void SetGkKitFileName_Generic(Char *dst, Char const *format, Int genKitId) {
    if (genKitId >= (201000 + 5000))
        sprintf(dst, format, 5555555);
    else
        sprintf(dst, format, (genKitId & 0xF) + 5000);
}

UChar kitnumberhotspot[] = {
   0x7C, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x39, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00,
   0x99, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
   0x38, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x99, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x67, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x37, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00,
   0x66, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
   0x36, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x66, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x35, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00,
   0xCC, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
   0x34, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x67, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x33, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
   0x32, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x66, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x31, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00,
   0x33, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
   0x30, 0x75, 0x6E, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x66, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void METHOD AddShapeWriterSections(void *shapeGen, DUMMY_ARG, void *sectionData) {
    CallMethodDynGlobal(GfxCoreAddress(0x387F36), shapeGen, sectionData);
    // add hotspot for kitnumbers, if needed
    if (gWritingKitNumbers) {
        Int sectionId = 0x7C;
        CallMethodDynGlobal(GfxCoreAddress(0x387502), shapeGen, &sectionId);
        gfx::Buffer buf;
        buf.data = kitnumberhotspot;
        buf.size = std::size(kitnumberhotspot);
        CallMethodDynGlobal(GfxCoreAddress(0x387489), shapeGen, &buf);
    }
}

UChar SetupPlayerAndKitForRender(Int playerId, Bool bHomeKit, void *teamIndex, PlayerRenderData *playerRenderData, DefaultKitRenderData *kitRenderData) {
    UChar result = CallAndReturn<UChar, 0xD33B40>(playerId, bHomeKit, teamIndex, playerRenderData, kitRenderData);
    if ((playerRenderData->nFlags & 1) && playerRenderData->bHasUserKit) {
        Path gkPath;
        UInt teamId;
        UChar kitType;
        if (GetCustomGkKitPathFromKitPath(kitRenderData->szUserKitPath, gkPath, teamId, kitType) && FmFileExists(gkPath))
            wcscpy(kitRenderData->szUserKitPath, gkPath.c_str());
    }
    return result;
}

PlayerRenderData *gRenderPlayerDataForGkKit = nullptr;
DefaultKitRenderData *gRenderKitDataForGkKit = nullptr;

void *OnGenerateKitResourceForScenes(void *out, PlayerRenderData *kitParams, DefaultKitRenderData *data, void *resMan) {
    gRenderPlayerDataForGkKit = kitParams;
    gRenderKitDataForGkKit = data;
    return CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x38FAEE), out, kitParams, data, resMan);
}

Bool UseGenericKit() {
    if (gRenderPlayerDataForGkKit && gRenderKitDataForGkKit && gRenderPlayerDataForGkKit->nFlags & 1) {
        if (!gRenderPlayerDataForGkKit->bHasUserKit)
            return true;
        if (!gRenderKitDataForGkKit->szUserKitPath[0])
            return true;
        UInt teamId = 0;
        UChar kitType = 0;
        if (!GetCustomKitTeamAndType(gRenderKitDataForGkKit->szUserKitPath, teamId, kitType))
            return true;
        //WriteToLog(Utils::Format(L"kitType: %d, path: %s, generic: %d", kitType, gRenderKitDataForGkKit->szUserKitPath, kitType != 2));
        if (kitType != 2)
            return true;
    }
    return false;
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

    // kit config
    patch::RedirectCall(GfxCoreAddress(0x23FF3D), gfxOnSetupKitParametersTeam1);
    patch::RedirectCall(GfxCoreAddress(0x23FF7C), gfxOnSetupKitParametersTeam2);

    // dynamic textures

    patch::SetPointer(GfxCoreAddress(0x27CC10 + 2), gDynamicTextures);
    patch::SetUChar(GfxCoreAddress(0x27CC45 + 2), numDynTextures);
    patch::SetUChar(GfxCoreAddress(0x27D2DF + 2), numDynTextures);
    patch::SetUChar(GfxCoreAddress(0x27D3BF + 2), numDynTextures);
    patch::SetUChar(GfxCoreAddress(0x27D42A + 2), numDynTextures);
    patch::SetUChar(GfxCoreAddress(0x27D497 + 2), numDynTextures);
    patch::SetUChar(GfxCoreAddress(0x27D53A + 2), numDynTextures);
    patch::SetUInt(GfxCoreAddress(0x27D5D0 + 1), dynTexNewSize);
    patch::SetUInt(GfxCoreAddress(0x27D5F3 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27CBB9 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27CBD0 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27CBD6 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27CBEB + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27CBFF + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D2D9 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D30A + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D3B9 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D3EA + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D421 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D455 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D48E + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D4C2 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D531 + 2), dynTexCountOffset);
    patch::SetUInt(GfxCoreAddress(0x27D561 + 2), dynTexCountOffset);

    patch::Nop(GfxCoreAddress(0x27D567), 9);
    patch::RedirectCall(GfxCoreAddress(0x27D60C), OnGenerateDynamicTextures);
    patch::RedirectCall(GfxCoreAddress(0x27D16C), OnSetupDynamicTexturesFor3dMatch);
    //patch::RedirectCall(GfxCoreAddress(0x27D56B), OnSetupDynamicTexturesFor3dMatch); - we nop'ed it

    patch::Nop(GfxCoreAddress(0x370CBD), 3);
    patch::RedirectCall(GfxCoreAddress(0x370CB8), SetGkKitFileName);

    patch::RedirectCall(GfxCoreAddress(0x387DE8), AddShapeWriterSections);

    // fix number/name colors for custom kits
    patch::Nop(GfxCoreAddress(0x23F48F), 2); // don't rewrite edi
    patch::SetUChar(GfxCoreAddress(0x23F491), 0x50); // push eax

    // other scenes (not 3d-match)
    patch::RedirectCall(GfxCoreAddress(0x379170), OnGenerateKitResourceForScenes);
    patch::RedirectCall(GfxCoreAddress(0x37968A), OnGenerateKitResourceForScenes);
    patch::RedirectCall(GfxCoreAddress(0x37974F), OnGenerateKitResourceForScenes);
    patch::RedirectCall(GfxCoreAddress(0x37A26C), OnGenerateKitResourceForScenes);
    patch::RedirectCall(GfxCoreAddress(0x38FBFD), UseGenericKit);

    patch::Nop(GfxCoreAddress(0x370B5C), 8);
    patch::RedirectCall(GfxCoreAddress(0x370B72), SetGkKitFileName_Generic);
}

void InstallKits_FM11() {

    // custom captain armband

    //patch::RedirectCall(GfxCoreAddress(0x3E2DA8), OnGenerateKit_FM11);
    //patch::RedirectCall(GfxCoreAddress(0x3F6B48), OnGenerateKit_FM11);
    //patch::RedirectCall(GfxCoreAddress(0x3E67C5), ReadCaptainArmband_FM11);
    //patch::RedirectCall(GfxCoreAddress(0x3E67D8), ApplyCaptainArmbandColor_FM11);

    // gk kit

    //patch::Nop(GfxCoreAddress(0x38FC05), 1);
    //patch::SetUChar(GfxCoreAddress(0x38FC05 + 1), 0xE9);
}

void PatchKits(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadKitsFile();
        patch::RedirectCall(0x44D4F0, OnGetKitsFor3dMatch);
        patch::RedirectCall(0x44D73D, OnSetupKitForTeam1);
        patch::RedirectCall(0x44D751, OnSetupKitForTeam2);
        patch::RedirectCall(0x44BE33, OnSetupKitForTeams_Highlights);

        // other scenes (not 3d-match)
        patch::RedirectCall(0x66224B, SetupPlayerAndKitForRender);
        patch::RedirectCall(0x66443C, SetupPlayerAndKitForRender);
        patch::RedirectCall(0x89684D, SetupPlayerAndKitForRender);
        patch::RedirectCall(0x8CAC9B, SetupPlayerAndKitForRender);
        patch::RedirectCall(0x98BFE2, SetupPlayerAndKitForRender);
        patch::RedirectCall(0xBA74AA, SetupPlayerAndKitForRender);
        patch::RedirectCall(0xD39C7E, SetupPlayerAndKitForRender);
        patch::RedirectCall(0xD3D3FE, SetupPlayerAndKitForRender);
        patch::RedirectCall(0xD3D6DC, SetupPlayerAndKitForRender);
    }
}
