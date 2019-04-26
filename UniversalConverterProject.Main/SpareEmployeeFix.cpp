#include "SpareEmployeeFix.h"

using namespace plugin;

void PatchSpareEmployeeFix(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::Nop(0xFF8530, 1);
        patch::SetUChar(0xFF8530 + 1, 0xE9);
    }
}
