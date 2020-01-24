#include "Kits.h"
#include "GfxCoreHook.h"
#include "Utils.h"
#include "FifamTypes.h"
#include "FifamReadWrite.h"
#include "GameInterfaces.h"
#include "shared.h"
#include "Random.h"
#include "Settings.h"
#include "FifamCompetition.h"

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
void *gKitGen = nullptr;
Bool gbUserKit = false;
void *gKitFileDesc = nullptr;

//#define KITS_DEBUG

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

Bool UserKitAvailable() {
    return !gKitTypeStr.empty() && (gCurrCompId != 0 || gTeamId != 0);
}

String GetUserTexturePath(String const &directory, UInt compId, UInt clubId, String const &kitTypeStr) {
    String result;
    if (clubId > 0) {
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
    }
    else if (compId != 0) {
        String compIdStr = Utils::Format(compId < 0xFFFF ? L"%04X" : L"%08X", compId);
        if (FmFileImageExists(directory + L"\\" + compIdStr, result))
            return result;
    }
    return String();
}

String GetUserKitNumberTexturePath(String const &directory, UInt compId, UInt clubId, String const &kitTypeStr, Int colorId) {
    String result;
    if (clubId > 0) {
        String clubIdStr = Utils::Format(L"%08X", clubId);
        if (compId != 0) {
            String compIdStr = Utils::Format(compId < 0xFFFF ? L"%04X" : L"%08X", compId);
            if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr + kitTypeStr, result)) // 2D010000_002D000E_h
                return result;
            if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr, result)) // 2D010000_002D000E
                return result;
            if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + Utils::Format(L"%d", colorId), result)) // 2D010000_1
                return result;
        }
        if (FmFileImageExists(directory + L"\\" + clubIdStr + kitTypeStr, result)) // 002D000E_h
            return result;
        if (FmFileImageExists(directory + L"\\" + clubIdStr, result)) // 002D000E
            return result;
    }
    else if (compId != 0) {
        String compIdStr = Utils::Format(compId < 0xFFFF ? L"%04X_%d" : L"%08X_%d", compId, colorId); // 2D010000_1
        if (FmFileImageExists(directory + L"\\" + compIdStr, result))
            return result;
    }
    return String();
}

String GetUserCompBadgesTexturePath(String const &directory, UInt compId, Bool right, UInt clubId, String const &kitTypeStr, UInt lastSeasonYear, Bool isChampion, UInt numTitles, Bool has3inrow, Bool isELChampion) {
    if (compId != 0) {
        String result;
        if (compId == 0xF909) {
            // right: winner/EL winner
            // left: titles
            if (right) {
                if (isChampion) {
                    if (FmFileImageExists(directory + L"\\ChampionsLeague_Winner_" + Utils::Format(L"%u", lastSeasonYear), result))
                        return result;
                }
                else if (isELChampion) {
                    if (FmFileImageExists(directory + L"\\ChampionsLeague_EuropaLeagueWinner_" + Utils::Format(L"%u", lastSeasonYear), result))
                        return result;
                }
            }
            else {
                if (numTitles >= 4) {
                    if (FmFileImageExists(directory + L"\\ChampionsLeague_Titles_" + Utils::Format(L"%u", numTitles), result))
                        return result;
                }
                else if (has3inrow) {
                    if (FmFileImageExists(directory + L"\\ChampionsLeague_Titles_3", result))
                        return result;
                }
            }
        }
        else if (compId == 0xF90E) {
            // right: winner
            // left: titles
            if (right) {
                if (isChampion) {
                    if (FmFileImageExists(directory + L"\\EuropaLeague_Winner_" + Utils::Format(L"%u", lastSeasonYear), result))
                        return result;
                }
            }
            else {
                if (numTitles >= 5) {
                    if (FmFileImageExists(directory + L"\\EuropaLeague_Titles_" + Utils::Format(L"%u", numTitles), result))
                        return result;
                }
            }
        }
        String placement = right ? L"_r" : L"_l";
        String compIdStr = Utils::Format(compId < 0xFFFF ? L"%04X" : L"%08X", compId);
        if (clubId != 0) {
            String clubIdStr = Utils::Format(L"%08X", clubId);
            if (isChampion) {
                if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr + kitTypeStr + placement + L"_champion", result))
                    return result;
                if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr + placement + L"_champion", result))
                    return result;
                if (FmFileImageExists(directory + L"\\" + compIdStr + placement + L"_champion", result))
                    return result;
            }
            if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr + kitTypeStr + placement, result))
                return result;
            if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr + placement, result))
                return result;
            if (FmFileImageExists(directory + L"\\" + compIdStr + placement, result))
                return result;
        }
        else {
            if (FmFileImageExists(directory + L"\\" + compIdStr + placement, result))
                return result;
        }
    }
    return String();
}

void ClearKitParams() {
    gpUserKitName = nullptr;
    gTeamId = 0;
    gCurrCompId = 0;
    gKitTypeStr.clear();
    gbUserKit = false;
    gKitFileDesc = nullptr;
    gKitGen = nullptr;
}

void METHOD OnGenerateKit(void *kitGen, DUMMY_ARG, void *kitDesc, void *kitParams, void *resMan) {
    ClearKitParams();
    gKitGen = kitGen;
    gKitFileDesc = kitDesc;
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
            }
            if (*raw_ptr<UChar>(kitParams, 0x34) && FmFileExists(pUserKitPath))
                gbUserKit = true;
        }
    }
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

void METHOD ReadCaptainArmband(void *_this, DUMMY_ARG, void *dataDesc, const Char * imageName, const WideChar * fshName) {
    WriteToLog(L"ReadCaptainArmband");
    gUsedCustomCaptainArmband = false;
    if (UserKitAvailable() || gCurrCompId) {
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
    unsigned char frontnumber : 1;
    unsigned char used : 1;

    unsigned char jerseynumbercolor : 4; // 15 - unset
    unsigned char jerseynumbersize : 4; // 0 - default (max), 1 - min, 10 - max

    unsigned char jerseynamecolor : 7; // 127 - unset
    unsigned char canusecompbadges : 1;

    unsigned char jerseynumberoffset : 4; // 0 - min, 15 - max
    unsigned char canusesponsorlogo : 1;

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

int gResultKits[2] = { 0, 0 };

void OnGetKitsFor3dMatch(DefaultKitRenderData *data, UInt *resultKits) {
    Call<0x440A00>(data, resultKits);
    gResultKits[0] = resultKits[0];
    gResultKits[1] = resultKits[1];
}

void *METHOD OnSetupKitForTeam1(CDBTeam *team) {
    void *kit = CallMethodAndReturn<void *, 0xED3D60>(team);
    UChar color = CallMethodAndReturn<UChar, 0xFFCC50>(kit, gResultKits[0] ? 1 : 0);
    SetVarInt("HOME_JNUMCOLOR", color);
    SetVarInt("HOME_TEAMID", team->GetTeamUniqueID());
    SetVarInt("HOME_KITTYPE", gResultKits[0]);
    SetVarInt("HOME_USERKIT", CallMethodAndReturn<Bool, 0xFFD940>(kit, gResultKits[0] ? 1 : 0));
    Bool youth = 0;
    void *match = *(void **)0x3124748;
    if (match) {
        CTeamIndex teamIndex;
        CallMethod<0xE7FD30>(match, &teamIndex, true);
        youth = teamIndex.type == 2 || teamIndex.type == 4;
    }
    SetVarInt("HOME_YOUTH", youth);
    return kit;
}

void *METHOD OnSetupKitForTeam2(CDBTeam *team) {
    void *kit = CallMethodAndReturn<void *, 0xED3D60>(team);
    UChar color = CallMethodAndReturn<UChar, 0xFFCC50>(kit, gResultKits[1] ? 1 : 0);
    SetVarInt("AWAY_JNUMCOLOR", color);
    SetVarInt("AWAY_TEAMID", team->GetTeamUniqueID());
    SetVarInt("AWAY_KITTYPE", gResultKits[1]);
    SetVarInt("AWAY_USERKIT", CallMethodAndReturn<Bool, 0xFFD940>(kit, gResultKits[1] ? 1 : 0));
    Bool youth = 0;
    void *match = *(void **)0x3124748;
    if (match) {
        CTeamIndex teamIndex;
        CallMethod<0xE7FD30>(match, &teamIndex, false);
        youth = teamIndex.type == 2 || teamIndex.type == 4;
    }
    SetVarInt("AWAY_YOUTH", youth);
    return kit;
}

void OnSetupKitForTeams_Highlights(WideChar *dst, WideChar *src) {
    Call<0x44B2C0>(dst, src);
    void *match = *(void **)0x3124748;
    UChar homeColor = 0;
    UChar awayColor = 0;
    UInt homeTeamId = 0;
    UInt awayTeamId = 0;
    Bool homeUserKit = 0;
    Bool awayUserKit = 0;
    Bool homeYouth = 0;
    Bool awayYouth = 0;
    void *gfxCoreInterface = *reinterpret_cast<void **>(0x30ABBD0);
    void *aiInterface = CallVirtualMethodAndReturn<void *, 7>(gfxCoreInterface);
    void *matchData = CallVirtualMethodAndReturn<void *, 65>(aiInterface);
    void *kitsData = raw_ptr<void *>(matchData, 0x30);
    UInt homeTeamKitId = *raw_ptr<UInt>(kitsData, 0xED8);
    UInt awayTeamKitId = *raw_ptr<UInt>(kitsData, 0xEDC);
    if (match) {
        CDBTeam *homeTeam = CallMethodAndReturn<CDBTeam *, 0xE7FD10>(match, true);
        if (homeTeam) {
            void *kit = CallMethodAndReturn<void *, 0xED3D60>(homeTeam);
            homeTeamId = homeTeam->GetTeamUniqueID();
            homeColor = CallMethodAndReturn<UChar, 0xFFCC50>(kit, homeTeamKitId ? 1 : 0);
            homeUserKit = CallMethodAndReturn<Bool, 0xFFD940>(kit, homeTeamKitId ? 1 : 0);
            CTeamIndex teamIndex;
            CallMethod<0xE7FD30>(match, &teamIndex, true);
            homeYouth = teamIndex.type == 2 || teamIndex.type == 4;
        }
        CDBTeam *awayTeam = CallMethodAndReturn<CDBTeam *, 0xE7FD10>(match, false);
        if (awayTeam) {
            void *kit = CallMethodAndReturn<void *, 0xED3D60>(awayTeam);
            awayTeamId = awayTeam->GetTeamUniqueID();
            awayColor = CallMethodAndReturn<UChar, 0xFFCC50>(kit, awayTeamKitId ? 1 : 0);
            awayUserKit = CallMethodAndReturn<Bool, 0xFFD940>(kit, awayTeamKitId ? 1 : 0);
            CTeamIndex teamIndex;
            CallMethod<0xE7FD30>(match, &teamIndex, false);
            awayYouth = teamIndex.type == 2 || teamIndex.type == 4;
        }
    }
    SetVarInt("HOME_JNUMCOLOR", homeColor);
    SetVarInt("HOME_TEAMID", homeTeamId);
    SetVarInt("HOME_KITTYPE", homeTeamKitId);
    SetVarInt("AWAY_JNUMCOLOR", awayColor);
    SetVarInt("AWAY_TEAMID", awayTeamId);
    SetVarInt("AWAY_KITTYPE", awayTeamKitId);
    SetVarInt("HOME_USERKIT", homeUserKit);
    SetVarInt("AWAY_USERKIT", awayUserKit);
    SetVarInt("HOME_YOUTH", homeYouth);
    SetVarInt("AWAY_YOUTH", awayYouth);
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
                Bool canusecompbadges = false;
                Bool canusesponsorlogo = false;
                UChar jerseynumbersize = 0;
                UChar jerseynumberoffset = 0;
                reader.ReadLine(d, d, d, Hexadecimal(teamid), kittype, collar, nameplacement, frontnumber, jerseynumbercolor,
                    jerseynamecolor, jerseynumbersize, jerseynumberoffset, canusecompbadges, canusesponsorlogo);
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
                    if (jerseynumbersize > 10)
                        jerseynumbersize = 10;
                    kitDesc.jerseynumbersize = jerseynumbersize;
                    if (jerseynumberoffset > 15)
                        jerseynumberoffset = 15;
                    kitDesc.jerseynumberoffset = jerseynumberoffset;
                    kitDesc.canusecompbadges = canusecompbadges;
                    kitDesc.canusesponsorlogo = canusesponsorlogo;
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
            else {
                UInt homeTeamId = gfxGetVarInt("HOME_TEAMID", 0);
                if ((homeTeamId & 0xFFFF) == 0xFFFF) {
                    *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * 0 + 0x4C) = 1;
                    *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * 1 + 0x4C) = 1;
                }
                UInt awayTeamId = gfxGetVarInt("AWAY_TEAMID", 0);
                if ((awayTeamId & 0xFFFF) == 0xFFFF) {
                    *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * 2 + 0x4C) = 1;
                    *raw_ptr<Int>((void *)GfxCoreAddress(0xABEB50), 0x6C * 3 + 0x4C) = 1;
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
        if (kitData) {
            if (GetCustomGkKitPathFromKitPath(kitData->szUserKitPath, gkPath, (i == 0) ? homeTeamId : awayTeamId, (i == 0) ? homeTeamKitType : awayTeamKitType)) {
                if (FmFileExists(gkPath)) {
                    KitFileDesc kitDesc;
                    swprintf(bigFileName, L"%s\\%s", raw_ptr<WideChar>(dynTextures, 0x150), (i == 0) ? L"GenKitGkHome.big" : L"GenKitGkAway.big");
                    kitDesc.pBigFileName = bigFileName;
                    kitDesc.nFifaId = 201010 + i;
                    kitDesc.pData = kitData;
                    kitDesc.pszBadgePath = nullptr;
                    kitDesc.field_10 = *raw_ptr<Int>(kitData, 0x44);
                    kitDesc.pszUserKitPath = gkPath.c_str();
                    kitDesc.nKitType = 2 + i;
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
            if (i == 0) {
                if (homeTeamId > 0) {
                    SetVarInt("HOME_TEAMID", homeTeamId);
                    SetVarInt("HOME_KITTYPE", homeTeamKitType);
                }
            }
            else {
                if (awayTeamId > 0) {
                    SetVarInt("AWAY_TEAMID", awayTeamId);
                    SetVarInt("AWAY_KITTYPE", awayTeamKitType);
                }
            }
        }
    }

    // comp id
    UInt compId = 0;
    Bool isFriendly = false;
    void *match = *(void **)0x3124748;
    if (match) {
        CallMethod<0xE80190>(match, &compId);
        UInt compType = (compId >> 16) & 0xFF;
        if (compType == COMP_FRIENDLY || compType == COMP_INTERNATIONAL_FRIENDLY || compType == COMP_ICC)
            isFriendly = true;
        if (compId > 0xFFFF) {
            UInt compRegion = (compId >> 24) & 0xFF;
            if (compRegion <= 0 || compRegion > 207)
                compId = (compId >> 16) & 0xFFFF;
        }
    }

    for (UInt i = 0; i < 2; i++) {
        UInt teamId = (i == 0) ? homeTeamId : awayTeamId;
        UChar kitType = (i == 0) ? homeTeamKitType : awayTeamKitType;
        UChar kitNumberColor = gfxGetVarInt((i == 0) ? "HOME_JNUMCOLOR" : "AWAY_JNUMCOLOR", 1);
        Bool disableName = gfxGetVarInt((i == 0) ? "HOME_YOUTH" : "AWAY_YOUTH", 0);
        if (!disableName && isFriendly)
            disableName = true;
        for (int k = 0; k < 2; k++) {
            team_kit_desc *desc = nullptr;
            if (teamId > 0)
                desc = GetTeamKitInfo(teamId, (k == 0) ? kitType : 2);
            if (desc && desc->used) {
                if (i == 0) {
                    if (k == 0) {
                        additionalData->homeTeamKitDesc = *desc;
                        if (disableName)
                            additionalData->homeTeamKitDesc.nameplacement = 0;
                    }
                    else {
                        additionalData->homeTeamGkKitDesc = *desc;
                        if (disableName)
                            additionalData->homeTeamGkKitDesc.nameplacement = 0;
                    }
                }
                else {
                    if (k == 0) {
                        additionalData->awayTeamKitDesc = *desc;
                        if (disableName)
                            additionalData->awayTeamKitDesc.nameplacement = 0;
                    }
                    else {
                        additionalData->awayTeamGkKitDesc = *desc;
                        if (disableName)
                            additionalData->awayTeamGkKitDesc.nameplacement = 0;
                    }
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

void METHOD SetupJerseyNumberHotspot(UInt _this, DUMMY_ARG, void *playerDesc) {
    UInt kitId = *raw_ptr<UInt>(playerDesc, 0x12CC);
    team_kit_desc *kitDesc = nullptr;
    Bool userKit = false;

    if (kitId == 0 || kitId == 2) {
        UInt teamId = gfxGetVarInt((kitId == 0) ? "HOME_TEAMID" : "AWAY_TEAMID", 0);
        UInt kitType = gfxGetVarInt((kitId == 0) ? "HOME_KITTYPE" : "AWAY_KITTYPE", 99);
        if (teamId > 0 && kitType <= 3)
            kitDesc = GetTeamKitInfo(teamId, kitType);
        userKit = gfxGetVarInt((kitId == 0) ? "HOME_USERKIT" : "AWAY_USERKIT", 0);
    }
    else if (kitId == 1 || kitId == 3) {
        UInt teamId = gfxGetVarInt((kitId == 1) ? "HOME_TEAMID" : "AWAY_TEAMID", 0);
        if (teamId > 0)
            kitDesc = GetTeamKitInfo(teamId, 2);
        userKit = gfxGetVarInt((kitId == 1) ? "HOME_USERKIT" : "AWAY_USERKIT", 0);
    }

    static const UInt jnum_sizes[] = { 78, 84, 90, 96, 100, 104, 108, 112, 116, 120 };

    UInt height = 120;
    UInt offsety = 0;

    if (kitDesc && kitDesc->used) {
        
        if (kitDesc->jerseynumbersize == 0)
            height = 120;
        else if (kitDesc->jerseynumbersize <= 10)
            height = jnum_sizes[kitDesc->jerseynumbersize - 1];
        offsety = kitDesc->jerseynumberoffset;
    }

    UInt width = height / 2;

    *(UInt *)(160 * kitId + _this + 4) = 64 - width;
    *(UInt *)(160 * kitId + _this + 8) = offsety;
    *(UInt *)(160 * kitId + _this + 12) = width;
    *(UInt *)(160 * kitId + _this + 16) = height;

    *(UInt *)(160 * kitId + _this + 20) = 64; 
    *(UInt *)(160 * kitId + _this + 24) = offsety;
    *(UInt *)(160 * kitId + _this + 28) = width;
    *(UInt *)(160 * kitId + _this + 32) = height;

    *(UInt *)(160 * kitId + _this + 36) = 64 - width / 2;
    *(UInt *)(160 * kitId + _this + 40) = offsety;
    *(UInt *)(160 * kitId + _this + 44) = width;
    *(UInt *)(160 * kitId + _this + 48) = height;

    *(UInt *)(160 * kitId + _this) = 0;
}

void METHOD OnGetRefKitId(void *, DUMMY_ARG, const char *, int *outId, int) {
    UInt compId = 0;
    void *match = *(void **)0x3124748;
    if (match) {
        CallMethod<0xE80190>(match, &compId);
        if (compId == 0x0E010000) {
            static Array<UInt, 4> refKitIdsEPL = { 6105, 6106, 6108, 6109 };
            *outId = Random::SelectFromArray(refKitIdsEPL);
            return;
        }
        else if (compId >= 0x0E010001 && compId <= 0x0E010003) {
            static Array<UInt, 4> refKitIdsEFL = { 6114, 6115, 6116, 6117 };
            *outId = Random::SelectFromArray(refKitIdsEFL);
            return;
        }
        else if (compId == 0x0E030000) {
            static Array<UInt, 4> refKitIdsFA = { 6110, 6111, 6112, 6113 };
            *outId = Random::SelectFromArray(refKitIdsFA);
            return;
        }
        else {
            compId = (compId >> 16) & 0xFFFF;
            if (compId == 0xF909 || compId == 0xF90A || compId == 0xF90C || compId == 0xF909 || compId == 0xF90E || compId == 0xF926) {
                static Array<UInt, 4> refKitIdsFA = { 6019, 6020, 6021, 6022 };
                *outId = Random::SelectFromArray(refKitIdsFA);
                return;
            }
        }
    }
    static Array<UInt, 5> refKitIdsDefault = { 6004, 6005, 6006, 6007, 6009 };
    *outId = Random::SelectFromArray(refKitIdsDefault);
}

void METHOD OnWriteKitShape(void *shapeGen, DUMMY_ARG, gfx::RawImageDesc *dstImg, const char *fshName, int imageFormat) {
    SafeLog::Write(Utils::Format(L"kitGen %p kitFileDesc %p compId %08X", gKitGen, gKitFileDesc, gCurrCompId));
    if (gKitGen && gKitFileDesc) {
        team_kit_desc *kitDesc = nullptr;
        SafeLog::Write(Utils::Format(L"userKit %d", gbUserKit));
        Bool canUseCompBadges = gbUserKit == false;
        Bool canUseSponsorLogo = gbUserKit == false;
        UInt kitId = *raw_ptr<UInt>(gKitFileDesc, 0);
        UInt teamId = 0;
        UInt numberColor = 0;
        if (kitId == 0 || kitId == 1) {
            teamId = gfxGetVarInt((kitId == 0) ? "HOME_TEAMID" : "AWAY_TEAMID", 0);
            numberColor = gfxGetVarInt((kitId == 0) ? "HOME_JNUMCOLOR" : "AWAY_JNUMCOLOR", 0);
            if (numberColor > 6)
                numberColor = 1;
            UInt kitType = gfxGetVarInt((kitId == 0) ? "HOME_KITTYPE" : "AWAY_KITTYPE", 99);
            if (teamId > 0 && kitType <= 3)
                kitDesc = GetTeamKitInfo(teamId, kitType);
        }
        else if (kitId == 2 || kitId == 3) {
            teamId = gfxGetVarInt((kitId == 2) ? "HOME_TEAMID" : "AWAY_TEAMID", 0);
            numberColor = gfxGetVarInt((kitId == 2) ? "HOME_JNUMCOLOR" : "AWAY_JNUMCOLOR", 0);
            if (numberColor != 0)
                numberColor = 1;
            if (teamId > 0)
                kitDesc = GetTeamKitInfo(teamId, 2);
        }
        if (gbUserKit && kitDesc) {
            canUseCompBadges = kitDesc->canusecompbadges;
            canUseSponsorLogo = kitDesc->canusesponsorlogo;
        }
        CDBTeam *team = nullptr;
        if (teamId != 0)
            team = GetTeamByUniqueID(teamId & 0xFFFFFF);
        SafeLog::Write(Utils::Format(L"team %08X kitDesc %p canusecompbadges %d canUseSponsorLogo %d", teamId, kitDesc, canUseCompBadges, canUseSponsorLogo));
        if (gCurrCompId != 0 && canUseCompBadges) {
            String kitTypeStr;
            if (kitId == 0 || kitId == 1) {
                UInt kitType = gfxGetVarInt((kitId == 0) ? "HOME_KITTYPE" : "AWAY_KITTYPE", 99);
                switch (kitType) {
                case 0:
                    kitTypeStr = L"_h";
                    break;
                case 1:
                    kitTypeStr = L"_a";
                    break;
                case 2:
                    kitTypeStr = L"_g";
                    break;
                case 3:
                    kitTypeStr = L"_t";
                    break;
                }
            }
            else if (kitId == 2 || kitId == 3)
                kitTypeStr = L"_g";

            auto PutOverlay = [](void *sg, gfx::RawImageDesc *dst, Path const &overlayPath, UInt inputW, UInt inputH, UInt left, UInt top, UInt width, UInt height) {
                if (!overlayPath.empty()) {
                    WideChar tempPath[MAX_PATH + 1];
                    GetTempPathW(259, tempPath);
                    wcscat(tempPath, L"\\tempoverlay.fsh");
                    SafeLog::Write(Utils::Format(L"tempPath %s overlayPath %s", tempPath, overlayPath.c_str()));
                    UInt acc;
                    Bool createdTempImage = false;
                    CallMethodDynGlobal(GfxCoreAddress(0x373B6E), &acc);
                    gfx::RawImageDesc img;
                    CallVirtualMethod<5>(acc, &img, overlayPath.c_str(), inputW * inputH * 4 + 256, 1, 0); // loadImage
                    SafeLog::Write(Utils::Format(L"img.width %d img.height %d", img.width, img.height));
                    if (img.width == inputW && img.height == inputH) {
                        if (img.width != width || img.height != height)
                            CallVirtualMethod<10>(acc, &img, width, height); // resizeImage
                        CallVirtualMethod<12>(acc, tempPath, &img, 1); // writeImage
                        createdTempImage = true;
                    }
                    CallVirtualMethod<8>(acc, &img); // deleteImage
                    CallMethodDynGlobal(GfxCoreAddress(0x373BA9), &acc);
                    if (createdTempImage) {
                        gfx::RawImageDesc dstRegion;
                        CallMethodDynGlobal(GfxCoreAddress(0x385E30), dst, &dstRegion, left, top, width, height); // GetImageRegionImg
                        gfx::RawImageDesc overlayFsh;
                        overlayFsh.width = width;
                        overlayFsh.height = height;
                        overlayFsh.stride = width * 4;
                        CallMethodDynGlobal(GfxCoreAddress(0x37382C), &overlayFsh.buffer, overlayFsh.width * overlayFsh.height * 4); // Buffer ctor
                        if (CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x38578E), gKitGen, &overlayFsh, tempPath)) {
                            CallMethodDynGlobal(GfxCoreAddress(0x383675), sg, &dstRegion, &img); // overlayCopyPixels
                            SafeLog::Write(L"copied");
                        }
                        CallVirtualMethod<8>(acc, &overlayFsh); // deleteImage
                        DeleteFileW(tempPath);
                    }
                }
            };

            // TODO: enable this
            //PutOverlay(shapeGen, dstImg, GetUserTexturePath(L"data\\kitoverlay", gCurrCompId, teamId, kitTypeStr), 512, 1024, 0, 0, 512, 1024);

            UInt lastSeasonYear = 0;
            Bool isChampion = false;
            UInt numTitles = 0;
            Bool has3inrow = false;
            Bool isELChampion = false;
            if (team) {
                CDBCompetition *currComp = GetCompetition((gCurrCompId > 0xFFFF) ? gCurrCompId : ((gCurrCompId << 16) & 0xFFFF0000));
                if (currComp) {
                    if (currComp->GetCompID().countryId >= 1 && currComp->GetCompID().countryId <= 207) {
                        if (currComp->GetCompID().type != COMP_RELEGATION) {
                            if (currComp->GetChampion().ToInt() == team->GetTeamID().ToInt())
                                isChampion = true;
                        }
                    }
                    else {
                        if (currComp->GetCompID().countryId >= 249 && currComp->GetCompID().countryId <= 255) {
                            CDBRound *finalRound = GetRoundByRoundType(currComp->GetCompID().countryId, currComp->GetCompID().type, 15);
                            if (finalRound && finalRound->GetChampion().ToInt() == team->GetTeamID().ToInt())
                                isChampion = true;
                        }
                    }
                    if (gCurrCompId == 0xF909 || gCurrCompId == 0xF90A) {
                        lastSeasonYear = CDBGame::GetInstance()->GetCurrentSeasonStartDate().GetYear();
                        struct CompWinYear {
                            CCompID id;
                            UShort year;
                            UShort _pad;
                        };
                        void *clubHistory = CallMethodAndReturn<void *, 0xED1BE0>(team);
                        if (clubHistory) {
                            Int numWinYears = CallMethodAndReturn<Int, 0x1124440>(clubHistory);
                            if (numWinYears > 0) {
                                Vector<UShort> years;
                                for (Int i = 0; i < numWinYears; i++) {
                                    CompWinYear *win = CallMethodAndReturn<CompWinYear *, 0x11242B0>(clubHistory, i);
                                    if (win->id.ToInt() == currComp->GetCompID().ToInt())
                                        years.push_back(win->year);
                                }
                                if (!years.empty()) {
                                    numTitles = years.size();
                                    std::sort(years.begin(), years.end());
                                    UInt counter = 0;
                                    UInt prevYear = 0;
                                    for (UInt y : years) {
                                        if (y - prevYear == 1) {
                                            counter++;
                                            if (counter == 3) {
                                                has3inrow = true;
                                                break;
                                            }
                                        }
                                        else
                                            counter = 0;
                                        prevYear = y;
                                    }
                                }
                            }
                        }
                        if (!isChampion) {
                            CDBRound *ELFinalRound = GetRoundByRoundType(FifamCompRegion::Europe, FifamCompType::UefaCup, 15);
                            if (ELFinalRound && ELFinalRound->GetChampion().ToInt() == team->GetTeamID().ToInt())
                                isELChampion = true;
                        }
                    }
                }
            }
            Path badgeLeft = GetUserCompBadgesTexturePath(L"data\\kitcompbadges", gCurrCompId, false, teamId, kitTypeStr,
                lastSeasonYear, isChampion, numTitles, has3inrow, isELChampion);
            Path badgeRight = GetUserCompBadgesTexturePath(L"data\\kitcompbadges", gCurrCompId, true, teamId, kitTypeStr,
                lastSeasonYear, isChampion, numTitles, has3inrow, isELChampion);

            PutOverlay(shapeGen, dstImg, badgeLeft, 64, 64, 330, 34, 56, 56);
            PutOverlay(shapeGen, dstImg, badgeRight, 64, 64, 115, 34, 56, 56);
            PutOverlay(shapeGen, dstImg, badgeLeft, 64, 64, 438, 382, 28, 44);
            PutOverlay(shapeGen, dstImg, badgeRight, 64, 64, 48, 382, 28, 44);

            //String overlayPath = GetUserTexturePath(L"data\\kitoverlay", gCurrCompId, teamId, kitTypeStr);
            //SafeLog::Write(Utils::Format(L"overlayPath %s", overlayPath.c_str()));
            //if (!overlayPath.empty()) {
            //    WideChar tempPath[MAX_PATH + 1];
            //    GetTempPathW(259, tempPath);
            //    wcscat(tempPath, L"\\tempoverlay.fsh");
            //    SafeLog::Write(Utils::Format(L"tempPath %s", tempPath));
            //    UInt acc;
            //    Bool createdTempImage = false;
            //    CallMethodDynGlobal(GfxCoreAddress(0x373B6E), &acc);
            //    gfx::RawImageDesc img;
            //    CallVirtualMethod<5>(acc, &img, overlayPath.c_str(), 512 * 1024 * 4 + 256, 1, 0); // loadImage
            //    SafeLog::Write(Utils::Format(L"img.width %d img.height %d", img.width, img.height));
            //    if (img.width == 512 && img.height == 1024) {
            //        CallVirtualMethod<12>(acc, tempPath, &img, 1); // writeImage
            //        createdTempImage = true;
            //    }
            //    CallVirtualMethod<8>(acc, &img); // deleteImage
            //    CallMethodDynGlobal(GfxCoreAddress(0x373BA9), &acc);
            //    if (createdTempImage) {
            //        gfx::RawImageDesc overlayFsh;
            //        overlayFsh.width = 512;
            //        overlayFsh.height = 1024;
            //        overlayFsh.stride = 512 * 4;
            //        CallMethodDynGlobal(GfxCoreAddress(0x37382C), &overlayFsh.buffer, overlayFsh.width * overlayFsh.height * 4); // Buffer ctor
            //        if (CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x38578E), gKitGen, &overlayFsh, tempPath)) {
            //            CallMethodDynGlobal(GfxCoreAddress(0x383675), shapeGen, dstImg, &img); // overlayCopyPixels
            //            SafeLog::Write(L"copied");
            //        }
            //        CallVirtualMethod<8>(acc, &overlayFsh); // deleteImage
            //        DeleteFileW(tempPath);
            //    }
            //}

        }
        if (Settings::GetInstance().getClubSponsorLogos() && team && ((teamId & 0xFFFF) != 0xFFFF) && canUseSponsorLogo) {
            SafeLog::Write(Utils::Format(L"team_uid %08X", team->GetTeamUniqueID()));
            auto &sponsor = team->GetSponsor();
            SafeLog::Write(Utils::Format(L"sponsor %p", &sponsor));
            auto &mainSponsor = sponsor.GetMainSponsor();
            SafeLog::Write(Utils::Format(L"mainSponsor %p", &mainSponsor));
            if (mainSponsor.IsActive()) {
                SafeLog::Write(Utils::Format(L"mainSponsorActive %d", mainSponsor.IsActive()));
                auto &placement = mainSponsor.GetPlacement();
                SafeLog::Write(Utils::Format(L"sponsor country %d index %d", placement.countryId, placement.index));
                void *sponsorsList = CallAndReturn<void *, 0x69E9E0>();
                if (sponsorsList) {
                    void *sponsorDesc = CallMethodAndReturn<void *, 0x126E910>(sponsorsList, placement.countryId, placement.index);
                    if (sponsorDesc) {
                        SafeLog::Write(Utils::Format(L"sponsorDesc %p", sponsorDesc));
                        String sponsorPath;
                        Int sponsorImageType = 0;
                        Path iconPath = CallMethodAndReturn<wchar_t const *, 0x126D4A0>(sponsorDesc);
                        String iconName = iconPath.stem().c_str();
                        if (!iconName.empty()) {
                            sponsorPath = L"sponsors\\200x120\\" + iconName + L"_" + Utils::Format(L"%d", numberColor) + L".tga";
                            if (FmFileExists(sponsorPath))
                                sponsorImageType = 2;
                            else {
                                sponsorPath = L"sponsors\\200x120\\" + iconName + L".tga";
                                if (FmFileExists(sponsorPath))
                                    sponsorImageType = 2;
                                else {
                                    iconPath.replace_extension(L".tga");
                                    if (FmFileExists(iconPath)) {
                                        sponsorPath = iconPath.c_str();
                                        sponsorImageType = 1;
                                    }
                                }
                            }
                            if (sponsorImageType != 0) {
                                WideChar tempPath[MAX_PATH + 1];
                                GetTempPathW(259, tempPath);
                                wcscat(tempPath, L"\\tempsponsor.fsh");
                                SafeLog::Write(Utils::Format(L"tempSponsorPath %s", tempPath));
                                UInt acc;
                                Bool createdTempImage = false;
                                CallMethodDynGlobal(GfxCoreAddress(0x373B6E), &acc);
                                gfx::RawImageDesc img;
                                UInt sW = sponsorImageType == 2 ? 200 : 96;
                                UInt sH = sponsorImageType == 2 ? 120 : 96;
                                CallVirtualMethod<5>(acc, &img, sponsorPath.c_str(), sW * sH * 4 + 256, 1, 0); // loadImage
                                SafeLog::Write(Utils::Format(L"img.width %d img.height %d", img.width, img.height));
                                if (img.width == sW && img.height == sH) {
                                    if (sponsorImageType == 1) {
                                        sW = sH = 128;
                                        CallVirtualMethod<10>(acc, &img, sW, sH); // resizeImage
                                    }
                                    CallVirtualMethod<12>(acc, tempPath, &img, 1); // writeImage
                                    createdTempImage = true;
                                }
                                CallVirtualMethod<8>(acc, &img); // deleteImage
                                CallMethodDynGlobal(GfxCoreAddress(0x373BA9), &acc);
                                if (createdTempImage) {
                                    gfx::RawImageDesc dstRegion;
                                    UInt sX = sponsorImageType == 2 ? 158 : 194;
                                    UInt sY = sponsorImageType == 2 ? 778 : 778;
                                    CallMethodDynGlobal(GfxCoreAddress(0x385E30), dstImg, &dstRegion, sX, sY, sW, sH); // GetImageRegionImg
                                    gfx::RawImageDesc sponsorFsh;
                                    sponsorFsh.width = sW;
                                    sponsorFsh.height = sH;
                                    sponsorFsh.stride = sW * 4;
                                    CallMethodDynGlobal(GfxCoreAddress(0x37382C), &sponsorFsh.buffer, sponsorFsh.width * sponsorFsh.height * 4); // Buffer ctor
                                    if (CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x38578E), gKitGen, &sponsorFsh, tempPath)) {
                                        CallMethodDynGlobal(GfxCoreAddress(0x383675), shapeGen, &dstRegion, &img); // overlayCopyPixels
                                        SafeLog::Write(L"copied");
                                    }
                                    CallVirtualMethod<8>(acc, &sponsorFsh); // deleteImage
                                    DeleteFileW(tempPath);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    CallMethodDynGlobal(GfxCoreAddress(0x3876A5), shapeGen, dstImg, fshName, imageFormat);
}

void METHOD OnClubShirtDesignSetupKit(void *screen) {
    Bool dataWasPassed = false;
    CTeamIndex &teamIndex = *raw_ptr<CTeamIndex>(screen, 0x5E0);
    if (teamIndex.countryId != 0) {
        CDBTeam *team = GetTeam(teamIndex);
        if (team) {
            void *cmb = *raw_ptr<void *>(screen, 0x508);
            if (cmb) {
                UInt bHomeKit = CallVirtualMethodAndReturn<UInt, 85>(cmb);
                void *kit = CallMethodAndReturn<void *, 0xED3D60>(team);
                UChar color = CallMethodAndReturn<UChar, 0xFFCC50>(kit, (bHomeKit == 0) ? 1 : 0);
                SetVarInt("HOME_JNUMCOLOR", color);
                SetVarInt("HOME_TEAMID", team->GetTeamUniqueID());
                SetVarInt("HOME_KITTYPE", (bHomeKit == 0) ? 1 : 0);
                SetVarInt("HOME_USERKIT", false);
                SetVarInt("HOME_YOUTH", false);
                dataWasPassed = true;
            }
        }
    }
    CallMethod<0x599AE0>(screen);
    if (dataWasPassed) {
        SetVarInt("HOME_JNUMCOLOR", 0);
        SetVarInt("HOME_TEAMID", 0);
        SetVarInt("HOME_KITTYPE", 0);
        SetVarInt("HOME_USERKIT", 0);
        SetVarInt("HOME_YOUTH", 0);
    }
}

CDBTeam *OnGetTeamForTeamPhoto(CTeamIndex teamIndex) {
    CDBTeam *team = GetTeam(teamIndex);
    if (team) {
        void *kit = CallMethodAndReturn<void *, 0xED3D60>(team);
        UChar color = CallMethodAndReturn<UChar, 0xFFCC50>(kit, 0);
        SetVarInt("HOME_JNUMCOLOR", color);
        SetVarInt("HOME_TEAMID", team->GetTeamUniqueID());
        SetVarInt("HOME_KITTYPE", 0);
        SetVarInt("HOME_USERKIT", false);
        SetVarInt("HOME_YOUTH", false);
    }
    return team;
}

WideChar const *OnFinishTeamPhoto(WideChar *dst, WideChar const *src) {
    SetVarInt("HOME_JNUMCOLOR", 0);
    SetVarInt("HOME_TEAMID", 0);
    SetVarInt("HOME_KITTYPE", 0);
    SetVarInt("HOME_USERKIT", 0);
    SetVarInt("HOME_YOUTH", 0);
    return CallAndReturn<WideChar const *, 0x1493F2F>(dst, src);
}

void CheckKitSelection(const CTeamIndex &homeTeamIndex, const CTeamIndex &awayTeamIndex, DefaultKitRenderData *outputData, int *resultKits, char bFalse){
    //Error(L"Check kit selection");
    CDBTeam *team1 = GetTeam(homeTeamIndex);
    if (team1) {
        //Error(L"team1");
        CDBTeam *team2 = GetTeam(awayTeamIndex);
        if (team2) {
            //Error(L"team2");
            UInt *team3dIds = (UInt *)0x311BA68;
            UChar *team3dKitIds = (UChar *)0x311BA50;
            Bool manualKitSelection = false;
            for (UInt i = 0; i < 8; i++) {
                if (team3dIds[i] == homeTeamIndex.ToInt() || team3dIds[i] == awayTeamIndex.ToInt()) {
                    manualKitSelection = true;
                    //Error(L"Manual kit selection");
                    break;
                }
            }
            if (!manualKitSelection) {
                FifamReader reader(L"plugins\\ucp\\kit_selection.csv", 14);
                if (reader.Available()) {
                    reader.SkipLine();
                    while (!reader.IsEof()) {
                        if (!reader.EmptyLine()) {
                            UInt team1id = 0;
                            UInt team2id = 0;
                            String a, b;
                            reader.ReadLine(Hexadecimal(team1id), Hexadecimal(team2id), a, b);
                            //Error(Utils::Format(L"%08X - %08X (%08X - %08X), %s, %s", team1id, team2id, team1->GetTeamUniqueID(), team2->GetTeamUniqueID(), a, b));
                            if (team1id == team1->GetTeamUniqueID() && team2id == team2->GetTeamUniqueID()) {
                                auto TranslateKitType = [](String const &val) {
                                    if (val == L"h" || val == L"H")
                                        return 0;
                                    if (val == L"a" || val == L"A")
                                        return 1;
                                    if (val == L"t" || val == L"T")
                                        return 2;
                                    UInt intVal = Utils::SafeConvertInt<UInt>(val);
                                    if (intVal == 1)
                                        return 1;
                                    if (intVal == 2 || intVal == 3)
                                        return 2;
                                    return 0;
                                };
                                UChar homeKitId = TranslateKitType(a);
                                UChar awayKitId = TranslateKitType(b);
                                if (CallMethodAndReturn<Bool, 0xFFD940>(CallMethodAndReturn<void *, 0xED3D60>(team1), homeKitId)
                                    && CallMethodAndReturn<Bool, 0xFFD940>(CallMethodAndReturn<void *, 0xED3D60>(team2), awayKitId))
                                {
                                    //Error(Utils::Format(L"home %d away %d", homeKitId, awayKitId));
                                    UInt oldTeam1Id = team3dIds[0];
                                    UInt oldTeam2Id = team3dIds[1];
                                    UInt oldTeam1KitId = team3dKitIds[0];
                                    UInt oldTeam2KitId = team3dKitIds[1];
                                    team3dIds[0] = homeTeamIndex.ToInt();
                                    team3dIds[1] = awayTeamIndex.ToInt();
                                    team3dKitIds[0] = homeKitId;
                                    team3dKitIds[1] = awayKitId;
                                    Call<0x43FCB0>(&homeTeamIndex, &awayTeamIndex, outputData, resultKits, bFalse);
                                    team3dIds[0] = oldTeam1Id;
                                    team3dIds[1] = oldTeam2Id;
                                    team3dKitIds[0] = oldTeam1KitId;
                                    team3dKitIds[1] = oldTeam2KitId;
                                    return;
                                }
                            }
                        }
                        else
                            reader.SkipLine();
                    }
                    //Error(L"not found");
                }
                //else
                    //Error(L"failed to open file");
            }
        }
    }
    Call<0x43FCB0>(&homeTeamIndex, &awayTeamIndex, outputData, resultKits, bFalse);
}

void InstallKits_FM13() {

    // custom captain armband

    patch::RedirectCall(GfxCoreAddress(0x3728EB), OnGenerateKit);
    patch::RedirectCall(GfxCoreAddress(0x38FF20), OnGenerateKit);
    patch::RedirectCall(GfxCoreAddress(0x384D6F), ReadCaptainArmband);
    patch::RedirectCall(GfxCoreAddress(0x384DA5), ApplyCaptainArmbandColor);

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
    //patch::RedirectCall(GfxCoreAddress(0x379170), OnGenerateKitResourceForScenes);
    patch::RedirectCall(GfxCoreAddress(0x37968A), OnGenerateKitResourceForScenes);
    patch::RedirectCall(GfxCoreAddress(0x37974F), OnGenerateKitResourceForScenes);
    patch::RedirectCall(GfxCoreAddress(0x37A26C), OnGenerateKitResourceForScenes);
    patch::RedirectCall(GfxCoreAddress(0x38FBFD), UseGenericKit);

    patch::Nop(GfxCoreAddress(0x370B5C), 8);
    patch::RedirectCall(GfxCoreAddress(0x370B72), SetGkKitFileName_Generic);

    // custom jersey number size and offset
    patch::RedirectCall(GfxCoreAddress(0x210065), SetupJerseyNumberHotspot);

    // referee kits
    patch::RedirectCall(GfxCoreAddress(0x23FEA6), OnGetRefKitId);

    // kit overlay
    patch::RedirectCall(GfxCoreAddress(0x384E15), OnWriteKitShape);
}

void InstallKits_FM11() {
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

        // kit preview
        patch::RedirectCall(0x599ED6, OnClubShirtDesignSetupKit);

        // team photo
        patch::RedirectCall(0x66212E, OnGetTeamForTeamPhoto);
        patch::RedirectCall(0x6623B2, OnFinishTeamPhoto);

        // generic 3rd kit
        patch::Nop(0x440601, 1);
        patch::SetUChar(0x440601 + 1, 0xE9);

        // kit clashing file
        patch::RedirectCall(0x4409E8, CheckKitSelection);
        patch::RedirectCall(0x440A35, CheckKitSelection);
        //patch::SetUChar(0xADB245 + 6, 0);
        //patch::SetUInt(0x43FDD7 + 3, 0);
    }
}
