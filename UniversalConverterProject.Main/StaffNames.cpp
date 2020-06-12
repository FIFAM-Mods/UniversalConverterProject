#include "StaffNames.h"

using namespace plugin;

namespace fm13 {
unsigned short METHOD ReadStaffNickname(void *reader) {
    // read nickname
    CallMethod<0x1338A10>(reader);
    // read and return pseudonym
    return CallMethodAndReturn<unsigned short, 0x1338A10>(reader);
}

wchar_t * METHOD GetFullStaffName(unsigned int staff, DUMMY_ARG, wchar_t *nameBuf) {
    return CallAndReturn<wchar_t *, 0x149A616>(*(DWORD *)(staff + 12) + 8, nameBuf);
}

wchar_t * METHOD GetStaffNameWithPseudonym(unsigned int staff, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CallMethodAndReturn<void *, 0x1102930>(staff, buf, 0);
    return CallAndReturn<wchar_t *, 0x149A616>(desc, nameBuf);
}

wchar_t * METHOD GetStaffShortNameWithPseudonym(unsigned int staff, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CallMethodAndReturn<void *, 0x1102930>(staff, buf, 0);
    return CallAndReturn<wchar_t *, 0x149A738>(desc, nameBuf);
}

wchar_t * METHOD GetEmployeeBaseNameWithPseudonym(unsigned int base, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CallMethodAndReturn<void *, 0xE7EA60>(base, buf, 0);
    return CallAndReturn<wchar_t *, 0x149A616>(desc, nameBuf);
}

wchar_t * METHOD GetEmployeeBaseShortNameV1WithPseudonym(unsigned int base, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CallMethodAndReturn<void *, 0xE7EA60>(base, buf, 0);
    return CallAndReturn<wchar_t *, 0x149A738>(desc, nameBuf);
}

wchar_t * METHOD GetEmployeeBaseShortNameV2WithPseudonym(unsigned int base, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CallMethodAndReturn<void *, 0xE7EA60>(base, buf, 0);
    return CallAndReturn<wchar_t *, 0x149A89F>(desc, nameBuf);
}

void __declspec(naked) SetTeamPresidentName() {
    __asm {
        mov  dx, word ptr [esp + 0x38] // first name index
        mov  byte ptr [esi + 0x5EC], al // language id
        mov  ax, word ptr [esp + 0x3C] // common name index
        test ax, ax
        jz LABEL_NORMAL_NAME

        mov  word ptr [esi + 0x5EE], ax // first name
        mov  word ptr [esi + 0x5F0], 0 // second name
        mov  byte ptr [esi + 0x5ED], 2 // common name flag

        jmp LABEL_RETURN

    LABEL_NORMAL_NAME:
        mov  ax, word ptr [esp + 0x20] // second name index
        mov  word ptr [esi + 0x5EE], dx // first name
        mov  word ptr [esi + 0x5F0], ax // second name

    LABEL_RETURN:
        mov  eax, 0xF33BD2
        jmp  eax
    }
}
}

void PatchStaffNames(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        /* STAFF */

        /* reading from database */

        // disable original pseudonym read
        patch::Nop(0x1102F9B, 5);
        // replace original nickname read
        patch::RedirectCall(0x1102F91, fm13::ReadStaffNickname);

        /* portrait resolver */

        patch::SetUChar(0x1395B4D + 1, 2);

        /* displayed name */

        patch::RedirectJump(0x11028D0, fm13::GetStaffNameWithPseudonym);
        patch::RedirectCall(0x69C1D2, fm13::GetFullStaffName);
        patch::RedirectJump(0x11028E0, fm13::GetStaffShortNameWithPseudonym);

        /* EMPLOYEE */

        patch::SetUChar(0xE7EA7F + 1, 2);
        patch::SetUChar(0xE7EAB4 + 1, 2);

        patch::SetUChar(0xE7E9BA + 1, 2);
        patch::RedirectCall(0xE7E9C8, (void *)0x149A5A6);

        patch::RedirectJump(0xE7E9E0, fm13::GetEmployeeBaseNameWithPseudonym);
        patch::RedirectJump(0xE7EA00, fm13::GetEmployeeBaseShortNameV1WithPseudonym);
        patch::RedirectJump(0xE7EA20, fm13::GetEmployeeBaseShortNameV2WithPseudonym);

        /* PRESIDENT */

        //patch::RedirectJump(0xF33BB4, SetTeamPresidentName);

        // Staff market min/max age
        const unsigned int MIN_AGE = 18;
        const unsigned int MAX_AGE = 90;
        patch::SetUInt(0x690134 + 4, MIN_AGE);
        patch::SetUInt(0x69013C + 4, MAX_AGE);
        patch::SetUInt(0x68DF12 + 4, MIN_AGE);
        patch::SetUInt(0x68DEB6 + 4, MAX_AGE);

    }
}
