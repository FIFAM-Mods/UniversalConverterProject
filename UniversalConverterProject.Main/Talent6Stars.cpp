#include "Talent6Stars.h"

using namespace plugin;

void PatchTalentStars(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD)
        patch::Nop(0xFD30E6, 4); // talent 6 stars
}
