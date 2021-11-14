#include "3dModels.h"
#include "GfxCoreHook.h"
#include "FifamTypes.h"

using namespace plugin;

#define DISABLE_3D_MODELS_PATCHES

#ifndef DISABLE_3D_MODELS_PATCHES
const UInt MAX_MODELS_PER_SCENE_INSTANCE = 32; // default 16
const UInt MAX_SCENE_GROUPS = 120;

unsigned char modelRefsData[0x4C * 32 * MAX_MODELS_PER_SCENE_INSTANCE];
unsigned char sceneGroupsData[0x4FC * MAX_SCENE_GROUPS + MAX_MODELS_PER_SCENE_INSTANCE * 4 * MAX_SCENE_GROUPS];
#endif

void Install3DModelPatches() {
#ifndef DISABLE_3D_MODELS_PATCHES
    // model refs
    patch::SetPointer(GfxCoreAddress(0x4535CB + 1), modelRefsData);
    patch::SetUInt(GfxCoreAddress(0x4535C5 + 1), std::size(modelRefsData));
    patch::SetPointer(GfxCoreAddress(0x45445C + 2), modelRefsData);
    patch::SetUInt(GfxCoreAddress(0x1EB723 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x1F242C + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x2027A9 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x202804 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x20375D + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x203F20 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x2099B2 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x20CE3D + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x20CEB2 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x20D178 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x20CFCF + 2), 0x4FC + 0xC);
    patch::SetUInt(GfxCoreAddress(0x21205E + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x451DA3 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x452088 + 3), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x452450 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x452482 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x453D90 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x453E7C + 4), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x454462 + 3), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x4545A1 + 3), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x454FF4 + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x4589CC + 2), 0x4FC);
    patch::SetUInt(GfxCoreAddress(0x458AE9 + 2), 0x4FC);
    // scene groups
    patch::SetPointer(GfxCoreAddress(0x451C9E + 2), sceneGroupsData);
    patch::SetUInt(GfxCoreAddress(0x451C98 + 2), 0x4FC + MAX_MODELS_PER_SCENE_INSTANCE * 4);
    patch::SetUInt(GfxCoreAddress(0x451CA5 + 1), 0x4FC + MAX_MODELS_PER_SCENE_INSTANCE * 4);
    patch::SetPointer(GfxCoreAddress(0x4535B5 + 1), sceneGroupsData);
    patch::SetUInt(GfxCoreAddress(0x4535AF + 1), std::size(sceneGroupsData));
    // TODO: realloc more structs
    //
#endif
}
