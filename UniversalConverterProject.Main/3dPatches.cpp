#include "3dPatches.h"
#include "GfxCoreHook.h"

using namespace plugin;

float NewDist = 8.0f;
double NewAngleX = -2.0;

void Install3dPatches_FM13() {
    patch::SetDouble(GfxCoreAddress(0x549610), 90.0f);
    patch::SetPointer(GfxCoreAddress(0x378D22 + 2), &NewDist);
    patch::SetPointer(GfxCoreAddress(0x378DAC + 2), &NewAngleX);
    
    // Jersey number texture size
    //const unsigned int newSize = 256; // default 128x128
    //
    //patch::SetUInt(GfxCoreAddress(0x2022EE + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x2022F3 + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x202338 + 1), newSize);
    //patch::SetUInt(GfxCoreAddress(0x20233D + 1), newSize);
    
    patch::SetUInt(GfxCoreAddress(0x20F780 + 4),  4); // 25
    patch::SetUInt(GfxCoreAddress(0x20F794 + 4),  0); // 4
    patch::SetUInt(GfxCoreAddress(0x20F7A8 + 4), 60); // 39
    patch::SetUInt(GfxCoreAddress(0x20F7BC + 4), 120); // 78
    
    patch::SetUInt(GfxCoreAddress(0x20F7D0 + 4), 64); // 64
    patch::SetUInt(GfxCoreAddress(0x20F7E4 + 4),  0); // 4
    patch::SetUInt(GfxCoreAddress(0x20F7F8 + 4), 60); // 39
    patch::SetUInt(GfxCoreAddress(0x20F80C + 4), 120); // 78
    
    patch::SetUInt(GfxCoreAddress(0x20F820 + 4), 34); // 44
    patch::SetUInt(GfxCoreAddress(0x20F834 + 4),  0); // 4
    patch::SetUInt(GfxCoreAddress(0x20F848 + 4), 60); // 39
    patch::SetUInt(GfxCoreAddress(0x20F85C + 4), 120); // 78
    
    // player name
    
    //patch::SetUChar(GfxCoreAddress(0x204789 + 1), 40);
    
    // BE Scene
    patch::SetPointer(GfxCoreAddress(0x1E85E5 + 6), "data/BEscene_ucp.cs");
    
    // accs
    
    //gSetAcc = patch::RedirectCall(GfxCoreAddress(0x23D175), set_acc);
    //patch::SetUInt(GfxCoreAddress(0x281F15 + 4), 5);
    //patch::SetUInt(GfxCoreAddress(0x3B50E + 1), 120);
    //patch::SetUInt(GfxCoreAddress(0x3B519 + 1), 120);
    
    //patch::SetUInt(GfxCoreAddress(0x23CE18 + 1), 5);
}
