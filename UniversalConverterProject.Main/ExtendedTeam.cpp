#include "ExtendedTeam.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "ClubSponsors.h"

using namespace plugin;

const UInt DEF_TEAM_SZ = 0x7A60;
const UShort TEAM_EXT_VERSION = 0x1;
const UInt FORMATION_OFFSET = DEF_TEAM_SZ;
UInt gTeamExtenderTeamLoaderAddr[5] = {};

struct TeamExtension {
    UInt bestFormationID[5];
};

void METHOD TeamClearBestFormations(CDBTeam *team) {
    TeamExtension *ext = raw_ptr<TeamExtension>(team, DEF_TEAM_SZ);
    for (UInt i = 0; i < 5; i++)
        ext->bestFormationID[i] = 0;
}

void ClearTeamFormationInfo(CDBTeam *team) {
    TeamClearBestFormations(team);
    *raw_ptr<UInt>(team, 0xF54) = *raw_ptr<UInt>(team, 0xF54) - 8; // force formation update
    team->SetFlag(0x100, true);
    *raw_ptr<UInt>(team, 0xC5C + 0x48) = 0;
    *raw_ptr<UInt>(team, 0xC5C + 0x4C) = 0;
    if (team->GetTeamID().index != 0xFFFF) {
        *raw_ptr<UInt>(team, 0xC5C + 0x58 * 1 + 0x48) = 0;
        *raw_ptr<UInt>(team, 0xC5C + 0x58 * 1 + 0x4C) = 0;
        *raw_ptr<UInt>(team, 0xC5C + 0x58 * 2 + 0x48) = 0;
        *raw_ptr<UInt>(team, 0xC5C + 0x58 * 2 + 0x4C) = 0;
        *raw_ptr<UInt>(team, 0xC5C + 0x58 * 3 + 0x48) = 0;
        *raw_ptr<UInt>(team, 0xC5C + 0x58 * 3 + 0x4C) = 0;
    }
}

template<Bool Constructor>
void METHOD OnClearTeam(CDBTeam *team) {
    CallMethod<0xF1D8B0>(team);
    TeamClearBestFormations(team);
}

UInt METHOD TeamGetBestFormation(CDBTeam *team, DUMMY_ARG, UChar index) {
    TeamExtension *ext = raw_ptr<TeamExtension>(team, DEF_TEAM_SZ);
    return ext->bestFormationID[index];
}

void METHOD TeamSetBestFormation(CDBTeam *team, DUMMY_ARG, UChar index, UInt formationId) {
    TeamExtension *ext = raw_ptr<TeamExtension>(team, DEF_TEAM_SZ);
    ext->bestFormationID[index] = formationId;
}

UInt METHOD TeamGetNumOfBestFormations(CDBTeam *team) {
    TeamExtension *ext = raw_ptr<TeamExtension>(team, DEF_TEAM_SZ);
    for (UInt i = 0; i < 5; i++) {
        if (!ext->bestFormationID[i])
            return i;
    }
    return 5;
}

void __declspec(naked) TeamFormationExt1() {
    __asm {
        mov eax, FORMATION_OFFSET
        mov [esi + eax + 0], 0
        mov [esi + eax + 4], 0
        mov [esi + eax + 8], 0
        mov [esi + eax + 12], 0
        mov [esi + eax + 16], 0
        mov eax, 0x1004014
        jmp eax
    }
}

void __declspec(naked) TeamFormationExt4() {
    __asm {
        shl edx, 2
        add edx, FORMATION_OFFSET
        mov [edx + esi], ebx
        mov edx, 0x1004159
        jmp edx
    }
}

void METHOD TeamLoadBestFormation(void *loader, DUMMY_ARG, UInt *out) {
    if (SaveGameLoadGetVersion(loader) >= 43)
        SaveGameReadInt32(loader, *out);
    else {
        UChar value = 0;
        SaveGameReadInt8(loader, value);
        *out = value;
    }
}

void METHOD TeamSaveBestFormation(void *writer, DUMMY_ARG, UInt value) {
    SaveGameWriteInt32(writer, value);
}

void __declspec(naked) TeamFormationExt2() {
    __asm {
        add edi, 4
        sub ebx, 1
        jnz loc_F44B60
        mov eax, 0xF44B72
        jmp eax
loc_F44B60:
        mov eax, 0xF44B60
        jmp eax
    }
}

void __declspec(naked) TeamFormationExt3() {
    __asm {
        add esi, 4
        sub edi, 1
        jnz loc_F0EC92
        mov eax, 0xF0ECA7
        jmp eax
loc_F0EC92:
        mov eax, 0xF0EC92
        jmp eax
    }
}

template<UInt id>
void METHOD OnReadTeamFromMaster(CDBTeam *team, DUMMY_ARG, void *loader, Int countryId, Int index) {
    SetupClubSponsorBeforeDBRead(team);
    CallMethodDynGlobal(gTeamExtenderTeamLoaderAddr[id], team, loader, countryId, index);
    SetupClubSponsorAfterDBRead(team);
}

void PatchExtendedTeam(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        const UInt NEW_TEAM_SZ = DEF_TEAM_SZ + sizeof(TeamExtension);
        patch::SetUInt(0xF40DDE + 1, NEW_TEAM_SZ);
        patch::SetUInt(0xF42134 + 1, NEW_TEAM_SZ);
        patch::SetUInt(0xF432AA + 1, NEW_TEAM_SZ);
        patch::SetUInt(0xDDCC00 + 1, NEW_TEAM_SZ);
        patch::SetUInt(0xDEE230 + 1, NEW_TEAM_SZ);
        patch::RedirectCall(0xF432E2, OnClearTeam<true>);
        patch::RedirectCall(0xF4171F, OnClearTeam<false>);
        gTeamExtenderTeamLoaderAddr[0] = patch::RedirectCall(0xFF2272, OnReadTeamFromMaster<0>);
        gTeamExtenderTeamLoaderAddr[1] = patch::RedirectCall(0xFF24C4, OnReadTeamFromMaster<1>);
        gTeamExtenderTeamLoaderAddr[2] = patch::RedirectCall(0xFF2904, OnReadTeamFromMaster<2>);
        gTeamExtenderTeamLoaderAddr[3] = patch::RedirectCall(0xFF2A1A, OnReadTeamFromMaster<3>);
        gTeamExtenderTeamLoaderAddr[4] = patch::RedirectCall(0xFF2AC8, OnReadTeamFromMaster<4>);
        // FORMATIONS
        patch::RedirectJump(0x1002BC0, TeamClearBestFormations);
        patch::RedirectJump(0x1002BD0, TeamGetBestFormation);
        patch::RedirectJump(0x1002BE0, TeamSetBestFormation);
        patch::RedirectJump(0x10031F0, TeamGetNumOfBestFormations);
        patch::RedirectCall(0xF44B67, TeamLoadBestFormation);
        patch::RedirectCall(0xF0EC9C, TeamSaveBestFormation);
        // CDBTeam::UpdateFormation()
        // 1
        patch::SetUChar(0x1003FCA, 0x83);
        patch::SetUInt(0x1003FCA + 2, FORMATION_OFFSET);
        // 2
        patch::RedirectJump(0x100400E, TeamFormationExt1);
        patch::Nop(0x100401B, 6);
        // 3
        patch::SetUChar(0x1004065, 0x8B);
        patch::SetUInt(0x1004065 + 2, FORMATION_OFFSET);
        patch::SetUChar(0x100406B, 0x85);
        // 4
        patch::SetUChar(0x100414B, 0x8B);
        patch::RedirectJump(0x1004152, TeamFormationExt4);
        // CDBTeam::AutoFillLineUp()
        patch::SetUChar(0x1004236, 0x83);
        patch::SetUInt(0x1004236 + 2, FORMATION_OFFSET);
        // CDBTeam::GetBestFormations()
        // 1
        patch::SetUChar(0x1003E23, 0x83);
        patch::SetUInt(0x1003E23 + 3, FORMATION_OFFSET);
        // 2
        patch::Nop(0x1003F10, 1);
        patch::SetUChar(0x1003F10 + 1, 0x8B);
        patch::SetUInt(0x1003F10 + 4, FORMATION_OFFSET);
        // CDBTeam::Load()
        patch::SetUInt(0xF44B4F + 2, FORMATION_OFFSET);
        patch::RedirectJump(0xF44B6C, TeamFormationExt2);
        // CDBTeam::Save()
        patch::SetUInt(0xF0EC87 + 2, FORMATION_OFFSET);
        patch::Nop(0xF0EC92, 1);
        patch::SetUChar(0xF0EC92 + 1, 0x8B);
        patch::RedirectJump(0xF0ECA1, TeamFormationExt3);
    }
}
