#include "PlayerAppearance.h"
#include "GfxCoreHook.h"
#include "PlayerAppearanceIDs.h"
#include "GameInterfaces.h"
#include "Utils.h"
#include "UcpSettings.h"
#include "FifamAppearanceDefs.h"

using namespace plugin;

Int hairLODs[std::size(hairTypes)];

UChar METHOD OnSetupPlayer3D(void *player3d, DUMMY_ARG, UInt playerId, CTeamIndex *teamID, Int a4, Int a5) {
    UChar result = CallMethodAndReturn<UChar, 0x413CA0>(player3d, playerId, teamID, a4, a5);
    auto player = CallAndReturn<CDBPlayer *, 0xF97C70>(playerId);
    if (player && *raw_ptr<UInt>(player, 0xE0) == 43093171) {
        Error("player: %d player3D: %d", *raw_ptr<UChar>(player, 0x1A8 + 3), *raw_ptr<UChar>(player3d, 0x31F));
    }
    return result;
}

UChar METHOD RetGender0(void *) { return 0; }

CManagerAppearance *METHOD GetManagerAppearanceWithExendedHairId(void *manager) {
    CManagerAppearance *app = CallMethodAndReturn<CManagerAppearance *, 0x15079D5>(manager);
    app->m_nBeard = app->m_nSideburns;
    app->m_nSideburns = 0;
    return app;
}

void __declspec(naked) HairID_GameStartPlayerAppearance_Action1() {
    __asm {
        dec eax
        mov [esp + 0xE], al
        mov dl, ah
        and dl, 0xF
        mov cl, [esp + 0x12]
        and cl, 0xF0
        or cl, dl
        mov [esp + 0x12], cl
        mov ecx, [esi + 0x49C]
        mov eax, 0x537958
        jmp eax
    }
}

void __declspec(naked) HairID_GameStartPlayerAppearance_RedrawManagerHead() {
    __asm {
        dec eax
        mov [esp + 0x6], al
        mov dl, ah
        and dl, 0xF
        mov cl, [esp + 0xA]
        and cl, 0xF0
        or cl, dl
        mov [esp + 0xA], cl
        mov ecx, [esi + 0x49C]
        mov eax, 0x536EF3
        jmp eax
    }
}

void __declspec(naked) HairID_RenderDataFromPlayerAppearance_1() {
    __asm {
        mov eax, [esp + 4]
        movzx ecx, byte ptr [eax + 2] // hair ID bits 0-7
        movzx edx, byte ptr [eax + 6] // hair ID bits 8-11
        and edx, 0xF
        shl edx, 8
        or ecx, edx
        mov edx, 0x4534F8
        jmp edx
    }
}

void __declspec(naked) HairID_RenderDataFromPlayerAppearance_2() {
    __asm {
        mov eax, 0
        mov edx, 0x45357C
        jmp edx
    }
}

void METHOD OnAppearanceDefGenerateRandomAppearance(void *t, DUMMY_ARG, Int *params) {
    CallMethod<0x1381400>(t, params);
    Int hairId = params[FifamAppearanceDefs::ParamHair];
    params[FifamAppearanceDefs::ParamHair] = hairId & 0xFF;
    params[FifamAppearanceDefs::ParamSideburns] = (hairId >> 8) & 0xF;
}

UInt METHOD GetPlayerEmpics0(void *p) { return 0; }

void METHOD GenerateRefereeAppearance(CDBReferee *ref, DUMMY_ARG, CPlayerAppearance *app) {
    Int hash = 0;
    Int seed1 = 0, seed2 = 0;
    auto name = ref->GetName();
    CallMethod<0x14B5C9D>(&hash, name, wcslen(name) * 2); // CGuiHash::Generate
    GlobalRandom().GetSeed(seed1, seed2);
    GlobalRandom().SetSeed(hash, ~hash);
    GetCountry(ref->GetCountryId())->GenerateAppearance(app);
    GlobalRandom().SetSeed(seed1, seed2);
}

void METHOD ConvertPlayerAppearanceFromFM09(CPlayerAppearance *app, UChar hairStyle, UInt face, UChar hairColor, UChar beard) {
    app->Clear();
}

void PatchPlayerAppearance(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

        // Extended hairstyle ID

        // CGameStartPlayerAppearance::CreateUI
        patch::RedirectCall(0x537559, GetManagerAppearanceWithExendedHairId); // hairstyle
        patch::SetUShort(0x53755E, 0xB70F); // movzx ecx, byte ptr [eax+2] => movzx ecx, word ptr [eax+2]
        patch::RedirectCall(0x5375B7, GetManagerAppearanceWithExendedHairId); // sideburns
        // CGameStartPlayerAppearance::Action1
        patch::RedirectJump(0x53794C, HairID_GameStartPlayerAppearance_Action1); // hairstyle
        patch::Nop(0x537998, 4); // sideburns
        // CGameStartPlayerAppearance::RedrawManagerHead
        patch::RedirectJump(0x536EE7, HairID_GameStartPlayerAppearance_RedrawManagerHead);
        patch::Nop(0x536F33, 4); // sideburns
        // RenderDataFromPlayerAppearance
        patch::RedirectJump(0x4534F0, HairID_RenderDataFromPlayerAppearance_1); // hairstyle
        patch::RedirectJump(0x453575, HairID_RenderDataFromPlayerAppearance_2); // sideburns
        // CPlayerGenerator::SetAppearance
        patch::Nop(0xFA7A22, 2); // always generate appearance
        patch::RedirectCall(0xFF0E52, OnAppearanceDefGenerateRandomAppearance);
        // remove appearance recreation feature
        patch::RedirectCall(0xEF639D, GetPlayerEmpics0);
        patch::RedirectCall(0xF38EF4, GetPlayerEmpics0);
        patch::RedirectCall(0xF3B501, GetPlayerEmpics0);
        // TODO: CTeamYouthPlayerGenerator::SetupYouthPlayer
        patch::RedirectJump(0xFD5AA0, GenerateRefereeAppearance);
        patch::RedirectJump(0xEA26D0, ConvertPlayerAppearanceFromFM09);

        //patch::RedirectCall(0x417F87, OnSetupPlayer3D);

        // sideburns
        patch::SetUInt(0x24A1854 + 4 * 1, 0); // sideburns 1

        // beard types
        patch::SetUInt(0x24A1880, std::size(beardTypes));
        patch::SetPointer(0x40BE90 + 0x4 + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[eax*4]
        patch::SetPointer(0x40C650 + 0xDA + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[edx*4]
        patch::SetPointer(0x413CA0 + 0x23D + 4, beardTypes); // movzx   edx, byte ptr ds:gFifaBeardTypes[ecx*4]
        patch::SetPointer(0x4534F0 + 0x2A + 3, beardTypes); // mov     edx, ds:gFifaBeardTypes[edx*4]
        patch::SetPointer(0xCB5AE0 + 0x168 + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[ebx*4]
        patch::SetPointer(0x30994DC, beardTypes); // .data:030994DC	dd offset gFifaBeardTypes
        patch::SetPointer(0xEA264D + 1, &beardTypes[0]);
        patch::SetPointer(0xEA2648 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA265E + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA2675 + 1, &beardTypes[0]);
        patch::SetPointer(0xEA2670 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA268A + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA269C + 1, &beardTypes[0]);
        patch::SetPointer(0xEA25CD + 1, &beardTypes[0]);
        patch::SetPointer(0xEA25C8 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA25DE + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA25F7 + 1, &beardTypes[0]);
        patch::SetPointer(0xEA19EC + 1, &beardTypes[0]);
        patch::SetPointer(0xEA19E7 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA19F9 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA1A08 + 1, &beardTypes[0]);

        // skin colors
        auto skinColorsToUse = skinColors;
        //std::error_code dummy_ec;
        //if (exists("IM_USING_SEASON_2020_1_0_SAVEGAME", dummy_ec)) {
        //    Warning("The fix for skin tones on Season 2020 1.0 savegames is enabled.\nIn order to remove the fix, delete the file 'IM_USING_SEASON_2020_1_0_SAVEGAME' in the game folder");
        //    skinColorsToUse = skinColors_oldSave;
        //}
        patch::SetUInt(0x24A1884, std::size(skinColors));
        patch::SetPointer(0x40BEB0 + 0x7 + 3, skinColorsToUse); // mov     eax, ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x40C650 + 0x82 + 3, skinColorsToUse); // mov     ecx, ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x413BB0 + 0xAB + 3, skinColorsToUse); // mov     al, byte ptr ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x413CA0 + 0x259 + 4, skinColorsToUse); // movzx   ecx, byte ptr ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x4534F0 + 0x59 + 3, skinColorsToUse); // mov     edx, ds:gFifaSkinColors[edx*4]
        patch::SetPointer(0xCB5AE0 + 0x78 + 3, skinColorsToUse); // mov     eax, ds:gFifaSkinColors[ebx*4]
        patch::SetPointer(0x30994CC, skinColorsToUse); // .data:030994CC	dd offset gFifaSkinColors
        patch::SetPointer(0xEA1C63 + 1, &skinColorsToUse[0]);
        patch::SetPointer(0xEA1C5E + 1, &skinColorsToUse[std::size(skinColors)]);
        patch::SetPointer(0xEA1C74 + 1, &skinColorsToUse[std::size(skinColors)]);
        patch::SetPointer(0xEA1C7F + 1, &skinColorsToUse[0]);
        patch::SetPointer(0xEA1EE0 + 1, &skinColorsToUse[0]);
        patch::SetPointer(0xEA1EDB + 1, &skinColorsToUse[std::size(skinColors)]);
        patch::SetPointer(0xEA1EF1 + 1, &skinColorsToUse[std::size(skinColors)]);
        patch::SetPointer(0xEA1F07 + 1, &skinColorsToUse[0]);
        patch::SetPointer(0xEA1F02 + 1, &skinColorsToUse[std::size(skinColors)]);
        patch::SetPointer(0xEA1F18 + 1, &skinColorsToUse[std::size(skinColors)]);
        patch::SetPointer(0xEA1F23 + 1, &skinColorsToUse[0]);
        patch::SetPointer(0xEA1A6C + 1, &skinColorsToUse[0]);
        patch::SetPointer(0xEA1A67 + 1, &skinColorsToUse[std::size(skinColors)]);
        patch::SetPointer(0xEA1A79 + 1, &skinColorsToUse[std::size(skinColors)]);
        patch::SetPointer(0xEA1A88 + 1, &skinColorsToUse[0]);

        // hair colors
        patch::SetUInt(0x24A1890, std::size(hairColors));
        patch::SetPointer(0x40BE70 + 0x7 + 3, hairColors); // mov     eax, ds:gFifaHairColors[eax*4]
        patch::SetPointer(0x40C650 + 0x18D + 3, hairColors); // mov     ecx, ds:gFifaHairColors[eax*4]
        patch::SetPointer(0x413BB0 + 0xBB + 4, hairColors); // movzx   ecx, byte ptr ds:gFifaHairColors[ecx*4]
        patch::SetPointer(0x413CA0 + 0x275 + 4, hairColors); // movzx   eax, byte ptr ds:gFifaHairColors[edx*4]
        patch::SetPointer(0x4534F0 + 0x1C + 3, hairColors); // mov     edx, ds:gFifaHairColors[edx*4]
        patch::SetPointer(0xCB5AE0 + 0x2A8 + 3, hairColors); // mov     eax, ds:gFifaHairColors[ebx*4]
        patch::SetPointer(0x30994E4, hairColors); // .data:030994E4	dd offset gFifaHairColors
        patch::SetPointer(0x5A0269 + 1, &hairColors[0]);
        patch::SetPointer(0x5A0264 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A027A + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0285 + 1, &hairColors[0]);
        patch::SetPointer(0x5A036F + 1, &hairColors[0]);
        patch::SetPointer(0x5A036A + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0380 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0397 + 1, &hairColors[0]);
        patch::SetPointer(0x5A0392 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A03AC + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A03B8 + 1, &hairColors[0]);
        patch::SetPointer(0xEA196C + 1, &hairColors[0]);
        patch::SetPointer(0xEA1967 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0xEA1979 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0xEA1988 + 1, &hairColors[0]);

        // hair models
        patch::SetUInt(0x24A187C, std::size(hairTypes));
        patch::SetPointer(0x40BE60 + 0x4 + 3, hairTypes); // mov     eax, ds:gFifamGenericHairIdToFIFA[eax*4]
        patch::SetPointer(0x40C650 + 0x56 + 3, hairTypes); // mov     eax, ds:gFifamGenericHairIdToFIFA[edx*4]
        patch::SetPointer(0x413BB0 + 0x3F + 4, hairTypes); // movzx   edx, byte ptr ds:gFifamGenericHairIdToFIFA[ecx*4]
        patch::SetPointer(0x413CA0 + 0x1D0 + 4, hairTypes); // movzx   eax, byte ptr ds:gFifamGenericHairIdToFIFA[edx*4]
        patch::SetPointer(0x4534F0 + 0x8 + 3, hairTypes); // mov     edx, ds:gFifamGenericHairIdToFIFA[ecx*4]
        patch::SetPointer(0xCB5AE0 + 0x258 + 3, hairTypes); // mov     eax, ds:gFifamGenericHairIdToFIFA[ebx*4]
        patch::SetPointer(0x30994C8, hairTypes); // .data:030994C8	dd offset gFifamGenericHairIdToFIFA
        patch::SetPointer(0xEA2210 + 1, &hairTypes[0]);
        patch::SetPointer(0xEA220B + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA2221 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA2238 + 1, &hairTypes[0]);
        patch::SetPointer(0xEA2233 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA224D + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA2259 + 1, &hairTypes[0]);
        patch::SetPointer(0xEA20D1 + 1, &hairTypes[0]);
        patch::SetPointer(0xEA20CC + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA20E2 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA20ED + 1, &hairTypes[0]);
        patch::SetPointer(0xEA192C + 1, &hairTypes[0]);
        patch::SetPointer(0xEA1927 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA1939 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA1948 + 1, &hairTypes[0]);

        // head models
        patch::SetUInt(0x24A1878, std::size(headTypes));
        patch::SetPointer(0x40BE50 + 0x3 + 3, headTypes); // mov     eax, ds:gFifamGenericFaceIdToFIFA[eax*4]
        patch::SetPointer(0x40C650 + 0x25 + 3, headTypes); // mov     edx, ds:gFifamGenericFaceIdToFIFA[ecx*4]
        patch::SetPointer(0x413BB0 + 0x2D + 3, headTypes); // mov     eax, ds:gFifamGenericFaceIdToFIFA[edx*4]
        patch::SetPointer(0x413CA0 + 0x1B8 + 3, headTypes); // mov     ecx, ds:gFifamGenericFaceIdToFIFA[eax*4]
        patch::SetPointer(0x4534F0 + 0x37 + 3, headTypes); // mov     edx, ds:gFifamGenericFaceIdToFIFA[edx*4]
        patch::SetPointer(0xCB5AE0 + 0x28 + 3, headTypes); // mov     eax, ds:gFifamGenericFaceIdToFIFA[ebx*4]
        patch::SetPointer(0x30994C4, headTypes); // .data:off_30994C4	dd offset gFifamGenericFaceIdToFIFA
        patch::SetPointer(0xEA1E31 + 1, &headTypes[0]);
        patch::SetPointer(0xEA1E2C + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1E42 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1E4D + 1, &headTypes[0]);
        patch::SetPointer(0xEA1E6C + 1, &headTypes[0]);
        patch::SetPointer(0xEA1E67 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1E7D + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1E88 + 1, &headTypes[0]);
        patch::SetPointer(0xEA1BF1 + 1, &headTypes[0]);
        patch::SetPointer(0xEA1BEC + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1C02 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1C0D + 1, &headTypes[0]);
        patch::SetPointer(0xEA18EC + 1, &headTypes[0]);
        patch::SetPointer(0xEA18E7 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA18F9 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1908 + 1, &headTypes[0]);

        patch::SetUInt(0x24A1898, std::size(eyeColors));
        patch::SetPointer(0x40BED0 + 0x7 + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[eax*4]
        patch::SetPointer(0x40C650 + 0x15E + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0x413BB0 + 0x6F + 4, eyeColors); // movzx   edx, byte ptr ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0x413CA0 + 0x208 + 4, eyeColors); // movzx   ecx, byte ptr ds:gFifaEyeColors[eax*4]
        patch::SetPointer(0x4534F0 + 0x48 + 3, eyeColors); // mov     edx, ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0xCB5AE0 + 0xC8 + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[ebx*4]
        patch::SetPointer(0x30994D8, eyeColors); // .data:030994D8	dd offset gFifaEyeColors
        patch::SetPointer(0xEA2019 + 1, &eyeColors[0]);
        patch::SetPointer(0xEA2014 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA202A + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA2040 + 1, &eyeColors[0]);
        patch::SetPointer(0xEA203B + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA2051 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA205C + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1D73 + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1D6E + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1D84 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1D8F + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1DAF + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1DAA + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1DC4 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1DD0 + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1AEC + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1AE7 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1AF9 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1B08 + 1, &eyeColors[0]);

        // skin types
        patch::SetUInt(0x24A1888, std::size(skinTypes));
        patch::SetPointer(0x40BEC7 + 3, skinTypes);
        patch::SetPointer(0x40C6FE + 3, skinTypes);
        patch::SetPointer(0x413C07 + 4, skinTypes);
        patch::SetPointer(0x413E8C + 4, skinTypes);
        patch::SetPointer(0x45355A + 3, skinTypes);
        patch::SetPointer(0x30994D0, skinTypes);
        patch::SetPointer(0xEA1AAC + 1, &skinTypes[0]);
        patch::SetPointer(0xEA1AA7 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0xEA1AB9 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0xEA1AC8 + 1, &skinTypes[0]);
        patch::SetPointer(0xEA1CD4 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0xEA1CEA + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0xEA1CD9 + 1, &skinTypes[0]);
        patch::SetPointer(0xEA1CF5 + 1, &skinTypes[0]);
        patch::SetPointer(0xEA1F76 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0xEA1F8C + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0xEA1F7B + 1, &skinTypes[0]);
        patch::SetPointer(0xEA1FA2 + 1, &skinTypes[0]);
        patch::SetPointer(0xEA1F9D + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0xEA1FB3 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0xEA1FBE + 1, &skinTypes[0]);

        // FIFA head IDs
        patch::SetUInt(0x3062748, 500'000);

        // 3D appearance for female managers
        //patch::RedirectCall(0x53CE37, RetGender0);
        //patch::RedirectCall(0x53D4AC, RetGender0);

        // male manager limits
        static UInt NumMaleFaces = 338;
        patch::SetPointer(0x5372D1 + 2, &NumMaleFaces);
        static UInt NumMaleSkinTypes = 3;
        patch::SetPointer(0x53731E + 1, &NumMaleSkinTypes);
    }
}

Int METHOD GetLowMedHairId(void *player) {
    if (Settings::GetInstance().UseHairLODs) {
        Int specialfaceid = *raw_ptr<int>(player, 0x1BC);
        if (specialfaceid != 0 && FmFileExists(Utils::Format("m728__%d.o", specialfaceid))) {
            return specialfaceid;
        }
        Int hairstyleid = *raw_ptr<int>(player, 0x1DC);
        if (hairstyleid < (Int)std::size(hairLODs))
            return hairLODs[hairstyleid];
    }
    return 0;
}

void InstallPlayerAppearance_GfxCore() {
    for (Int i = 0; i < (Int)std::size(hairLODs); i++) {
        if (!Settings::GetInstance().UseHairLODs || i == 0)
            hairLODs[i] = 0;
        else
            hairLODs[i] = -i;
    }
    //patch::SetPointer(GfxCoreAddress(0x920B2 + 3), hairLODs);

    patch::SetPointer(GfxCoreAddress(0x23C2D7 + 3), hairLODs);
    patch::SetPointer(GfxCoreAddress(0x23C359 + 3), hairLODs);
    patch::SetPointer(GfxCoreAddress(0x23C3B6 + 3), hairLODs);

    patch::SetPointer(GfxCoreAddress(0x23C995 + 3), hairLODs);
    patch::SetPointer(GfxCoreAddress(0x23CA59 + 3), hairLODs);
    patch::SetPointer(GfxCoreAddress(0x23CB07 + 3), hairLODs);
    patch::SetPointer(GfxCoreAddress(0x23CC69 + 3), hairLODs);
    patch::SetPointer(GfxCoreAddress(0x23CD2D + 3), hairLODs);
    patch::SetPointer(GfxCoreAddress(0x23CDDD + 3), hairLODs);

    patch::Nop(GfxCoreAddress(0x920B0), 2);
    patch::SetUShort(GfxCoreAddress(0x920B2), 0xCE8B);
    patch::RedirectCall(GfxCoreAddress(0x920B2 + 2), GetLowMedHairId);
}
