#include "WomensDatabase.h"
#include "shared.h"

using namespace plugin;

void PatchWomensDatabase(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // database/Master.dat -> database_womens/Master
        // Restore.dat -> Restore_womens.dat
        // Database/TownDataUniques.txt -> Database_womens/TownDataUniques.txt
        // script -> script_womens
        // historic -> historic_womens
        // parameter files?
    }
}
