#include "DatabaseName.h"

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

void __declspec(naked) GetEmployeePseudonym_11() {
    __asm {
        lea  eax, [edi + 0x80]
        push eax
        push 2
        mov  eax, 0x4E6A8A
        jmp  eax
    }
}

void PatchStaffNames(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::RedirectJump(0x51F5EF, GetEmployeePseudonym_13);
    }
    else if (v.id() == ID_ED_11_1003) {
        patch::RedirectJump(0x4E6A84, GetEmployeePseudonym_11);
    }
}
