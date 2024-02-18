#include "Kits.h"
#include "RendererHook.h"
#include "UcpSettings.h"
#include "CustomShaders.h"
#include "FifamReadWrite.h"
#include "json/json.hpp"
#include <fstream>

using namespace nlohmann;

UInt gShirtNumberColors[] = { 0, 0xFFFFFFFF, 0xFF000000, 0xFFFFFF00, 0xFFFF0000, 0xFF00FF00, 0xFF0000FF };
UChar gShirtNameColors[] = { 0xF1,0xF5,0xF2,0x14,0x17,0x13,0xF0,0xCD,0xF0,0xF0,0x80,0xF0,0xF2,0x3D,0xEF,0xF0,0x40,0xEF,0xF1,0x36,0xDF,0xEA,0x43,0xC5,0xCC,0x2F,0xA8,0xF1,0x2E,0xB0,0xF0,0x89,0xD2,0xF3,0x51,0xA9,0xDE,0x3B,0x7F,0xE3,0x2F,0x6C,0xC5,0x17,0x4C,0xDD,0x40,0x4E,0xC4,0x29,0x39,0x96,0x2C,0x17,0xEE,0x31,0x3C,0xDC,0x2A,0x2A,0xBD,0x37,0x1C,0xB5,0x37,0x1B,0xDA,0x48,0x31,0xB0,0x43,0x2C,0x96,0x28,0x12,0x9C,0x4E,0x17,0xE6,0xED,0xEA,0xFE,0xDC,0xD5,0xCC,0x67,0xCC,0x02,0x02,0x7C,0x77,0x03,0x0D,0x74,0x8B,0x45,0x00,0x00,0x00,0xFF,0x00,0xFF,0xC4,0x92,0xAC,0xC4,0x90,0x99,0xA8,0x72,0x85,0x85,0x4B,0x49,0x6E,0x3A,0x38,0x4E,0x25,0x24,0x9E,0xC5,0xEA,0x60,0x84,0xBC,0x01,0xA9,0x99,0x3A,0x88,0x8F,0x34,0x77,0x78,0x91,0x3C,0x93,0x32,0x23,0x94,0x46,0x39,0x80,0x1B,0x3A,0x4D,0xDB,0xFA,0xE3,0xB5,0xCD,0xBE,0xA0,0xB0,0xAA,0x00,0xB6,0x47,0x9D,0x4E,0x88,0x01,0x51,0x02,0xF0,0x27,0xD9,0x00,0x00,0x7B,0x7D,0xD3,0xB1,0x00,0x52,0x13,0x3C,0xB9,0x5C,0x2C,0x8F,0x41,0x21,0x60,0x31,0xA6,0x04,0x00,0x0F,0x4D,0x1F };

Int gHotspots[60];

struct BodyPart {
    Char const *name;
    Bool32 status;
};

BodyPart gBodyPartsCollar[] = {
    { "enable_body_RCollar" , false },
    { "enable_body_TCollar" , false },
    { "enable_body_ACollar" , false },
    { "enable_body_NCollar" , false },
    { "enable_body_NPCollar", false },
    { "enable_body_PCollar" , false },
    { "enable_body_VCollar" , false },
    { "enable_body_SCollar" , false },
};

void SetCollar(UInt collarType) {
    if (collarType > std::size(gBodyPartsCollar))
        collarType = 0;
    else if (collarType > 1)
        collarType -= 1;
    for (UInt i = 0; i < std::size(gBodyPartsCollar); i++)
        gBodyPartsCollar[i].status = i == collarType;
}

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

team_kit_desc *gKitDesc = nullptr;
UChar gKitType = 0;
UInt gKitClubUID = 0;
UInt gKitClubLeagueID = 0;
WideChar gKitnumberPath[1024];
Bool gKitnumbersCreated = false;
WideChar gTempPath[MAX_PATH + 1];
UInt gNumber1X = 0;
UInt gNumber2X = 0;
UInt gNumberY = 0;
UInt gNumberWidth = 0;
UInt gNumberHeight = 0;

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

void SetTeamKitConfig(UInt teamId, UInt kitType, UChar collar, UChar nameplacement, Bool frontnumber, Int jerseynumbercolor, Int jerseynamecolor,
    UChar jerseynumbersize, UChar jerseynumberoffset, Bool canusecompbadges, Bool canusesponsorlogo)
{
    if (teamId != 0 && kitType <= 3) {
        auto &kits = GetTeamKitsMap()[teamId];
        auto &kitDesc = kits[kitType];
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
                Bool canusecompbadges = true;
                Bool canusesponsorlogo = false;
                UChar jerseynumbersize = 0;
                UChar jerseynumberoffset = 0;
                reader.ReadLine(d, d, d, Hexadecimal(teamid), kittype, collar, nameplacement, frontnumber, jerseynumbercolor,
                    jerseynamecolor, jerseynumbersize, jerseynumberoffset, canusecompbadges, canusesponsorlogo);
                SetTeamKitConfig(teamid, kittype, collar, nameplacement, frontnumber, jerseynumbercolor, jerseynamecolor,
                    jerseynumbersize, jerseynumberoffset, canusecompbadges, canusesponsorlogo);
            }
            else
                reader.SkipLine();
        }
    }

    Path kitConfigDir = "data\\kitconfig";
    auto jsonReadNumber = [](json const &node, StringA const &key, int defaultValue) {
        auto it = node.find(key);
        if (it == node.end() || !(*it).is_number())
            return defaultValue;
        return (*it).get<int>();
    };
    if (exists(kitConfigDir) && is_directory(kitConfigDir)) {
        for (auto const &i : directory_iterator(kitConfigDir)) {
            auto const &p = i.path();
            if (is_regular_file(p) && ToLower(p.extension().string()) == ".json") {
                auto filename = p.stem().string();
                UInt teamid = 0;
                Char kittype = 0;
                if (sscanf(filename.c_str(), "%X_%c", &teamid, &kittype) && teamid != 0) {
                    Int kitTypeId = -1;
                    if (kittype == 'h')
                        kitTypeId = 0;
                    else if (kittype == 'a')
                        kitTypeId = 1;
                    else if (kittype == 'g')
                        kitTypeId = 2;
                    else if (kittype == 't')
                        kitTypeId = 3;
                    if (kitTypeId != -1) {
                        try {
                            std::ifstream s(p);
                            json j;
                            s >> j;
                            Bool isNationalTeam = (teamid & 0xFFFF) == 0xFFFF;
                            SetTeamKitConfig(teamid, kitTypeId,
                                jsonReadNumber(j, "collar", 0),
                                jsonReadNumber(j, "nameplacement", 1),
                                jsonReadNumber(j, "frontnumber", isNationalTeam),
                                jsonReadNumber(j, "jerseynumbercolor", 0),
                                jsonReadNumber(j, "jerseynamecolor", 0),
                                jsonReadNumber(j, "jerseynumbersize", 0),
                                jsonReadNumber(j, "jerseynumberoffset", 0),
                                jsonReadNumber(j, "canusecompbadges", 1),
                                jsonReadNumber(j, "canusesponsorlogo", 0)
                            );
                        }
                        catch (...) {

                        }
                    }
                }
            }
        }
    }
}

void METHOD OnLoadClubKit(void *kitCtrl, DUMMY_ARG, void *kitData, UInt fifaKitId, Bool bHome) {
    if (Settings::GetInstance().ShadersReload)
        ReloadShaders();
    if (Settings::GetInstance().ReloadKitConfig) {
        GetTeamKitsMap().clear();
        ReadKitsFile();
    }
    void *dlg = (void *)((UInt)kitCtrl - 0x1B4C);
    Int currentKitView = *raw_ptr<Int>(dlg, 0x13A4);
    void *club = *raw_ptr<void *>(dlg, 0x13A8);
    if (club) {
        UInt uid = *raw_ptr<UInt>(club, 0xC);
        gKitDesc = GetTeamKitInfo(uid, currentKitView >= 2);
        if (gKitDesc && !gKitDesc->used)
            gKitDesc = nullptr;
        gKitClubUID = uid;
        gKitType = currentKitView >= 2;
        gKitClubLeagueID = CallMethodAndReturn<UInt, 0x4C3370>(club, 0);
    }
    else {
        gKitDesc = nullptr;
        gKitClubUID = 0;
        gKitClubLeagueID = 0;
        gKitType = 0;
    }
    //::Message("UID %08X View %d", uid, currentKitView);
    CallMethod<0x4AC850>(kitCtrl, kitData, fifaKitId, bHome);
    gKitDesc = nullptr;
    gKitClubUID = 0;
    gKitClubLeagueID = 0;
    gKitType = 0;
}

void METHOD OnLoadHeadScene(void *dlg) {
    if (Settings::GetInstance().ShadersReload)
        ReloadShaders();
    if (Settings::GetInstance().ReloadKitConfig) {
        GetTeamKitsMap().clear();
        ReadKitsFile();
    }
    void *club = *raw_ptr<void *>(dlg, 0x58);
    if (club) {
        UInt uid = *raw_ptr<UInt>(club, 0xC);
        gKitDesc = GetTeamKitInfo(uid, 0);
        if (gKitDesc && !gKitDesc->used)
            gKitDesc = nullptr;
        //::Message("UID %08X", uid);
    }
    else
        gKitDesc = nullptr;
    CallMethod<0x4B6AE0>(dlg);
    gKitDesc = nullptr;
}

void UpdateKitCollar(void *fifaPlayer) {
    void *bodyRes = *raw_ptr<void *>(fifaPlayer, 0x40);
    unsigned int buf[2] = {};
    void **pBuf = CallVirtualMethodAndReturn<void **, 7>(bodyRes, &buf);
    void *model = CallMethodAndReturnDynGlobal<void *>(RendererAddress(0x87211), *pBuf);
    SetCollar(gKitDesc ? gKitDesc->collar : 0);
    CallDynGlobal(RendererAddress(0x9CD31), model, gBodyPartsCollar, std::size(gBodyPartsCollar));
}

void METHOD SetKitnumbersTexture(void *fifaPlayer, DUMMY_ARG, void *kitnumbersTex) {
    CallMethodDynGlobal(RendererAddress(0x9F8F7), fifaPlayer, kitnumbersTex);
    UpdateKitCollar(fifaPlayer);
}

void *gBacknumberImg = nullptr;

Bool FmFileImageExists(String const &filepathWithoutExtension, String &resultPath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(RendererAddress(0x90D9F));
    if (fmFs) {
        String fpstr = filepathWithoutExtension + L".tga";
        if (CallVirtualMethodAndReturn<Bool, 9>(fmFs, fpstr.c_str())) {
            resultPath = fpstr;
            return true;
        }
    }
    resultPath.clear();
    return false;
}

Bool FmFileExists(Path const &filepath) {
    void *fmFs = CallAndReturnDynGlobal<void *>(RendererAddress(0x90D9F));
    if (fmFs)
        return CallVirtualMethodAndReturn<Bool, 9>(fmFs, filepath.c_str());
    return false;
}

String GetUserKitNumberTexturePath(String const &directory, UInt clubId, UInt leagueId, String const &kitTypeStr, Int colorId) {
    String result;
    if (clubId > 0) {
        String clubIdStr = Utils::Format(L"%08X", clubId);
        if (FmFileImageExists(directory + L"\\" + clubIdStr + kitTypeStr, result)) // 002D000E_h
            return result;
        if (FmFileImageExists(directory + L"\\" + clubIdStr, result)) // 002D000E
            return result;
        if (leagueId > 0) {
            String compIdStr = Utils::Format(L"%08X", leagueId);
            if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr + kitTypeStr, result)) // 2D010000_002D000E_h
                return result;
            if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + clubIdStr, result)) // 2D010000_002D000E
                return result;
            if (FmFileImageExists(directory + L"\\" + compIdStr + L"_" + Utils::Format(L"%d", colorId), result)) // 2D010000_1
                return result;
        }
    }
    return String();
}

void METHOD OnWriteBacknumber(void *writer, DUMMY_ARG, void *img, Char const *name, UInt colorType) {
    CallMethodDynGlobal(RendererAddress(0x93E53), writer, img, name, colorType);
    gBacknumberImg = img;
}

void METHOD OnWriteFrontnumber(void *writer, DUMMY_ARG, void *img, Char const *name, UInt colorType) {
    Bool enable = false;
    if (gKitDesc)
        enable = gKitDesc->frontnumber;
    else
        enable = (gKitClubUID & 0xFFFF) == 0xFFFF;
    if (enable)
        CallMethodDynGlobal(RendererAddress(0x93E53), writer, gBacknumberImg, name, colorType);
    else
        CallMethodDynGlobal(RendererAddress(0x93E53), writer, img, name, colorType);
    gBacknumberImg = nullptr;
}

void METHOD GenerateKitNumbers(void *kitGen, DUMMY_ARG, WideChar const *tempPath, void *data, Char *outFilename, Bool bHome) {
    gKitnumbersCreated = false;
    void *kitData = *raw_ptr<void *>(data, 0xC);
    UInt nameplacement = *raw_ptr<UInt>(kitData, 0x3C);
    UInt jerseynamecolor = *raw_ptr<UInt>(kitData, 0x38);
    UInt jerseynumbercolor = *raw_ptr<UInt>(kitData, 0x34);
    if (gKitDesc) {
        UInt newNameplacement = gKitDesc->nameplacement;
        if (newNameplacement)
            newNameplacement = newNameplacement == 2;
        else
            newNameplacement = 2;
        *raw_ptr<UInt>(kitData, 0x3C) = newNameplacement;
        if (gKitDesc->jerseynamecolor >= 1 && gKitDesc->jerseynamecolor <= 64) {
            UInt colorIndex = gKitDesc->jerseynamecolor - 1;
            UChar red = gShirtNameColors[colorIndex * 3 + 0];
            UChar green = gShirtNameColors[colorIndex * 3 + 2];
            UChar blue = gShirtNameColors[colorIndex * 3 + 1];
            UInt color = 0xFF000000 | (red << 16) | (green << 8) | blue;
            *raw_ptr<UInt>(kitData, 0x38) = 0;
            gShirtNumberColors[0] = color;
        }
        if (gKitDesc->jerseynumbercolor >= 1 && gKitDesc->jerseynumbercolor <= 6)
            *raw_ptr<UInt>(kitData, 0x34) = gKitDesc->jerseynumbercolor;
        static const UInt jnum_sizes[] = { 78, 84, 90, 96, 100, 104, 108, 112, 116, 120 };
        UInt height = 120;
        UInt offsety = 0;
        if (gKitDesc->jerseynumbersize == 0)
            height = 120;
        else if (gKitDesc->jerseynumbersize <= 10)
            height = jnum_sizes[gKitDesc->jerseynumbersize - 1];
        offsety = gKitDesc->jerseynumberoffset;
        UInt width = height / 2;
        gNumber1X = 64 - width;
        gNumber2X = 64;
        gNumberY = offsety;
        gNumberWidth = width;
        gNumberHeight = height;
    }
    else {
        gNumber1X = 4;
        gNumber2X = 64;
        gNumberY = 0;
        gNumberWidth = 60;
        gNumberHeight = 120;
    }
    Int colorId = *raw_ptr<UInt>(kitData, 0x34);
    if (colorId < 1 || colorId > 6)
        colorId = 1;
    String kitTypeStr = (gKitType == 1) ? L"_a" : L"_h";
    String numpath = GetUserKitNumberTexturePath(L"data\\kitnumbers\\jersey", gKitClubUID, gKitClubLeagueID, kitTypeStr, colorId);
    if (!numpath.empty() && FmFileExists(numpath) && numpath.size() < 1024) {
        wcscpy(gKitnumberPath, numpath.c_str());
        if (*raw_ptr<UInt>(kitData, 0x34) == 0)
            *raw_ptr<UInt>(kitData, 0x34) = 1;
    }
    CallMethodDynGlobal(RendererAddress(0x90250), kitGen, tempPath, data, outFilename, bHome);
    *raw_ptr<UInt>(kitData, 0x3C) = nameplacement;
    *raw_ptr<UInt>(kitData, 0x38) = jerseynamecolor;
    *raw_ptr<UInt>(kitData, 0x34) = jerseynumbercolor;
    gShirtNumberColors[0] = 0;
    gKitnumberPath[0] = 0;
    if (gKitnumbersCreated) {
        DeleteFileW(gTempPath);
        gKitnumbersCreated = false;
    }
}

void GenerateKitnumbersName(Char *out, Char const *format, UInt numbercolorid, UInt namecolor, UInt placement) {
    sprintf(out, "t70__%d_%d_%d_%d.fsh", gKitClubUID, gKitType, numbercolorid, namecolor);
}

void GetKitNumbersPath(WideChar *dst, UInt maxLen, WideChar const *format, UChar color) {
    _snwprintf(dst, maxLen, format, color);
    if (gKitnumberPath[0]) {
        GetTempPathW(259, gTempPath);
        wcscat(gTempPath, L"\\tempkitnumbers.fsh");
        UInt acc;
        CallMethodDynGlobal(RendererAddress(0x7E9AF), &acc);
        struct RawImageDesc {
            UInt width;
            UInt height;
            UInt stride;
            struct Buffer {
                void *data;
                UInt size;
            } buffer;
        } img;
        CallVirtualMethod<5>(acc, &img, gKitnumberPath, 1024 * 128 * 4 + 256, 1, 0); // loadImage
        if (img.width == 1024 && img.height == 128) {
            CallVirtualMethod<12>(acc, gTempPath, &img, 1); // writeImage
            wcscpy(dst, gTempPath);
            gKitnumbersCreated = true;
        }
        CallVirtualMethod<8>(acc, &img); // deleteImage
        CallMethodDynGlobal(RendererAddress(0x7E9EA), &acc);
    }
}

void *GetShapeHotspot(void *shape) {
    if (gKitnumberPath[0] && gKitnumbersCreated)
        return gHotspots;
    return CallAndReturnDynGlobal<void *>(RendererAddress(0x209166), shape);
}

// 0, 0, 58, 80
void OnNumberScale(void *shape, Int x, Int y, Int width, Int height) {
    CallDynGlobal(RendererAddress(0x20E93C), shape, x, y, gNumberWidth, gNumberHeight);
}

// 6, 0, 58, 128
void *METHOD OnNumberGetSubRect1(void *subRect, DUMMY_ARG, void *subRectDst, Int x, Int y, Int width, Int height) {
    void *result = CallMethodAndReturnDynGlobal<void *>(RendererAddress(0x9259C), subRect, subRectDst, gNumber1X, gNumberY, gNumberWidth, height);
    return result;
}

// 64, 0, 58, 128
void *METHOD OnNumberGetSubRect2(void *subRect, DUMMY_ARG, void *subRectDst, Int x, Int y, Int width, Int height) {
    void *result = CallMethodAndReturnDynGlobal<void *>(RendererAddress(0x9259C), subRect, subRectDst, gNumber2X, gNumberY, gNumberWidth, height);
    return result;
}

void InstallKits_Renderer() {
    patch::SetPointer(RendererAddress(0x2CFCC8), "enable_body_shirt_tucked");
    patch::SetUInt(RendererAddress(0x2CFCCC), 1);
    patch::SetPointer(RendererAddress(0x2CFCE0), "enable_body_shirt_untucked");
    if (Settings::GetInstance().EditorKitExtensions) {
        patch::RedirectCall(RendererAddress(0x8472E), SetKitnumbersTexture);
        patch::RedirectCall(RendererAddress(0x90A86), OnWriteBacknumber);
        patch::RedirectCall(RendererAddress(0x90A9C), OnWriteFrontnumber);
        patch::RedirectCall(RendererAddress(0x9D3B8), GenerateKitNumbers);
        patch::RedirectCall(RendererAddress(0x90896), GenerateKitnumbersName);
        patch::RedirectCall(RendererAddress(0x90648), GetKitNumbersPath);
        patch::RedirectCall(RendererAddress(0x906A8), GetShapeHotspot);

        patch::RedirectCall(RendererAddress(0x907A1), OnNumberScale);
        patch::RedirectCall(RendererAddress(0x907BE), OnNumberGetSubRect1);
        patch::RedirectCall(RendererAddress(0x907F4), OnNumberGetSubRect2);

        patch::SetPointer(RendererAddress(0x90372 + 3), gShirtNumberColors);
        patch::SetPointer(RendererAddress(0x90569 + 2), gShirtNameColors);
        patch::SetPointer(RendererAddress(0x90961 + 1), L"MANAGER");
        patch::SetUInt(RendererAddress(0x906CC + 2), 20);
        patch::SetUInt(RendererAddress(0x906DE + 2), 16);
        patch::SetUInt(RendererAddress(0x90722 + 2), 12);
        patch::SetUInt(RendererAddress(0x90736 + 2), 8);
    }
}

void PatchKits(FM::Version v) {
    if (v.id() == VERSION_ED_13) {
        if (Settings::GetInstance().EditorKitExtensions) {
            ReadKitsFile();
            gHotspots[2] = 921;
            gHotspots[3] = 0;
            gHotspots[4] = 103;
            gHotspots[5] = 128;
            patch::RedirectCall(0x435821, OnLoadClubKit);
            patch::RedirectCall(0x4B97A8, OnLoadHeadScene);
        }
    }
}
