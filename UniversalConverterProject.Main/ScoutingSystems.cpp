#include "ScoutingSystems.h"

using namespace plugin;

static_assert(sizeof(long long) == 8, "Wrong long long size");

bool METHOD SetProScoutingSystemCost(void *money, DUMMY_ARG, __int64, int currency) {
    return CallMethodAndReturn<bool, 0x149C6BB>(money, 500'000ll, currency);
}

bool METHOD SetProScoutingSystemUpdatesCost(void *money, DUMMY_ARG, __int64, int currency) {
    return CallMethodAndReturn<bool, 0x149C6BB>(money, 10'000ll, currency);
}

void PatchScoutingSystems(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0xEA52F5, SetProScoutingSystemCost);
        patch::RedirectCall(0xEA5385, SetProScoutingSystemUpdatesCost);
    }
}
