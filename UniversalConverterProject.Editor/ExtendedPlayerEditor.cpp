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

void BinaryFileWriteInt(void *binaryFile, UInt value) {
    CallMethod<0x551060>(binaryFile, value);
}

bool ReaderIsVersionGreaterOrEqual(void *reader, UInt year, UShort build) {
    return CallMethodAndReturn<bool, 0x511C70>(reader, year, build);
}

const UInt DEF_PLAYER_SZ = 0x2C0;
const UInt DEF_CLUB_SZ = 0x391C;
const UInt DEF_REFEREE_SZ = 0x4A;
const UInt COUNTRY_FIFARANKING_OFFSET = 0x1B0C;

PlayerExtension *GetPlayerExtension(void *player) {
    return raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
}

ClubExtension *GetClubExtension(void *club) {
    return raw_ptr<ClubExtension>(club, DEF_CLUB_SZ);
}

void *METHOD OnConstructPlayer(void *player, DUMMY_ARG, void *world) {
    CallMethod<0x528B30>(player, world);
    PlayerExtension *ext = raw_ptr<PlayerExtension>(player, DEF_PLAYER_SZ);
    ext->jerseyName = nullptr;
    ext->creator = 0;
    ext->fifaId = 0;
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
    BinaryFileWriteInt(binaryFile, ext->fifaId);
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
    PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabasePlayer, DEF_PLAYER_SZ);
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xE)) {
        CallMethod<0x513560>(t, &ext->creator);
        CallMethod<0x513560>(t, &ext->fifaId);
    }
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xC))
        CallMethod<0x513560>(t, &ext->footballManagerId);
    return CallMethodAndReturn<Bool, 0x512190>(t, name);
}

Bool METHOD OnWritePlayerFoomIdToDatabase(void *t, DUMMY_ARG, WideChar const *name) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabasePlayer, DEF_PLAYER_SZ);
    CallMethod<0x514510>(t, &ext->creator);
    CallMethod<0x514510>(t, &ext->fifaId);
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
    PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabaseStaff, DEF_PLAYER_SZ);
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xE))
        CallMethod<0x513560>(t, &ext->creator);
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xC))
        CallMethod<0x513560>(t, &ext->footballManagerId);
    return CallMethodAndReturn<Bool, 0x512190>(t, name);
}

Bool METHOD OnWriteStaffFoomIdToDatabase(void *t, DUMMY_ARG, WideChar const *name) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabaseStaff, DEF_PLAYER_SZ);
    CallMethod<0x514510>(t, &ext->creator);
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
    PlayerExtension *ext = raw_ptr<PlayerExtension>(*ps, DEF_PLAYER_SZ);
    if (CallMethodAndReturn<Bool, 0x511C70>(reader, 0x2013, 0xE))
        CallMethod<0x513560>(reader, &ext->creator);
    if (CallMethodAndReturn<Bool, 0x511C70>(reader, 0x2013, 0xC))
        CallMethod<0x513560>(reader, &ext->footballManagerId);
}

void METHOD OnWriteStaffNoHeader(void **ps, DUMMY_ARG, void *writer) {
    CallMethod<0x5736F0>(ps, writer);
    PlayerExtension *ext = raw_ptr<PlayerExtension>(*ps, DEF_PLAYER_SZ);
    CallMethod<0x514510>(writer, &ext->creator);
    CallMethod<0x514510>(writer, &ext->footballManagerId);
}

void *OnAllocReferee(UInt size) {
    void *result = CallAndReturn<void *, 0x5B0474>(DEF_REFEREE_SZ + sizeof(RefereeExtension));
    if (result) {
        RefereeExtension *ext = raw_ptr<RefereeExtension>(result, DEF_REFEREE_SZ);
        ext->creator = 0;
        ext->footballManagerId = -1;
    }
    return result;
}

void METHOD OnReadRefereeType(void *t, DUMMY_ARG, UChar *out) {
    CallMethod<0x5133A0>(t, out);
    RefereeExtension *ext = raw_ptr<RefereeExtension>(out, 2);
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xE))
        CallMethod<0x513560>(t, &ext->creator);
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xC))
        CallMethod<0x513560>(t, &ext->footballManagerId);
}

void METHOD OnWriteRefereeType(void *t, DUMMY_ARG, UChar *in) {
    CallMethod<0x514630>(t, in);
    RefereeExtension *ext = raw_ptr<RefereeExtension>(in, 2);
    CallMethod<0x514510>(t, &ext->creator);
    CallMethod<0x514510>(t, &ext->footballManagerId);
}

UInt METHOD GetCountryFifaRanking(void *country) {
	Float *pFifaRanking = raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET);
	return *(UInt *)pFifaRanking;
}

void METHOD SetCountryFifaRanking(void *country, DUMMY_ARG, UInt value) {
	*raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET) = *(Float *)(&value);
}

void METHOD FifaRankingSetMinMax(void *t, DUMMY_ARG, WPARAM wParam, LPARAM lParam) {
	CallMethod<0x585050>(t, 0.0f, 5000.0f, 2);
}

void METHOD FifaRankingSetValue(void *t, DUMMY_ARG, UInt value) {
	CallMethod<0x584FB0>(t, *(Float *)(&value));
}

UInt METHOD FifaRankingGetValue(void *t) {
	Float value = CallMethodAndReturn<Float, 0x585300>(t);
	return *(UInt *)(&value);
}

void METHOD OnWriteFifaRanking(void *writer, DUMMY_ARG, void *country) {
	CallMethod<0x5138B0>(writer, raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET));
}

Array<Float, 207> FifaRanking2023 = {
1357.39f,1022.3f,1252.6f,1528.06f,1143.21f,1212.28f,1788.55f,1381.1f,1315.48f,1742.55f,1163.73f,1503.69f,1597.37f,1797.39f,1179.3f,1126.3f,1419.47f,1843.53f,1350.56f,1312.8f,1635.24f,1441.06f,1504.57f,1352.98f,1426.26f,1323.8f,1726.58f,1096.19f,848.82f,1069.98f,1262.72f,972.87f,990.73f,1731.23f,1361.17f,1458.48f,1536.98f,1718.25f,1443.98f,1495.53f,753.11f,1520.25f,1447.05f,1391.04f,1703.45f,1547.12f,1661.13f,1484.46f,1542.32f,1506.03f,1539.04f,1843.73f,1295.09f,1828.27f,1511.31f,1624.9f,1486.47f,1442.64f,1561.2f,1633.13f,1417.24f,995.58f,1107.51f,850.88f,852.87f,984.05f,939.96f,966.27f,804.11f,1454.15f,859.83f,1458.21f,973.21f,904.88f,1036.73f,1317.64f,960.77f,1178.21f,981.69f,1297.13f,1289.02f,1380.26f,1646.62f,938.02f,1265.43f,1074.98f,1420.46f,996.25f,1078.06f,978.91f,938.28f,1073.63f,1213.87f,839.39f,1664.19f,816.59f,1511.15f,1170.76f,1248.12f,1052.38f,1419.18f,1085.06f,1470.97f,1354.65f,1133.5f,930.22f,1190.63f,1433.37f,885.39f,1354.23f,1509.88f,1251.83f,855.56f,1074.47f,1285.34f,1159.82f,1391.13f,1296.75f,1178.93f,1194.9f,1030.99f,1049.94f,1130.75f,1186.09f,1133.36f,1438.01f,1205.18f,936.03f,1655.5f,1165.66f,1175.14f,1129.67f,1486.48f,1089.46f,900.07f,1612.61f,860.13f,1156.11f,854.72f,1369.5f,1122.72f,1058.92f,1138.79f,1144.98f,1516.66f,1246.17f,1290.19f,1138.56f,1020.31f,1282.05f,894.03f,900.64f,891.12f,942.97f,1304.78f,1028.18f,838.33f,1049.73f,1211.67f,1047.46f,1559.53f,1345.22f,1595.96f,1296.3f,1198.25f,1169.96f,1529.29f,1088.28f,1220.82f,899.58f,1200.85f,913.67f,1091.58f,1021.83f,908.71f,1000.26f,958.08f,1336.43f,847.67f,1233.02f,1095.65f,1365.6f,1421.46f,1014.78f,825.25f,1241.62f,1179.54f,1174.37f,1089.78f,1336.28f,1324.64f,1238.22f,1020.37f,900.27f,1530.44f,899.33f,980.48f,1205.82f,1003.28f,894.26f,1097.6f,995.11f,861.81f,986.44f,851.63f,1343.45f,1179.69f
};

void METHOD OnReadFifaRanking(void *reader, DUMMY_ARG, void *country) {
	if (CallMethodAndReturn<Bool, 0x511C70>(reader, 0x2013, 0xF))
	    CallMethod<0x5124B0>(reader, raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET));
	else {
		UShort value = 0;
		CallMethod<0x513420>(reader, &value);
		*raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET) = (Float)value;
	}
	// TODO: REMOVE THIS!
	UInt countryIndex = *raw_ptr<UInt>(country, 0x1A4);
	*raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET) = FifaRanking2023[countryIndex - 1];
}

void METHOD OnWriteFifaRankingToMaster(void *binaryFile, DUMMY_ARG, void *country) {
	CallMethod<0x551160>(binaryFile, *raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET));
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

        // foom id - referee
        patch::RedirectCall(0x492126, OnAllocReferee);
        patch::RedirectCall(0x49218E, OnAllocReferee);
        patch::RedirectCall(0x4E9842, OnAllocReferee);
        patch::RedirectCall(0x53FDF5, OnReadRefereeType);
        patch::RedirectCall(0x53FDA1, OnWriteRefereeType);

		// CCountry extension
		patch::SetUInt(0x4EA26B + 1, 196 - 4);
		patch::SetUInt(0x4EA27C + 1, 196 - 4);
		patch::SetUInt(0x4DED12 + 1, 195 - 4);
		patch::RedirectJump(0x4DEE40, GetCountryFifaRanking);
		patch::RedirectJump(0x4DEE50, SetCountryFifaRanking);
		patch::RedirectCall(0x446DDE, FifaRankingSetMinMax);
		patch::RedirectCall(0x4473DA, FifaRankingSetValue);
		patch::RedirectCall(0x4471A5, FifaRankingGetValue);
		patch::Nop(0x4471B0, 1); // cwde
		patch::SetUChar(0x4E57D3, 0x56); // push esi
		patch::RedirectCall(0x4E57D6, OnWriteFifaRanking);
		patch::SetUChar(0x4EA22B, 0x56); // push esi
		patch::RedirectCall(0x4EA22E, OnReadFifaRanking);
		patch::SetUChar(0x4E850D, 0x56); // push esi
		patch::RedirectCall(0x4E8510, OnWriteFifaRankingToMaster);
    }
}
