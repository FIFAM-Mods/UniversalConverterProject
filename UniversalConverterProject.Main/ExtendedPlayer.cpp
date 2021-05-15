#include "ExtendedPlayer.h"
#include "FifamTypes.h"

using namespace plugin;

void *FmNew(UInt size) {
    return CallAndReturn<void *, 0x15738F3>(size);
}

void FmDelete(void *data) {
    Call<0x157347A>(data);
}

bool BinaryReaderIsVersionGreaterOrEqual(void *reader, UInt year, UInt build) {
    return CallMethodAndReturn<bool, 0x1338EA0>(reader, year, build);
}

void BinaryReaderReadString(void *reader, WideChar *out, UInt maxLen) {
    CallMethod<0x1338700>(reader, out, maxLen);
}

void SaveGameReadString(void *save, WideChar *out, UInt maxLen) {
    CallMethod<0x1080EB0>(save, out, maxLen);
}

void SaveGameWriteString(void *save, WideChar const *str) {
    CallMethod<0x1080130>(save, str);
}

UInt SaveGameLoadGetVersion(void *save) {
    return CallMethodAndReturn<UInt, 0x107F730>(save);
}

const UInt DEF_PLAYER_SZ = 0x3B8;
const UShort PLAYER_EXT_VERSION = 0x1;

// V 1.0
// ------
// String jerseyName
// ------

struct PlayerExtension {
    WideChar *jerseyName;
};

void *METHOD OnConstructPlayer(void *player, DUMMY_ARG, UInt playerId) {
    CallMethod<0xFBDD20>(player, playerId);
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    ext->jerseyName = nullptr;
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

void METHOD OnReadPlayerJerseyNameFromSaveGame(void *player) {
    CallMethod<0xFC40B0>(player);
    void *loader = *(void **)0x3179DD8;
    static WideChar jerseyName[20];
    jerseyName[0] = 0;
    if (SaveGameLoadGetVersion(loader) >= 42)
        SaveGameReadString(loader, jerseyName, std::size(jerseyName));
    SetPlayerJerseyName(player, jerseyName);
}

void METHOD OnWritePlayerJerseyNameToSaveGame(void *player) {
    CallMethod<0xFC33F0>(player);
    void *writer = *(void **)0x3179DD4;
    SaveGameWriteString(writer, GetPlayerJerseyName(player));
}

void METHOD MyShowMsg23(void *t, DUMMY_ARG, const unsigned __int16 *a2, int a3, WideChar const *text) {
    Message(text);
    //CallMethod<0xD4A4C0>(t, a2, a3, text);
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
        patch::RedirectCall(0xFC6746, OnWritePlayerJerseyNameToSaveGame);
        // load from savegame
        patch::RedirectCall(0xFC67DD, OnReadPlayerJerseyNameFromSaveGame);
        // get from extension
        patch::SetUChar(0x413D16, 0xEB);
        patch::RedirectCall(0x413D07, OnGetPlayerJerseyName1);
        patch::RedirectCall(0x413D52, OnGetPlayerJerseyName2);
        const UInt NEW_PLAYER_SZ = DEF_PLAYER_SZ + sizeof(PlayerExtension);
        patch::SetUInt(0xFBDD54 + 1, NEW_PLAYER_SZ);
        patch::SetUInt(0xFC6622 + 1, NEW_PLAYER_SZ);
    }
}
