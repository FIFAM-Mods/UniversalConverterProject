#include "WorldCup.h"
#include "GameInterfaces.h"
#include "FifamCompRegion.h"

using namespace plugin;

Bool gChangedStartDateForWC = false;

void METHOD SetSeasonStartDateForWorldCup(CDBGame *g, DUMMY_ARG, CJDate d) {
    gChangedStartDateForWC = true;
    g->SetCurrentDate(d);
    //Message(L"SetSeasonStartDateForWorldCup");
}

void METHOD OnLoadGameStartDatabase(void *t, DUMMY_ARG, Int e) {
    if (gChangedStartDateForWC) {
        gChangedStartDateForWC = false;
        CDBGame *g = CDBGame::GetInstance();
        if (g) {
            CJDate date;
            date.Set(2019, 7, 1); // Change this with new start year
            g->SetCurrentDate(date);
            //Message(L"OnLoadGameStartDatabase");
        }
    }
    CallMethod<0x524A90>(t, e);
}

void PatchWorldCup(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUInt(0x108A8E6 + 1, 2022); // Start year
        patch::SetUInt(0x108C7FC + 1, 2021); // Quali start year

        patch::RedirectCall(0x108C81D, SetSeasonStartDateForWorldCup);
        patch::RedirectCall(0x47F7D7, OnLoadGameStartDatabase);

        patch::SetUInt(0x308DC88, FifamCompRegion::Qatar); // Hosting team

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
        patch::SetUInt(0x145E240, FifamCompRegion::Qatar); // Hosting team
    }
}
