#include "GameOptionsScreen.h"
#include "FifamTypes.h"
#include "UcpSettings.h"
#include "AbilityColors.h"
#include "PlayerMarks.h"

using namespace plugin;

const UInt OptionsGeneralDefaultSize = 0x580;

OptionsGeneral_AbilityColorsExtension *GetAbilityColorsExtension(CXgFMPanel *screen) {
    return raw_ptr<OptionsGeneral_AbilityColorsExtension>(screen, OptionsGeneralDefaultSize);
}

OptionsGeneral_PlayerMarksExtension *GetPlayerMarksExtension(CXgFMPanel *screen) {
    return raw_ptr<OptionsGeneral_PlayerMarksExtension>(screen, OptionsGeneralDefaultSize +
        sizeof(OptionsGeneral_AbilityColorsExtension));
}

CXgCheckBox *METHOD OnOptionsGeneralSetupUI(CXgFMPanel *screen, DUMMY_ARG, Char const *name) {
    AbilityColors_CreateUI(screen, GetAbilityColorsExtension(screen));
    PlayerMarks_CreateUI(screen, GetPlayerMarksExtension(screen));
    return screen->GetCheckBox(name);
}

void METHOD OnOptionsGeneralProcessCheckBoxes(CXgFMPanel *screen, DUMMY_ARG, GuiMessage *msg, Int unk) {
    if (AbilityColors_ProcessCheckBoxes(GetAbilityColorsExtension(screen), msg))
        return;
    CallMethod<0x7AFCF0>(screen, msg, unk);
}

void METHOD OnOptionsGeneralProcessComboBoxes(CXgFMPanel *screen, DUMMY_ARG, GuiMessage *msg, Int unk1, Int unk2) {
    if (AbilityColors_ProcessComboBoxes(GetAbilityColorsExtension(screen), msg))
        return;
    if (PlayerMarks_ProcessComboBoxes(GetPlayerMarksExtension(screen), msg))
        return;
    CallMethod<0x7AE640>(screen, msg, unk1, unk2);
}

void PatchGameOptionsScreen(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        const UInt OptionsGeneralNewSize =
            OptionsGeneralDefaultSize
            + sizeof(OptionsGeneral_AbilityColorsExtension)
            + sizeof(OptionsGeneral_PlayerMarksExtension);
        patch::SetUInt(0x573004 + 1, OptionsGeneralNewSize);
        patch::SetUInt(0x57300B + 1, OptionsGeneralNewSize);
        patch::RedirectCall(0x7AF893, OnOptionsGeneralSetupUI);
        patch::SetPointer(0x2400C9C, OnOptionsGeneralProcessCheckBoxes);
        patch::SetPointer(0x2400C88, OnOptionsGeneralProcessComboBoxes);
    }
}
