#include "EABFFixes.h"
#include "GameInterfaces.h"
#include "shared.h"

using namespace plugin;

class FmUtils {
public:
    static CTeamIndex GetPlayerTeamIndex(CDBPlayer *player) { return CallMethodAndReturn<CTeamIndex, 0xFB5290>(player); }
    static CDBTeam *GetTeamByIndex(CTeamIndex index) { return CallAndReturn<CDBTeam *, 0xEC8F50>(index); }
    static CDBTeam *GetPlayerTeam(CDBPlayer *player) { return GetTeamByIndex(GetPlayerTeamIndex(player)); }
    static UInt GetTeamUniqueId(CDBTeam *team) { return CallMethodAndReturn<UInt, 0xEC9490>(team); }
    static CTeamIndex GetTeamIndex(CDBTeam *team) { return CallMethodAndReturn<CTeamIndex, 0xEC9440>(team); }
    static WideChar *GetTeamName(CDBTeam *team, Bool reserve = false) { return CallMethodAndReturn<WideChar *, 0xEEB670>(team, reserve); }
    static WideChar *GetTeamName(CDBTeam *team, CTeamIndex index, Bool allowYouthTeamNames = true) { return CallMethodAndReturn<WideChar *, 0xEEB670>(team, index, allowYouthTeamNames); }
    static WideChar *GetPlayerName(CDBPlayer *player, WideChar *dst = nullptr) { return CallMethodAndReturn<WideChar *, 0xFA2010>(player, dst); }
};

CTeamIndex METHOD MyPlayerGetTeam(CDBPlayer *player) {
    CTeamIndex teamIndex = FmUtils::GetPlayerTeamIndex(player);
    CDBTeam *team = FmUtils::GetTeamByIndex(teamIndex);

    String logLine = String() + FmUtils::GetPlayerName(player) + L",";
    if (team)
        logLine += FmUtils::GetTeamName(team, teamIndex);
    else
        logLine += L"(nullptr)";

    SafeLog::Write(logLine);

    return teamIndex;
}

CTeamIndex gResult;

void __declspec(naked) MyPlayerGetTeam_Exec() {
    __asm pushad
    __asm call MyPlayerGetTeam
    __asm mov gResult, eax
    __asm popad
    __asm mov eax, gResult
    __asm retn
}

void __declspec(naked) Fix_ACA1A0_FBFF80() {
    __asm {
        test ecx, ecx
        je SKIP_ACA1A0_FBFF80
        mov eax, 0xECA1A0
        call eax
        mov ecx, 0xFBFF80
        jmp ecx
        SKIP_ACA1A0_FBFF80 :
        mov ecx, 0xFBFFA6
            jmp ecx
    }
}

void *METHOD MyHook(void *panel, DUMMY_ARG, char const *element) {

    /*
        CALL ORIGINAL FUNCTION
    */

    void *result = CallMethodAndReturn<void *, 0xD44360>(panel, element);

    /*
        AND NOW WE CAN DO WHAT WE WANT
    */

    // get our text box
    void *TbBadge = CallMethodAndReturn<void *, 0xD44380>(panel, "TbBadge");

    if (TbBadge) {
        // apply our texture
        Call<0xD32860>(TbBadge, L"0058FFFF_h.png", 0, 0);
    }

    return result;
}

void makeDump(void *area) {
    FILE *f = fopen("dump.bin", "wb");
    fwrite(area, 100, 1, f);
    fclose(f);
}

void __declspec(naked) hook() {
    __asm {
        mov eax, dword ptr cs : [esp - 4]
        push eax
        call makeDump
        add esp, 4
        jmp eax
    }
}

unsigned int gAddress = 0;
wchar_t gMessage[128];

void __declspec(naked) ShowError() {
    __asm mov eax, dword ptr[esp]
        __asm mov gAddress, eax
    swprintf(gMessage, L"Unhandled exception at 0x%X\n\nPlease make a screensot of this error\nand show to patch developers.\n\nMachen Sie einen Screenshot dieses Fehlers\nund zeigen Sie ihn den Patch-Entwicklern.\n\nПожалуйста, сделайте скриншот этого\nсообщения и покажите его разработичкам.", gAddress);
    MessageBoxW(NULL, gMessage, L"Season 2019 (v.1.3) - Exception", MB_ICONERROR);
    __asm retn
}

char METHOD TeamGetStatus_SpectatorsCheckFix(void *team) {
    void *spec = CallMethodAndReturn<void *, 0xECFFE0>(team);
    int numMatches = *raw_ptr<int>(spec, 0x150 + 0x18);
    if (numMatches <= 0)
        return 1;
    return CallMethodAndReturn<char, 0xECC130>(team);
}

unsigned char METHOD GetFACompCountryId_SkipReserveTeams(void *comp) {
    return 45;
}

void PatchEABFFixes(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectJump(0xFBFF7B, Fix_ACA1A0_FBFF80);

        //patch::RedirectCall(0x501521, MyHook);
        //patch::RedirectJump(0xAC4CBF, hook);

        // __throw_error redirect
        patch::RedirectJump(0x1573B40, ShowError);

        // spectators fix
        patch::RedirectCall(0x1519BD6, TeamGetStatus_SpectatorsCheckFix);

        // skip reserve teams for FA_CUP
        patch::RedirectCall(0x1049924, GetFACompCountryId_SkipReserveTeams);
    }
}
