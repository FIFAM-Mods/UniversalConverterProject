#include "ExtendedPlayerEditor.h"

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
    //ext->isFemale = false;
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
    //if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0x11)) {
    //    UInt isFemale = 0;
    //    CallMethod<0x513560>(t, &isFemale);
    //    ext->isFemale = isFemale;
    //}
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xE))
        CallMethod<0x513560>(t, &ext->creator);
    if (CallMethodAndReturn<Bool, 0x511C70>(t, 0x2013, 0xC))
        CallMethod<0x513560>(t, &ext->footballManagerId);
    return CallMethodAndReturn<Bool, 0x512190>(t, name);
}

Bool METHOD OnWriteStaffFoomIdToDatabase(void *t, DUMMY_ARG, WideChar const *name) {
    PlayerExtension *ext = raw_ptr<PlayerExtension>(gDatabaseStaff, DEF_PLAYER_SZ);
    //UInt isFemale = ext->isFemale;
    //CallMethod<0x514510>(t, &isFemale);
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
    //if (CallMethodAndReturn<Bool, 0x511C70>(reader, 0x2013, 0x11)) {
    //    UInt isFemale = 0;
    //    CallMethod<0x513560>(reader, &isFemale);
    //    ext->isFemale = isFemale;
    //}
    if (CallMethodAndReturn<Bool, 0x511C70>(reader, 0x2013, 0xE))
        CallMethod<0x513560>(reader, &ext->creator);
    if (CallMethodAndReturn<Bool, 0x511C70>(reader, 0x2013, 0xC))
        CallMethod<0x513560>(reader, &ext->footballManagerId);
}

void METHOD OnWriteStaffNoHeader(void **ps, DUMMY_ARG, void *writer) {
    CallMethod<0x5736F0>(ps, writer);
    PlayerExtension *ext = raw_ptr<PlayerExtension>(*ps, DEF_PLAYER_SZ);
    //UInt isFemale = ext->isFemale;
    //CallMethod<0x514510>(writer, &isFemale);
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

// UPDATE
Array<Float, 207> FifaRanking2024 = {
1379.40f,996.05f,1230.79f,1560.03f,1179.88f,1219.78f,1797.98f,1332.30f,1295.50f,1728.30f,1136.78f,1506.23f,1610.76f,1787.88f,1146.47f,1093.70f,1393.40f,1837.47f,1348.63f,1338.91f,1646.78f,1455.95f,1529.00f,1353.48f,1403.84f,1312.54f,1724.37f,1095.98f,835.83f,1100.66f,1276.73f,970.62f,1033.75f,1746.66f,1341.33f,1466.48f,1541.49f,1747.04f,1462.35f,1506.58f,739.64f,1497.18f,1467.77f,1429.74f,1729.92f,1522.19f,1617.24f,1493.38f,1565.37f,1521.40f,1514.76f,1860.14f,1282.43f,1791.85f,1496.13f,1669.44f,1517.54f,1427.11f,1515.01f,1663.44f,1442.29f,1008.92f,1052.80f,873.90f,823.58f,932.64f,901.43f,971.15f,801.29f,1461.74f,865.34f,1454.20f,975.01f,912.93f,1045.64f,1303.65f,953.67f,1199.21f,1026.61f,1274.46f,1313.05f,1442.82f,1652.33f,934.46f,1272.71f,1119.88f,1482.10f,1020.22f,1059.53f,979.57f,938.40f,1087.77f,1219.44f,817.03f,1676.52f,797.29f,1474.13f,1248.94f,1254.18f,1064.19f,1375.16f,1091.24f,1458.47f,1380.53f,1129.30f,929.97f,1176.45f,1499.69f,875.13f,1400.93f,1502.34f,1260.50f,0.00f,1066.16f,1288.45f,1112.55f,1381.25f,1324.80f,1168.49f,1195.45f,1050.83f,1067.05f,1165.73f,1203.66f,1137.83f,1456.74f,1177.50f,927.94f,1669.44f,1205.82f,1219.83f,1125.50f,1498.93f,1114.15f,878.09f,1623.34f,831.96f,1149.40f,829.81f,1416.66f,1152.29f,1025.18f,1174.99f,1158.70f,1494.06f,1242.96f,1258.40f,1122.54f,1034.37f,1302.86f,896.67f,904.10f,881.73f,924.52f,1267.51f,991.34f,821.91f,1011.91f,1139.39f,1108.73f,1611.16f,1433.07f,1628.81f,1374.13f,1193.62f,1183.96f,1572.87f,1098.42f,1213.58f,889.62f,1167.64f,896.62f,1107.58f,1003.48f,884.92f,988.98f,935.93f,1326.18f,842.59f,1231.25f,1053.03f,1504.06f,1431.30f,1008.26f,820.32f,1246.68f,1212.41f,1218.56f,1065.42f,1368.84f,1397.41f,1168.02f,1021.24f,890.97f,1571.29f,896.59f,983.81f,1197.68f,985.32f,920.54f,1111.02f,999.48f,833.12f,972.14f,836.14f,1345.02f,1203.16f
};

void METHOD OnReadFifaRanking(void *reader, DUMMY_ARG, void *country) {
	if (CallMethodAndReturn<Bool, 0x511C70>(reader, 0x2013, 0xF))
	    CallMethod<0x5124B0>(reader, raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET));
	else {
		UShort value = 0;
		CallMethod<0x513420>(reader, &value);
        if (value == 0) {
            UInt countryIndex = *raw_ptr<UInt>(country, 0x1A4);
            if (countryIndex >= 1 && countryIndex <= 207)
                *raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET) = FifaRanking2024[countryIndex - 1];
        }
        else
		    *raw_ptr<Float>(country, COUNTRY_FIFARANKING_OFFSET) = (Float)value;
	}
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
