#include "PlayerAccessories.h"
#include "Utils.h"
#include "FifamReadWrite.h"
#include "GfxCoreHook.h"
#include "shared.h"

struct PlayerAccessories {
    unsigned char jerseystyle : 1; // 0 - tucked, 1 - untucked (not used currently)
    unsigned char sleeves : 1; // 0 - short, 1 - long
    unsigned char accclr1 : 3; // 0 - 5, 0 - default (white), 1 - white, 2 - black, 3 - blue, 4 - red, 5 - yellow
    unsigned char accclr2 : 3;
    unsigned char acc1 : 4; // 0 - 14, 0 - no accessory
    unsigned char acc2 : 4;
    unsigned char acc3 : 4;
    unsigned char acc4 : 4;
    unsigned char accclr3 : 3;
    unsigned char accclr4 : 3;
    unsigned char socks : 2; // 0 - normal, 1 - low, 2 - high; FM: 1 - low thin, 2 - normal thin, 3 - high thin, 4 - low, 5 - normal, 6 - high
};

static_assert(sizeof(PlayerAccessories) == 4, "PlayerAccessories size error");

Map<UInt, PlayerAccessories> &GetPlayerAccessoriesMap() {
    static Map<UInt, PlayerAccessories> playerAccessoriesMap;
    return playerAccessoriesMap;
}

Map<UInt, UInt> &GetPlayerCustomShoesMap() {
    static Map<UInt, UInt> playerCustomShoesMap;
    return playerCustomShoesMap;
}

Map<UInt, UInt> &GetPlayerCustomGlovesMap() {
    static Map<UInt, UInt> playerCustomGlovesMap;
    return playerCustomGlovesMap;
}

PlayerAccessories *GetPlayerAccessories(UInt playerEmpicsId) {
    auto it = GetPlayerAccessoriesMap().find(playerEmpicsId);
    if (it != GetPlayerAccessoriesMap().end()) {
        return &(*it).second;
    }
    return nullptr;
}

Int GetPlayerCustomShoe(UInt playerEmpicsId) {
    auto it = GetPlayerCustomShoesMap().find(playerEmpicsId);
    if (it != GetPlayerCustomShoesMap().end()) {
        return (*it).second;
    }
    return -1;
}

Int GetPlayerCustomGlove(UInt playerEmpicsId) {
    auto it = GetPlayerCustomGlovesMap().find(playerEmpicsId);
    if (it != GetPlayerCustomGlovesMap().end()) {
        return (*it).second;
    }
    return -1;
}

void ReadPlayerAccessoriesFile() {
    FifamReader reader(L"plugins\\ucp\\player_accessories.csv", 14);
    if (reader.Available()) {
        reader.SkipLine();
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                UInt playerid = 0;
                String comment;
                UChar jerseystyle = 0;
                UChar sleeves = 0;
                UChar socks = 0;
                UChar acc1 = 0;
                UChar acc2 = 0;
                UChar acc3 = 0;
                UChar acc4 = 0;
                UChar accclr1 = 0;
                UChar accclr2 = 0;
                UChar accclr3 = 0;
                UChar accclr4 = 0;
                reader.ReadLine(playerid, comment, jerseystyle, sleeves, socks, acc1, accclr1, acc2, accclr2, acc3, accclr3, acc4, accclr4);
                if (playerid != 0) {
                    if (jerseystyle > 1)
                        jerseystyle = 0;
                    if (sleeves > 1)
                        sleeves = 0;
                    if (socks > 6)
                        socks = 0;
                    if (acc1 > 14)
                        acc1 = 0;
                    if (accclr1 > 5)
                        accclr1 = 1;
                    if (acc2 > 14)
                        acc2 = 0;
                    if (accclr2 > 5)
                        accclr2 = 1;
                    if (acc3 > 14)
                        acc3 = 0;
                    if (accclr3 > 5)
                        accclr3 = 1;
                    if (acc4 > 14)
                        acc4 = 0;
                    if (accclr4 > 5)
                        accclr4 = 1;
                    if (jerseystyle != 0 || sleeves != 0 || socks != 0 || acc1 != 0 || acc2 != 0 || acc3 != 0 || acc4 != 0) {
                        auto &accessories = GetPlayerAccessoriesMap()[playerid];
                        accessories.jerseystyle = jerseystyle;
                        accessories.sleeves = sleeves;
                        accessories.socks = socks;
                        accessories.acc1 = acc1;
                        accessories.acc2 = acc2;
                        accessories.acc3 = acc3;
                        accessories.acc4 = acc4;
                        accessories.accclr1 = accclr1;
                        accessories.accclr2 = accclr2;
                        accessories.accclr3 = accclr3;
                        accessories.accclr4 = accclr4;
                    }
                }
            }
            else
                reader.SkipLine();
        }
    }
}

void ReadPlayerCustomShoesFile() {
    FifamReader reader(L"plugins\\ucp\\player_shoes.csv", 14);
    if (reader.Available()) {
        reader.SkipLine();
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                UInt playerid = 0;
                UInt shoeid = 0;
                reader.ReadLine(playerid, shoeid);
                if (playerid != 0)
                    GetPlayerCustomShoesMap()[playerid] = shoeid;
            }
            else
                reader.SkipLine();
        }
    }
}

void ReadPlayerCustomGlovesFile() {
    FifamReader reader(L"plugins\\ucp\\player_gloves.csv", 14);
    if (reader.Available()) {
        reader.SkipLine();
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                UInt playerid = 0;
                UInt gloveid = 0;
                reader.ReadLine(playerid, gloveid);
                if (playerid != 0)
                    GetPlayerCustomGlovesMap()[playerid] = gloveid;
            }
            else
                reader.SkipLine();
        }
    }
}

//
// Pass player accessories to gfxcore
//

PlayerAccessories *gCurrentPlayerAccessories = nullptr;
Int gCurrentPlayerCustomShoe = -1;
Int gCurrentPlayerCustomGlove = -1;

void ObtainCurrentPlayerAccessories(char *a, const char *b, Int id) { // manager
    sprintf(a, b, id);
    gCurrentPlayerAccessories = nullptr;
    void *player = CallAndReturn<void *, 0xF97C70>(id); // PlayerFromUID
    if (player) {
        UInt empicsid = *raw_ptr<UInt>(player, 0xE0);
        if (empicsid != 0) {
            gCurrentPlayerAccessories = GetPlayerAccessories(empicsid);
            gCurrentPlayerCustomShoe = GetPlayerCustomShoe(empicsid);
            gCurrentPlayerCustomGlove = GetPlayerCustomGlove(empicsid);
        }
    }
}

void PassPlayerAccessoriesSleeves(char *a, const char *b, UInt sleeves) { // manager
    if (gCurrentPlayerAccessories)
        sleeves = gCurrentPlayerAccessories->sleeves;
    sprintf(a, b, sleeves);
}

void PassPlayerAccessoriesShoeColor(void *a, const char *b, UInt shoecolor) { // manager
    if (gCurrentPlayerCustomShoe >= 0)
        shoecolor = gCurrentPlayerCustomShoe;
    Call<0x413AB0>(a, b, shoecolor);
}

void PassPlayerAccessoriesSocksAndAccessories(void *a, const char *b, UInt socks) { // manager
    if (gCurrentPlayerAccessories) {
        switch (gCurrentPlayerAccessories->socks) {
        case 1:
            socks = 4;
            break;
        case 2:
            socks = 6;
            break;
        default:
            socks = 5;
            break;
        }
    }
    else if (socks == 0)
        socks = 5;
    Call<0x413AB0>(a, b, socks);
    if (gCurrentPlayerAccessories) {
        Call<0x413AB0>(a, "accessoryid1\t%d\r\n", gCurrentPlayerAccessories->acc1);
        Call<0x413AB0>(a, "accessoryid2\t%d\r\n", gCurrentPlayerAccessories->acc2);
        Call<0x413AB0>(a, "accessoryid3\t%d\r\n", gCurrentPlayerAccessories->acc3);
        Call<0x413AB0>(a, "accessoryid4\t%d\r\n", gCurrentPlayerAccessories->acc4);
        Call<0x413AB0>(a, "accessoryid5\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessorycolor1\t%d\r\n", gCurrentPlayerAccessories->accclr1);
        Call<0x413AB0>(a, "accessorycolor2\t%d\r\n", gCurrentPlayerAccessories->accclr2);
        Call<0x413AB0>(a, "accessorycolor3\t%d\r\n", gCurrentPlayerAccessories->accclr3);
        Call<0x413AB0>(a, "accessorycolor4\t%d\r\n", gCurrentPlayerAccessories->accclr4);
        Call<0x413AB0>(a, "accessorycolor5\t%d\r\n", 0);
    }
    else {
        Call<0x413AB0>(a, "accessoryid1\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessoryid2\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessoryid3\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessoryid4\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessoryid5\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessorycolor1\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessorycolor2\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessorycolor3\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessorycolor4\t%d\r\n", 0);
        Call<0x413AB0>(a, "accessorycolor5\t%d\r\n", 0);
    }
}

//
// Read player accessories in gfxcore
//

void *gPlayerAccessoriesAttributesInfo = nullptr;

void METHOD OnReadPlayerAttributesInGfxCore(void *info) {
    gPlayerAccessoriesAttributesInfo = info;
    CallMethodDynGlobal(GfxCoreAddress(0x91E40), info);
    gPlayerAccessoriesAttributesInfo = nullptr;
}

Int METHOD ReadPlayerAccessoriesInGfxCore(void *io, DUMMY_ARG, Int type, const char *attr) { // gfxcore
    if (gPlayerAccessoriesAttributesInfo) {
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x194) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessorycolor1");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x198) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessorycolor2");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x19C) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessorycolor3");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1A0) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessorycolor4");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1A4) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessorycolor5");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1A8) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessoryid1");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1AC) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessoryid2");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1B0) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessoryid3");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1B4) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessoryid4");
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1B8) = CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, "accessoryid5");
    }
    else {
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x194) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x198) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x19C) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1A0) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1A4) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1A8) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1AC) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1B0) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1B4) = 0;
        *raw_ptr<Int>(gPlayerAccessoriesAttributesInfo, 0x1B8) = 0;
    }
    return CallMethodAndReturnDynGlobal<Int>(GfxCoreAddress(0x25C840), io, type, attr);
}

//
// Setup player accessories on player model in gfxcore
//

Bool gPlayerAccessoriesIsCaptain = false;
void *gPlayerAccessoriesTeamDb = nullptr;
UInt gPlayerAccessoriesPlayerIndex = 0;

Bool METHOD StoreGlobalParamsForPlayerModel(void *teamdb, DUMMY_ARG, UInt playerIndex) { // gfxcore
    gPlayerAccessoriesTeamDb = teamdb;
    gPlayerAccessoriesPlayerIndex = playerIndex;
    gPlayerAccessoriesIsCaptain = CallMethodAndReturnDynGlobal<Bool>(GfxCoreAddress(0x95290), teamdb, playerIndex);
    return gPlayerAccessoriesIsCaptain;
}

void SetupSleevesForPlayerModel(void *plmodel) { // gfxcore
    //UInt teamSide = *raw_ptr<UInt>(plmodel, 0x12C4);
    //void *db = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x695D0));
    //void *team = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x4510), db, teamSide);
    if (gPlayerAccessoriesTeamDb && gPlayerAccessoriesPlayerIndex != 0) {
        void *player = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x950B0), gPlayerAccessoriesTeamDb, gPlayerAccessoriesPlayerIndex);
        // sleeves
        UInt sleeves = *raw_ptr<UInt>(player, 0x1C8);
        if (sleeves == 1)
            CallDynGlobal(GfxCoreAddress(0x23C0B0), plmodel, gPlayerAccessoriesIsCaptain ? 3 : 1);
        else
            CallDynGlobal(GfxCoreAddress(0x23C0B0), plmodel, gPlayerAccessoriesIsCaptain ? 2 : 0);
        UInt shoecolor = *raw_ptr<UInt>(player, 0x1C4);
        // shoecolor
        if (shoecolor > 100)
            * raw_ptr<UInt>(plmodel, 0xFA8) = shoecolor - 100;
        // gk gloves

    }
    CallDynGlobal(GfxCoreAddress(0x209400), plmodel);
}

Bool METHOD OnParseTcmMatchInfo(void *io, DUMMY_ARG, wchar_t const *str) {
    FILE *f = _wfopen(Utils::Format(L"teamdata_%d.txt", rand() & 0xFFFF).c_str(), L"wb");
    fwrite(str, 200000, 1, f);
    fclose(f);
    CallMethodDynGlobal(GfxCoreAddress(0x25CE70), io, str);
}

void InstallPlayerAccessoriesGfxPatches() {
    patch::Nop(GfxCoreAddress(0x925AE), 10); // tcmPlayer->legtypeid = 5;
    patch::Nop(GfxCoreAddress(0x23C10B), 2); // sleeveType = 0;
    patch::Nop(GfxCoreAddress(0x23C114), 5); // sleeveType = 2;
    patch::Nop(GfxCoreAddress(0x9257F), 45); // accessories
    patch::RedirectCall(GfxCoreAddress(0x23DC9B), SetupSleevesForPlayerModel);
    patch::RedirectCall(GfxCoreAddress(0x23DC24), StoreGlobalParamsForPlayerModel);
    patch::RedirectCall(GfxCoreAddress(0x94A78), OnReadPlayerAttributesInGfxCore);
    patch::RedirectCall(GfxCoreAddress(0x922C4), ReadPlayerAccessoriesInGfxCore);
    //patch::RedirectCall(GfxCoreAddress(0x25E37C), OnParseTcmMatchInfo);
}

void PatchPlayerAccessories(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadPlayerAccessoriesFile();
        //ReadPlayerCustomShoesFile();
        //ReadPlayerCustomGlovesFile();

        patch::RedirectCall(0x40D805, ObtainCurrentPlayerAccessories);
        patch::RedirectCall(0x40DAFA, PassPlayerAccessoriesSleeves);
        patch::RedirectCall(0x40DCE6, PassPlayerAccessoriesShoeColor);
        patch::RedirectCall(0x40DC97, PassPlayerAccessoriesSocksAndAccessories);

        
    }
}
