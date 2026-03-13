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

Bool OnReadAppearanceDefsFromBinaryDatabase(CBinaryFile *file) {
    Bool result = CallAndReturn<Bool, 0x1382F10>(file);
    // binary database version 20130012
    ClearCitiesAndRegions();
    if (file->IsVersionGreaterOrEqual(0x2013, 0x12) && file->ValidateFourcc('CTRG')) {
        UInt cityCount = file->ReadUInt();
        for (UInt c = 0; c < cityCount; c++) {
            Int cityId = file->ReadInt();
            DBCity &city = DBCities()[cityId];
            city.id = cityId;
            file->ReadUChar(city.countryId);
            file->ReadUChar(city.population);
            file->ReadFloat(city.latitude);
            file->ReadFloat(city.longitude);
            file->ReadInt(city.regionId);
            delete[] city.name;
            city.name = nullptr;
            for (UInt i = 0; i < NUM_TRANSLATION_LANGUAGES; i++) {
                WideChar cityName[256];
                file->ReadString(cityName, std::size(cityName));
                if (i == CurrentLanguageId) {
                    city.name = new WideChar[wcslen(cityName) + 1];
                    wcscpy(city.name, cityName);
                }
            }
        }
        UInt regionCount = file->ReadUInt();
        for (UInt r = 0; r < regionCount; r++) {
            Int regionId = file->ReadInt();
            DBRegion &region = DBRegions()[regionId];
            region.id = regionId;
            file->ReadUChar(region.countryId);
            file->ReadFloat(region.latitude);
            file->ReadFloat(region.longitude);
            delete[] region.name;
            region.name = nullptr;
            for (UInt i = 0; i < NUM_TRANSLATION_LANGUAGES; i++) {
                WideChar regionName[256];
                file->ReadString(regionName, std::size(regionName));
                if (i == CurrentLanguageId) {
                    region.name = new WideChar[wcslen(regionName) + 1];
                    wcscpy(region.name, regionName);
                }
            }
        }
        file->ValidateFourcc('CTRG');
    }
    return result;
}

void OnLoadCompetitions() {
    auto file = GetDBLoad();
    ClearCitiesAndRegions();
    if (file->GetVersion() >= 49) {
        UInt cityCount = file->ReadUInt();
        for (UInt c = 0; c < cityCount; c++) {
            Int cityId = file->ReadInt();
            DBCity &city = DBCities()[cityId];
            city.id = cityId;
            file->ReadUChar(city.countryId);
            file->ReadUChar(city.population);
            file->ReadFloat(city.latitude);
            file->ReadFloat(city.longitude);
            file->ReadInt(city.regionId);
            delete[] city.name;
            city.name = nullptr;
            WideChar cityName[256];
            file->ReadString(cityName, std::size(cityName));
            city.name = new WideChar[wcslen(cityName) + 1];
            wcscpy(city.name, cityName);
        }
        UInt regionCount = file->ReadUInt();
        for (UInt r = 0; r < regionCount; r++) {
            Int regionId = file->ReadInt();
            DBRegion &region = DBRegions()[regionId];
            region.id = regionId;
            file->ReadUChar(region.countryId);
            file->ReadFloat(region.latitude);
            file->ReadFloat(region.longitude);
            delete[] region.name;
            region.name = nullptr;
            WideChar regionName[256];
            file->ReadString(regionName, std::size(regionName));
            region.name = new WideChar[wcslen(regionName) + 1];
            wcscpy(region.name, regionName);
        }
    }
    Call<0xF93760>(); // CDBCompetition::LoadAll
}

void OnSaveCompetitions() {
    auto file = GetDBSave();
    file->WriteUInt(DBCities().size());
    for (auto const &[id, city] : DBCities()) {
        file->WriteInt(city.id);
        file->WriteUChar(city.countryId);
        file->WriteUChar(city.population);
        file->WriteFloat(city.latitude);
        file->WriteFloat(city.longitude);
        file->WriteInt(city.regionId);
        file->WriteString(city.name);
    }
    file->WriteUInt(DBRegions().size());
    for (auto const &[id, region] : DBRegions()) {
        file->WriteInt(region.id);
        file->WriteUChar(region.countryId);
        file->WriteFloat(region.latitude);
        file->WriteFloat(region.longitude);
        file->WriteString(region.name);
    }
    Call<0xF90F00>(); // CDBCompetition::SaveAll
}

void METHOD OnTeamLoadTown(CDBLoad *file, DUMMY_ARG, WideChar *out, UInt maxLen) {
    if (file->GetVersion() >= 49) {
        CDBTeam *team = raw_ptr<CDBTeam>(out, -0x11C);
        file->ReadInt(GetTeamExtension(team)->cityId);
        *out = L'\0';
        // TODO: remove this (copying new city name to an old destination)
        if (GetTeamExtension(team)->cityId != -1) {
            auto city = GetCity(GetTeamExtension(team)->cityId);
            if (city) {
                wcsncpy(out, city->name, maxLen - 1);
                out[maxLen - 1] = L'\0';
            }
        }
    }
    else
        file->ReadString(out, maxLen);
}

void METHOD OnTeamSaveTown(CDBSave *file, DUMMY_ARG, WideChar const *str) {
    CDBTeam *team = raw_ptr<CDBTeam>(str, -0x11C);
    file->WriteInt(GetTeamExtension(team)->cityId);
}

void *OnAfterCountriesLoaded() {
    if (GetDBLoad()->GetVersion() < 49) {
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

void METHOD OnReadTeamTownFromMasterDb(CBinaryFile *file, DUMMY_ARG, WideChar *out, UInt maxLen) {
    if (!file->IsVersionGreaterOrEqual(0x2013, 0x12))
        file->ReadString(out, maxLen);
}

void METHOD OnReadTeamMascotFromMasterDb(CBinaryFile *file, DUMMY_ARG, WideChar *out, UInt maxLen) {
    if (file->IsVersionGreaterOrEqual(0x2013, 0x12)) {
        CDBTeam *team = raw_ptr<CDBTeam>(out, -0x29A);
        file->ReadInt(GetTeamExtension(team)->cityId);
    }
    file->ReadString(out, maxLen);
}

void METHOD OnReadTeamFifaIdFromMasterDb(CBinaryFile *file, DUMMY_ARG, UInt *out) {
    file->ReadUInt(out);
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

const UInt DEF_LEAGUE_SZ = 0x3EC8;

struct LeagueRegion {
    Int regionID;
    UChar priority;
    UChar direction;
};

struct LeagueExtension {
    UChar order;
    UChar numberOfRegions;
    LeagueRegion *regions;
};

LeagueExtension *GetLeagueExtension(CDBLeague *league) {
    return raw_ptr<LeagueExtension>(league, DEF_LEAGUE_SZ);
}

CDBLeague *METHOD OnConstructLeague(CDBLeague *league, DUMMY_ARG, UInt dbType, UInt compID, UInt rootID) {
    CallMethod<0x10CE180>(league, dbType, compID, rootID); // CDBLeagueBase::CDBLeagueBase
    auto ext = GetLeagueExtension(league);
    ext->order = 0;
    ext->numberOfRegions = 0;
    ext->regions = nullptr;
    return league;
}

void METHOD OnDestructLeague(CDBLeague *league) {
    auto ext = GetLeagueExtension(league);
    delete[] ext->regions;
    ext->regions = nullptr;
    CallMethod<0x10CD950>(league); // CDBLeagueBase::~CDBLeagueBase
}

void METHOD OnLeagueLoad(CDBLeague *league) {
    CallMethod<0x10CE2F0>(league); // CDBLeagueBase::Load
    auto file = GetDBLoad();
    if (file->GetVersion() >= 49) {
        auto ext = GetLeagueExtension(league);
        file->ReadUChar(ext->order);
        file->ReadUChar(ext->numberOfRegions);
        if (ext->numberOfRegions) {
            ext->regions = new LeagueRegion[ext->numberOfRegions];
            for (UInt i = 0; i < ext->numberOfRegions; i++) {
                file->ReadInt(ext->regions[i].regionID);
                file->ReadUChar(ext->regions[i].priority);
                file->ReadUChar(ext->regions[i].direction);
            }
        }
    }
}

void METHOD OnLeagueSave(CDBLeague *league) {
    CallMethod<0x10CBF20>(league); // CDBLeagueBase::Save
    auto file = GetDBSave();
    auto ext = GetLeagueExtension(league);
    file->WriteUChar(ext->order);
    file->WriteUChar(ext->numberOfRegions);
    if (ext->numberOfRegions) {
        for (UInt i = 0; i < ext->numberOfRegions; i++) {
            file->WriteInt(ext->regions[i].regionID);
            file->WriteUChar(ext->regions[i].priority);
            file->WriteUChar(ext->regions[i].direction);
        }
    }
}

void METHOD OnReadLeagueFromMasterDb(CDBLeague *league, DUMMY_ARG, CBinaryFile *file) {
    CallMethod<0x10CBE80>(league, file); // CDBLeagueBase::ReadFromMasterDb
    auto ext = GetLeagueExtension(league);
    if (file->IsVersionGreaterOrEqual(0x2013, 0x12)) {
        auto ext = GetLeagueExtension(league);
        file->ReadUChar(ext->order);
        file->ReadUChar(ext->numberOfRegions);
        if (ext->numberOfRegions) {
            ext->regions = new LeagueRegion[ext->numberOfRegions];
            for (UInt i = 0; i < ext->numberOfRegions; i++) {
                file->ReadInt(ext->regions[i].regionID);
                file->ReadUChar(ext->regions[i].priority);
                file->ReadUChar(ext->regions[i].direction);
            }
        }
    }
}

UChar METHOD OnPlayerInfoScreen_GetIsBasque(CDBPlayer *) {
    return 0;
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

        // remove basque flag on player info screens
        patch::RedirectCall(0x510A8B, OnPlayerInfoScreen_GetIsBasque); // CSeasonTrialDayResults
        patch::RedirectCall(0x5CD3AD, OnPlayerInfoScreen_GetIsBasque); // CPlayerInfo
        patch::RedirectCall(0x5D92A4, OnPlayerInfoScreen_GetIsBasque); // CPlayerInfoCompare
        patch::RedirectCall(0x5E7BE0, OnPlayerInfoScreen_GetIsBasque); // CPlayerInfoYouth
        patch::RedirectCall(0x8224CB, OnPlayerInfoScreen_GetIsBasque);
        patch::RedirectCall(0x828C28, OnPlayerInfoScreen_GetIsBasque);
        patch::RedirectCall(0x82BB3B, OnPlayerInfoScreen_GetIsBasque);
        patch::RedirectCall(0x8E5F2C, OnPlayerInfoScreen_GetIsBasque);
        patch::RedirectCall(0x9259A2, OnPlayerInfoScreen_GetIsBasque);
        patch::RedirectCall(0x9B52F6, OnPlayerInfoScreen_GetIsBasque);
        patch::RedirectCall(0xA2ABBC, OnPlayerInfoScreen_GetIsBasque);
        patch::RedirectCall(0xDDC51B, OnPlayerInfoScreen_GetIsBasque);
        patch::RedirectCall(0xDDC5F6, OnPlayerInfoScreen_GetIsBasque);

        // league regions
        const UInt NEW_LEAGUE_SZ = DEF_LEAGUE_SZ + sizeof(LeagueExtension);
        patch::SetUInt(0xF92827 + 1, NEW_LEAGUE_SZ);
        patch::SetUInt(0xF92B6B + 1, NEW_LEAGUE_SZ);
        patch::SetUInt(0xF9386F + 1, NEW_LEAGUE_SZ);
        patch::SetUInt(0xFF2314 + 1, NEW_LEAGUE_SZ);
        patch::RedirectCall(0x106457B, OnConstructLeague);
        patch::RedirectCall(0x10614EC, OnDestructLeague);
        patch::RedirectCall(0x105FD08, OnLeagueLoad);
        patch::RedirectCall(0x1059AD5, OnLeagueSave);
        patch::RedirectCall(0x1055FFC, OnReadLeagueFromMasterDb);

        //patch::RedirectCall(0x11C51F3, OnGetNumTeams1);
        //patch::RedirectCall(0x11C527B, OnGetNumTeams1);
        //patch::RedirectCall(0x11C5203, OnGetTeam1);
    }
}
