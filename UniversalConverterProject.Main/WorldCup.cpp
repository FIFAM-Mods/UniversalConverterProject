#include "WorldCup.h"

using namespace plugin;

void PatchWorldCup(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUInt(0x108A8E6 + 1, 2018); // Start year
        patch::SetUInt(0x108C7FC + 1, 2017); // Quali start year

        patch::SetUInt(0x308DC88, 40); // Hosting team

        using CdeclRetInt = int(__cdecl *)();
        //patch::RedirectCall(0x108C6F0, (CdeclRetInt)([] {
        //    return 8;
        //}));
        patch::Nop(0x108A9C3, 2);
        patch::Nop(0x108A9A1, 2);
        patch::Nop(0x108A9B7, 5);
        patch::RedirectCall(0x108C733, (CdeclRetInt)([] {
            return 10;
        }));
        using Thiscall = void(__fastcall *)(void *);
        patch::RedirectCall(0x108B67E, (Thiscall)([](void *t) {
            CallMethod<0x1010500>(t);
            int date = 0;
            Call<0x108A8B0>(&date);
            void *game = CallAndReturn<void *, 0xF61410>();
            CallMethod<0xF49960>(game, date);
        }));
    }
    else if (v.id() == ID_FM_11_1003) {
        patch::SetUInt(0x145E240, 40); // Hosting team
    }
}
