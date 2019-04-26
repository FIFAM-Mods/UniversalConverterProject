#include "MasterDatLimit.h"
#include <Windows.h>

using namespace plugin;

void PatchTalentStars(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::Nop(0xFD30E6, 4); // talent 6 stars
    }
    else if (v.id() == ID_FM_11_1003) {
        patch::Nop(0xDC97DF, 5); // talent 6 stars
    }
}
