#include "CustomStadiums.h"
#include "GfxCoreHook.h"
#include "GameInterfaces.h"
#include "3dAdboardsAndBanners.h"
#include "UcpSettings.h"
#include "Utils.h"
#include "Random.h"

unsigned int gCustomStadiumTeamUID = 0;
char const *gFifaTemplateFilename = "data\\stadium\\FIFA\\template.xml";
bool gResourceCacheEnabled = true;
UInt gCurrentLighting = 0;
Float gStadiumScaling[3] = { 1.0f, 1.0f, 1.0f };
UInt gStadiumIntroSceneId = 0;
bool gTrainingStadium = false;

void *shapeBannersHome = nullptr;
void *shapeBannersAway = nullptr;
void *modelShadow = nullptr;

String &GetCustomStadiumFolder() {
    static String stadiumFolder;
    return stadiumFolder;
}

bool IsCustomStadiumLoaded() {
    void *stadiumEngine = (void *)GfxCoreAddress(0xD49E78);
    void *origStadium = raw_ptr<void>(stadiumEngine, 0x1008);
    if (*raw_ptr<UChar>(origStadium, 4))
        return true;
    return false;
}

bool METHOD ReadXmlDataForStadium(void *orgStadScene, DUMMY_ARG, char (*outFileNames)[256], unsigned int stadiumId) {
    gCustomStadiumTeamUID = 0;
    //Error("ReadXmlDataForStadium: %X", stadiumId);
    if (stadiumId >= 0x00010000) {
        static char customFifaTemplateFilename[256];
        sprintf(customFifaTemplateFilename, "data\\stadium\\FIFA\\%08X\\template.xml", stadiumId);
        bool templateExists = false;
        if (FmFileExists(customFifaTemplateFilename)) {
            strcpy(outFileNames[0], customFifaTemplateFilename);
            templateExists = true;
        }
        else if (FmFileExists(gFifaTemplateFilename)) {
            strcpy(outFileNames[0], gFifaTemplateFilename);
            templateExists = true;
        }
        if (templateExists) {
            strcpy(outFileNames[1], "pitch.scx");
            gCustomStadiumTeamUID = stadiumId;
            return true;
        }
    }
    return CallMethodAndReturnDynGlobal<bool>(GfxCoreAddress(0x37BAC3), orgStadScene, outFileNames, stadiumId);
}

void SetupStadiumXmlResolver(void *templates, unsigned int numTemplates, void *customFilenames) {
    CallDynGlobal(GfxCoreAddress(0x37C080), templates, numTemplates, customFilenames);
    if (gCustomStadiumTeamUID != 0) {
        *raw_ptr<char const *>(templates, 0x10) = "%UID%";
        static char TeamUIDStr[128];
        sprintf(TeamUIDStr, "%08X", gCustomStadiumTeamUID);
        *raw_ptr<char const *>(templates, 0x14) = TeamUIDStr;
        //if (CheckBannersFolder(AtoW(TeamUIDStr), nullptr)) {
            static char const *customBannersFilename = "t13__201002_0_0.fsh";
            *raw_ptr<char const *>(templates, 0x4) = customBannersFilename;
            *raw_ptr<char const *>(templates, 0xC) = customBannersFilename;
        //}
        //Error("ReadXmlDataForStadium3: %X", gCustomStadiumTeamUID);
    }
}

UInt METHOD GetTeamStadiumId(void *stad, DUMMY_ARG, Int a, Int b) {
    // get CDBTeam* from CDBStadiumDevelopment
    CDBTeam *team = (CDBTeam *)(Int(stad) - 0x11F8);
    // validate CDBTeam*
    if (*raw_ptr<UInt>(team) == 0x24A370C) {
        if (CallMethodAndReturn<Bool, 0xF705E0>(stad)) {
            CDBGame *game = CDBGame::GetInstance();
            if (game) {
                if (team->IsManagedByAI()) {
                    if (!game->GetOptions().CheckFlag(34))
                        return 0;
                }
                else {
                    if (!game->GetOptions().CheckFlag(33))
                        return 0;
                }
            }
        }
        StringA stadFilename = "data\\stadium\\FIFA\\" + Format("%08X", team->GetTeamUniqueID()) + "\\stadium_1.o";
        if (FmFileExists(stadFilename)) {
            //Error("GetTeamStadiumId : %X", team->GetTeamUniqueID());
            return team->GetTeamUniqueID();
        }
    }
    return CallMethodAndReturn<UChar, 0xF70670>(stad, a, b);
}

void METHOD OnAddResourceToCache(void *resMan, DUMMY_ARG, void *res) {
    if (gResourceCacheEnabled)
        CallMethodDynGlobal(GfxCoreAddress(0x376072), resMan, res);
}

void **OnLoadEaglXSceneTexture(void **outRes, const WideChar *filename, void *resMan, Int a4) {
    gResourceCacheEnabled = false;
    void **result = CallAndReturnDynGlobal<void **>(GfxCoreAddress(0x37B86F), outRes, filename, resMan, a4);
    gResourceCacheEnabled = true;
    return result;
}

void GetStadiumExtraFilenameFor3dMatch(Char *dst, Char const *format, UInt stadiumId, UInt lightingId,
    Char const *fifaStadFilenameFormat, Char const *fifaStadFilenameFormatDefault, Char const *extension = nullptr) {
    //Error(fifaStadFilenameFormat, stadiumId, gCurrentLighting);
    if (stadiumId == 999 && IsCustomStadiumLoaded()) {
        bool exists = false;
        static Char tmpFilename[260];
        StringA stadiumFolder = Utils::WtoA(GetCustomStadiumFolder());
        sprintf(tmpFilename, fifaStadFilenameFormat, stadiumFolder.c_str(), gCurrentLighting);
        if (FmFileExists(tmpFilename))
            exists = true;
        else if (fifaStadFilenameFormatDefault) {
            sprintf(tmpFilename, fifaStadFilenameFormatDefault, stadiumFolder.c_str());
            if (FmFileExists(tmpFilename))
                exists = true;
        }
        if (exists) {
            strcpy(dst, tmpFilename);
            return;
        }
    }
    if (extension)
        sprintf(dst, format, stadiumId, lightingId, extension);
    else
        sprintf(dst, format, stadiumId, lightingId);
}

void OnGetFlagsFilenameFor3dMatch(Char *dst, Char const *format, UInt stadiumId, UInt lightingId) {
    GetStadiumExtraFilenameFor3dMatch(dst, format, stadiumId, lightingId, "%s\\flags_%d.loc", "%s\\flags.loc");
}

void OnGetLightsFilenameFor3dMatch(Char *dst, Char const *format, UInt stadiumId, UInt lightingId) {
    GetStadiumExtraFilenameFor3dMatch(dst, format, stadiumId, lightingId, "%s\\lights_%d.loc", "%s\\lights.loc");
}

void OnGetCollisionFilenameFor3dMatch(Char *dst, Char const *format, UInt stadiumId, UInt lightingId) {
    GetStadiumExtraFilenameFor3dMatch(dst, format, stadiumId, lightingId, "%s\\collision_%d.bin", "%s\\collision.bin");
}

void OnGetEffectsFilenameFor3dMatch(Char *dst, Char const *format, UInt stadiumId, UInt lightingId) {
    GetStadiumExtraFilenameFor3dMatch(dst, format, stadiumId, lightingId, "%s\\effects_%d.txt", "%s\\effects.txt");
}

Char gShadowFilename[256];

void OnGetShadowFilenameFor3dMatch(Char *dst, Char const *format, UInt stadiumId, UInt lightingId) {
    GetStadiumExtraFilenameFor3dMatch(gShadowFilename, format, stadiumId, lightingId, "%s\\shadow_%d.shd", "%s\\shadow.shd");
}

void OnGetCovmapFilenameFor3dMatch(Char *dst, Char const *format, UInt stadiumId, UInt lightingId, Char const *extension) {
    GetStadiumExtraFilenameFor3dMatch(dst, format, stadiumId, lightingId, "%s\\covmap_%d.fsh", "%s\\covmap.fsh", extension);
}

void OnGetFxFilenameFor3dMatch(Char *dst, Char const *format, UInt stadiumId, UInt lightingId) {
    GetStadiumExtraFilenameFor3dMatch(dst, format, stadiumId, lightingId, "%s\\fx_%d.big", "%s\\fx.big");
}

void *METHOD OnSetupCustomStadiumCrowdEffects(CDBTeam *team, DUMMY_ARG, void *a2) {
    SetVarInt("OPTIONS/CROWDFLAGS", 100);
    SetVarInt("OPTIONS/CROWDFLARES", 100);
    SetVarInt("OPTIONS/CROWDFLASHES", 100);
    SetVarInt("OPTIONS/CROWDSTREAMERS", 100);
    //Error("OnSetupCustomStadiumCrowdEffects");
    return CallMethodAndReturn<void *, 0xEC9450>(team, a2);
}

void METHOD OnOrigStadiumLoad(void *origStadium, DUMMY_ARG, const WideChar *stadiumFolder, UChar lighting) {
    GetCustomStadiumFolder() = stadiumFolder;
    gTrainingStadium = wcsstr(stadiumFolder, L"\\FIFA\\00000000") != 0;
    gCurrentLighting = lighting;
    gStadiumScaling[0] = 1.0f;
    gStadiumScaling[1] = 1.0f;
    gStadiumScaling[2] = 1.0f;
    gStadiumIntroSceneId = 0;
    void *stadiumEngine = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x4100B0));
    CallMethodDynGlobal(GfxCoreAddress(0x4211A0), origStadium, stadiumFolder, lighting);
    auto LoadShape = [&](WideChar const *fileName, Bool isGlobal, void *&outShape) {
        outShape = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x418DA0), CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x418D30), 12, "NewStadGen::OrigStad::Textures"));
        if (CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x418DF0), outShape, fileName, stadiumEngine, isGlobal))
            CallMethodDynGlobal(GfxCoreAddress(0x420780), *raw_ptr<void *>(origStadium, 8), outShape);
    };
    LoadShape(L"t13__201002_0_0.fsh", true, shapeBannersHome);
    LoadShape(L"t13__201003_0_0.fsh", true, shapeBannersAway);

    static WideChar filename[512];

    _snwprintf(filename, std::size(filename), L"%s\\config.ini", stadiumFolder);
    if (FmFileExists(filename)) {
        WideChar scaleStr[256];
        GetPrivateProfileStringW(L"MAIN", L"SCALE", L"", scaleStr, 256, filename);
        if (scaleStr[0] != L'\0') {
            String sstr = scaleStr;
            if (!sstr.empty()) {
                auto scaleParts = Utils::Split(sstr, L',');
                if (scaleParts.size() >= 1 && scaleParts.size() <= 3) {
                    Float x = Utils::SafeConvertFloat(scaleParts[0]);
                    if (x != 0.0f)
                        gStadiumScaling[0] = x;
                    if (scaleParts.size() >= 2) {
                        Float y = Utils::SafeConvertFloat(scaleParts[1]);
                        if (y != 0.0f)
                            gStadiumScaling[1] = y;
                        if (scaleParts.size() >= 3) {
                            Float z = Utils::SafeConvertFloat(scaleParts[2]);
                            if (z != 0.0f)
                                gStadiumScaling[2] = z;
                        }
                    }
                    else {
                        gStadiumScaling[1] = x;
                        gStadiumScaling[2] = x;
                    }
                }
            }
        }
        WideChar introStr[512];
        GetPrivateProfileStringW(L"MAIN", L"INTRO", L"", introStr, 512, filename);
        if (introStr[0] != L'\0') {
            String istr = introStr;
            auto iparts = Utils::Split(istr, L',');
            if (!iparts.empty()) {
                if (iparts.size() == 1)
                    gStadiumIntroSceneId = Utils::SafeConvertInt<Int>(iparts[0]);
                else
                    gStadiumIntroSceneId = Utils::SafeConvertInt<Int>(iparts[Random::Get(0, iparts.size() - 1)]);
            }
        }
    }

    _snwprintf(filename, std::size(filename), L"%s\\shadow_%d.o", stadiumFolder, lighting);

    if (FmFileExists(filename)) {
        modelShadow = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x41F8F0), 172, "NewStadGen::OrigStad::ShadowModel");
        if (modelShadow) {
            modelShadow = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x41F930), modelShadow);
            void *fileData = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x410950), stadiumEngine, filename);
            if (fileData)
                CallMethodDynGlobal(GfxCoreAddress(0x41F9B0), modelShadow, fileData);
            void *textures = *raw_ptr<void *>(origStadium, 0x10);
            if (textures)
                CallMethodDynGlobal(GfxCoreAddress(0x420780), modelShadow, textures);
        }
    }
    else
        modelShadow = nullptr;
}

void METHOD OnOrigStadiumDestroy(void *origStadium) {
    CallMethodDynGlobal(GfxCoreAddress(0x420FE0), origStadium);
    if (modelShadow) {
        CallVirtualMethod<4>(modelShadow);
        CallMethodDynGlobal(GfxCoreAddress(0x410BF0), modelShadow, 1);
    }
    modelShadow = nullptr;
    auto DestroyShape = [](void *&shape) {
        if (shape) {
            CallVirtualMethod<4>(shape);
            CallMethodDynGlobal(GfxCoreAddress(0x413590), shape, 1);
            shape = nullptr;
        }
    };
    DestroyShape(shapeBannersHome);
    DestroyShape(shapeBannersAway);
}

UInt METHOD OnGetTeamIdForStadium(CDBTeam *team) {
    void *match = *(void **)0x3124748;
    if (match && CallMethodAndReturn<int, 0xE81510>(match) == 2)
        return 0;
    if (CallMethodAndReturn<Bool, 0xF705E0>(CallMethodAndReturn<void *, 0xECFFC0>(team))) {
        CDBGame *game = CDBGame::GetInstance();
        if (game) {
            if (team->IsManagedByAI()) {
                if (!game->GetOptions().CheckFlag(34))
                    return 0;
            }
            else {
                if (!game->GetOptions().CheckFlag(33))
                    return 0;
            }
        }
    }
    return CallMethodAndReturn<UInt, 0xEC9440>(team);
}

UInt METHOD OnGetTeamIdForStadium2(CDBTeam *team) {
    void *match = *(void **)0x3124748;
    if (match && CallMethodAndReturn<int, 0xE81510>(match) == 2)
        return 0;
    return CallMethodAndReturn<UInt, 0xEC9440>(team);
}

void METHOD OnAddCustomStadiumPath(void *t, DUMMY_ARG, void *a, void *b) {
    void *match = *(void **)0x3124748;
    WideChar const *stadiumPath = (WideChar const *)(UInt(a) + 12);
    if (!wcsstr(stadiumPath, L"FIFA\\00000000\\stadium_") || !match || CallMethodAndReturn<int, 0xE81510>(match) == 2)
        CallMethod<0x441EE0>(t, a, b);
}

UInt METHOD OnGetMatchStadiumType(void *t) {
    void *match = *(void **)0x3124748;
    if (match && CallMethodAndReturn<int, 0xE81510>(match) == 2)
        return 0;
    return CallMethodAndReturn<UInt, 0xE7FDD0>(t);
}

void METHOD OnReadFifaStadiumIdFromDb(void *t, DUMMY_ARG, unsigned int *out) {
    CallMethod<0x1338B10>(t, out);
    *out = 0;
}

void METHOD OnReadFifaStadiumIdFromSave(void *t, DUMMY_ARG, unsigned char *out) {
    CallMethod<0x1080390>(t, out);
    *out = 0;
}

void NewStadiumAndEffectsRender() {
    if (*(UInt *)GfxCoreAddress(0xADDC14) == 99) {
        void *stadiumEngine = (void *)GfxCoreAddress(0xD49E78);
        void *origStadium = raw_ptr<void>(stadiumEngine, 0x1008);
        if (*raw_ptr<UChar>(origStadium, 4)) {
            void *model = *raw_ptr<void *>(origStadium, 8);
            if (model) {
                float transform[16];
                CallMethodDynGlobal(GfxCoreAddress(0x2D8C50), transform, 1.0f, 1.0f, 1.0f, 1.0f);
                CallMethodDynGlobal(GfxCoreAddress(0x41FB60), model, transform);
            }
        }
    }
    CallDynGlobal(GfxCoreAddress(0x218950));
}

Bool gRenderingShadow = false;

void RenderStadiumShadow() {
    CallDynGlobal(GfxCoreAddress(0x20C6D0));
    gRenderingShadow = true;
    if (modelShadow && *(UInt *)GfxCoreAddress(0xADDC14) == 99) {
        void *modelShadowModel = *raw_ptr<void *>(modelShadow, 0xA8);
        if (modelShadowModel) {
            void *stadiumEngine = (void *)GfxCoreAddress(0xD49E78);
            void *origStadium = raw_ptr<void>(stadiumEngine, 0x1008);
            if (*raw_ptr<UChar>(origStadium, 4)) {
                CallDynGlobal(GfxCoreAddress(0x451540), GfxCoreAddress(0xABE9D4), 0.53f, 90.0f);
                float transform[16];
                CallMethodDynGlobal(GfxCoreAddress(0x2D8C50), transform, 1.0f, 1.0f, 1.0f, 1.0f);
                //transform[13] = 20.0f;
                //*(UInt *)GfxCoreAddress(0x64C898) = 123;
                CallMethodDynGlobal(GfxCoreAddress(0x41FB60), modelShadow, transform);
                //CallMethodDynGlobal(GfxCoreAddress(0x2E2230), modelShadowModel, transform);
                CallDynGlobal(GfxCoreAddress(0x451540), nullptr, 0.0f, 0.0f);
            }
        }
    }
    gRenderingShadow = false;
    
}

Int METHOD OnShadowFilename1(void *t, DUMMY_ARG, Char const *filename) {
    return CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x44C5E0), t, gShadowFilename);
}

Int METHOD OnShadowFilename2(void *t, DUMMY_ARG, Char const *memPoolName, Char const *fileName, Int priority, Int flags, void *callback, void *callbackData, Int arg_18) {
    return CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x44D7F0), t, memPoolName, gShadowFilename, priority, flags, callback, callbackData, arg_18);
}

Int OnShadowFilename3(Char const *dst, Char const *format, Int lighting) {
    if (*(UInt *)GfxCoreAddress(0xADDC14) == 99) {
        void *stadiumEngine = (void *)GfxCoreAddress(0xD49E78);
        void *origStadium = raw_ptr<void>(stadiumEngine, 0x1008);
        if (*raw_ptr<UChar>(origStadium, 4))
            lighting = gCurrentLighting;
        else {
            void *match = *(void **)0x3124748;
            if (match) {
                UChar hour = CallMethodAndReturn<UChar, 0xE811C0>(match);
                if (hour >= 20 || hour <= 6)
                    lighting = 3;
                else {
                    UChar weather = CallMethodAndReturn<UChar, 0xE81160>(match);
                    if (weather <= 2)
                        lighting = 0;
                    else {
                        if (hour >= 18 || hour <= 8)
                            lighting = 4;
                        else
                            lighting = 1; // TODO: mistake was here?
                    }
                }
            }
        }
    }
    return sprintf(gShadowFilename, "shad_xx%d.shd", lighting);
}

void METHOD OnStadGeoPrimStateCullEnable(void *t, DUMMY_ARG, Int arg) {
    CallMethodDynGlobal(GfxCoreAddress(0x2D9BE0), t, arg);
    if (gRenderingShadow)
        CallMethodDynGlobal(GfxCoreAddress(0x2D9DC0), t, 0);
}

void OnSetupShadow(float *color, float a, float b) {
    CallDynGlobal(GfxCoreAddress(0x451540), color, a, b);
    Error("%g %g %g %g - %g - %g", color[0], color[1], color[2], color[3], a, b);
}

void METHOD OnSetCustomStadiumScale(void *m, DUMMY_ARG, float x, float y, float z, float w) {
    CallMethodDynGlobal(GfxCoreAddress(0x2D8C50), m, gStadiumScaling[0], gStadiumScaling[1], gStadiumScaling[2], w);
}

Int METHOD OnGetStadiumIntroID(void *vars, DUMMY_ARG, Char const *varname, Int a3) {
    Int stadiumId = gfxGetVarInt(varname, a3);
    if (stadiumId == 999 && gStadiumIntroSceneId != 0) {
        void *stadiumEngine = (void *)GfxCoreAddress(0xD49E78);
        void *origStadium = raw_ptr<void>(stadiumEngine, 0x1008);
        if (*raw_ptr<UChar>(origStadium, 4))
            stadiumId = gStadiumIntroSceneId;
    }
    return stadiumId;
}

int METHOD OnGetIntTrainingStadium(void *t, DUMMY_ARG, const char *varname, int varoption) {
    Int stadiumId = gfxGetVarInt("STADIUM", 1);
    if (stadiumId == 999) {
        if (IsCustomStadiumLoaded()) {
            if (gTrainingStadium)
                return 39;
        }
    }
    return stadiumId;
}

void OnSetupRenderCamera(float cameraLensAngle, float nearClip, float farClip) {
    CallDynGlobal(GfxCoreAddress(0x453750), cameraLensAngle, nearClip, 150000.0f);
}

int METHOD OnGetDayOfTimeForStadium(void *t) {
    void *match = *(void **)0x3124748;
    if (match && CallMethodAndReturn<int, 0xE81510>(match) == 2) {
        if (Random::Get(1, 100) > 50)
            return 6;
        return 0;
    }
    return CallMethodAndReturn<int, 0x1494FBA>(t);
}

char METHOD OnCheckStadiumWeatherForTeamType(void *t) {
    void *match = *(void **)0x3124748;
    if (match && CallMethodAndReturn<int, 0xE81510>(match) == 2)
        return true;
    return CallMethodAndReturn<char, 0xF72D70>(t);
}

bool gMatchPreviewSwapMatchHomeAway = false;

void * METHOD OnGetMatchForPreview(void *matchList, DUMMY_ARG, unsigned int index) {
    unsigned int numMatches = CallMethodAndReturn<unsigned int, 0xE884A0>(matchList);
    if (index >= numMatches) {
        index -= numMatches;
        gMatchPreviewSwapMatchHomeAway = true;
    }
    else
        gMatchPreviewSwapMatchHomeAway = false;
    return CallMethodAndReturn<void *, 0xE884B0>(matchList, index);
}

unsigned int METHOD OnGetMatchPreviewNumMatches(void *matchList) {
    return CallMethodAndReturn<unsigned int, 0xE884A0>(matchList) * 2;
}

CTeamIndex *METHOD OnGetMatchPreviewHomeTeamID(void *match, DUMMY_ARG, CTeamIndex *teamId) {
    if (gMatchPreviewSwapMatchHomeAway)
        return CallMethodAndReturn<CTeamIndex *, 0xE7FD00>(match, teamId);
    else
        return CallMethodAndReturn<CTeamIndex *, 0xE7FCF0>(match, teamId);
}

CTeamIndex *METHOD OnGetMatchPreviewAwayTeamID(void *match, DUMMY_ARG, CTeamIndex *teamId) {
    if (gMatchPreviewSwapMatchHomeAway)
        return CallMethodAndReturn<CTeamIndex *, 0xE7FCF0>(match, teamId);
    else
        return CallMethodAndReturn<CTeamIndex *, 0xE7FD00>(match, teamId);
}

void *METHOD OnCopyMatchForMatchPreview(void *matchList, DUMMY_ARG, unsigned int index, CDBOneMatch *outMatch) {
    unsigned int numMatches = CallMethodAndReturn<unsigned int, 0xE884A0>(matchList);
    Bool isSwapped = index >= numMatches;
    if (isSwapped)
        index -= numMatches;
    void *result = CallMethodAndReturn<void *, 0xE8BB20>(matchList, index, outMatch);
    if (isSwapped) {
        CTeamIndex homeTeamID = outMatch->GetHomeTeamID();
        CTeamIndex awayTeamID = outMatch->GetAwayTeamID();
        *raw_ptr<CTeamIndex>(outMatch, 8) = awayTeamID;
        *raw_ptr<CTeamIndex>(outMatch, 12) = homeTeamID;
    }
    return result;
}

void PatchCustomStadiums(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // movzx   ebp, al  >>  mov ebp, eax
        patch::SetUShort(0x65D777, 0xE88B);
        patch::Nop(0x65D777 + 2, 1);
        // fifa stadium check
        patch::RedirectCall(0x65D772, GetTeamStadiumId);
        // remove generic FIFA stadiums
        patch::RedirectJump(0x442B43, (void *)0x442D3F);
        // default stadium
        patch::RedirectCall(0x4422E3, OnAddCustomStadiumPath);
        patch::RedirectCall(0x4422F9, OnGetMatchStadiumType);
        // default stadium random time of day
        patch::RedirectCall(0x442822, OnGetDayOfTimeForStadium);
        patch::RedirectCall(0x44284D, OnCheckStadiumWeatherForTeamType);
        //
        patch::RedirectCall(0x442B25, OnSetupCustomStadiumCrowdEffects);

        patch::RedirectCall(0x4422B6, OnGetTeamIdForStadium);
        patch::RedirectCall(0x4423F9, OnGetTeamIdForStadium2);

        // weather fix
        patch::SetUChar(0x44231D + 1, 0x97);

        // weather fix for training ground matches
        patch::RedirectCall(0xA942E6, OnGetMatchForPreview);
        patch::RedirectCall(0xA94322, OnGetMatchPreviewNumMatches);
        patch::RedirectCall(0xA942F4, OnGetMatchPreviewHomeTeamID);
        patch::RedirectCall(0xA94304, OnGetMatchPreviewAwayTeamID);
        patch::RedirectCall(0xA94340, OnCopyMatchForMatchPreview);

        if (!Settings::GetInstance().EnableDefaultStadiums) {
            patch::RedirectCall(0xF80AC7, OnReadFifaStadiumIdFromDb);
            patch::RedirectCall(0xF77BCC, OnReadFifaStadiumIdFromSave);
        }
    }
}

void InstallCustomStadiums3DPatches() {
    patch::RedirectCall(GfxCoreAddress(0x37BEDD), ReadXmlDataForStadium);
    patch::RedirectCall(GfxCoreAddress(0x37BF26), SetupStadiumXmlResolver);
    patch::RedirectCall(GfxCoreAddress(0x375E57), OnAddResourceToCache);
    patch::RedirectCall(GfxCoreAddress(0x3A1094), OnLoadEaglXSceneTexture);
    patch::RedirectCall(GfxCoreAddress(0x208440), OnGetFlagsFilenameFor3dMatch);
    patch::RedirectCall(GfxCoreAddress(0x21CB85), OnGetLightsFilenameFor3dMatch);
    patch::RedirectCall(GfxCoreAddress(0x12C0EA), OnGetCollisionFilenameFor3dMatch);
    patch::RedirectCall(GfxCoreAddress(0x218C35), OnGetEffectsFilenameFor3dMatch);
    patch::RedirectCall(GfxCoreAddress(0x20A498), OnGetShadowFilenameFor3dMatch);
    patch::RedirectCall(GfxCoreAddress(0x213FB7), OnGetCovmapFilenameFor3dMatch);
    patch::RedirectCall(GfxCoreAddress(0x1FFFCA), OnGetFxFilenameFor3dMatch);

    patch::RedirectCall(GfxCoreAddress(0x410868), OnOrigStadiumLoad);
    patch::RedirectCall(GfxCoreAddress(0x420F43), OnOrigStadiumDestroy);
    patch::SetPointer(GfxCoreAddress(0x55D20C), OnOrigStadiumDestroy);

    // remove custom FIFA stadium scaling
    //patch::Nop(GfxCoreAddress(0x420FB9), 5);
    //static Float fOne = 1.0f;
    //patch::SetPointer(GfxCoreAddress(0x420F9D + 2), &fOne);
    //patch::SetPointer(GfxCoreAddress(0x420FAD + 2), &fOne);
    patch::RedirectCall(GfxCoreAddress(0x420FB9), OnSetCustomStadiumScale);

    // temporary - remove flag reset for custom stadiums - TODO
    patch::Nop(GfxCoreAddress(0x208450), 5);

    // move stadium rendering
    //patch::Nop(GfxCoreAddress(0x420FD0), 12); // disable original rendering
    //patch::SetPointer(GfxCoreAddress(0x638334), NewStadiumAndEffectsRender); // put rendering to new place

    // preview move
    patch::SetInt(GfxCoreAddress(0x37BF98 + 2), -4464);
    patch::SetInt(GfxCoreAddress(0x37BFB1 + 2), -4720);

    patch::RedirectCall(GfxCoreAddress(0x20A4C3), OnShadowFilename1);
    patch::RedirectCall(GfxCoreAddress(0x20A563), OnShadowFilename2);
    patch::RedirectCall(GfxCoreAddress(0x20A523), OnShadowFilename3);

    patch::SetPointer(GfxCoreAddress(/*0x638434*/ 0x63843C), RenderStadiumShadow);

    patch::RedirectCall(GfxCoreAddress(0x41FC2B), OnStadGeoPrimStateCullEnable);

    //patch::RedirectCall(GfxCoreAddress(0x454665), OnSetupShadow);

    const float StadiumFarClip = 150000.0f;
    patch::SetFloat(GfxCoreAddress(0x630DE4), StadiumFarClip);
    patch::SetFloat(GfxCoreAddress(0x630DEC), StadiumFarClip);
    patch::RedirectCall(GfxCoreAddress(0x1E833F), OnSetupRenderCamera);
    patch::RedirectCall(GfxCoreAddress(0x1E837F), OnSetupRenderCamera);

    patch::RedirectCall(GfxCoreAddress(0x21CF80), OnGetStadiumIntroID);

    patch::RedirectCall(GfxCoreAddress(0x22BB2E), OnGetIntTrainingStadium);
    patch::RedirectCall(GfxCoreAddress(0x22BB69), OnGetIntTrainingStadium);
    patch::RedirectCall(GfxCoreAddress(0x22FEEB), OnGetIntTrainingStadium);
    patch::RedirectCall(GfxCoreAddress(0x233CC9), OnGetIntTrainingStadium);
    patch::RedirectCall(GfxCoreAddress(0x2356CB), OnGetIntTrainingStadium);
}
