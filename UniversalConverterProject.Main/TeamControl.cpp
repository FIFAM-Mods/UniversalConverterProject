#include "TeamControl.h"

using namespace plugin;

void PatchTeamControl(FM::Version v) {
    if (v.id() == ID_FM_11_1003) {
        patch::SetUShort(0x406210, 0x1B0); // mov al, 1
        patch::SetUChar(0x406210 + 2, 0xC3); // retn
    }
}
