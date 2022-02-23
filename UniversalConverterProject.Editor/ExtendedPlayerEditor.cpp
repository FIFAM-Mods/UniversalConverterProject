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
const UShort PLAYER_EXT_VERSION = 0x2;
const UInt DEF_CLUB_SZ = 0x391C;
const UShort CLUB_EXT_VERSION = 0x1;

PlayerExtension *GetPlayerExtension(void *player) {
    return raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
}

ClubExtension *GetClubExtension(void *club) {
    return raw_ptr<ClubExtension>(club, DEF_CLUB_SZ);
}

// V 1.0
// ------
// String jerseyName
// ------

void *METHOD OnConstructPlayer(void *player, DUMMY_ARG, void *world) {
    CallMethod<0x528B30>(player, world);
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    ext->jerseyName = nullptr;
    ext->footballManagerId = -1;
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

void *gDatabasePlayer = nullptr;

Bool METHOD OnReadPlayerFoomIdFromDatabase(void *t, DUMMY_ARG, Char const *name) {
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xC)) {
        PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabasePlayer, DEF_PLAYER_SZ);
        CallMethod<0x513560>(t, &ext->footballManagerId);
    }
    return CallMethodAndReturn<Bool, 0x512190>(t, name);
}

Bool METHOD OnWritePlayerFoomIdToDatabase(void *t, DUMMY_ARG, WideChar const *name) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabasePlayer, DEF_PLAYER_SZ);
    CallMethod<0x514510>(t, &ext->footballManagerId);
    return CallMethodAndReturn<Bool, 0x513290>(t, name);
}

void METHOD OnReadPlayer(void *t, DUMMY_ARG, void *reader, const WideChar *clubName) {
    gDatabasePlayer = t;
    CallMethod<0x5269E0>(t, reader, clubName);
    gDatabasePlayer = nullptr;
}

void METHOD OnWritePlayer(void *t, DUMMY_ARG, void *writer) {
    gDatabasePlayer = t;
    CallMethod<0x51F770>(t, writer);
    gDatabasePlayer = nullptr;
}

void *gDatabaseStaff = nullptr;

Bool METHOD OnReadStaffFoomIdFromDatabase(void *t, DUMMY_ARG, Char const *name) {
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xC)) {
        PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabaseStaff, DEF_PLAYER_SZ);
        CallMethod<0x513560>(t, &ext->footballManagerId);
    }
    return CallMethodAndReturn<Bool, 0x512190>(t, name);
}

Bool METHOD OnWriteStaffFoomIdToDatabase(void *t, DUMMY_ARG, WideChar const *name) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabaseStaff, DEF_PLAYER_SZ);
    CallMethod<0x514510>(t, &ext->footballManagerId);
    return CallMethodAndReturn<Bool, 0x513290>(t, name);
}

void METHOD OnReadStaff(void *t, DUMMY_ARG, void *reader) {
    gDatabaseStaff = t;
    CallMethod<0x51FCA0>(t, reader);
    gDatabaseStaff = nullptr;
}

void METHOD OnWriteStaff(void *t, DUMMY_ARG, void *writer) {
    gDatabaseStaff = t;
    CallMethod<0x51B6C0>(t, writer);
    gDatabaseStaff = nullptr;
}

void METHOD OnReadStaffNoHeader(void **ps, DUMMY_ARG, void *reader) {
    CallMethod<0x5738E0>(ps, reader);
    if (CallMethodAndReturn<Bool, 0x511C70>(reader, 0x2013, 0xC)) {
        PlayerExtension *ext = raw_ptr<PlayerExtension>(*ps, DEF_PLAYER_SZ);
        CallMethod<0x513560>(reader, &ext->footballManagerId);
    }
}

void METHOD OnWriteStaffNoHeader(void **ps, DUMMY_ARG, void *writer) {
    CallMethod<0x5736F0>(ps, writer);
    PlayerExtension *ext = raw_ptr<PlayerExtension>(*ps, DEF_PLAYER_SZ);
    CallMethod<0x514510>(writer, &ext->footballManagerId);
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
        patch::RedirectCall(0x5276AB, OnReadPlayerFoomIdFromDatabase);
        patch::RedirectCall(0x51FC87, OnWritePlayerFoomIdToDatabase);
        patch::RedirectCall(0x4C591D, OnReadPlayer);
        patch::RedirectCall(0x53B62B, OnReadPlayer);
        patch::RedirectCall(0x4C60B5, OnWritePlayer);
        patch::RedirectCall(0x53AED1, OnWritePlayer);
        patch::RedirectCall(0x51FEF3, OnReadStaffFoomIdFromDatabase);
        patch::RedirectCall(0x51B85D, OnWriteStaffFoomIdToDatabase);
        patch::RedirectCall(0x4C598E, OnReadStaff);
        patch::RedirectCall(0x4E9E58, OnReadStaff);
        patch::RedirectCall(0x4C6136, OnWriteStaff);
        patch::RedirectCall(0x4E53A4, OnWriteStaff);
        patch::RedirectCall(0x4E9DBF, OnReadStaffNoHeader);
        patch::RedirectCall(0x4E52F0, OnWriteStaffNoHeader);
    }
}
