#include "TournamentFinalists.h"
#include "FifamNation.h"

using namespace plugin;

void PatchTournamentFinalists(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUChar(0xF97103 + 1, FifamNation::England); // Euro runner-up
        patch::SetUChar(0xF97116 + 1, FifamNation::Spain); // Euro winner
        patch::SetUChar(0xF97138 + 1, FifamNation::Colombia); // Copa America runner-up
        patch::SetUChar(0xF9714B + 1, FifamNation::Argentina); // Copa America winner
    }
}
