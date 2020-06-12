#include "Country207.h"

using namespace plugin;

void *METHOD OnGetCountryFor5YearTable(void *world, DUMMY_ARG, int countryId) {
    if (countryId == 207)
        return 0;
    return CallMethodAndReturn<void *, 0x4C9DC0>(world, countryId);
}

int METHOD OnGetNumberOfLinesInAssessmentSav(void *) {
    return 207;
}

void PatchCountry207(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::SetUInt(0x413DA2 + 2, 208);
        patch::SetUChar(0x4C61DF + 1, 207);

        patch::SetUInt(0x413ED2 + 2, 208);

        //patch::SetUInt(0x416819 + 2, 207);
        //patch::SetUInt(0x416939 + 2, 208);

        patch::SetUInt(0x421445 + 1, 208);

        patch::SetUInt(0x4238D8 + 2, 208);

        patch::SetUInt(0x428A6E + 2, 208);
        patch::SetUInt(0x428B35 + 1, 208);
        patch::SetUInt(0x42930C + 2, 208);
        patch::SetUInt(0x42C403 + 2, 208);
        patch::SetUInt(0x457901 + 2, 208);
        patch::SetUInt(0x45D32B + 2, 208);

        patch::SetUInt(0x463BBA + 2, 208);
        patch::SetUInt(0x468A5B + 2, 208);
        patch::SetUInt(0x481B2E + 2, 208);
        patch::SetUInt(0x483C88 + 2, 208);
        patch::SetUInt(0x48535F + 2, 208);
        // 48BB80 jle
        // 48EC40 loop
        // 492A19 not sure

        patch::SetUChar(0x4A79C3 + 2, 208);
        patch::SetUChar(0x4A79E3 + 2, 208);

        // 4CA0FF unused?

        // db load
        patch::SetUInt(0x4D4080 + 2, 208);
        patch::SetUInt(0x4D4118 + 2, 208);

        patch::SetUInt(0x4D452B + 2, 208);

        patch::SetUInt(0x4DA3E4 + 2, 208);

        // league errors
        patch::SetUChar(0x4FD82F + 1, 208);

        // 50A1A3 related to liveseason?
        // 50BCD4 not sure
        patch::SetUInt(0x50C02D + 2, 208);

        patch::SetUInt(0x50D5BB + 2, 208);
        patch::SetUInt(0x50D5CE + 1, 208);
        patch::SetUInt(0x50D66A + 2, 208);
        // 50E6F6 errcheck - not sure
        // 50EB81 errcheck - not sure

        patch::SetUChar(0x52000F + 1, 208);

        // not sure
        patch::SetUChar(0x53DA80 + 2, 208);

        patch::SetUInt(0x5437F5 + 2, 208);

        patch::SetUInt(0x545293 + 2, 208);

        // 5522AB not sure
        // hotfix
        patch::SetUInt(0x5532D1 + 2, 208);

        // restoration - not sure
        // 5537C4

        // save club badges (hotfix)
        patch::SetUInt(0x5538A3 + 2, 208);
        patch::SetUInt(0x5546B4 + 2, 208);
        patch::SetUInt(0x556D38 + 2, 208);
        patch::SetUInt(0x556DCF + 2, 208);
        patch::SetUInt(0x556E16 + 2, 208);
        patch::SetUInt(0x556ED4 + 2, 208);

        patch::SetUInt(0x55972F + 2, 208);

        // list competitions IDs
        patch::SetUChar(0x55D139 + 1, 208);
        patch::SetUInt(0x563088 + 1, 208);
        // choose script to run
        patch::SetUInt(0x5639A9 + 1, 208);
        patch::SetUInt(0x563B3E + 2, 208);


        patch::SetUInt(0x4E679F + 2, 208);

        // 4F5B8E not sure

        //patch::SetUInt(0x4153F4 + 1, 207); // country sorter
        //patch::SetUInt(0x416819 + 2, 207);
        patch::SetUInt(0x5524F2 + 4, 207);
        // fix legal issues
        patch::SetUInt(0x556E97 + 1, 207);

        patch::SetUInt(0x4C9DFB + 2, 206);
        patch::SetUInt(0x4C9FF9 + 2, 206);
        patch::SetUInt(0x4CA08D + 1, 206);
        patch::SetUInt(0x551637 + 2, 206);
        patch::SetUInt(0x55168A + 1, 206);

        // badges and graphics
        patch::SetUInt(0x4CA1A3 + 2, 208);

        patch::SetUInt(0x45F4BA + 1, 208);
        patch::SetUInt(0x45DCCB + 2, 208);
        patch::SetUInt(0x45DDD6 + 1, 208);

        patch::RedirectCall(0x4167FB, OnGetCountryFor5YearTable);
        patch::RedirectCall(0x4CA3C5, OnGetNumberOfLinesInAssessmentSav);

    }
}
