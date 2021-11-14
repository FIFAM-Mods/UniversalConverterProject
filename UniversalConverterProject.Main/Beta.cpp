#include "LowestLeagues.h"
#include "GameInterfaces.h"

using namespace plugin;

const unsigned int CURRENT_YEAR = 2019;

void METHOD OnRegisterBigSpenders1(void *t, DUMMY_ARG, int id, int a, int b) {
    if (GetCurrentYear() != CURRENT_YEAR)
        CallMethod<0x1120440>(t, id, a, b);
}

void METHOD OnRegisterBigSpenders2(void *t) {
    if (GetCurrentYear() != CURRENT_YEAR)
        CallMethod<0x111F730>(t);
}

void METHOD OnRegisterLeagueFacts1(void *t, DUMMY_ARG, int id, int compId, int a, int b) {
    if (GetCurrentYear() != CURRENT_YEAR)
        CallMethod<0x11206A0>(t, id, compId, a, b);
}

void METHOD OnRegisterLeagueFacts2(void *t) {
    if (GetCurrentYear() != CURRENT_YEAR)
        CallMethod<0x111F730>(t); // TODO: incorrect address
}

void DoBetaPatches(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // The Big Spenders screen
        patch::RedirectCall(0xF6F26A, OnRegisterBigSpenders1);
        patch::RedirectCall(0xF6F274, OnRegisterBigSpenders2);
        // League Facts screen
        patch::RedirectCall(0xFF6224, OnRegisterLeagueFacts1);
        //patch::RedirectCall(0xFF622E, OnRegisterLeagueFacts2);
    }
}
