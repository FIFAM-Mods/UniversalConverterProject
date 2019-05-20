#include "MiscFormerCupOpponents.h"

using namespace plugin;

void PatchMiscFormerCupOpponents(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUChar(0x89780E + 1, 28); // number of fixtures: 24 => 28
    }
}
