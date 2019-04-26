#include "Country207.h"

using namespace plugin;

void PatchCountry207(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::SetUInt(0x413DA2 + 2, 208);
        patch::SetUChar(0x4C61DF + 1, 207);

        patch::SetUInt(0x413ED2 + 2, 208);

        //patch::SetUInt(0x416819 + 2, 207);
        patch::SetUInt(0x416939 + 2, 208);

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

        patch::SetUInt(0x4153F4 + 1, 207); // country sorter
        patch::SetUInt(0x416819 + 2, 207);
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

    }
    else if (v.id() == ID_ED_11_1003) {
        patch::SetUInt(0x410173 + 2, 208);
        patch::SetUInt(0x4102A3 + 2, 208);
        
        // sorter?
        //patch::SetUInt(0x411178 + 1, 208);

        patch::SetUInt(0x41E139 + 2, 208);
        patch::SetUInt(0x421B6E + 2, 208);
        patch::SetUInt(0x421C07 + 1, 208);

        patch::SetUInt(0x4223DC + 2, 208);

        patch::SetUInt(0x4253D4 + 2, 208);

        // not sure
        //patch::SetUInt(0x43D6D9 + 1, 208);

        patch::SetUInt(0x44B853 + 2, 208);

        // not sure
        //patch::SetUInt(0x44EF60 + 1, 208);

        patch::SetUInt(0x44FDDC + 2, 208);

        patch::SetUInt(0x4507FD + 2, 208);
        patch::SetUInt(0x450906 + 1, 208);

        patch::SetUInt(0x4518F1 + 1, 208);

        patch::SetUInt(0x451DAC + 1, 208);

        patch::SetUInt(0x455138 + 2, 208);

        patch::SetUInt(0x459195 + 1, 208);

        patch::SetUInt(0x46B0AE + 2, 208);

        patch::SetUInt(0x46C920 + 1, 208);

        patch::SetUInt(0x475723 + 1, 208);
        // not sure
        patch::SetUInt(0x477A13 + 4, 208);

        patch::SetUInt(0x47B7DD + 1, 208);

        patch::SetUInt(0x48A8AA + 2, 208);

        // not sure
        patch::SetUChar(0x48EF14 + 2, 208);
        patch::SetUChar(0x48EF38 + 2, 208);

        patch::SetUInt(0x4A9ACA + 1, 208);
        patch::SetUInt(0x4A9CB8 + 1, 208);
        patch::SetUInt(0x4A9D4C + 2, 208);
        patch::SetUInt(0x4A9DC1 + 2, 208);
        patch::SetUInt(0x4A9E6D + 2, 208);
        patch::SetUInt(0x4A9F71 + 2, 208);

        //5-year
        //patch::SetUInt(0x4AF0B0 + 2, 208);

        patch::SetUInt(0x4AFA56 + 2, 208);
        patch::SetUInt(0x4AFAE4 + 2, 208);

        patch::SetUInt(0x4B1D3D + 2, 208);

        patch::SetUInt(0x4BA5AD + 2, 208);

        //not sure, db problem tool
        //patch::SetUInt(0x4C8CA0 + 1, 208);

        patch::SetUInt(0x4C92A3 + 1, 208);
        patch::SetUChar(0x4CC0EF + 1, 208);
        patch::SetUInt(0x4D8678 + 1, 208);
        patch::SetUInt(0x4D898C + 2, 208);
        patch::SetUInt(0x4D8AE9 + 2, 208);
        patch::SetUInt(0x4D8AFC + 1, 208);
        patch::SetUInt(0x4D8B87 + 2, 208);
        patch::SetUInt(0x4D8C22 + 1, 208);
        patch::SetUInt(0x4D8F9F + 1, 208);
        patch::SetUInt(0x4D9755 + 1, 208);

        patch::SetUInt(0x4D9B33 + 2, 208);
        patch::SetUInt(0x4DE616 + 2, 208);
        patch::SetUChar(0x4E6DA4 + 1, 208);
        patch::SetUChar(0x4E6DE9 + 1, 208);
        patch::SetUInt(0x4FF341 + 1, 208);
        patch::SetUInt(0x4FF4C4 + 2, 208);
        patch::SetUInt(0x504481 + 2, 208);
        patch::SetUInt(0x505DA0 + 2, 208);
        patch::SetUInt(0x50D719 + 1, 208);
        patch::SetUInt(0x50DAC0 + 2, 208);
        patch::SetUInt(0x50DB3A + 1, 208);
        patch::SetUInt(0x50DC7A + 1, 208);
        patch::SetUInt(0x50DCCD + 1, 208);
        patch::SetUInt(0x50DE91 + 1, 208);
        patch::SetUChar(0x50E3D1 + 1, 208);

        patch::SetUInt(0x50E5CD + 2, 208);
        patch::SetUInt(0x50EAF1 + 2, 208);
        patch::SetUInt(0x50EFA4 + 1, 208);
        patch::SetUInt(0x50F082 + 2, 208);
        patch::SetUInt(0x50FBDC + 2, 208);
        patch::SetUInt(0x510FA7 + 2, 208);
        patch::SetUInt(0x511D36 + 2, 208);
        patch::SetUInt(0x511DCF + 2, 208);
        patch::SetUInt(0x511E16 + 2, 208);
        patch::SetUInt(0x511EC4 + 2, 208);
        patch::SetUInt(0x513D4C + 2, 208);
        patch::SetUChar(0x5167A9 + 1, 208);
        patch::SetUInt(0x519B77 + 1, 208);
        patch::SetUInt(0x519D11 + 1, 208);
        patch::SetUInt(0x51B844 + 2, 208);

        patch::SetUChar(0x4A5173 + 1, 207);
        patch::SetUChar(0x4E6C2F + 1, 207);
    }
}
