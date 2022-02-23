#include "ExtendedPlayer.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "FifamReadWrite.h"
#include "FifamCompID.h"

using namespace plugin;

const UInt DEF_PLAYER_SZ = 0x3B8;
const UShort PLAYER_EXT_VERSION = 0x1;

// V 1.0
// ------
// String jerseyName
// ------

struct PlayerExtension {
    WideChar *jerseyName;
    //struct ExtensionStats {
    //    UChar worldCupMatches;
    //    UChar worldCupGoals;
    //    UChar euroMatches;
    //    UChar euroGoals;
    //    UChar copaAmericaMatches;
    //    UChar copaAmericaGoals;
    //    UChar championsLeagueMatches;
    //    UChar championsLeagueGoals;
    //    UChar europaLeagueMatches;
    //    UChar europaLeagueGoals;
    //    UChar copaLibertadoresMatches;
    //    UChar copaLibertadoresGoals;
    //} extensionStats;
};

struct AllTimeScorer {

};

void *METHOD OnConstructPlayer(void *player, DUMMY_ARG, UInt playerId) {
    CallMethod<0xFBDD20>(player, playerId);
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    ext->jerseyName = nullptr;
    //ext->extensionStats.worldCupMatches = 0;
    //ext->extensionStats.worldCupGoals = 0;
    //ext->extensionStats.euroMatches = 0;
    //ext->extensionStats.euroGoals = 0;
    //ext->extensionStats.copaAmericaMatches = 0;
    //ext->extensionStats.copaAmericaGoals = 0;
    //ext->extensionStats.championsLeagueMatches = 0;
    //ext->extensionStats.championsLeagueGoals = 0;
    //ext->extensionStats.europaLeagueMatches = 0;
    //ext->extensionStats.europaLeagueGoals = 0;
    //ext->extensionStats.copaLibertadoresMatches = 0;
    //ext->extensionStats.copaLibertadoresGoals = 0;
    return player;
}

void *METHOD OnDestructPlayer(void *player) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    if (ext->jerseyName) {
        FmDelete(ext->jerseyName);
        ext->jerseyName = nullptr;
    }
    CallMethod<0xFB4060>(player);
    return player;
}

UInt gPlayerExtenderPlayerLoaderAddr[3] = {};

void SetPlayerJerseyName(void *player, wchar_t const *name) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    if (name) {
        UInt len = wcslen(name);
        if (len > 0) {
            if (ext->jerseyName) {
                if (len > wcslen(ext->jerseyName)) {
                    FmDelete(ext->jerseyName);
                    ext->jerseyName = nullptr;
                }
            }
            if (!ext->jerseyName)
                ext->jerseyName = (WideChar *)FmNew(len * 2 + 2);
            wcscpy(ext->jerseyName, name);
            return;
        }
    }
    if (ext->jerseyName) {
        FmDelete(ext->jerseyName);
        ext->jerseyName = nullptr;
    }
}

template<UInt id>
void METHOD OnReadPlayerFromMaster(void *player, DUMMY_ARG, void *reader) {
    CallMethodDynGlobal(gPlayerExtenderPlayerLoaderAddr[id], player, reader);
    if (BinaryReaderIsVersionGreaterOrEqual(reader, 0x2013, 0x0B)) {
        WideChar jerseyName[20];
        jerseyName[0] = 0;
        BinaryReaderReadString(reader, jerseyName, std::size(jerseyName));
        SetPlayerJerseyName(player, jerseyName);
    }
}

wchar_t const *GetPlayerJerseyName(void *player) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    if (ext->jerseyName)
        return ext->jerseyName;
    return L"";
}

void METHOD OnGetPlayerJerseyName1(void *player, DUMMY_ARG, WideChar *out, UInt maxLen) {
    wcsncpy(out, CallMethodAndReturn<WideChar const *, 0xFA2330>(player), maxLen);
    if (maxLen > 0)
        out[maxLen - 1] = 0;
}

WideChar const *METHOD OnGetPlayerJerseyName2(void *player) {
    WideChar const *jerseyName = GetPlayerJerseyName(player);
    if (jerseyName[0] != 0)
        return jerseyName;
    return CallMethodAndReturn<WideChar const *, 0xFA25B0>(player);
}

void METHOD OnReadPlayerFromSaveGame(void *player) {
    CallMethod<0xFC40B0>(player);
    void *loader = *(void **)0x3179DD8;
    static WideChar jerseyName[20];
    jerseyName[0] = 0;
    if (SaveGameLoadGetVersion(loader) >= 42)
        SaveGameReadString(loader, jerseyName, std::size(jerseyName));
    SetPlayerJerseyName(player, jerseyName);
    //if (SaveGameLoadGetVersion(loader) >= 43) {
    //    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    //    SaveGameReadInt8(loader, ext->extensionStats.worldCupMatches);
    //    SaveGameReadInt8(loader, ext->extensionStats.worldCupGoals);
    //    SaveGameReadInt8(loader, ext->extensionStats.euroMatches);
    //    SaveGameReadInt8(loader, ext->extensionStats.euroGoals);
    //    SaveGameReadInt8(loader, ext->extensionStats.copaAmericaMatches);
    //    SaveGameReadInt8(loader, ext->extensionStats.copaAmericaGoals);
    //    SaveGameReadInt8(loader, ext->extensionStats.championsLeagueMatches);
    //    SaveGameReadInt8(loader, ext->extensionStats.championsLeagueGoals);
    //    SaveGameReadInt8(loader, ext->extensionStats.europaLeagueMatches);
    //    SaveGameReadInt8(loader, ext->extensionStats.europaLeagueGoals);
    //    SaveGameReadInt8(loader, ext->extensionStats.copaLibertadoresMatches);
    //    SaveGameReadInt8(loader, ext->extensionStats.copaLibertadoresGoals);
    //}
}

void METHOD OnWritePlayerToSaveGame(void *player) {
    CallMethod<0xFC33F0>(player);
    void *writer = *(void **)0x3179DD4;
    SaveGameWriteString(writer, GetPlayerJerseyName(player));
    //PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    //SaveGameWriteInt8(writer, ext->extensionStats.worldCupMatches);
    //SaveGameWriteInt8(writer, ext->extensionStats.worldCupGoals);
    //SaveGameWriteInt8(writer, ext->extensionStats.euroMatches);
    //SaveGameWriteInt8(writer, ext->extensionStats.euroGoals);
    //SaveGameWriteInt8(writer, ext->extensionStats.copaAmericaMatches);
    //SaveGameWriteInt8(writer, ext->extensionStats.copaAmericaGoals);
    //SaveGameWriteInt8(writer, ext->extensionStats.championsLeagueMatches);
    //SaveGameWriteInt8(writer, ext->extensionStats.championsLeagueGoals);
    //SaveGameWriteInt8(writer, ext->extensionStats.europaLeagueMatches);
    //SaveGameWriteInt8(writer, ext->extensionStats.europaLeagueGoals);
    //SaveGameWriteInt8(writer, ext->extensionStats.copaLibertadoresMatches);
    //SaveGameWriteInt8(writer, ext->extensionStats.copaLibertadoresGoals);
}

void METHOD MyShowMsg23(void *t, DUMMY_ARG, const unsigned __int16 *a2, int a3, WideChar const *text) {
    Message(text);
    //CallMethod<0xD4A4C0>(t, a2, a3, text);
}

void OnReadPlayerStartingConditions(void *t) {
    Call<0xF87C70>(t);
    {
        FifamReader r(L"fmdata\\historic\\StatsAllTimeGoals_ChampionsLeague.txt");
        if (r.Available()) {
            r.SkipLine();
            while (!r.IsEof()) {
                if (!r.EmptyLine()) {
                    String playerId, firstName, lastName, pseudonym;
                    UChar nationality, apps, goals;
                    r.ReadLineWithSeparator(L'\t', playerId, firstName, lastName, pseudonym, nationality, apps, goals);
                    if (!playerId.empty()) {
                        CDBPlayer *player = FindPlayerByStringID(playerId.c_str());
                        if (player) {
                            PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
                            //ext->extensionStats.championsLeagueMatches = apps;
                            //ext->extensionStats.championsLeagueGoals = goals;
                        }
                    }
                }
                else
                    r.SkipLine();
            }
        }
    }
}

void METHOD MyStatsScorerAllTime__FillCompetitionComboBox(void *t, DUMMY_ARG, void *comboBox, void *a, UChar countryId) {
    //::Error(L"%d", countryId);
    if (countryId)
        CallMethod<0x77B510>(t, comboBox, a, countryId);
    else {
        CallVirtualMethod<9>(comboBox, 1);
        CallVirtualMethod<9>(a, 1);
        CallVirtualMethod<81>(comboBox);
        auto comp = GetCompetition(FifamCompRegion::International, COMP_WORLD_CUP, 0);
        if (comp)
            CallVirtualMethod<83>(comboBox, comp->GetName(), comp->GetCompID().ToInt(), 0);
        comp = GetCompetition(FifamCompRegion::Europe, COMP_CHAMPIONSLEAGUE, 0);
        if (comp)
            CallVirtualMethod<83>(comboBox, comp->GetName(), comp->GetCompID().ToInt(), 0);
    }
}

void METHOD MyStatsScorerAllTime__FillList(void *t) {
    CCompID &compId = *raw_ptr<CCompID>(t, 0x12DC);
    //::Error(compId.ToStr());
    if (compId.ToInt() == 0xF9090000) {

    }
    else
        CallMethod<0x77B670>(t);
}

void PatchExtendedPlayer(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0xFC6646, OnConstructPlayer);
        patch::RedirectCall(0xFBE0D3, OnDestructPlayer);
        // read from master.dat to extension
        gPlayerExtenderPlayerLoaderAddr[0] = patch::RedirectCall(0xF33A0F, OnReadPlayerFromMaster<0>);
        gPlayerExtenderPlayerLoaderAddr[1] = patch::RedirectCall(0xF87E8A, OnReadPlayerFromMaster<1>);
        gPlayerExtenderPlayerLoaderAddr[2] = patch::RedirectCall(0xF975DF, OnReadPlayerFromMaster<2>);
        // write to savegame
        patch::RedirectCall(0xFC6746, OnWritePlayerToSaveGame);
        // load from savegame
        patch::RedirectCall(0xFC67DD, OnReadPlayerFromSaveGame);
        // get from extension
        patch::SetUChar(0x413D16, 0xEB);
        patch::RedirectCall(0x413D07, OnGetPlayerJerseyName1);
        patch::RedirectCall(0x413D52, OnGetPlayerJerseyName2);
        const UInt NEW_PLAYER_SZ = DEF_PLAYER_SZ + sizeof(PlayerExtension);
        patch::SetUInt(0xFBDD54 + 1, NEW_PLAYER_SZ);
        patch::SetUInt(0xFC6622 + 1, NEW_PLAYER_SZ);
        //patch::RedirectCall(0xF9765E, OnReadPlayerStartingConditions);

        // stats CL
        //patch::SetPointer(0x23FB214, (void *)0x7951F0);
        //patch::SetPointer(0x23FB218, MyStatsScorerAllTime__FillCompetitionComboBox);
        //patch::SetPointer(0x23FB220, MyStatsScorerAllTime__FillList);
    }
}
