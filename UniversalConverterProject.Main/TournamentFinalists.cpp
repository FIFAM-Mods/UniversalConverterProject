#include "TournamentFinalists.h"

using namespace plugin;

void PatchTournamentFinalists(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUChar(0xF97103 + 1, 18); // France
        patch::SetUChar(0xF97116 + 1, 38); // Portugal
        patch::SetUChar(0xF97138 + 1, 55); // Argentina
        patch::SetUChar(0xF9714B + 1, 52); // Chile
    }
}
