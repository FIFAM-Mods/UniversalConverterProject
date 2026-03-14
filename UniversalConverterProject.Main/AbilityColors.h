#pragma once
#include "plugin.h"
#include "GameInterfaces.h"

struct OptionsGeneral_AbilityColorsExtension {
    CXgComboBox *CbAbilitiesColorSchema;
    CXgComboBox *CbUnexploredAbilityColors;
    CXgCheckBox *ChkAbilitiesBoldFont;
};

void AbilityColors_CreateUI(CXgFMPanel *screen, OptionsGeneral_AbilityColorsExtension *ext);
Bool AbilityColors_ProcessCheckBoxes(OptionsGeneral_AbilityColorsExtension *ext, GuiMessage *msg);
Bool AbilityColors_ProcessComboBoxes(OptionsGeneral_AbilityColorsExtension *ext, GuiMessage *msg);
void UpdateStaffAbilityColors(void *screen);
void PatchAbilityColors(FM::Version v);
