#include "NationalTeamManagers.h"

using namespace plugin;

static void __stdcall AddNationalTeamManager_13(void *team) {
    void *manager = nullptr;
    unsigned int managerId = *raw_ptr<unsigned int>(team, 0x5D8);
    if (managerId)
        manager = CallAndReturn<void *, 0xEA2A00>(managerId);
    if (!manager) {
        manager = CallAndReturn<void *, 0xEC3640>();
        CallMethod<0xEB4070>(manager, *raw_ptr<unsigned char>(team, 0x984), *raw_ptr<unsigned char>(team, 0xEA));
        managerId = CallMethodAndReturn<unsigned int, 0xEA2BE0>(manager);
    }
    void *nationalContract = CallMethodAndReturn<void *, 0xEA3550>(manager);
    CallMethod<0xEFA6D0>(team, managerId, nationalContract);
    CallMethod<0xEF6420>(team, managerId);
    CallMethod<0xEDE150>(team, managerId);
    CallMethod<0xEB5AD0>(manager, nationalContract);
}

void PatchNationalTeamManagers(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUChar(0xF343BE, 0x56); // push esi
        patch::RedirectCall(0xF343BE + 1, AddNationalTeamManager_13);
        patch::RedirectJump(0xF343BE + 6, reinterpret_cast<void *>(0xF34424));
    }
}
