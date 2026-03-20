#include "CitiesAndRegions.h"
#include "GameInterfaces.h"
#include "TranslationShared.h"
#include "Translation.h"
#include "ExtendedTeam.h"
#include "FifamClubTeamType.h"
#include "ExtendedPlayer.h"
#include "Utils.h"

Array<DBCity *, 208> CountryCities;
Array<DBRegion *, 208> CountryRegions;
Array<UShort, 208> CountryNumCities = {};
Array<UChar, 208> CountryNumRegions = {};
Array<UShort, 208> CountryCityTotalWeight = {};

CityID::CityID() {
    countryId = 0;
    regionIndex = 255;
    index = 0;
}

CityID::CityID(UChar CountryId, UShort Index, UChar RegionIndex) {
    countryId = CountryId;
    index = Index;
    regionIndex = RegionIndex;
}

void CityID::Clear() {
    countryId = 0;
    index = 0;
}

Bool CityID::IsValid() {
    return countryId != 0;
}

RegionID::RegionID() {
    countryId = 0;
    index = 0;
}

RegionID::RegionID(UChar CountryId, UChar Index) {
    countryId = CountryId;
    index = Index;
}

void RegionID::Clear() {
    countryId = 0;
    index = 0;
}

Bool RegionID::IsValid() {
    return countryId != 0;
}

DBCity *GetCity(CityID ID) {
    if (ID.countryId >= 1 && ID.countryId <= 207 && ID.index < CountryNumCities[ID.countryId])
        return &CountryCities[ID.countryId][ID.index];
    return nullptr;
}

DBRegion *GetRegion(RegionID ID) {
    if (ID.index != 255 && ID.countryId >= 1 && ID.countryId <= 207 && ID.index < CountryNumRegions[ID.countryId])
        return &CountryRegions[ID.countryId][ID.index];
    return nullptr;
}

Bool IsCatalanCity(CityID cityID) {
    auto region = GetRegion(RegionID(cityID.countryId, cityID.regionIndex));
    return region && region->EditorID == 67023595;
}

Bool IsTeamCatalan(CDBTeam *team) {
    return IsCatalanCity(GetTeamExtension(team)->cityID);
}

void ClearCities() {
    for (UChar countryId = 1; countryId <= 207; countryId++) {
        for (UShort i = 0; i < CountryNumCities[countryId]; i++)
            delete[] CountryCities[countryId]->name;
        delete[] CountryCities[countryId];
        CountryCities[countryId] = nullptr;
        CountryNumCities[countryId] = 0;
        CountryCityTotalWeight[countryId] = 0;
    }
}

void ClearRegions() {
    for (UChar countryId = 1; countryId <= 207; countryId++) {
        for (UChar i = 0; i < CountryNumRegions[countryId]; i++)
            delete[] CountryRegions[countryId]->name;
        delete[] CountryRegions[countryId];
        CountryRegions[countryId] = nullptr;
        CountryNumRegions[countryId] = 0;
    }
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
        for (UChar countryId = 1; countryId <= 207; countryId++) {
            CountryNumCities[countryId] = file->ReadUShort();
            CountryCities[countryId] = new DBCity[CountryNumCities[countryId]];
            for (UShort c = 0; c < CountryNumCities[countryId]; c++) {
                DBCity &city = CountryCities[countryId][c];
                city.ID.index = c;
                file->ReadInt(city.EditorID);
                file->ReadUChar(city.ID.countryId);
                file->ReadUChar(city.ID.regionIndex);
                file->ReadUChar(city.population);
                file->ReadUShort(city.weight);
                file->ReadFloat(city.latitude);
                file->ReadFloat(city.longitude);
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
            CountryCityTotalWeight[countryId] = file->ReadUShort();
        }
        for (UChar countryId = 1; countryId <= 207; countryId++) {
            CountryNumRegions[countryId] = file->ReadUChar();
            CountryRegions[countryId] = new DBRegion[CountryNumRegions[countryId]];
            for (UChar r = 0; r < CountryNumRegions[countryId]; r++) {
                DBRegion &region = CountryRegions[countryId][r];
                region.ID.index = r;
                file->ReadInt(region.EditorID);
                file->ReadUChar(region.ID.countryId);
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
        }
        file->ValidateFourcc('CTRG');
    }
    return result;
}

void OnLoadCompetitions() {
    auto file = GetDBLoad();
    ClearCitiesAndRegions();
    if (file->GetVersion() >= 49) {
        for (UChar countryId = 1; countryId <= 207; countryId++) {
            CountryNumCities[countryId] = file->ReadUShort();
            CountryCities[countryId] = new DBCity[CountryNumCities[countryId]];
            for (UShort c = 0; c < CountryNumCities[countryId]; c++) {
                DBCity &city = CountryCities[countryId][c];
                city.ID.index = c;
                file->ReadInt(city.EditorID);
                file->ReadUChar(city.ID.countryId);
                file->ReadUChar(city.ID.regionIndex);
                file->ReadUChar(city.population);
                file->ReadUShort(city.weight);
                file->ReadFloat(city.latitude);
                file->ReadFloat(city.longitude);
                delete[] city.name;
                city.name = nullptr;
                WideChar cityName[256];
                file->ReadString(cityName, std::size(cityName));
                city.name = new WideChar[wcslen(cityName) + 1];
                wcscpy(city.name, cityName);
            }
            CountryCityTotalWeight[countryId] = file->ReadUShort();
        }
        for (UChar countryId = 1; countryId <= 207; countryId++) {
            CountryNumRegions[countryId] = file->ReadUChar();
            CountryRegions[countryId] = new DBRegion[CountryNumRegions[countryId]];
            for (UChar r = 0; r < CountryNumRegions[countryId]; r++) {
                DBRegion &region = CountryRegions[countryId][r];
                region.ID.index = r;
                file->ReadInt(region.EditorID);
                file->ReadUChar(region.ID.countryId);
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
    }
    Call<0xF93760>(); // CDBCompetition::LoadAll
}

void OnSaveCompetitions() {
    auto file = GetDBSave();
    for (UChar countryId = 1; countryId <= 207; countryId++) {
        file->WriteUShort(CountryNumCities[countryId]);
        for (UShort c = 0; c < CountryNumCities[countryId]; c++) {
            DBCity &city = CountryCities[countryId][c];
            file->WriteInt(city.EditorID);
            file->WriteUChar(city.ID.countryId);
            file->WriteUChar(city.ID.regionIndex);
            file->WriteUChar(city.population);
            file->WriteUShort(city.weight);
            file->WriteFloat(city.latitude);
            file->WriteFloat(city.longitude);
            file->WriteString(city.name);
        }
        file->WriteUShort(CountryCityTotalWeight[countryId]);
    }
    for (UChar countryId = 1; countryId <= 207; countryId++) {
        file->WriteUChar(CountryNumRegions[countryId]);
        for (UChar r = 0; r < CountryNumRegions[countryId]; r++) {
            DBRegion &region = CountryRegions[countryId][r];
            file->WriteInt(region.EditorID);
            file->WriteUChar(region.ID.countryId);
            file->WriteFloat(region.latitude);
            file->WriteFloat(region.longitude);
            file->WriteString(region.name);
        }
    }
    Call<0xF90F00>(); // CDBCompetition::SaveAll
}

void METHOD OnTeamLoadTown(CDBLoad *file, DUMMY_ARG, WideChar *out, UInt maxLen) {
    if (file->GetVersion() >= 49) {
        CDBTeam *team = raw_ptr<CDBTeam>(out, -0x11C);
        auto ext = GetTeamExtension(team);
        file->ReadUChar(ext->cityID.countryId);
        file->ReadUChar(ext->cityID.regionIndex);
        file->ReadUShort(ext->cityID.index);
        *out = L'\0';
        // TODO: remove this (copying new city name to an old destination)
        auto city = GetCity(ext->cityID);
        if (city) {
            wcsncpy(out, city->name, maxLen - 1);
            out[maxLen - 1] = L'\0';
        }
    }
    else
        file->ReadString(out, maxLen);
}

void METHOD OnTeamSaveTown(CDBSave *file, DUMMY_ARG, WideChar const *str) {
    CDBTeam *team = raw_ptr<CDBTeam>(str, -0x11C);
    auto cityID = GetTeamExtension(team)->cityID;
    file->WriteUChar(cityID.countryId);
    file->WriteUChar(cityID.regionIndex);
    file->WriteUShort(cityID.index);
}

void *OnAfterCountriesLoaded() {
    if (GetDBLoad()->GetVersion() < 49) {
        ClearCitiesAndRegions();
        Int NewCityId = 2100000001;
        for (UInt countryId = 1; countryId <= 207; countryId++) {
            auto country = GetCountry(countryId);
            Map<String, DBCity> countryCities;
            for (Int t = 1; t <= country->GetNumClubs(); t++) {
                auto team = GetTeam(CTeamIndex::make(country->GetCountryId(), FifamClubTeamType::First, t));
                if (team) {
                    auto cityName = raw_ptr<WideChar const>(team, 0x11C);
                    CityID cityId;
                    auto it = countryCities.find(cityName);
                    if (it == countryCities.end()) {
                        auto &city = countryCities[cityName];
                        city.EditorID = NewCityId++;
                        city.ID.countryId = countryId;
                        city.ID.index = (UShort)(countryCities.size() - 1);
                        city.weight = city.ID.index + 1;
                        city.latitude = (Float)team->GetLatitude() / 60.0f;
                        city.longitude = (Float)team->GetLongitude() / 60.0f;
                        delete[] city.name;
                        city.name = new WideChar[wcslen(cityName) + 1];
                        wcscpy(city.name, cityName);
                        cityId = city.ID;
                    }
                    else
                        cityId = (*it).second.ID;
                    GetTeamExtension(team)->cityID = cityId;
                }
            }
            CountryNumCities[countryId] = (UShort)countryCities.size();
            CountryCityTotalWeight[countryId] = (UShort)countryCities.size();
            CountryCities[countryId] = new DBCity[countryCities.size()];
            UShort cityIndex = 0;
            for (auto const &[name, city] : countryCities)
                CountryCities[countryId][cityIndex++] = city;
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
        auto ext = GetTeamExtension(team);
        file->ReadUChar(ext->cityID.countryId);
        file->ReadUChar(ext->cityID.regionIndex);
        file->ReadUShort(ext->cityID.index);
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
    auto city = GetCity(GetTeamExtension(team)->cityID);
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
        // Birthplace
        Bool cityFound = false;
        auto city = GetCity(GetPlayerBirthCityID(player));
        if (city) {
            String cityName = city->name;
            String regionName;
            auto region = GetRegion(RegionID(city->ID.countryId, city->ID.regionIndex));
            if (region)
                regionName = region->name;
            String text = cityName + L", " + CountryName(city->ID.countryId);
            String tooltip = cityName;
            if (!regionName.empty())
                tooltip += L", " + regionName;
            tooltip += L", " + CountryName(city->ID.countryId);
            ext->TbBirthplace->SetText(text.c_str());
            ext->TbBirthplace->SetTooltip(tooltip.c_str());
            SetControlCountryFlag(ext->ImgBirthCountry, city->ID.countryId);
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
        // Relatives
        SetTransformVisible(ext->TrfmLeadershipAndDiscipline, true);
        SetTransformVisible(ext->TrfmRelatives, false);
        UInt numRelatives = player->GetNumRelatives(1) + player->GetNumRelatives(2);
        ext->BtRelatives->SetVisible(numRelatives > 0);
    }
    return playerId;
}

void METHOD OnPlayerInfoPersonalButtonReleased(CXgFMPanel *screen, DUMMY_ARG, GuiMessage *msg, Int) {
    auto ext = raw_ptr<PlayerInfoPersonalExtended>(screen, PlayerInfoPersonalDefaultSize);
    if (msg->node == ext->BtRelatives->GetGuiNode()) {
        SetTransformVisible(ext->TrfmRelatives, true);
        SetTransformVisible(ext->TrfmLeadershipAndDiscipline, false);
    }
    else if (msg->node == ext->BtLeadershipAndDiscipline->GetGuiNode()) {
        SetTransformVisible(ext->TrfmLeadershipAndDiscipline, true);
        SetTransformVisible(ext->TrfmRelatives, false);
    }
}

const UInt DEF_LEAGUE_SZ = 0x3EC8;

struct LeagueRegion {
    RegionID regionID;
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
                file->ReadUChar(ext->regions[i].regionID.countryId);
                file->ReadUChar(ext->regions[i].regionID.index);
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
            file->WriteUChar(ext->regions[i].regionID.countryId);
            file->WriteUChar(ext->regions[i].regionID.index);
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
                file->ReadUChar(ext->regions[i].regionID.countryId);
                file->ReadUChar(ext->regions[i].regionID.index);
                file->ReadUChar(ext->regions[i].priority);
                file->ReadUChar(ext->regions[i].direction);
            }
        }
    }
}

UChar METHOD OnPlayerInfoScreen_GetIsBasque(CDBPlayer *) {
    return 0;
}

const UInt TfTransferMarketPlayerSearchOrigSize = 0x6D0;

struct TfTransferMarketPlayerSearchExtension {
    CXgComboBox *CbBirthplaceCountry;
    CXgComboBox *CbBirthplaceRegion;
    CXgComboBox *CbBirthplaceCity;
};

struct TfTransferListSearchDescExtension {
    UInt countryId;
    Int regionIndex;
    Int cityIndex;

    void Clear() {
        countryId = 0;
        regionIndex = -1;
        cityIndex = -1;
    }
};

void *GetTransferListScreenStructForCurrentManager() {
    return CallAndReturn<void *, 0x8DC090>();
}

void AddCititesToCombobox(CXgComboBox *comboBox, Vector<DBCity const *> &cities) {
    UInt maxSize = comboBox->GetTable()->GetMaxRows();
    if (cities.size() > maxSize) {
        Utils::Sort(cities, [](DBCity const *a, DBCity const *b) {
            if (a->population > b->population)
                return true;
            if (b->population > a->population)
                return false;
            return a->EditorID < b->EditorID;
        });
        cities.resize(maxSize);
    }
    for (auto const &city : cities)
        comboBox->AddItem(city->name, city->ID.index);
}

void BirthplaceComboBoxCountryChanged(TfTransferMarketPlayerSearchExtension *ext, TfTransferListSearchDescExtension *dataExt, Bool init) {
    Int countryId = ext->CbBirthplaceCountry->GetCurrentValue(0);
    ext->CbBirthplaceRegion->Clear();
    ext->CbBirthplaceCity->Clear();
    ext->CbBirthplaceRegion->AddItem(GetTranslation("AC_Blank"), -1);
    ext->CbBirthplaceCity->AddItem(GetTranslation("AC_Blank"), -1);
    if (countryId != 0) {
        for (UChar i = 0; i < CountryNumRegions[countryId]; i++)
            ext->CbBirthplaceRegion->AddItem(CountryRegions[countryId][i].name, i);
    }
    ext->CbBirthplaceRegion->SortByString();
    ext->CbBirthplaceRegion->SetCurrentValue(-1);
    if (init && dataExt->regionIndex != -1)
        ext->CbBirthplaceRegion->SetCurrentValue(dataExt->regionIndex);
    if (countryId != 0) {
        Vector<DBCity const *> cities;
        for (UShort i = 0; i < CountryNumCities[countryId]; i++)
            cities.push_back(&CountryCities[countryId][i]);
        AddCititesToCombobox(ext->CbBirthplaceCity, cities);
    }
    ext->CbBirthplaceCity->SortByString();
    ext->CbBirthplaceCity->SetCurrentValue(-1);
    if (init && dataExt->cityIndex != -1)
        ext->CbBirthplaceCity->SetCurrentValue(dataExt->cityIndex);
    dataExt->countryId = countryId;
    dataExt->regionIndex = ext->CbBirthplaceRegion->GetCurrentValue(-1);
    dataExt->cityIndex = ext->CbBirthplaceCity->GetCurrentValue(-1);
}

void BirthplaceComboBoxRegionChanged(TfTransferMarketPlayerSearchExtension *ext, TfTransferListSearchDescExtension *dataExt) {
    Int countryId = ext->CbBirthplaceCountry->GetCurrentValue(0);
    Int regionIndex = ext->CbBirthplaceRegion->GetCurrentValue(-1);
    ext->CbBirthplaceCity->Clear();
    ext->CbBirthplaceCity->AddItem(GetTranslation("AC_Blank"), -1);
    if (countryId != 0) {
        Vector<DBCity const *> cities;
        for (UShort i = 0; i < CountryNumCities[countryId]; i++) {
            if (regionIndex == -1 || regionIndex == CountryCities[countryId][i].ID.regionIndex)
                cities.push_back(&CountryCities[countryId][i]);
        }
        AddCititesToCombobox(ext->CbBirthplaceCity, cities);
        ext->CbBirthplaceCity->SortByString();
    }
    ext->CbBirthplaceCity->SetCurrentValue(-1);
    dataExt->regionIndex = regionIndex;
    dataExt->cityIndex = -1;
}

void BirthplaceComboBoxCityChanged(TfTransferMarketPlayerSearchExtension *ext, TfTransferListSearchDescExtension *dataExt) {
    dataExt->cityIndex = ext->CbBirthplaceCity->GetCurrentValue(-1);
}

CXgComboBox *METHOD OnTfTransferMarketPlayerSearchCreateUI(CXgFMPanel *screen, DUMMY_ARG, Char const *name) {
    TfTransferMarketPlayerSearchExtension *ext = raw_ptr<TfTransferMarketPlayerSearchExtension>(screen, TfTransferMarketPlayerSearchOrigSize);
    ext->CbBirthplaceCountry = screen->GetComboBox("CbBirthplaceCountry");
    ext->CbBirthplaceRegion = screen->GetComboBox("CbBirthplaceRegion");
    ext->CbBirthplaceCity = screen->GetComboBox("CbBirthplaceCity");
    return screen->GetComboBox(name);
}

void METHOD OnTfTransferMarketPlayerSearchSetup(CXgFMPanel *screen) {
    UChar &counter = *raw_ptr<UChar>(screen, 0x4D0);
    counter++;
    TfTransferMarketPlayerSearchExtension *ext = raw_ptr<TfTransferMarketPlayerSearchExtension>(screen, TfTransferMarketPlayerSearchOrigSize);
    auto data = GetTransferListScreenStructForCurrentManager();
    auto dataExt = raw_ptr<TfTransferListSearchDescExtension>(data, 4 + 1 + 60 * 2);
    ext->CbBirthplaceCountry->AddItem(GetTranslation("AC_Blank"), 0);
    for (UInt i = 1; i <= 207; i++)
        ext->CbBirthplaceCountry->AddItem(GetCountry(i)->GetName(), i);
    ext->CbBirthplaceCountry->SortByString();
    if (dataExt->countryId > 207)
        dataExt->Clear();
    ext->CbBirthplaceCountry->SetCurrentValue(dataExt->countryId);
    BirthplaceComboBoxCountryChanged(ext, dataExt, true);
    counter--;
    CallMethod<0x8DC230>(screen);
}

void METHOD OnTfTransferMarketPlayerSearchProcessComboBoxes(CXgFMPanel *screen, DUMMY_ARG, GuiMessage *msg, Int unk1, Int unk2) {
    UChar &counter = *raw_ptr<UChar>(screen, 0x4D0);
    Bool &bApplytingSearchFile = *raw_ptr<Bool>(screen, 0x4D4);
    if (!counter && !bApplytingSearchFile) {
        TfTransferMarketPlayerSearchExtension *ext = raw_ptr<TfTransferMarketPlayerSearchExtension>(screen, TfTransferMarketPlayerSearchOrigSize);
        auto data = GetTransferListScreenStructForCurrentManager();
        auto dataExt = raw_ptr<TfTransferListSearchDescExtension>(data, 4 + 1 + 60 * 2);
        if (msg->node == ext->CbBirthplaceCountry->GetGuiNode())
            BirthplaceComboBoxCountryChanged(ext, dataExt, false);
        else if (msg->node == ext->CbBirthplaceRegion->GetGuiNode())
            BirthplaceComboBoxRegionChanged(ext, dataExt);
        else if (msg->node == ext->CbBirthplaceCity->GetGuiNode())
            BirthplaceComboBoxCityChanged(ext, dataExt);
    }
    CallMethod<0x8E1F50>(screen, msg, unk1, unk2);
}

void METHOD OnTfTransferMarketPlayerSearchGetSearchDesc(CXgFMPanel *screen, DUMMY_ARG, void *out) {
    CallMethod<0x8DCB80>(screen, out);
    TfTransferMarketPlayerSearchExtension *ext = raw_ptr<TfTransferMarketPlayerSearchExtension>(screen, TfTransferMarketPlayerSearchOrigSize);
    TfTransferListSearchDescExtension *descExt = raw_ptr< TfTransferListSearchDescExtension>(out, 4 + 1 + 60 * 2);
    descExt->countryId = ext->CbBirthplaceCountry->GetCurrentValue(0);
    descExt->regionIndex = ext->CbBirthplaceRegion->GetCurrentValue(-1);
    descExt->cityIndex = ext->CbBirthplaceCity->GetCurrentValue(-1);
}

Bool METHOD OnTfTransferMarketPlayerSearchIsEmpty(CXgFMPanel *screen, DUMMY_ARG, void *data) {
    Bool isEmpty = CallMethodAndReturn<Bool, 0x8DD220>(screen, data);
    if (isEmpty) {
        auto dataExt = raw_ptr<TfTransferListSearchDescExtension>(data, 4 + 1 + 60 * 2);
        if (dataExt->countryId != 0)
            return false;
    }
    return isEmpty;
}

void METHOD OnTfTransferMarketPlayerSearchReset(CXgFMPanel *screen, DUMMY_ARG, void *data) {
    UChar &counter = *raw_ptr<UChar>(screen, 0x4D0);
    counter++;
    TfTransferMarketPlayerSearchExtension *ext = raw_ptr<TfTransferMarketPlayerSearchExtension>(screen, TfTransferMarketPlayerSearchOrigSize);
    auto dataExt = raw_ptr<TfTransferListSearchDescExtension>(data, 4 + 1 + 60 * 2);
    ext->CbBirthplaceCountry->SetCurrentValue(dataExt->countryId);
    BirthplaceComboBoxCountryChanged(ext, dataExt, true);
    counter--;
    CallMethod<0x8E1440>(screen, data);
}

void OnTfTransferListSearchDescAssign(WideChar *dst, WideChar *src) {
    wcscpy(dst, src);
    TfTransferListSearchDescExtension *extDst = raw_ptr<TfTransferListSearchDescExtension>(dst, 60 * 2);
    TfTransferListSearchDescExtension *extSrc = raw_ptr<TfTransferListSearchDescExtension>(src, 60 * 2);
    *extDst = *extSrc;
}

void METHOD OnTfTransferListSearchDescClear(void *t) {
    CallMethod<0xF9D400>(t);
    TfTransferListSearchDescExtension *ext = raw_ptr<TfTransferListSearchDescExtension>(t, 1 + 60 * 2);
    ext->Clear();
}

void METHOD OnTfTransferListScreenStructLoad(void *t) {
    CallMethod<0x1222210>(t);
    auto file = GetDBLoad();
    TfTransferListSearchDescExtension *ext = raw_ptr<TfTransferListSearchDescExtension>(t, 1 + 60 * 2);
    ext->Clear();
    if (file->GetVersion() >= 49) {
        file->ReadUInt(ext->countryId);
        file->ReadInt(ext->regionIndex);
        file->ReadInt(ext->cityIndex);
    }
}

void METHOD OnTfTransferListScreenStructSave(void *t) {
    CallMethod<0x1222520>(t);
    TfTransferListSearchDescExtension *ext = raw_ptr<TfTransferListSearchDescExtension>(t, 1 + 60 * 2);
    auto file = GetDBSave();
    file->WriteUInt(ext->countryId);
    file->WriteInt(ext->regionIndex);
    file->WriteInt(ext->cityIndex);
}

void METHOD OnPlayerMakeSearchDescription(CDBPlayer *player, DUMMY_ARG, void *out, UChar countryId, CTeamIndex teamID, void *desc, CDBEmployee *manager, void *unk) {
    CallMethod<0xFCC330>(player, out, countryId, teamID, desc, manager, unk);
    TfTransferListSearchDescExtension *outExt = raw_ptr<TfTransferListSearchDescExtension>(out, 0x158 + 60 * 2);
    CityID birthCityID = GetPlayerBirthCityID(player);
    outExt->cityIndex = birthCityID.IsValid() ? birthCityID.index : -1;
    outExt->regionIndex = (birthCityID.IsValid() && birthCityID.regionIndex != 255) ? birthCityID.regionIndex : -1;
    outExt->countryId = birthCityID.countryId;
}

Bool METHOD OnTfTransferListSearchDescComparePlayer(void *t, DUMMY_ARG, void *ps) {
    Bool result = CallMethodAndReturn<Bool, 0xF9D910>(t, ps);
    if (result) {
        TfTransferListSearchDescExtension *playerData = raw_ptr<TfTransferListSearchDescExtension>(ps, 0x158 + 60 * 2);
        TfTransferListSearchDescExtension *searchData = raw_ptr<TfTransferListSearchDescExtension>(t, 1 + 60 * 2);
        if (searchData->countryId != 0) {
            if (searchData->countryId != playerData->countryId)
                result = false;
            else {
                if (searchData->cityIndex != -1)
                    result = searchData->cityIndex == playerData->cityIndex;
                else if (searchData->regionIndex != -1)
                    result = searchData->regionIndex == playerData->regionIndex;
            }
        }
    }
    return result;
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
        patch::SetPointer(0x23D20B4, OnPlayerInfoPersonalButtonReleased);

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

        // player search by birthplace
        const UInt NewTfTransferMarketPlayerSearchSize = TfTransferMarketPlayerSearchOrigSize + sizeof(TfTransferMarketPlayerSearchExtension);
        patch::SetUInt(0x56FE14 + 1, NewTfTransferMarketPlayerSearchSize);
        patch::SetUInt(0x56FE1B + 1, NewTfTransferMarketPlayerSearchSize);
        patch::RedirectCall(0x8E2F3D, OnTfTransferMarketPlayerSearchCreateUI);
        patch::RedirectCall(0x8E2FD1, OnTfTransferMarketPlayerSearchSetup);
        patch::SetPointer(0x2425158, OnTfTransferMarketPlayerSearchProcessComboBoxes);
        patch::SetUChar(0x8E2938 + 1, 59); // limit of characters for TbNameValue EditBox
        patch::RedirectCall(0x8DD8FA, OnTfTransferMarketPlayerSearchGetSearchDesc);
        patch::RedirectCall(0x8DDF72, OnTfTransferMarketPlayerSearchGetSearchDesc);
        patch::RedirectCall(0x8E39FD, OnTfTransferMarketPlayerSearchGetSearchDesc);
        patch::RedirectCall(0x8E3A45, OnTfTransferMarketPlayerSearchGetSearchDesc);
        patch::RedirectCall(0x8E3D70, OnTfTransferMarketPlayerSearchGetSearchDesc);
        patch::RedirectCall(0x8DDFF1, OnTfTransferMarketPlayerSearchIsEmpty);
        patch::RedirectCall(0x8E23F2, OnTfTransferMarketPlayerSearchReset);
        patch::RedirectCall(0x8E39A9, OnTfTransferMarketPlayerSearchReset);
        patch::RedirectCall(0xF9D646, OnTfTransferListSearchDescAssign);
        patch::RedirectCall(0x8DD848, OnTfTransferListSearchDescClear);
        patch::RedirectJump(0x1222873, OnTfTransferListSearchDescClear);
        patch::SetPointer(0x2424AC8, OnTfTransferListScreenStructLoad);
        patch::SetPointer(0x2424ACC, OnTfTransferListScreenStructSave);
        patch::RedirectCall(0x927207, OnPlayerMakeSearchDescription);
        patch::RedirectCall(0x928B4F, OnPlayerMakeSearchDescription);
        patch::RedirectCall(0x928C4E, OnPlayerMakeSearchDescription);
        patch::RedirectCall(0x928E05, OnPlayerMakeSearchDescription);
        patch::RedirectJump(0x926B75, OnTfTransferListSearchDescComparePlayer);
        patch::RedirectCall(0x928B5E, OnTfTransferListSearchDescComparePlayer);
        patch::RedirectCall(0x928C5D, OnTfTransferListSearchDescComparePlayer);
        patch::RedirectCall(0x928E14, OnTfTransferListSearchDescComparePlayer);

        //patch::RedirectCall(0x11C51F3, OnGetNumTeams1);
        //patch::RedirectCall(0x11C527B, OnGetNumTeams1);
        //patch::RedirectCall(0x11C5203, OnGetTeam1);
    }
}
