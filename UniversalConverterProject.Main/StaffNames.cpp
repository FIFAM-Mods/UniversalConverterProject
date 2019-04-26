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

namespace fm11 {
unsigned short METHOD ReadStaffNickname(void *reader) {
    // read nickname
    CallMethod<0x103F450>(reader);
    // read and return pseudonym
    return CallMethodAndReturn<unsigned short, 0x103F450>(reader);
}

void *CDBStaff_GetName(void *staff, void *desc) {
    void *worker = *raw_ptr<void *>(staff, 0xC);
    if (*raw_ptr<unsigned short>(worker, 0xE))
        CallMethod<0x1081B90>(worker, desc);
    else
        CallMethod<0x1081B70>(worker, desc);
    return desc;
}

wchar_t * METHOD GetFullStaffName(unsigned int staff, DUMMY_ARG, wchar_t *nameBuf) {
    return CallAndReturn<wchar_t *, 0xBA5A97>(*(DWORD *)(staff + 12) + 8, nameBuf);
}

wchar_t * METHOD GetStaffNameWithPseudonym(void *staff, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CDBStaff_GetName(staff, buf);
    return CallAndReturn<wchar_t *, 0xBA5A97>(desc, nameBuf);
}

wchar_t * METHOD GetStaffShortNameWithPseudonym(void *staff, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CDBStaff_GetName(staff, buf);
    return CallAndReturn<wchar_t *, 0xBA5BB9>(desc, nameBuf);
}

wchar_t * METHOD GetEmployeeBaseNameWithPseudonym(unsigned int base, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CallMethodAndReturn<void *, 0xD9C750>(base, buf);
    return CallAndReturn<wchar_t *, 0xBA5A97>(desc, nameBuf);
}

wchar_t * METHOD GetEmployeeBaseShortNameV1WithPseudonym(unsigned int base, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CallMethodAndReturn<void *, 0xD9C750>(base, buf);
    return CallAndReturn<wchar_t *, 0xBA5BB9>(desc, nameBuf);
}

wchar_t * METHOD GetEmployeeBaseShortNameV2WithPseudonym(unsigned int base, DUMMY_ARG, wchar_t *nameBuf) {
    int buf[2];
    void *desc = CallMethodAndReturn<void *, 0xD9C750>(base, buf);
    return CallAndReturn<wchar_t *, 0xBA5D20>(desc, nameBuf);
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
    else if (v.id() == ID_FM_11_1003) {
        /* STAFF */

        /* reading from database */

        // disable original pseudonym read
        patch::Nop(0xEDF83A, 5);
        // replace original nickname read
        patch::RedirectCall(0xEDF831, fm11::ReadStaffNickname);

        /* portrait resolver */

        patch::SetUChar(0x1081B9F + 1, 2);

        /* displayed name */

        patch::RedirectJump(0xEDF190, fm11::GetStaffNameWithPseudonym);
        patch::RedirectCall(0x62940D, fm11::GetFullStaffName);
        patch::RedirectJump(0xEDF1A0, fm11::GetStaffShortNameWithPseudonym);

        /* EMPLOYEE */

        patch::SetUChar(0xD9C778 + 1, 2);

        patch::SetUChar(0xD9C6AC + 1, 2);
        patch::RedirectCall(0xD9C6BA, (void *)0xBA5A29);

        patch::RedirectJump(0xD9C6D0, fm11::GetEmployeeBaseNameWithPseudonym);
        patch::RedirectJump(0xD9C6F0, fm11::GetEmployeeBaseShortNameV1WithPseudonym);
        patch::RedirectJump(0xD9C710, fm11::GetEmployeeBaseShortNameV2WithPseudonym);

        // Empics ID
        patch::SetPointer(0x43AB4F + 1, L"%02d%02d%04d-%d");
        patch::SetPointer(0x43AB77 + 1, L"-%d");
    }
}
