#include "DatabaseName.h"
#include "shared.h"

using namespace plugin;

#define DB_VERSION_HEX 0x2019

void OnSetRadioButtonIndex(void **rb, unsigned int num, unsigned int index) {
    Call<0xD2D040>(rb, num, 0);
    for (int i = 1; i <= 2; i++) {
        CallVirtualMethod<9>(rb[i], 0); // SetEnabled
        CallVirtualMethod<11>(rb[i], 0); // SetVisible
    }
    CallVirtualMethod<9>(rb[0], 0);
}

void OnSetRadioButtonIndex_11(void **rb, unsigned int num, unsigned int index) {
    Call<0xB01370>(rb, num, 0);
    CallVirtualMethod<9>(rb[1], 0);
    CallVirtualMethod<11>(rb[1], 0);
    CallVirtualMethod<9>(rb[0], 0);
}

void PatchDatabaseName(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUInt(0x108F675 + 1, DB_VERSION_HEX);

        patch::SetPointer(0x524B09 + 1, (void *)GetMainDatabaseName());
        patch::SetPointer(0x524B24 + 1, (void *)GetEditorDatabaseName());
        patch::SetPointer(0x524B53 + 1, (void *)GetEditorDatabaseName());
        patch::SetPointer(0x54F17B + 1, (void *)GetWorldCupDatabaseName());

        patch::SetPointer(0x5247C0 + 1, (void *)GetDatabaseScreenName());
        patch::SetPointer(0x52E1F0 + 1, (void *)GetMainMenuScreenName());

        patch::RedirectCall(0x524E1A, OnSetRadioButtonIndex);

        //patch::SetUChar(0x1080E29 + 2, 40);
        //patch::SetUInt(0x1082C02 + 3, 40);
    }
    else if (v.id() == ID_FM_11_1003) {
        patch::RedirectCall(0x4C6EB9, OnSetRadioButtonIndex_11);

        patch::SetPointer(0x4C70D2 + 1, (void *)GetMainDatabaseName());
        patch::SetPointer(0x4C70EE + 1, (void *)GetEditorDatabaseName());
        patch::SetPointer(0x4C710D + 1, (void *)GetEditorDatabaseName());
        patch::SetPointer(0x4E7887 + 1, (void *)GetWorldCupDatabaseName());

        patch::SetPointer(0x4C6E30 + 1, (void *)GetDatabaseScreenName());
        patch::SetPointer(0x4CE320 + 1, (void *)GetMainMenuScreenName());
    }
}
