#include "Pitch.h"
#include "FifamTypes.h"
#include "GfxCoreHook.h"
#include "FifamReadWrite.h"
#include "license_check/license_check.h"
#include "GameInterfaces.h"
#include "FifamNation.h"
#include "Random.h"
#include "shared.h"
#include "UcpSettings.h"

using namespace plugin;

struct PitchColorConfig {
    UInt mPitchColorDay1 = 0x7ccc47;
    UInt mPitchColorDay2 = 0x93d012;
    UInt mPitchColorDay3 = 0x7ec701;
    UInt mPitchColorOvercast = 0x6edf24;
    UInt mPitchColorNight = 0x6edf24;
    UInt mPitchColorSynthetic = 0x20d835;
    UInt mGrassWear = 0x8d845c;
    UInt mGrassWearRain = 0x242117;
    Float mPitchBrightnessDay = 1.0f;
    Float mPitchBrightnessOvercast = 0.9f;
    Float mPitchBrightnessNight = 1.2f;
};

PitchColorConfig &GetPitchColorConfig() {
    static PitchColorConfig config;
    return config;
}

void ReadPitchColorConfig() {
    auto ReadFloat = [](String const &key, Float &out) {
        WideChar buf[260];
        UInt res = GetPrivateProfileStringW(L"MAIN", key.c_str(), Utils::Format(L"%f", out).c_str(), buf, 260, L"plugins\\ucp\\pitch_color.ini");
        if (res != 0)
            out = Utils::SafeConvertFloat(buf);

        //Error(L"%s : %g", key.c_str(), out);
    };

    auto ReadColor = [](String const &key, UInt &out) {
        WideChar buf[260];
        UInt res = GetPrivateProfileStringW(L"MAIN", key.c_str(), Utils::Format(L"%x", out).c_str(), buf, 260, L"plugins\\ucp\\pitch_color.ini");
        if (res != 0)
            out = Utils::SafeConvertInt<UInt>(buf, true);

        //Error(L"%s : %x", key.c_str(), out);
    };

    ReadColor(L"PITCH_COLOR_DAY1", GetPitchColorConfig().mPitchColorDay1);
    ReadColor(L"PITCH_COLOR_DAY2", GetPitchColorConfig().mPitchColorDay2);
    ReadColor(L"PITCH_COLOR_DAY3", GetPitchColorConfig().mPitchColorDay3);
    ReadColor(L"PITCH_COLOR_OVERCAST", GetPitchColorConfig().mPitchColorOvercast);
    ReadColor(L"PITCH_COLOR_NIGHT", GetPitchColorConfig().mPitchColorNight);
    ReadColor(L"PITCH_COLOR_SYNTHETIC", GetPitchColorConfig().mPitchColorSynthetic);
    ReadColor(L"PITCH_COLOR_GRASS_WEAR", GetPitchColorConfig().mGrassWear);
    ReadColor(L"PITCH_COLOR_GRASS_WEAR_RAIN", GetPitchColorConfig().mGrassWearRain);
    ReadFloat(L"PITCH_BRIGHTNESS_DAY", GetPitchColorConfig().mPitchBrightnessDay);
    ReadFloat(L"PITCH_BRIGHTNESS_OVERCAST", GetPitchColorConfig().mPitchBrightnessOvercast);
    ReadFloat(L"PITCH_BRIGHTNESS_NIGHT", GetPitchColorConfig().mPitchBrightnessNight);
}

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
    Bool enableTrack = false;
};

Map<UInt, StadiumData> &GetStadiumDataMap() {
    static Map<UInt, StadiumData> stadiumDataMap;
    return stadiumDataMap;
}

StadiumData *GetTeamStadiumData(UInt teamId, UInt teamType) {
    UInt finalId = teamId;
    if (teamType != 0)
        finalId |= (teamType << 24);
    auto it = GetStadiumDataMap().find(finalId);
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
                    Hexadecimal(data.pitchColorNight), data.pitchBrightnessNight, data.enableTrack);
                if (teamId != 0)
                    GetStadiumDataMap()[teamId] = data;
            }
            else
                reader.SkipLine();
        }
    }
}

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
                stadiumData = GetTeamStadiumData(hostTeam->GetTeamUniqueID(), hostTeamId.type);
                stadiumCapacity = hostTeam->GetStadiumDevelopment()->GetNumSeats();
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
        
        // TODO: mistake? gUnkColor2 must be used?
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

        if (Settings::GetInstance().UseNew3dPitch) {
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

        if (Settings::GetInstance().UseNew3dPitch || stadiumData) {

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
                    customColor = GetPitchColorConfig().mPitchColorSynthetic;
            }

            if (customColor != 0)
                grassColor->SetInt(customColor);
            else {
                if (stadWeather == StadWeather::Night)
                    grassColor->SetInt(GetPitchColorConfig().mPitchColorNight);
                else if (stadWeather == StadWeather::Overcast)
                    grassColor->SetInt(GetPitchColorConfig().mPitchColorOvercast);
                else {
                    UInt rndValue = Random::Get(0, 100);
                    if (rndValue < 33)
                        grassColor->SetInt(GetPitchColorConfig().mPitchColorDay1);
                    else if (rndValue < 66)
                        grassColor->SetInt(GetPitchColorConfig().mPitchColorDay2);
                    else
                        grassColor->SetInt(GetPitchColorConfig().mPitchColorDay3);
                }
            }
            if (customBrightness == 0.0f) {
                if (stadWeather == StadWeather::Night)
                    customBrightness = GetPitchColorConfig().mPitchBrightnessNight;
                else if (stadWeather == StadWeather::Overcast)
                    customBrightness = GetPitchColorConfig().mPitchBrightnessOvercast;
                else
                    customBrightness = GetPitchColorConfig().mPitchBrightnessDay;
            }

            gBrightnessColor->r = customBrightness;
            gBrightnessColor->g = customBrightness;
            gBrightnessColor->b = customBrightness;
            gBrightnessColor->a = 1.0f;

            if (Settings::GetInstance().UseNew3dPitch) {
                gPatternColor2->r = 0.06f;
                gPatternColor2->g = 0.108f;
                gPatternColor2->b = 0.119f;
                gPatternColor2->a = 0.4f;

                gUnkColor1->r = 0.0f;
                gUnkColor1->g = 0.0f;
                gUnkColor1->b = 0.0f;
                gUnkColor1->a = 0.4f;

                // TODO: mistake? Must be gUnkColor2?
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
                stadiumData = GetTeamStadiumData(hostTeam->GetTeamUniqueID(), hostTeamId.type);
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
                CStadiumDevelopment *stadiumDevelopment = hostTeam->GetStadiumDevelopment();
                UInt stadiumCapacity = stadiumDevelopment->GetNumSeats();
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
    // fix for generated banners/adboards
    //void *dynamicGraphics = *raw_ptr<void *>(t, 0xC3C);
    //if (dynamicGraphics) {
    //    if (exists("E:\\Documents\\FM\\Temp\\BannerHome.big"))
    //        remove("E:\\Documents\\FM\\Temp\\BannerHome.big");
    //    if (exists("E:\\Documents\\FM\\Temp\\BannerAway.big"))
    //        remove("E:\\Documents\\FM\\Temp\\BannerAway.big");
    //    if (exists("E:\\Documents\\FM\\Temp\\Adboards.big"))
    //        remove("E:\\Documents\\FM\\Temp\\Adboards.big");
    //    CallVirtualMethod<0>(dynamicGraphics);
    //    *raw_ptr<void *>(t, 0xC3C) = nullptr;
    //}
    //
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
    if (team && gCurrentTeamForStadium) {
        CTeamIndex teamId = gCurrentTeamForStadium->GetTeamID();
        StadiumData *stadiumData = GetTeamStadiumData(gCurrentTeamForStadium->GetTeamUniqueID(), 0);
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

// stadium venue

Bool MyIsValidFifaStadium(int id) {
    static UInt lightingIDs[] = { 0, 1, 3 };
    for (UInt i = 0; i < std::size(lightingIDs); i++) {
        if (FmFileExists(Utils::Format(L"m716__%d_%d.o", id, i)) || FmFileExists(Utils::Format(L"data\\assets\\m716__%d_%d.o", id, i)))
            return true;
    }
    return false;
}

UInt METHOD MyGetVenueId(void *stad, DUMMY_ARG, Int, Int) {
    // get CDBTeam* from CDBStadiumDevelopment
    CDBTeam *team = (CDBTeam *)(Int(stad) - 0x11F8);
    // validate CDBTeam*
   // if (*raw_ptr<UInt>(team) == 0x24A370C) {
   //     auto customData = GetTeamStadiumData(team->GetTeamUniqueID(), 0);
   //     if (customData)
   //         return customData->venueId;
   // }
    return 0;
}

UInt METHOD MyGetVenueId_0(void *stad) {
    return 0;
}

void OnSetupStadiumPitchTexture() {
    CallDynGlobal(GfxCoreAddress(0x23FBE0));
    void *match = *(void **)0x3124748;
    if (match) {
        CTeamIndex hostTeamId = CTeamIndex::make(0, 0, 0);
        CallMethod<0xE814C0>(match, &hostTeamId);
        if (hostTeamId.countryId) {
            CDBTeam *hostTeam = GetTeam(hostTeamId);
            if (hostTeam) {
                StadiumData *stadiumData = GetTeamStadiumData(hostTeam->GetTeamUniqueID(), hostTeamId.type);
                if (stadiumData && stadiumData->enableTrack) {
                    UInt &pitchTextureType = *(UInt *)(GfxCoreAddress(0xAE0B3C));
                    if (pitchTextureType == 0)
                        pitchTextureType = 10;
                    else if (pitchTextureType == 1)
                        pitchTextureType = 11;
                }
            }
        }
    }
}

void PatchPitch(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadStadiumDataFile();
        ReadPitchColorConfig();
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

        // venue
        return;

        patch::RedirectJump(0xF70420, MyIsValidFifaStadium);
        patch::RedirectJump(0xF70410, MyGetVenueId_0);
        patch::RedirectJump(0xF70670, MyGetVenueId);

        // movzx   ebp, al  >>  mov ebp, eax
        patch::SetUShort(0x65D777, 0xE88B);
        patch::Nop(0x65D777 + 2, 1);
       
        // movzx   eax, al  >>  -
        patch::Nop(0x65EF64, 3);
       
        // movzx   edx, al  >>  mov edx, eax
        patch::SetUShort(0x44D2BD, 0xD08B);
        patch::Nop(0x44D2BD + 2, 1);

        // test    al, al  >>  test eax, eax
        patch::SetUShort(0x40D395, 0xC085);

        // movzx   eax, al  >>  -
        patch::Nop(0x40D3AB, 3);

        // movzx   eax, al  >>  -
        patch::Nop(0x4429E7, 3);

        // movzx   eax, al  >>  -
        patch::Nop(0x442A87, 3);
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

    patch::RedirectCall(GfxCoreAddress(0x24000B), OnSetupStadiumPitchTexture);
}
