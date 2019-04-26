#include "MasterDatLimit.h"

using namespace plugin;

void PatchMasterDatLimit(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUInt(0x1338FE4 + 1, 0x8000000); // 64 MB > 128 MB master.dat size limit
    }
    else if(v.id() == ID_FM_11_1003) {
        patch::SetUInt(0x103EED1 + 1, 0x8000000); // 64 MB > 128 MB master.dat size limit
    }
}
