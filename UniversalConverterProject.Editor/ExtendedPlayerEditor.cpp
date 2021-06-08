#include "ExtendedPlayerEditor.h"
#include "FifamTypes.h"

using namespace plugin;

void *FmNew(UInt size) {
    return CallAndReturn<void *, 0x5B0474>(size);
}

void FmDelete(void *data) {
    Call<0x5B04A3>(data);
}

void BinaryFileWriteString(void *binaryFile, WideChar const *str) {
    CallMethod<0x550D10>(binaryFile, str);
}

bool ReaderIsVersionGreaterOrEqual(void *reader, UInt year, UShort build) {
    return CallMethodAndReturn<bool, 0x511C70>(reader, year, build);
}

const UInt DEF_PLAYER_SZ = 0x2C0;
const UShort PLAYER_EXT_VERSION = 0x1;

// V 1.0
// ------
// String jerseyName
// ------

struct PlayerExtension {
    WideChar *jerseyName;
};

void *METHOD OnConstructPlayer(void *player, DUMMY_ARG, void *world) {
    CallMethod<0x528B30>(player, world);
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
    CallMethod<0x5286E0>(player);
    return player;
}

void METHOD OnWritePlayerToMaster(void *binaryFile, DUMMY_ARG, void *) {
    void *player = *raw_ptr<void *>(binaryFile, 0x2C);
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    if (ext->jerseyName)
        BinaryFileWriteString(binaryFile, ext->jerseyName);
    else
        BinaryFileWriteString(binaryFile, L"");
    CallMethod<0x550640>(binaryFile, 0);
}

void SetPlayerJerseyName(void *player, WideChar const *name) {
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
            Call<0x578010>(ext->jerseyName, L'|', L'-');
            return;
        }
    }
    if (ext->jerseyName) {
        FmDelete(ext->jerseyName);
        ext->jerseyName = nullptr;
    }
}

void METHOD OnSetPlayerJerseyName(void *player, DUMMY_ARG, WideChar const *name) {
    SetPlayerJerseyName(player, name);
}

WideChar const *GetPlayerJerseyName(void *player) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    if (ext->jerseyName)
        return ext->jerseyName;
    return L"";
}

WideChar const *METHOD OnGetPlayerJerseyName(void *player) {
    return GetPlayerJerseyName(player);
}

UChar METHOD OnReadPlayerJerseyNameFromDatabase(void *file, DUMMY_ARG, WideChar *out, UInt maxSize, WideChar delim) {
    static WideChar jerseyName[20];
    UChar result = CallMethodAndReturn<UChar, 0x512400>(file, jerseyName, std::size(jerseyName), delim);
    if (ReaderIsVersionGreaterOrEqual(file, 0x2013, 0xB) && result)
        SetPlayerJerseyName((void *)(UInt(out) - 0x58), jerseyName);
    return result;
}

UChar METHOD OnWritePlayerJerseyNameToDatabase(void *file, DUMMY_ARG, WideChar const *name, WideChar delim) {
    return CallMethodAndReturn<UChar, 0x513820>(file, GetPlayerJerseyName((void *)(UInt(name) - 0x58)), delim);
}

void PatchExtendedPlayer(FM::Version v) {
    if (v.id() == VERSION_ED_13) {
        patch::RedirectCall(0x48953C, OnConstructPlayer);
        patch::RedirectCall(0x4D54B9, OnConstructPlayer);
        patch::RedirectCall(0x488113, OnDestructPlayer);
        patch::RedirectCall(0x488DFF, OnDestructPlayer);
        patch::RedirectCall(0x4D2051, OnDestructPlayer);
        patch::RedirectCall(0x4D74DB, OnDestructPlayer);
        patch::RedirectCall(0x4D7538, OnDestructPlayer);
        patch::RedirectCall(0x521F34, OnWritePlayerToMaster); // write extension to master.dat
        patch::RedirectCall(0x5261B7, OnWritePlayerToMaster); // write extension to master.dat
        patch::RedirectCall(0x526A5A, OnReadPlayerJerseyNameFromDatabase); // read from .sav to extension
        patch::RedirectCall(0x51F7CF, OnWritePlayerJerseyNameToDatabase); // write to .sav from extension
        patch::RedirectCall(0x46D28C, OnSetPlayerJerseyName); // store from TextBox to CPlayer
        patch::RedirectCall(0x4F8D12, OnGetPlayerJerseyName); // database problem tool check
        patch::RedirectCall(0x46E01C, OnGetPlayerJerseyName); // store to TextBox from CPlayer
        const UInt NEW_PLAYER_SZ = DEF_PLAYER_SZ + sizeof(PlayerExtension);
        patch::SetUInt(0x489518 + 1, NEW_PLAYER_SZ);
        patch::SetUInt(0x4D549B + 1, NEW_PLAYER_SZ);
    }
}
