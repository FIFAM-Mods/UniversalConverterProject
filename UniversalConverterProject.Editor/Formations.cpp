#include "Formations.h"

using namespace plugin;

unsigned int FormationIDs[] = {
    0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 66, 67, 68
};

bool IsValidFormation(unsigned int id) {
    for (unsigned int i = 0; i < std::size(FormationIDs); i++) {
        if (FormationIDs[i] == id)
            return true;
    }
    return false;
}

void PatchFormations(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::SetUInt(0x56C940 + 1, std::size(FormationIDs));
        patch::SetPointer(0x56C959 + 3, FormationIDs);
        patch::SetPointer(0x56D69C + 3, FormationIDs);
        patch::SetUChar(0x56C954 + 2, (unsigned char)(std::size(FormationIDs) - 1));
        patch::SetUChar(0x56D697 + 2, (unsigned char)(std::size(FormationIDs) - 1));
        patch::RedirectJump(0x56C970, IsValidFormation);
    }
}
