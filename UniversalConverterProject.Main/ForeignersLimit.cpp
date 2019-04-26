#include "ForeignersLimit.h"

using namespace plugin;

void __declspec(naked) CheckSpainNominationDateMonth_13() {
    __asm {
        cmp dword ptr[esp + 0x2D8], 1
        je LABEL_TRUE
        cmp dword ptr[esp + 0x2D8], 8
        je LABEL_TRUE
    LABEL_FALSE:
        mov eax, 0xF2342D
        jmp eax
    LABEL_TRUE:
        mov eax, 0xF23114
        jmp eax
    }
}

void __declspec(naked) CheckSpainNominationDateMonth_11() {
    __asm {
        cmp dword ptr[esp + 0x2D8], 1
        je LABEL_TRUE
        cmp dword ptr[esp + 0x2D8], 8
        je LABEL_TRUE
    LABEL_FALSE :
        mov eax, 0xD5A4B7
        jmp eax
    LABEL_TRUE :
        mov eax, 0xD5A4A6
        jmp eax
    }
}

void PatchForeignersLimit(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUChar(0xF8D672, 4); // default for Turkey
        patch::Nop(0xF8D540, 2);

        // Scotland
        patch::SetUChar(0xEE2983 + 6, 245);
        patch::SetUChar(0xF3FC9C + 1, 245);
        patch::SetUChar(0x1067F5E + 1, 245);
        patch::SetUChar(0x10680BF + 1, 245);
        patch::SetUChar(0x10682F1 + 3, 245);
        patch::SetUChar(0x106C451 + 1, 245);
        patch::SetUChar(0x62BF8F + 3, 245);
        patch::SetUChar(0x65400E + 3, 245);
        patch::SetUChar(0xAA1C25 + 1, 245);
        patch::SetUChar(0xE3F56D + 1, 245);
        patch::SetUChar(0xEA109D + 1, 245);
        patch::SetUChar(0xF6A9FD + 3, 245);
        patch::SetUChar(0xF6CD78 + 3, 245);
        patch::SetUChar(0xFF5EA2 + 3, 245);
        patch::SetUChar(0xFF5F4B + 3, 245);

        // Spain players registration
        patch::RedirectJump(0xF23106, CheckSpainNominationDateMonth_13);
    }
    else if (v.id() == ID_FM_11_1003) {
        patch::SetUChar(0xDFD896, 4); // default for Turkey
        patch::Nop(0xDFD70A, 2);


        // Spain players registration
        patch::RedirectJump(0xD5A49C, CheckSpainNominationDateMonth_11);
    }
}
