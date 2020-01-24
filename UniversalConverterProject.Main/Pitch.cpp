#include "Pitch.h"
#include "FifamTypes.h"
#include "GfxCoreHook.h"
#include "FifamReadWrite.h"
#include "license_check/license_check.h"
#include "GameInterfaces.h"
#include "FifamNation.h"
#include "Random.h"
#include "shared.h"
#include "Settings.h"

using namespace plugin;

struct StadiumData {
    UInt mowPattern = 1;
    UInt netColor = 0;
    UChar environment = 0;
    Bool isSynthetic = false;
    UInt pitchColorDay = 0;
    Float pitchBrightnessDay = 0.0f;
    UInt pitchColorOvercast = 0;
    Float pitchBrightnessOvercast = 0.0f;
    UInt pitchColorNight = 0;
    Float pitchBrightnessNight = 0.0f;
};

Map<UInt, StadiumData> &GetStadiumDataMap() {
    static Map<UInt, StadiumData> stadiumDataMap;
    return stadiumDataMap;
}

StadiumData *GetTeamStadiumData(UInt teamId) {
    auto it = GetStadiumDataMap().find(teamId);
    if (it != GetStadiumDataMap().end())
        return &(*it).second;
    return nullptr;
}

void ReadStadiumDataFile() {
    FifamReader reader(Magic<'p', 'l', 'u', 'g', 'i', 'n', 's', '\\', 'u', 'c', 'p', '\\', 's', 't', 'a', 'd', 'i', 'u', 'm', 'd', 'a', 't', 'a', '.', 'c', 's', 'v'>(1264444409), 14);
    if (reader.Available()) {
        reader.SkipLine();
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                UInt teamId = 0;
                StadiumData data;
                reader.ReadLine(Hexadecimal(teamId), data.mowPattern, data.netColor, data.environment, data.isSynthetic, 
                    Hexadecimal(data.pitchColorDay), data.pitchBrightnessDay,
                    Hexadecimal(data.pitchColorOvercast), data.pitchBrightnessOvercast,
                    Hexadecimal(data.pitchColorNight), data.pitchBrightnessNight);
                if (teamId != 0)
                    GetStadiumDataMap()[teamId] = data;
            }
            else
                reader.SkipLine();
        }
    }
}

Bool GetHour() {
    void *match = *(void **)0x3124748;
    if (match)
        return CallMethodAndReturn<UChar, 0xE811C0>(match);
    return 0;
}

Bool GetIsCloudy() {
    void *match = *(void **)0x3124748;
    if (match) {
        UChar weather = CallMethodAndReturn<UChar, 0xE81160>(match);
        if (weather == 2 || weather == 3)
            return true;
    }
    return false;
}

Bool GetIsRainy() {
    void *match = *(void **)0x3124748;
    if (match) {
        UChar weather = CallMethodAndReturn<UChar, 0xE81160>(match);
        if (weather == 0)
            return true;
    }
    return false;
}

Bool GetIsSnowy() {
    void *match = *(void **)0x3124748;
    if (match) {
        UChar weather = CallMethodAndReturn<UChar, 0xE81160>(match);
        if (weather == 1)
            return true;
    }
    return false;
}

UInt GetLighting() {
    return gfxGetVarInt("LIGHTING", 1);
}

struct RGBAReal {
    Float r, g, b, a;

    void Set(Float _r, Float _g, Float _b, Float _a) {
        r = _r; g = _g; b = _b; a = _a;
    }

    void SetRGB(UChar _r, UChar _g, UChar _b) {
        r = Float(_r) / 255.0f;
        g = Float(_g) / 255.0f;
        b = Float(_b) / 255.0f;
    }

    void SetInt(UInt value) {
        r = Float((value >> 16) & 0xFF) / 255.0f;
        g = Float((value >> 8) & 0xFF) / 255.0f;
        b = Float(value & 0xFF) / 255.0f;
    }
};

Int OnReadLightingFile(char const *filename) {
    Int result = CallAndReturnDynGlobal<Int>(GfxCoreAddress(0x1EE710), filename);

    CTeamIndex hostTeamId = CTeamIndex::make(0, 0, 0);
    CDBTeam *hostTeam = nullptr;
    StadiumData *stadiumData = nullptr;
    UInt stadiumCapacity = 100'000;
    UChar lighting = 0;
    Bool isCloudy = GetIsCloudy();
    Bool isRainy = GetIsRainy();
    Bool isSnowy = GetIsSnowy();
    UChar dayOfWeek = CDBGame::GetInstance()->GetCurrentDate().Value() % 7;
    if (dayOfWeek != 5 && dayOfWeek != 6)
        lighting = 2;
    else if (isCloudy || isRainy || isSnowy)
        lighting = 1;

    SafeLog::Write(Utils::Format(L"hour: %d lighting: %d isCloudy: %d isRainy: %d isSnowy: %d", GetHour(), lighting, isCloudy, isRainy, isSnowy));

    void *match = *(void **)0x3124748;
    if (match) {
        CallMethod<0xE814C0>(match, &hostTeamId);
        if (hostTeamId.countryId) {
            hostTeam = GetTeam(hostTeamId);
            if (hostTeam) {
                stadiumData = GetTeamStadiumData(hostTeam->GetTeamUniqueID());
                void *stadiumDevelopment = CallMethodAndReturn<void *, 0xECFFC0>(hostTeam);
                if (stadiumDevelopment) {
                    stadiumCapacity = CallMethodAndReturn<UInt, 0xF74220>(stadiumDevelopment);
                }
            }
        }
    }

    if (isSnowy) {
        RGBAReal *grassColor = (RGBAReal *)GfxCoreAddress(0xA7C1D8);
        RGBAReal *gPatternColor = (RGBAReal *)GfxCoreAddress(0xA7C200);
        RGBAReal *gBrightnessColor = (RGBAReal *)GfxCoreAddress(0xA7C1F0);
        RGBAReal *gGrassWearColor = (RGBAReal *)GfxCoreAddress(0x710FA8);
        RGBAReal *gRoughnessColor = (RGBAReal *)GfxCoreAddress(0xA7C1C4);
        RGBAReal *gPatternColor2 = (RGBAReal *)GfxCoreAddress(0x710FB8);
        RGBAReal *gUnkColor1 = (RGBAReal *)GfxCoreAddress(0x711030);
        RGBAReal *gUnkColor2 = (RGBAReal *)GfxCoreAddress(0x711068);

        const Float grassClr = 1.0f;
        grassColor->Set(grassClr, grassClr, grassClr, 1);
        //gBrightnessColor->Set(1, 1, 1, 1);
        gPatternColor->Set(1, 1, 0, 0);
        //gRoughnessColor->Set(1, 1, 1, 0);
        //gPatternColor2->Set(1, 1, 1, 0);

        gRoughnessColor->r = 16.0f; // x scale?
        gPatternColor2->r = 0.1f;
        gPatternColor2->g = 0.0f;
        gPatternColor2->b = 0.0f;
        gPatternColor2->a = 1.0f;

        gGrassWearColor->SetInt(0x242117);
        gGrassWearColor->a = 0.0f; // unknown
        gRoughnessColor->a = 0.0f;

        gUnkColor1->r = 0.0f;
        gUnkColor1->g = 0.0f;
        gUnkColor1->b = 0.0f;
        gUnkColor1->a = 0.0f;
        
        gUnkColor1->r = 0.0f;
        gUnkColor1->g = 0.0f;
        gUnkColor1->b = 0.0f;
        gUnkColor1->a = 0.0f;

    }
    else {
        RGBAReal *grassColor = (RGBAReal *)GfxCoreAddress(0xA7C1D8);
        RGBAReal *gSpecialVisibility = (RGBAReal *)GfxCoreAddress(0xA7C200);
        RGBAReal *gBrightnessColor = (RGBAReal *)GfxCoreAddress(0xA7C1F0);
        RGBAReal *gGrassWearColor = (RGBAReal *)GfxCoreAddress(0x710FA8);
        RGBAReal *gGrassScale = (RGBAReal *)GfxCoreAddress(0xA7C1C4);
        RGBAReal *gPatternColor2 = (RGBAReal *)GfxCoreAddress(0x710FB8);
        RGBAReal *gUnkColor1 = (RGBAReal *)GfxCoreAddress(0x711030);
        RGBAReal *gUnkColor2 = (RGBAReal *)GfxCoreAddress(0x711068);

        if (Settings::GetInstance().getUseNew3dPitch()) {
            gSpecialVisibility->r = 1.0f; // unknown (default 1)
        }

        // Lines visibility
        if (stadiumData && stadiumData->isSynthetic)
            gSpecialVisibility->b = 1.0f; // lines visibility
        else if (stadiumCapacity < 500)
            gSpecialVisibility->b = 0.0f; // lines visibility
        else {
            if (lighting == 2)
                gSpecialVisibility->b = 0.85f; // lines visibility
            else
                gSpecialVisibility->b = 0.55f; // lines visibility
        }
        // Pattern visibility
        if (stadiumData && stadiumData->isSynthetic)
            gSpecialVisibility->a = 1.0f; // pattern visibility
        else {
            if (stadiumCapacity < 2000)
                gSpecialVisibility->a = 0.0f; // pattern visibility
            else {
                if (lighting == 2)
                    gSpecialVisibility->a = 0.25f; // pattern visibility
                else
                    gSpecialVisibility->a = 0.55f; // pattern visibility
            }
        }
        // Dirt lines
        if (stadiumCapacity > 35000 || (stadiumData && stadiumData->isSynthetic))
            gSpecialVisibility->g = 0.0f; // lines dirt (brown) visibility
        else {
            if (stadiumCapacity > 3000) {
                gSpecialVisibility->g = Float(stadiumCapacity - 3000) / 32.000f;
                if (gSpecialVisibility->g > 1.0f)
                    gSpecialVisibility->g = 1.0f;
                else if (gSpecialVisibility->g < 0.0f)
                    gSpecialVisibility->g = 0.0f;
                SafeLog::Write(Utils::Format(L"gSpecialVisibility->g : %g", gSpecialVisibility->g));
            }
            else
                gSpecialVisibility->g = 1.0f;
        }
        // Dirt lines color
        if (isRainy)
            gGrassWearColor->SetInt(0x242117);
        else
            gGrassWearColor->SetInt(0x8d845c);
        // Grass color and brightness
        gGrassScale->r = 8.0f; // x scale?
        gGrassScale->g = 8.0f; // y scale?
        gGrassScale->b = 18.0f;
        gGrassScale->a = 0.0f;

        if (Settings::GetInstance().getUseNew3dPitch() || stadiumData) {

            UInt customColor = 0;
            Float customBrightness = 0.0f;

            enum class StadWeather { Day, Overcast, Night } stadWeather = StadWeather::Day;
            if (lighting == 2)
                stadWeather = StadWeather::Night;
            else if (lighting == 1)
                stadWeather = StadWeather::Overcast;
            else {
                if (Random::Get(0, 100) < 33)
                    stadWeather = StadWeather::Overcast;
                else
                    stadWeather = StadWeather::Day;
            }

            if (stadiumData) {
                if (stadWeather == StadWeather::Night) {
                    customColor = stadiumData->pitchColorNight;
                    customBrightness = stadiumData->pitchBrightnessNight;
                }
                else if (stadWeather == StadWeather::Overcast) {
                    customColor = stadiumData->pitchColorOvercast;
                    customBrightness = stadiumData->pitchBrightnessOvercast;
                }
                else {
                    customColor = stadiumData->pitchColorDay;
                    customBrightness = stadiumData->pitchBrightnessDay;
                }
                if (customColor == 0 && stadiumData->isSynthetic)
                    customColor = 0x20d835;
            }

            if (customColor != 0)
                grassColor->SetInt(customColor);
            else {
                if (stadWeather == StadWeather::Night)
                    grassColor->SetInt(0x6edf24);
                else if (stadWeather == StadWeather::Overcast)
                    grassColor->SetInt(0x6edf24);
                else {
                    UInt rndValue = Random::Get(0, 100);
                    if (rndValue < 33)
                        grassColor->SetInt(0x7ccc47);
                    else if (rndValue < 66)
                        grassColor->SetInt(0x93d012);
                    else
                        grassColor->SetInt(0x7ec701);
                }
            }
            if (customBrightness == 0.0f) {
                if (stadWeather == StadWeather::Night)
                    customBrightness = 1.2f;
                else if (stadWeather == StadWeather::Overcast)
                    customBrightness = 0.9f;
                else
                    customBrightness = 1.0f;
            }

            gBrightnessColor->r = customBrightness;
            gBrightnessColor->g = customBrightness;
            gBrightnessColor->b = customBrightness;
            gBrightnessColor->a = 1.0f;

            if (Settings::GetInstance().getUseNew3dPitch()) {
                gPatternColor2->r = 0.06f;
                gPatternColor2->g = 0.108f;
                gPatternColor2->b = 0.119f;
                gPatternColor2->a = 0.4f;

                gUnkColor1->r = 0.0f;
                gUnkColor1->g = 0.0f;
                gUnkColor1->b = 0.0f;
                gUnkColor1->a = 0.4f;

                gUnkColor1->r = 0.0f;
                gUnkColor1->g = 0.0f;
                gUnkColor1->b = 0.4f;
                gUnkColor1->a = 0.0f;
            }
        }
    }

    return result;
}

void OnSetupStadiumPitch() {
    CallDynGlobal(GfxCoreAddress(0x2403D0));

    UInt mowPattern = 1;
    UInt netColor = 0;
    CTeamIndex hostTeamId = CTeamIndex::make(0, 0, 0);
    CDBTeam *hostTeam = nullptr;
    StadiumData *stadiumData = nullptr;

    void *match = *(void **)0x3124748;
    if (match) {
        CallMethod<0xE814C0>(match, &hostTeamId);
        if (hostTeamId.countryId) {
            hostTeam = GetTeam(hostTeamId);
            if (hostTeam) {
                stadiumData = GetTeamStadiumData(hostTeam->GetTeamUniqueID());
                if (stadiumData) {
                    mowPattern = stadiumData->mowPattern;
                    netColor = stadiumData->netColor;
                }
            }
        }
    }

    patch::SetUInt(GfxCoreAddress(0xAE0B20), mowPattern, false);
    patch::SetUInt(GfxCoreAddress(0xADDCD4), netColor, false);

    if (GetIsSnowy()) {
        patch::SetUInt(GfxCoreAddress(0xAE0B24), 4, false); // drywet
        patch::SetUInt(GfxCoreAddress(0xAE0B38), 3, false); // wearlevel
        patch::SetUInt(GfxCoreAddress(0xAE0B3C), 1, false); // snowypitch
    }
    else {
        if (hostTeam) {
            if (!stadiumData || !stadiumData->isSynthetic) {
            //UChar teamDivision = CallMethodAndReturn<UChar, 0xECC430>(hostTeam, hostTeamId.type);
                void *stadiumDevelopment = CallMethodAndReturn<void *, 0xECFFC0>(hostTeam);
                if (stadiumDevelopment) {
                    UInt stadiumCapacity = CallMethodAndReturn<UInt, 0xF74220>(stadiumDevelopment);
                    if (stadiumCapacity < 5000)
                        patch::SetUInt(GfxCoreAddress(0xAE0B38), 3, false);
                    else if (stadiumCapacity < 10000)
                        patch::SetUInt(GfxCoreAddress(0xAE0B38), 2, false);
                    else if (stadiumCapacity < 15000)
                        patch::SetUInt(GfxCoreAddress(0xAE0B38), 1, false);
                }
            }
        }
    }
}

UChar METHOD OnGetMatchWeather(void *m) {
    return 4;
}

Int METHOD GetRainAlpha1(void *t, DUMMY_ARG, const Char *, Int, Int) {
    if (GetIsSnowy()) {
        static Float f0005 = 0.005f;
        patch::SetPointer(GfxCoreAddress(0x205CBB) + 2, &f0005);
        patch::SetPointer(GfxCoreAddress(0x205CCE) + 2, &f0005);
        return 110;
    }
    static Float f003 = 0.03f;
    patch::SetPointer(GfxCoreAddress(0x205CBB) + 2, &f003);
    static Float f005 = 0.05f;
    patch::SetPointer(GfxCoreAddress(0x205CCE) + 2, &f005);
    return 70;
}

Int METHOD GetRainAlpha2(void *t, DUMMY_ARG, const Char *, Int, Int) {
    return GetIsSnowy() ? 110 : 70;
}

CDBTeam *gCurrentTeamForStadium = nullptr;

void METHOD OnGetTeam_StadiumView(void *t) {
    gCurrentTeamForStadium = GetTeam(*raw_ptr<CTeamIndex>(t, 0x494));
    CallMethod<0x65D730>(t);
    gCurrentTeamForStadium = nullptr;
}

void METHOD OnGetTeam_StadiumEditor(void *t, DUMMY_ARG, Int unk) {
    gCurrentTeamForStadium = CallMethodAndReturn<CDBTeam *, 0x430930>((void *)0x3062D28);
    CallMethod<0x686C20>(t, unk);
    gCurrentTeamForStadium = nullptr;
}

void OnGetTeam_StadiumMatch(UChar unk) {
    void *match = *(void **)0x3124748;
    if (match) {
        CTeamIndex hostTeamId = CTeamIndex::make(0, 0, 0);
        CallMethod<0xE814C0>(match, &hostTeamId);
        if (hostTeamId.countryId)
            gCurrentTeamForStadium = GetTeam(hostTeamId);
    }
    Call<0x442190>(unk);
    gCurrentTeamForStadium = nullptr;
}

void GetTeamStadiumEnvironment(CDBTeam *team, UInt &environment) {
    if (team) {
        CTeamIndex teamId = gCurrentTeamForStadium->GetTeamID();
        StadiumData *stadiumData = GetTeamStadiumData(teamId.ToInt());
        if (stadiumData && stadiumData->environment >= 1 && stadiumData->environment <= 5) {
            switch (stadiumData->environment) {
            case 1:
                environment = (environment & 0xFFFF) | (9028 << 16);
            case 2:
                environment = (environment & 0xFFFF) | (9029 << 16);
            case 3:
                environment = (environment & 0xFFFF) | (9030 << 16);
            case 4:
                environment = (environment & 0xFFFF) | (9049 << 16);
            case 5:
                environment = (environment & 0xFFFF) | (9051 << 16);
            }
        }
        else if (teamId.countryId == FifamNation::Iceland)
            environment = (environment & 0xFFFF) | (9049 << 16);
        else if (teamId.countryId == FifamNation::Egypt)
            environment = (environment & 0xFFFF) | (9051 << 16);
        else {
            void *facilities = *raw_ptr<void *>(team, 0x76E4);
            if (facilities) {
                UInt landscape = *raw_ptr<UInt>(facilities, 0x1A8);
                UInt settlement = *raw_ptr<UInt>(facilities, 0x1AC);
                if (landscape == 0)
                    environment = (environment & 0xFFFF) | (9051 << 16);
                else if (landscape == 3 || settlement == 0)
                    environment = (environment & 0xFFFF) | (9030 << 16);
                else if (settlement == 2)
                    environment = (environment & 0xFFFF) | (9028 << 16);
                else if (settlement == 1)
                    environment = (environment & 0xFFFF) | (9029 << 16);
            }
        }
    }
}

void METHOD OnSetupStadiumEnvironment(void *t, DUMMY_ARG, UInt element, Int index) {
    GetTeamStadiumEnvironment(gCurrentTeamForStadium, element);
    CallMethodDynGlobal(GfxCoreAddress(0x40BFF0), t, element, index);
}

void PatchPitch(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadStadiumDataFile();
        // moderate climate base temperature
        patch::SetUChar(0x124366C + 1, 27);

        // 442190 - load stadium 3d match
        // 65D730 - stadium viewer
        // 686C20 - stadium editor
        patch::RedirectCall(0x65E8EC, OnGetTeam_StadiumView);
        patch::RedirectCall(0x65E903, OnGetTeam_StadiumView);
        patch::RedirectCall(0x65E91A, OnGetTeam_StadiumView);

        patch::SetPointer(0x23E52B8, OnGetTeam_StadiumEditor);
        
        patch::RedirectCall(0x442D93, OnGetTeam_StadiumMatch);

        //patch::SetUInt(GfxCoreAddress(0x40B819) + 1, 9051);
    }
}

void InstallPitch3D() {
    //patch::SetPointer(GfxCoreAddress(0x1F0F8E) + 1, "lines.dds");
    //patch::SetPointer(GfxCoreAddress(0x201E8A) + 1, "lines.dds");
    //patch::SetPointer(GfxCoreAddress(0x20C5C3) + 1, "lines.dds");
    //DWORD old;
    //VirtualProtect((void *)GfxCoreAddress(0x514590), 100, PAGE_READWRITE, &old);
    //strcpy((char *)GfxCoreAddress(0x514590), "lines.dds");

    //static Float f1 = -0.2f;
    //patch::SetPointer(GfxCoreAddress(0x205DA6) + 2, &f1);
    //
    patch::RedirectCall(GfxCoreAddress(0x205B39), GetRainAlpha1);
    //patch::RedirectCall(GfxCoreAddress(0x205B50), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x205B69), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x205B85), GetC<255>);
    patch::RedirectCall(GfxCoreAddress(0x205B9E), GetRainAlpha2);
    //patch::RedirectCall(GfxCoreAddress(0x205BB5), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x205BCE), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x205BEA), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x205C03), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x), GetC<255>);
    //patch::RedirectCall(GfxCoreAddress(0x), GetC<255>);

    //patch::RedirectJump(0xE81160, OnGetMatchWeather);

    patch::RedirectCall(GfxCoreAddress(0x1EE7FA), OnReadLightingFile);
    patch::RedirectCall(GfxCoreAddress(0x2414A1), OnSetupStadiumPitch);

    patch::RedirectCall(GfxCoreAddress(0x40B82B), OnSetupStadiumEnvironment);
}
