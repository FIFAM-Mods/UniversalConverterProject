#include "Languages.h"
#include "GameInterfaces.h"
#include "Translation.h"
#include "Utils.h"

using namespace plugin;

const UInt NUM_LANGUAGES = 105 + 1;
UInt NewSortedLanguagesIDs[NUM_LANGUAGES];

void __declspec(naked) CareerManagerLanguages_1() {
    __asm {
        lea edx, NewSortedLanguagesIDs
        push edx
        mov eax, 0x7C8E81
        jmp eax
    }
}

void __declspec(naked) CareerManagerLanguages_2() {
    __asm {
        sub eax, [esp + 0x30]
        lea edi, NewSortedLanguagesIDs
        mov bl, [edi + ecx * 4]
        mov edi, 0x7C8E9C
        jmp edi
    }
}

void __declspec(naked) CareerManagerLanguages_3() {
    __asm {
        movzx eax, byte ptr[esp + 0x10]
        lea edx, NewSortedLanguagesIDs
        cmp [edx + ecx * 4], eax
        mov edx, 0x7C8ED6
        jmp edx
    }
}

void __declspec(naked) CareerManagerLanguages_4() {
    __asm {
        lea edx, NewSortedLanguagesIDs
        mov al, [edx + ecx * 4]
        movzx ecx, al
        mov edx, 0x7C8EDF
        jmp edx
    }
}

void __declspec(naked) CareerManagerLanguages_5() {
    __asm {
        mov edi, [esp + 0x20]
        lea ecx, NewSortedLanguagesIDs
        mov eax, [ecx + edi * 4]
        mov ecx, 0x7C8F07
        jmp ecx
    }
}

void __declspec(naked) CareerManagerLanguages_6() {
    __asm {
        lea ecx, NewSortedLanguagesIDs
        cmp dword ptr [ecx + edi * 4], 0
        mov ecx, 0x7C8F26
        jmp ecx
    }
}

void __declspec(naked) CareerManagerLanguages_7() {
    __asm {
        lea eax, NewSortedLanguagesIDs
        movzx ecx, al
        cmp [eax + edx * 4], ecx
        mov eax, 0x7C8F64
        jmp eax
    }
}

void __declspec(naked) CareerManagerLanguages_8() {
    __asm {
        lea ecx, NewSortedLanguagesIDs
        mov edi, edx
        mov al, [ecx + edi * 4]
        mov ecx, 0x7C8F6C
        jmp ecx
    }
}

void __declspec(naked) CareerManagerLanguages_9() {
    __asm {
        lea ebx, NewSortedLanguagesIDs
        mov eax, [ebx + edi * 4]
        mov ecx, [esi]
        mov ebx, 0x7C8F95
        jmp ebx
    }
}

void GetLanguageIDsSortedByName(UInt *outIDs) {
    Vector<Pair<UInt, String>> langNames(NUM_LANGUAGES - 1);
    for (UInt i = 1; i < NUM_LANGUAGES; i++) {
        langNames[i - 1] = { i, GetTranslation(FormatStatic("IDCSTR_LANGUAGES_%d", i)) };
        if (IsUkrainianLanguage)
            ProcessStringUkrainianLanguage(langNames[i - 1].second);
    }
    Utils::Sort(langNames, [](Pair<UInt, String> const &a, Pair<UInt, String> const &b) {
        return a.second < b.second;
    });
    outIDs[0] = 0;
    for (UInt i = 1; i < NUM_LANGUAGES; i++)
        outIDs[i] = langNames[i - 1].first;
}

void PatchLanguages(FM::Version v) {
    patch::SetUChar(0x8E03DE + 1, NUM_LANGUAGES); // transfer search - screen 1
    patch::SetUChar(0x8E0442 + 2, NUM_LANGUAGES); // transfer search - screen 2
    patch::SetUChar(0xF9D42B + 6, NUM_LANGUAGES); // transfer search - init
    patch::SetUChar(0xF9DAAC + 1, NUM_LANGUAGES); // transfer search - compare
    patch::RedirectCall(0x7C8E81, GetLanguageIDsSortedByName);
    patch::RedirectJump(0x7C8E7C, CareerManagerLanguages_1);
    patch::RedirectJump(0x7C8E94, CareerManagerLanguages_2);
    patch::RedirectJump(0x7C8ECD, CareerManagerLanguages_3);
    patch::RedirectJump(0x7C8ED8, CareerManagerLanguages_4);
    patch::RedirectJump(0x7C8EFF, CareerManagerLanguages_5);
    patch::RedirectJump(0x7C8F21, CareerManagerLanguages_6);
    patch::RedirectJump(0x7C8F5D, CareerManagerLanguages_7);
    patch::RedirectJump(0x7C8F66, CareerManagerLanguages_8);
    patch::RedirectJump(0x7C8F8F, CareerManagerLanguages_9);
    patch::SetUChar(0x7C8FB2 + 2, NUM_LANGUAGES);
}
