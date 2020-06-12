#include "MasterDatLimit.h"

using namespace plugin;

void PatchMasterDatLimit(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUInt(0x1338FE4 + 1, 0x10000000); // 64 MB > 256 MB master.dat size limit
    }
}
