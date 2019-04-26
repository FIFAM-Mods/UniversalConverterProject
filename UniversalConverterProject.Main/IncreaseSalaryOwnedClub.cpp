#include "IncreaseSalaryOwnedClub.h"

using namespace plugin;

void PatchIncreaseSalaryOwnedClub(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUChar(0xCF4FD7, 0xEB);
    }
}
