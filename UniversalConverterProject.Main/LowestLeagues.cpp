#include "LowestLeagues.h"

using namespace plugin;

void __declspec(naked) SetLowestLeaguesMultiplier() {
    __asm {
        mov dword ptr[esp + 40], 3
        mov ebp, 0x10F07C7
        jmp ebp
    }
}

void PatchLowestLeagues(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectJump(0x10F07C0, SetLowestLeaguesMultiplier);
        patch::Nop(0x10F07C0 + 5, 2);
    }
}
