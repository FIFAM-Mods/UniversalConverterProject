#pragma once
#include "plugin.h"
#include "GameInterfaces.h"

struct OptionsGeneral_PlayerMarksExtension {
    CXgComboBox *CbPlayerMarksType;
    CXgComboBox *CbPlayerMarksFormat;
};

void PlayerMarks_CreateUI(CXgFMPanel *screen, OptionsGeneral_PlayerMarksExtension *ext);
Bool PlayerMarks_ProcessComboBoxes(OptionsGeneral_PlayerMarksExtension *ext, GuiMessage *msg);
void PatchPlayerMarks(FM::Version v);
