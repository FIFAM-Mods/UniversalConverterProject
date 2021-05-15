#include "PlayerNameEdit.h"
#include "FifamTypes.h"
#include "Settings.h"

using namespace plugin;

// mode 0 : no changes
// mode 1 : name editing is enabled for all teams; only for regen players; all restrictions are preserved
// mode 2 : name editing is enabled for all teams; all players; all restrictions are preserved
// mode 3 : name editing is enabled for all teams; all players; all restrictions are removed

void *gPlayerInfoScreenForNameEdit = nullptr;
void *gYouthInfoScreenForNameEdit = nullptr;

void METHOD OnDisablePlayerNameEditButtonForOtherTeam(void *button) {
    if (Settings::GetInstance().getPlayerNameEditMode() == 0)
        CallVirtualMethod<9>(button, false);
}

void METHOD OnEnablePlayerNameEditButtonNoRestrictions(void *button) {
    if (Settings::GetInstance().getPlayerNameEditMode() != 3)
        CallVirtualMethod<9>(button, false);
}

void METHOD OnSetPlayerNameEditButtonStatus(void *button, DUMMY_ARG, bool enable) {
    if (Settings::GetInstance().getPlayerNameEditMode() == 3)
        enable = true;
    CallVirtualMethod<9>(button, enable);
}

bool METHOD OnPlayerNameEditCheckIfPlayerIsRegen(void *player, DUMMY_ARG, UInt flag) {
    if (Settings::GetInstance().getPlayerNameEditMode() != 0)
        return true;
    return CallMethodAndReturn<bool, 0xFB07B0>(player, flag);
}

void PatchPlayerNameEdit(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

        patch::RedirectCall(0x5CD0A6, OnPlayerNameEditCheckIfPlayerIsRegen);
        patch::RedirectCall(0x5CF119, OnPlayerNameEditCheckIfPlayerIsRegen);
        patch::RedirectCall(0x5E8A27, OnPlayerNameEditCheckIfPlayerIsRegen);

        patch::RedirectCall(0x5CCAFF, OnDisablePlayerNameEditButtonForOtherTeam);
        patch::Nop(0x5CCB04, 4);

        patch::RedirectCall(0x5CC50E, OnDisablePlayerNameEditButtonForOtherTeam);
        patch::Nop(0x5CC513, 4);
        patch::RedirectCall(0x5CC61C, OnDisablePlayerNameEditButtonForOtherTeam);
        patch::Nop(0x5CC621, 4);
        patch::RedirectCall(0x5CC6A0, OnDisablePlayerNameEditButtonForOtherTeam);
        patch::Nop(0x5CC6A5, 4);
        patch::RedirectCall(0x5CC782, OnDisablePlayerNameEditButtonForOtherTeam);
        patch::Nop(0x5CC787, 4);

        patch::RedirectCall(0x5CC858, OnSetPlayerNameEditButtonStatus);


    }
}
