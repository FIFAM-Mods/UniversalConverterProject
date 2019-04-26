#include "EditorChiefExecExport.h"

using namespace plugin;

void PatchEditorChiefExecExport(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::SetUChar(0x51F538, 0xEB); // english clubs - chief exec
    }
    else if (v.id() == ID_ED_11_1003) {
        patch::SetUChar(0x4E69C8, 0xEB); // english clubs - chief exec
    }
}
