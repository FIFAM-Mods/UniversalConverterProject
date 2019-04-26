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

static void __stdcall AddNationalTeamManager_11(void *team) {
    void *manager = nullptr;
    unsigned int managerId = *raw_ptr<unsigned int>(team, 0xC54);
    if (managerId)
        manager = CallAndReturn<void *, 0xD7FAC0>(managerId);
    if (!manager) {
        manager = CallAndReturn<void *, 0xD99E50>();
        CallMethod<0xD95330>(manager, *raw_ptr<unsigned char>(team, 0x1110), *raw_ptr<unsigned char>(team, 0x511));
        managerId = CallMethodAndReturn<unsigned int, 0xD7FCE0>(manager);
    }
    void *nationalContract = CallMethodAndReturn<void *, 0xD80350>(manager);
    CallMethod<0xD01E80>(team, managerId, nationalContract);
    CallMethod<0xCFF800>(team, managerId);
    CallMethod<0xD58050>(team, managerId);
    CallMethod<0xD8E9C0>(manager, nationalContract);
}

void PatchNationalTeamManagers(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUChar(0xF343BE, 0x56); // push esi
        patch::RedirectCall(0xF343BE + 1, AddNationalTeamManager_13);
        patch::RedirectJump(0xF343BE + 6, reinterpret_cast<void *>(0xF34424));
    }
    else if (v.id() == ID_FM_11_1003) {
        patch::SetUChar(0xD76694, 0x56); // push esi
        patch::RedirectCall(0xD76694 + 1, AddNationalTeamManager_11);
        patch::RedirectJump(0xD76694 + 6, reinterpret_cast<void *>(0xD766F5));
    }
}
