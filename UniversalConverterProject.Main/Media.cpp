#include "Media.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "FifamReadWrite.h"
#include "FifamCompRegion.h"
#include "Translation.h"

using namespace plugin;

// Initialization order matters
Array<Pair<String, String>, 207> &CountryName() {
    static Array<Pair<String, String>, 207> countryName;
    return countryName;
}

Bool MediaImageExists(Path const &p) {
    Path newp = p;
    return FmFileExists(newp.replace_extension(L".tga")) ||
        FmFileExists(newp.replace_extension(L".png")) ||
        FmFileExists(newp.replace_extension(L".bmp")) ||
        FmFileExists(newp.replace_extension(L".jpg")) ||
        FmFileExists(newp.replace_extension(L".tpi"));
}

void ReadCountriesInfo() {
    Bool countriesRead = false;
    FifamReader countriesReader(L"database\\Countries.sav", 13);
    if (countriesReader.Available()) {
        auto &reader = countriesReader;
        UInt countriesVer = 0;
        auto firstLine = reader.ReadFullLine();
        if (Utils::StartsWith(firstLine, L"Countries Version: ")) {
            auto verParams = Utils::Split(firstLine, ' ', false);
            if (verParams.size() >= 3)
                countriesVer = Utils::ToNumber(verParams[2]);
        }
        if (countriesVer >= 1 && countriesVer <= 3) {
            for (UInt i = 0; i < 207; i++) {
                FifamTrArray<String> name;
                reader.ReadLineTranslationArray(name);
                CountryName()[i].first = name[1];
                FifamTrArray<String> abbr;
                reader.ReadLineTranslationArray(abbr);
                CountryName()[i].second = abbr[1];
                if (countriesVer >= 2)
                    reader.SkipLine();
                reader.SkipLines(4);
            }
            countriesRead = true;
        }
    }
    if (!countriesRead) {
        static WideChar *defaultCountryAbbr[207] = {
            L"ALB",
            L"AND",
            L"ARM",
            L"AUT",
            L"AZE",
            L"BLR",
            L"BEL",
            L"BIH",
            L"BUL",
            L"HRV",
            L"CYP",
            L"CZE",
            L"DNK",
            L"ENG",
            L"EST",
            L"FRO",
            L"FIN",
            L"FRA",
            L"MKD",
            L"GEO",
            L"GER",
            L"GRC",
            L"HUN",
            L"ISL",
            L"IRL",
            L"ISR",
            L"ITA",
            L"LVA",
            L"LIE",
            L"LTU",
            L"LUX",
            L"MLT",
            L"MDA",
            L"NED",
            L"NIR",
            L"NOR",
            L"POL",
            L"POR",
            L"ROM",
            L"RUS",
            L"SMR",
            L"SCO",
            L"SVK",
            L"SVN",
            L"ESP",
            L"SWE",
            L"SUI",
            L"TUR",
            L"UKR",
            L"WAL",
            L"SER",
            L"ARG",
            L"BOL",
            L"BRA",
            L"CHL",
            L"COL",
            L"ECU",
            L"PRY",
            L"PER",
            L"URY",
            L"VEN",
            L"NCL",
            L"ATG",
            L"ABW",
            L"BAH",
            L"BRB",
            L"BLZ",
            L"BMU",
            L"VGB",
            L"CAN",
            L"CAY",
            L"CRI",
            L"CUB",
            L"DMA",
            L"DOM",
            L"SLV",
            L"GRD",
            L"GTM",
            L"GUY",
            L"HTI",
            L"HND",
            L"JAM",
            L"MEX",
            L"MSR",
            L"CUW",
            L"NIC",
            L"PAN",
            L"PRI",
            L"KNA",
            L"LCA",
            L"VCT",
            L"SUR",
            L"TTO",
            L"TCA",
            L"USA",
            L"VIR",
            L"ALG",
            L"AGO",
            L"BEN",
            L"BWA",
            L"BFA",
            L"BDI",
            L"CMR",
            L"CPV",
            L"CAF",
            L"TCD",
            L"CON",
            L"CIV",
            L"DJI",
            L"CDR",
            L"EGY",
            L"GNQ",
            L"ERI",
            L"ETH",
            L"GAB",
            L"GAM",
            L"GHA",
            L"GIN",
            L"GNB",
            L"KEN",
            L"LSO",
            L"LBR",
            L"LBY",
            L"MDG",
            L"MWI",
            L"MLI",
            L"MRT",
            L"MUS",
            L"MAR",
            L"MOZ",
            L"NAM",
            L"NER",
            L"NGA",
            L"RWA",
            L"STP",
            L"SEN",
            L"SYC",
            L"SLE",
            L"SOM",
            L"RSA",
            L"SDN",
            L"SWZ",
            L"TZA",
            L"TGO",
            L"TUN",
            L"UGA",
            L"ZMB",
            L"ZWE",
            L"AFG",
            L"BHR",
            L"BGD",
            L"BTN",
            L"BRN",
            L"KHM",
            L"CHN",
            L"TWN",
            L"GUM",
            L"HKG",
            L"IND",
            L"IDN",
            L"IRN",
            L"IRQ",
            L"JPN",
            L"JOR",
            L"KAZ",
            L"PRK",
            L"KOR",
            L"KWT",
            L"KGZ",
            L"LAO",
            L"LBN",
            L"MAC",
            L"MYS",
            L"MDV",
            L"MGL",
            L"MYA",
            L"NPL",
            L"OMN",
            L"PAK",
            L"PSE",
            L"PHI",
            L"QAT",
            L"KSA",
            L"SIN",
            L"SRI",
            L"SYR",
            L"TJK",
            L"THA",
            L"TKM",
            L"ARE",
            L"UZB",
            L"VIE",
            L"YEM",
            L"ASM",
            L"AUS",
            L"COK",
            L"FJI",
            L"NZL",
            L"PNG",
            L"SAM",
            L"SOL",
            L"TAH",
            L"TON",
            L"VUT",
            L"GIB",
            L"MNE",
            L"XKX"
        };
        static WideChar *defaultCountryNames[207] = {
            L"Albania",
            L"Andorra",
            L"Armenia",
            L"Austria",
            L"Azerbaijan",
            L"Belarus",
            L"Belgium",
            L"Bosnia-Herzegovina",
            L"Bulgaria",
            L"Croatia",
            L"Cyprus",
            L"Czech Republic",
            L"Denmark",
            L"England",
            L"Estonia",
            L"Faroe Islands",
            L"Finland",
            L"France",
            L"North Macedonia",
            L"Georgia",
            L"Germany",
            L"Greece",
            L"Hungary",
            L"Iceland",
            L"Ireland",
            L"Israel",
            L"Italy",
            L"Latvia",
            L"Liechtenstein",
            L"Lithuania",
            L"Luxembourg",
            L"Malta",
            L"Moldova",
            L"Netherlands",
            L"Northern Ireland",
            L"Norway",
            L"Poland",
            L"Portugal",
            L"Romania",
            L"Russia",
            L"San Marino",
            L"Scotland",
            L"Slovakia",
            L"Slovenia",
            L"Spain",
            L"Sweden",
            L"Switzerland",
            L"Turkey",
            L"Ukraine",
            L"Wales",
            L"Serbia",
            L"Argentina",
            L"Bolivia",
            L"Brazil",
            L"Chile",
            L"Colombia",
            L"Ecuador",
            L"Paraguay",
            L"Peru",
            L"Uruguay",
            L"Venezuela",
            L"New Caledonia",
            L"Antigua and Barbuda",
            L"Aruba",
            L"Bahamas",
            L"Barbados",
            L"Belize",
            L"Bermuda",
            L"British Virgin Is.",
            L"Canada",
            L"Cayman Islands",
            L"Costa Rica",
            L"Cuba",
            L"Dominica",
            L"Dominican Republic",
            L"El Salvador",
            L"Grenada",
            L"Guatemala",
            L"Guyana",
            L"Haiti",
            L"Honduras",
            L"Jamaica",
            L"Mexico",
            L"Montserrat",
            L"Curaçao",
            L"Nicaragua",
            L"Panama",
            L"Puerto Rico",
            L"St. Kitts & Nevis",
            L"St. Lucia",
            L"St. Vincent & Gren.",
            L"Surinam",
            L"Trinidad & Tobago",
            L"Turks and Caicos",
            L"United States",
            L"US Virgin Islands",
            L"Algeria",
            L"Angola",
            L"Benin",
            L"Botswana",
            L"Burkina Faso",
            L"Burundi",
            L"Cameroon",
            L"Cape Verde Islands",
            L"Central African Rep.",
            L"Chad",
            L"Congo",
            L"Cote d' Ivoire",
            L"Djibouti",
            L"DR Congo",
            L"Egypt",
            L"Equatorial Guinea",
            L"Eritrea",
            L"Ethiopia",
            L"Gabon",
            L"Gambia",
            L"Ghana",
            L"Guinea",
            L"Guinea-Bissau",
            L"Kenya",
            L"Lesotho",
            L"Liberia",
            L"Libya",
            L"Madagascar",
            L"Malawi",
            L"Mali",
            L"Mauritania",
            L"Mauritius",
            L"Morocco",
            L"Mozambique",
            L"Namibia",
            L"Niger",
            L"Nigeria",
            L"Rwanda",
            L"Sao Tome e Principe",
            L"Senegal",
            L"Seychelles",
            L"Sierra Leone",
            L"Somalia",
            L"South Africa",
            L"Sudan",
            L"Eswatini",
            L"Tanzania",
            L"Togo",
            L"Tunisia",
            L"Uganda",
            L"Zambia",
            L"Zimbabwe",
            L"Afghanistan",
            L"Bahrain",
            L"Bangladesh",
            L"Bhutan",
            L"Brunei Darussalam",
            L"Cambodia",
            L"China PR",
            L"Taiwan",
            L"Guam",
            L"Hong Kong",
            L"India",
            L"Indonesia",
            L"Iran",
            L"Iraq",
            L"Japan",
            L"Jordan",
            L"Kazakhstan",
            L"Korea DPR",
            L"Korea Republic",
            L"Kuwait",
            L"Kyrgyzstan",
            L"Laos",
            L"Lebanon",
            L"Macao",
            L"Malaysia",
            L"Maldives",
            L"Mongolia",
            L"Myanmar",
            L"Nepal",
            L"Oman",
            L"Pakistan",
            L"Palestinian Authority",
            L"Philippines",
            L"Qatar",
            L"Saudi Arabia",
            L"Singapore",
            L"Sri Lanka",
            L"Syria",
            L"Tajikistan",
            L"Thailand",
            L"Turkmenistan",
            L"United Arab Emirates",
            L"Uzbekistan",
            L"Vietnam",
            L"Yemen",
            L"American Samoa",
            L"Australia",
            L"Cook Islands",
            L"Fiji",
            L"New Zealand",
            L"Papua New Guinea",
            L"Samoa",
            L"Solomon Islands",
            L"Tahiti",
            L"Tonga",
            L"Vanuatu",
            L"Gibraltar",
            L"Montenegro",
            L"Kosovo"
        };
        for (UInt i = 0; i < 207; i++) {
            CountryName()[i].first = defaultCountryNames[i];
            CountryName()[i].second = defaultCountryAbbr[i];
        }
    }
    else {
        //FifamWriter w(L"CountryName.txt", 13, FifamVersion());
        //w.SetReplaceQuotes(false);
        //for (auto const &i : CountryName()) {
        //    w.WriteLine(L"\"" + i.first + L"\",");
        //}
    }
    for (UInt i = 0; i < 207; i++) {
        if (!CountryName()[i].first.empty())
            CountryName()[i].first = Utils::GetStringWithoutUnicodeChars(CountryName()[i].first);
        if (!CountryName()[i].second.empty())
            CountryName()[i].second = Utils::GetStringWithoutUnicodeChars(CountryName()[i].second);
    }
}

WideChar const *gpCustomMediaFolderName = nullptr;

UShort METHOD OnGetTeamCountryForMediaScreen(CDBTeam *team) {
    gpCustomMediaFolderName = nullptr;
    const UChar COUNTRY_ID_OK = 47;
    auto id = team->GetTeamID().countryId;
    if (id != COUNTRY_ID_OK && id >= 1 && id <= 207) {
        if (!CountryName()[id - 1].second.empty()) {
            String basePath = String(L"art_fm\\lib\\Media\\") + CountryName()[id - 1].second + L"\\";
            if (MediaImageExists(basePath + L"InterviewAfterTheMatch.jpg") &&
                MediaImageExists(basePath + L"InterviewBeforeMatch.jpg") &&
                MediaImageExists(basePath + L"Newspaper.jpg") &&
                MediaImageExists(basePath + L"PressConference.jpg") &&
                MediaImageExists(basePath + L"TVStudio.jpg"))
            {
                id = COUNTRY_ID_OK;
                gpCustomMediaFolderName = CountryName()[id - 1].second.c_str();
            }
        }
    }
    return id;
}

void *METHOD OnConstructMediaFolderNameForMediaScreen(void *str, DUMMY_ARG, WideChar const *folderName) {
    if (gpCustomMediaFolderName) {
        String newFolderName = String(gpCustomMediaFolderName) + L"\\";
        gpCustomMediaFolderName = nullptr;
        return CallMethodAndReturn<void *, 0x14978B3>(str, newFolderName.c_str());
    }
    return CallMethodAndReturn<void *, 0x14978B3>(str, folderName);
}

Char const *gLocFilenamePath = nullptr;
Char *gLocFilenameSubstr = nullptr;

Char *OnStoreLocFilePath(Char const *str, Char const *substr) {
    gLocFilenamePath = str;
    gLocFilenameSubstr = CallAndReturn<Char *, 0x1577960>(str, substr);
    return gLocFilenameSubstr;
}

const WideChar *METHOD OnGetFolderNameForLocFiles(void *metric) {
    static String lang_w = Utils::GetStringWithoutUnicodeChars(GameLanguage());
    static std::string lang = Utils::WtoA(lang_w);
    if (lang.size() == 3) {
        gLocFilenameSubstr[1] = lang[0];
        gLocFilenameSubstr[2] = lang[1];
        gLocFilenameSubstr[3] = lang[2];
        if (MediaImageExists(gLocFilenamePath))
            return lang_w.c_str();
    }
    return L"eng";
}

void OnSetupNewspaper(UInt countryId, Char *dst) {
    Int metricId = -1;
    switch (countryId) {
    case FifamCompRegion::Austria:
        metricId = METRIC_AUSTRIA;
        break;
    case FifamCompRegion::England:
        metricId = METRIC_UNITED_KINGDOM;
       break;
    case FifamCompRegion::France:
        metricId = METRIC_FRANCE;
       break;
    case FifamCompRegion::Germany:
        metricId = METRIC_GERMANY;
       break;
    case FifamCompRegion::Italy:
        metricId = METRIC_ITALY;
       break;
    case FifamCompRegion::Spain:
        metricId = METRIC_SPAIN;
       break;
    case FifamCompRegion::Switzerland:
        metricId = METRIC_SWITHERLAND;
    }
    if (metricId != -1) {
        void *metrics = CallAndReturn<void *, 0x14AC159>();
        void *metricInfo = CallMethodAndReturn<void *, 0x14AC29A>(metrics, metricId);
        WideChar const *metricShortName = CallMethodAndReturn<WideChar const *, 0x14ABAAB>(metricInfo);
        dst[14] = Char(metricShortName[0]);
        dst[15] = Char(metricShortName[1]);
        dst[16] = Char(metricShortName[2]);
        if (MediaImageExists(dst))
            return;
    }
    else {
        if (countryId >= 1 && countryId <= 207) {
            if (!CountryName()[countryId - 1].second.empty()) {
                auto abbr = Utils::WtoA(CountryName()[countryId - 1].second);
                if (abbr.size() == 3) {
                    dst[14] = abbr[0];
                    dst[15] = abbr[1];
                    dst[16] = abbr[2];
                    if (MediaImageExists(dst))
                        return;
                }
            }
        }
    }
    dst[14] = 'd';
    dst[15] = 'e';
    dst[16] = 'f';
}

void __declspec(naked) OnSetupNewspaperExe() {
    __asm {
        push esi
        push eax
        call OnSetupNewspaper
        add esp, 8
        mov eax, 0x4F1841
        jmp eax
    }
}

const Char *GetWebsiteScreenFileName(CDBEmployee *manager) {
    static std::string result;
    UInt countryId = CallAndReturn<UInt, 0x97DFD0>(manager);
    if (countryId >= 1 && countryId <= 207 && !CountryName()[countryId - 1].first.empty()) {
        result = "screens/13Website" + Utils::WtoA(CountryName()[countryId - 1].first) + ".xml";
        if (FmFileExists(result))
            return result.c_str();
    }
    return "screens/13WebsiteDefault.xml";
}

template<UInt ID>
Char *GetScreenNameWeekTransfers() {
    static Char buf[260];
    buf[0] = '\0';
    Char const *format = nullptr;
    switch (ID) {
    case 0:
        format = "Screens/13WeekTransfersCoachRumours%s.xml";
        break;
    case 1:
        format = "Screens/13WeekTransfersContractExtensions%s.xml";
        break;
    case 2:
        format = "Screens/13WeekTransfers%s.xml";
        break;
    case 3:
        format = "Screens/13WeekTransfersOverview%s.xml";
        break;
    case 4:
        format = "Screens/13WeekTransfersRumourMill%s.xml";
        break;
    }
    if (format) {
        UShort countryId = CallAndReturn<UShort, 0xD33ED0>();
        if (countryId >= 1 && countryId <= 207 && !CountryName()[countryId - 1].first.empty()) {
            std::string countryName = WtoA(CountryName()[countryId - 1].first);
            sprintf(buf, format, countryName.c_str());
            if (FmFileExists(buf))
                return buf;
        }
        sprintf(buf, format, "Default");
    }
    return buf;
}

void PatchMedia(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadCountriesInfo();
        patch::RedirectCall(0xA32741, OnGetTeamCountryForMediaScreen);
        patch::RedirectCall(0xA32755, OnConstructMediaFolderNameForMediaScreen);
        patch::RedirectCall(0x4F16C2, OnGetFolderNameForLocFiles);
        patch::RedirectCall(0x4F169C, OnStoreLocFilePath);
        patch::RedirectJump(0x4F17FF, OnSetupNewspaperExe);
        patch::RedirectJump(0x97E050, GetWebsiteScreenFileName);
        patch::RedirectJump(0x9348B0, GetScreenNameWeekTransfers<0>);
        patch::RedirectJump(0x935770, GetScreenNameWeekTransfers<1>);
        patch::RedirectJump(0x938B10, GetScreenNameWeekTransfers<2>);
        patch::RedirectJump(0x939CC0, GetScreenNameWeekTransfers<3>);
        patch::RedirectJump(0x93CCA0, GetScreenNameWeekTransfers<4>);
    }
}
