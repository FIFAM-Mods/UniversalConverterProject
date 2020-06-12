#include "StaffNames.h"

using namespace plugin;

void __declspec(naked) GetEmployeePseudonym_13() {
    __asm {
        lea  eax, [edi + 0x80]
        push eax
        push 2
        mov  eax, 0x51F5F5
        jmp  eax
    }
}

void PatchStaffNames(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::RedirectJump(0x51F5EF, GetEmployeePseudonym_13);

        patch::RedirectJump(0x5729F2, (void *)0x51CC10);
        patch::RedirectJump(0x572A02, (void *)0x51CC00);
    }
}
