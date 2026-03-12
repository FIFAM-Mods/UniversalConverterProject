#include "CitiesAndRegions.h"
#include "GameInterfaces.h"
#include "TranslationShared.h"
#include "Translation.h"
#include "ExtendedTeam.h"
#include "FifamClubTeamType.h"
#include "ExtendedPlayer.h"
#include "Utils.h"

Map<Int, DBCity> &DBCities() {
    static Map<Int, DBCity> dbCities;
    return dbCities;
}

Map<Int, DBRegion> &DBRegions() {
    static Map<Int, DBRegion> dbRegions;
    return dbRegions;
}

DBCity *GetCity(Int id) {
    if (id != -1 && Utils::Contains(DBCities(), id))
        return &DBCities()[id];
    return nullptr;
}

DBRegion *GetRegion(Int id) {
    if (id != -1 && Utils::Contains(DBRegions(), id))
        return &DBRegions()[id];
    return nullptr;
}

Bool IsCatalanCity(Int cityID) {
    if (cityID != -1) {
        auto it = DBCities().find(cityID);
        if (it != DBCities().end()) {
            Int regionID = (*it).second.regionId;
            return regionID == 67023595;
        }
    }
    return false;
}

Bool IsTeamCatalan(CDBTeam *team) {
    return IsCatalanCity(GetTeamExtension(team)->cityId);
}

void ClearCities() {
    for (auto const &[id, city] : DBCities())
        delete[] city.name;
    DBCities().clear();
}

void ClearRegions() {
    for (auto const &[id, region] : DBRegions())
        delete[] region.name;
    DBRegions().clear();
}

void ClearCitiesAndRegions() {
    ClearCities();
    ClearRegions();
}

Bool OnReadAppearanceDefsFromBinaryDatabase(void *reader) {
    Bool result = CallMethodAndReturn<Bool, 0x1382F10>(reader);
    // binary database version 20130012
    ClearCitiesAndRegions();
    if (BinaryReaderIsVersionGreaterOrEqual(reader, 0x2013, 0x12) && BinaryReaderCheckFourcc(reader, 'CTRG')) {
        UInt cityCount = 0;
        BinaryReaderReadUInt32(reader, &cityCount);
        for (UInt c = 0; c < cityCount; c++) {
            Int cityId = -1;
            BinaryReaderReadInt32(reader, &cityId);
            DBCity &city = DBCities()[cityId];
            city.id = cityId;
            BinaryReaderReadUInt8(reader, &city.countryId);
            BinaryReaderReadUInt8(reader, &city.population);
            BinaryReaderReadFloat(reader, &city.latitude);
            BinaryReaderReadFloat(reader, &city.longitude);
            BinaryReaderReadInt32(reader, &city.regionId);
            delete[] city.name;
            city.name = nullptr;
            for (UInt i = 0; i < NUM_TRANSLATION_LANGUAGES; i++) {
                WideChar cityName[256];
                BinaryReaderReadString(reader, cityName, std::size(cityName));
                if (i == CurrentLanguageId) {
                    city.name = new WideChar[wcslen(cityName) + 1];
                    wcscpy(city.name, cityName);
                }
            }
        }
        UInt regionCount = 0;
        BinaryReaderReadUInt32(reader, &regionCount);
        for (UInt r = 0; r < regionCount; r++) {
            Int regionId = -1;
            BinaryReaderReadInt32(reader, &regionId);
            DBRegion &region = DBRegions()[regionId];
            region.id = regionId;
            BinaryReaderReadUInt8(reader, &region.countryId);
            BinaryReaderReadFloat(reader, &region.latitude);
            BinaryReaderReadFloat(reader, &region.longitude);
            delete[] region.name;
            region.name = nullptr;
            for (UInt i = 0; i < NUM_TRANSLATION_LANGUAGES; i++) {
                WideChar regionName[256];
                BinaryReaderReadString(reader, regionName, std::size(regionName));
                if (i == CurrentLanguageId) {
                    region.name = new WideChar[wcslen(regionName) + 1];
                    wcscpy(region.name, regionName);
                }
            }
        }
        BinaryReaderCheckFourcc(reader, 'CTRG');
    }
    return result;
}

void OnLoadCompetitions() {
    void *save = *(void **)0x3179DD8;
    ClearCitiesAndRegions();
    if (SaveGameLoadGetVersion(save) >= 49) {
        UInt cityCount = 0;
        SaveGameReadUInt32(save, cityCount);
        for (UInt c = 0; c < cityCount; c++) {
            Int cityId = -1;
            SaveGameReadInt32(save, cityId);
            DBCity &city = DBCities()[cityId];
            city.id = cityId;
            SaveGameReadUInt8(save, city.countryId);
            SaveGameReadUInt8(save, city.population);
            SaveGameReadFloat(save, city.latitude);
            SaveGameReadFloat(save, city.longitude);
            SaveGameReadInt32(save, city.regionId);
            delete[] city.name;
            city.name = nullptr;
            WideChar cityName[256];
            SaveGameReadString(save, cityName, std::size(cityName));
            city.name = new WideChar[wcslen(cityName) + 1];
            wcscpy(city.name, cityName);
        }
        UInt regionCount = 0;
        SaveGameReadUInt32(save, regionCount);
        for (UInt r = 0; r < regionCount; r++) {
            Int regionId = -1;
            SaveGameReadInt32(save, regionId);
            DBRegion &region = DBRegions()[regionId];
            region.id = regionId;
            SaveGameReadUInt8(save, region.countryId);
            SaveGameReadFloat(save, region.latitude);
            SaveGameReadFloat(save, region.longitude);
            delete[] region.name;
            region.name = nullptr;
            WideChar regionName[256];
            SaveGameReadString(save, regionName, std::size(regionName));
            region.name = new WideChar[wcslen(regionName) + 1];
            wcscpy(region.name, regionName);
        }
    }
    Call<0xF93760>(); // CDBCompetition::LoadAll
}

void OnSaveCompetitions() {
    void *save = *(void **)0x3179DD4;
    SaveGameWriteUInt32(save, DBCities().size());
    for (auto const &[id, city] : DBCities()) {
        SaveGameWriteInt32(save, city.id);
        SaveGameWriteUInt8(save, city.countryId);
        SaveGameWriteUInt8(save, city.population);
        SaveGameWriteFloat(save, city.latitude);
        SaveGameWriteFloat(save, city.longitude);
        SaveGameWriteInt32(save, city.regionId);
        SaveGameWriteString(save, city.name);
    }
    SaveGameWriteUInt32(save, DBRegions().size());
    for (auto const &[id, region] : DBRegions()) {
        SaveGameWriteInt32(save, region.id);
        SaveGameWriteUInt8(save, region.countryId);
        SaveGameWriteFloat(save, region.latitude);
        SaveGameWriteFloat(save, region.longitude);
        SaveGameWriteString(save, region.name);
    }
    Call<0xF90F00>(); // CDBCompetition::SaveAll
}

void METHOD OnTeamLoadTown(void *save, DUMMY_ARG, WideChar *out, UInt maxLen) {
    if (SaveGameLoadGetVersion(save) >= 49) {
        CDBTeam *team = raw_ptr<CDBTeam>(out, -0x11C);
        SaveGameReadInt32(save, GetTeamExtension(team)->cityId);
        *out = L'\0';
        // TODO: remove this (copying new city name to an old destination)
        if (GetTeamExtension(team)->cityId != -1) {
            auto city = GetCity(GetTeamExtension(team)->cityId);
            if (city)
                wcscpy_s(out, 30, city->name);
        }
    }
    else
        SaveGameReadString(save, out, maxLen);
}

void METHOD OnTeamSaveTown(void *save, DUMMY_ARG, WideChar const *str) {
    CDBTeam *team = raw_ptr<CDBTeam>(str, -0x11C);
    SaveGameWriteInt32(save, GetTeamExtension(team)->cityId);
}

void *OnAfterCountriesLoaded() {
    void *save = *(void **)0x3179DD8;
    if (SaveGameLoadGetVersion(save) < 49) {
        Int NewCityId = 2100000001;
        for (UInt countryId = 1; countryId <= 207; countryId++) {
            auto country = GetCountry(countryId);
            Map<String, DBCity> countryCities;
            for (Int t = 1; t <= country->GetNumClubs(); t++) {
                auto team = GetTeam(CTeamIndex::make(country->GetCountryId(), FifamClubTeamType::First, t));
                if (team) {
                    auto cityName = raw_ptr<WideChar const>(team, 0x11C);
                    Int cityId = -1;
                    auto it = countryCities.find(cityName);
                    if (it == countryCities.end()) {
                        auto &city = countryCities[cityName];
                        city.id = NewCityId++;
                        city.countryId = countryId;
                        city.latitude = (Float)team->GetLatitude() / 60.0f;
                        city.longitude = (Float)team->GetLongitude() / 60.0f;
                        delete[] city.name;
                        city.name = new WideChar[wcslen(cityName) + 1];
                        wcscpy(city.name, cityName);
                        cityId = city.id;
                    }
                    else
                        cityId = (*it).second.id;
                    GetTeamExtension(team)->cityId = cityId;
                }
            }
            for (auto const &[name, city] : countryCities)
                DBCities()[city.id] = city;
        }
    }
    return CallAndReturn<void *, 0x61FC60>();
}

void METHOD OnReadTeamTownFromMasterDb(void *reader, DUMMY_ARG, WideChar *out, UInt maxLen) {
    if (!BinaryReaderIsVersionGreaterOrEqual(reader, 0x2013, 0x12))
        BinaryReaderReadString(reader, out, maxLen);
}

void METHOD OnReadTeamMascotFromMasterDb(void *reader, DUMMY_ARG, WideChar *out, UInt maxLen) {
    if (BinaryReaderIsVersionGreaterOrEqual(reader, 0x2013, 0x12)) {
        CDBTeam *team = raw_ptr<CDBTeam>(out, -0x29A);
        BinaryReaderReadInt32(reader, &GetTeamExtension(team)->cityId);
    }
    BinaryReaderReadString(reader, out, maxLen);
}

void METHOD OnReadTeamFifaIdFromMasterDb(void *reader, DUMMY_ARG, UInt *out) {
    BinaryReaderReadUInt32(reader, out);
    CDBTeam *team = raw_ptr<CDBTeam>(out, -0xEC);
    if (team->GetTeamUniqueID() == 0x002D0001) // Athletic Club
        team->SetRegionalAffiliationRestriction(PLAYER_REGIONAL_BASQUE);
    else if (team->GetTeamUniqueID() == 0x002D1038) // U.E. Olot
        team->SetRegionalAffiliationRestriction(PLAYER_REGIONAL_CATALAN);
}

WideChar *METHOD OnGetTeamTown(CDBTeam *team) {
    auto city = GetCity(GetTeamExtension(team)->cityId);
    if (city)
        return city->name;
    return L"City";
}

UInt METHOD OnGetNumTeams1(CDBCompetition *comp) {
    return GetCountry(comp->GetCompID().countryId)->GetNumClubs();
}

CTeamIndex *METHOD OnGetTeam1(CDBCompetition *comp, DUMMY_ARG, UInt index) {
    static CTeamIndex teamID;
    teamID.countryId = comp->GetCompID().countryId;
    teamID.index = index + 1;
    teamID.type = 0;
    return &teamID;
}

void OnClearCountries() {
    Call<0xFF4820>();
    ClearCitiesAndRegions();
}

const UInt PlayerInfoPersonalDefaultSize = 0x80C;

struct PlayerInfoPersonalExtended {
    CTrfmNode *TrfmLeadershipAndDiscipline;
    CTrfmNode *TrfmRelatives;
    CXgTextBox *TbBirthplaceUnknown;
    CXgTextBox *TbBirthplace;
    CXgImage *ImgBirthCountry;
    CXgTextBox *TbRegionBlank;
    CXgTextBox *TbRegion;
    CXgImage *ImgRegion;
    CXgTextButton *BtRelatives;
    CXgTextButton *BtLeadershipAndDiscipline;
};

void METHOD OnPlayerInfoPersonalCreateUI(CXgFMPanel *screen) {
    CallMethod<0x5DE530>(screen); // CPlayerInfoPersonal::CreateUI
    auto ext = raw_ptr<PlayerInfoPersonalExtended>(screen, PlayerInfoPersonalDefaultSize);
    ext->TrfmLeadershipAndDiscipline = screen->GetTransform("TrfmLeadershipAndDiscipline");
    ext->TrfmRelatives = screen->GetTransform("TrfmRelatives");
    ext->TbBirthplaceUnknown = screen->GetTextBox("TbBirthplaceUnknown");
    ext->TbBirthplace = screen->GetTextBox("TbBirthplace");
    ext->ImgBirthCountry = screen->GetImage("ImgBirthCountry");
    ext->TbRegionBlank = screen->GetTextBox("TbRegionBlank");
    ext->TbRegion = screen->GetTextBox("TbRegion");
    ext->ImgRegion = screen->GetImage("ImgRegion");
    ext->BtRelatives = screen->GetTextButton("BtRelatives");
    ext->BtLeadershipAndDiscipline = screen->GetTextButton("BtLeadershipAndDiscipline");
}

UInt METHOD OnPlayerInfoPersonalFill(CXgFMPanel *screen) {
    UInt playerId = CallMethodAndReturn<Int, 0x5DE220>(screen); // CPlayerInfoPanel::GetCurrentPlayerId
    CDBPlayer *player = GetPlayer(playerId);
    if (player) {
        auto ext = raw_ptr<PlayerInfoPersonalExtended>(screen, PlayerInfoPersonalDefaultSize);
        SetTransformVisible(ext->TrfmLeadershipAndDiscipline, true);
        SetTransformVisible(ext->TrfmRelatives, false);
        // Birthplace
        Bool cityFound = false;
        auto city = GetCity(GetPlayerBirthCityID(player));
        if (city) {
            String cityName = city->name;
            String regionName;
            auto region = GetRegion(city->regionId);
            if (region)
                regionName = region->name;
            String text = cityName + L", " + CountryName(city->countryId);
            String tooltip = cityName;
            if (!regionName.empty())
                tooltip += L", " + regionName;
            tooltip += L", " + CountryName(city->countryId);
            ext->TbBirthplace->SetText(text.c_str());
            ext->TbBirthplace->SetTooltip(tooltip.c_str());
            SetControlCountryFlag(ext->ImgBirthCountry, city->countryId);
            cityFound = true;
        }
        ext->TbBirthplace->SetVisible(cityFound);
        ext->ImgBirthCountry->SetVisible(cityFound);
        ext->TbBirthplaceUnknown->SetVisible(!cityFound);
        // Affiliated region
        Bool regionFound = false;
        if (player->GetRegionalAffiliation() == PLAYER_REGIONAL_BASQUE) {
            ext->TbRegion->SetText(GetTranslation("IDS_BASQUE"));
            SetImageFilename(ext->ImgRegion, L"art/Lib/CountryFlags/32x32/basque.tga", 4, 4);
            regionFound = true;
        }
        else if (player->GetRegionalAffiliation() == PLAYER_REGIONAL_CATALAN) {
            ext->TbRegion->SetText(GetTranslation("IDS_CATALAN"));
            SetImageFilename(ext->ImgRegion, L"art/Lib/CountryFlags/32x32/catalan.tga", 4, 4);
            regionFound = true;
        }
        ext->TbRegion->SetVisible(regionFound);
        ext->ImgRegion->SetVisible(regionFound);
        ext->TbRegionBlank->SetVisible(!regionFound);
    }
    return playerId;
}

void PatchCitiesAndRegions(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0xF9745E, OnReadAppearanceDefsFromBinaryDatabase);
        patch::RedirectCall(0x1083FED, OnLoadCompetitions);
        patch::RedirectCall(0x10831C2, OnSaveCompetitions);
        patch::RedirectCall(0xF4432B, OnTeamLoadTown);
        patch::RedirectCall(0xF0E4B3, OnTeamSaveTown);
        patch::RedirectCall(0x10840F4, OnAfterCountriesLoaded);
        patch::RedirectJump(0xED2340, OnGetTeamTown);
        patch::RedirectCall(0x108D80C, OnClearCountries);

        patch::RedirectCall(0xF3307E, OnReadTeamTownFromMasterDb);
        patch::RedirectCall(0xF331C1, OnReadTeamMascotFromMasterDb);

        // CPlayerInfoPersonal
        patch::SetUInt(0x5C9594 + 1, PlayerInfoPersonalDefaultSize + sizeof(PlayerInfoPersonalExtended));
        patch::SetUInt(0x5C959B + 1, PlayerInfoPersonalDefaultSize + sizeof(PlayerInfoPersonalExtended));
        patch::SetPointer(0x23D2064, OnPlayerInfoPersonalCreateUI);
        patch::RedirectCall(0x5DE827, OnPlayerInfoPersonalFill);

        // regional status
        patch::RedirectCall(0xF332E1, OnReadTeamFifaIdFromMasterDb);
        patch::Nop(0xF332EF, 7);

        //patch::RedirectCall(0x11C51F3, OnGetNumTeams1);
        //patch::RedirectCall(0x11C527B, OnGetNumTeams1);
        //patch::RedirectCall(0x11C5203, OnGetTeam1);
    }
}
