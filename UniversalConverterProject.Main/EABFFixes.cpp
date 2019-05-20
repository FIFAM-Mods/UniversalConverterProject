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
    static WideChar * GetTeamName(CDBTeam *team, Bool reserve = false) { return CallMethodAndReturn<WideChar *, 0xEEB670>(team, reserve); }
    static WideChar * GetTeamName(CDBTeam *team, CTeamIndex index, Bool allowYouthTeamNames = true) { return CallMethodAndReturn<WideChar *, 0xEEB670>(team, index, allowYouthTeamNames); }
    static WideChar * GetPlayerName(CDBPlayer *player, WideChar *dst = nullptr) { return CallMethodAndReturn<WideChar *, 0xFA2010>(player, dst); }
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
    SKIP_ACA1A0_FBFF80:
        mov ecx, 0xFBFFA6
        jmp ecx
    }
}





void * METHOD MyHook(void *panel, DUMMY_ARG, char const *element) {
	
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

void PatchEABFFixes(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectJump(0xFBFF7B, Fix_ACA1A0_FBFF80);

		//patch::RedirectCall(0x501521, MyHook);
    }
}
