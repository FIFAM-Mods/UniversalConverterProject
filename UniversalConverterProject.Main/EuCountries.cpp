#include "EuCountries.h"

using namespace plugin;

unsigned char euCountryFlags[] = {
3 , // 
0 , // Albania
0 , // Andorra
1 , // Armenia
0 , // Austria
1 , // Azerbaijan
1 , // Belarus
0 , // Belgium
0 , // Bosnia-Herzegovina
0 , // Bulgaria
0 , // Croatia
0 , // Cyprus
0 , // Czech Republic
0 , // Denmark
1 , // England
0 , // Estonia
0 , // Faroe Islands
0 , // Finland
0 , // France
1 , // FYR Macedonia
1 , // Georgia
0 , // Germany
0 , // Greece
0 , // Hungary
0 , // Iceland
0 , // Ireland
1 , // Israel
0 , // Italy
0 , // Latvia
0 , // Liechtenstein
0 , // Lithuania
0 , // Luxembourg
0 , // Malta
1 , // Moldova
0 , // Netherlands
1 , // Northern Ireland
0 , // Norway
0 , // Poland
0 , // Portugal
0 , // Romania
1 , // Russia
0 , // San Marino
1 , // Scotland
0 , // Slovakia
0 , // Slovenia
0 , // Spain
0 , // Sweden
0 , // Switzerland
1 , // Turkey
1 , // Ukraine
1 , // Wales
0 , // Serbia
3 , // Argentina
3 , // Bolivia
3 , // Brazil
3 , // Chile
3 , // Colombia
3 , // Ecuador
3 , // Paraguay
3 , // Peru
3 , // Uruguay
3 , // Venezuela
3 , // Anguilla
3 , // Antigua and Barbuda
3 , // Aruba
3 , // Bahamas
3 , // Barbados
3 , // Belize
3 , // Bermuda
3 , // British Virgin Is.
3 , // Canada
3 , // Cayman Islands
3 , // Costa Rica
3 , // Cuba
3 , // Dominica
3 , // Dominican Republic
3 , // El Salvador
3 , // Grenada
3 , // Guatemala
3 , // Guyana
3 , // Haiti
3 , // Honduras
3 , // Jamaica
3 , // Mexico
3 , // Montserrat
3 , // Curaçao
3 , // Nicaragua
3 , // Panama
3 , // Puerto Rico
3 , // St. Kitts & Nevis
3 , // St. Lucia
3 , // St. Vincent & Gren.
3 , // Surinam
3 , // Trinidad & Tobago
3 , // Turks and Caicos
3 , // United States
3 , // US Virgin Islands
3 , // Algeria
3 , // Angola
3 , // Benin
3 , // Botswana
3 , // Burkina Faso
3 , // Burundi
3 , // Cameroon
3 , // Cape Verde Islands
3 , // Central African Rep.
3 , // Chad
3 , // Congo
3 , // Cote d' Ivoire
3 , // Djibouti
3 , // DR Congo
3 , // Egypt
3 , // Equatorial Guinea
3 , // Eritrea
3 , // Ethiopia
3 , // Gabon
3 , // Gambia
3 , // Ghana
3 , // Guinea
3 , // Guinea-Bissau
3 , // Kenya
3 , // Lesotho
3 , // Liberia
3 , // Libya
3 , // Madagascar
3 , // Malawi
3 , // Mali
3 , // Mauritania
3 , // Mauritius
3 , // Morocco
3 , // Mozambique
3 , // Namibia
3 , // Niger
3 , // Nigeria
3 , // Rwanda
3 , // Sao Tome e Principe
3 , // Senegal
3 , // Seychelles
3 , // Sierra Leone
3 , // Somalia
3 , // South Africa
3 , // Sudan
3 , // Swaziland
3 , // Tanzania
3 , // Togo
3 , // Tunisia
3 , // Uganda
3 , // Zambia
3 , // Zimbabwe
3 , // Afghanistan
3 , // Bahrain
3 , // Bangladesh
3 , // Bhutan
3 , // Brunei Darussalam
3 , // Cambodia
3 , // China PR
3 , // Taiwan
3 , // Guam
3 , // Hong Kong
3 , // India
3 , // Indonesia
3 , // Iran
3 , // Iraq
3 , // Japan
3 , // Jordan
1 , // Kazakhstan
3 , // Korea DPR
3 , // Korea Republic
3 , // Kuwait
3 , // Kyrgyzstan
3 , // Laos
3 , // Lebanon
3 , // Macao
3 , // Malaysia
3 , // Maldives
3 , // Mongolia
3 , // Myanmar
3 , // Nepal
3 , // Oman
3 , // Pakistan
3 , // Palestinian Authority
3 , // Philippines
3 , // Qatar
3 , // Saudi Arabia
3 , // Singapore
3 , // Sri Lanka
3 , // Syria
3 , // Tajikistan
3 , // Thailand
3 , // Turkmenistan
3 , // United Arab Emirates
3 , // Uzbekistan
3 , // Vietnam
3 , // Yemen
3 , // American Samoa
3 , // Australia
3 , // Cook Islands
3 , // Fiji
3 , // New Zealand
3 , // Papua New Guinea
3 , // Samoa
3 , // Solomon Islands
3 , // Tahiti
3 , // Tonga
3 , // Vanuatu
0 , // Gibraltar
0 , // Montenegro
1   // Kosovo
};

void PatchEuCountries(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetPointer(0xFD8A1F + 2, euCountryFlags);
        patch::SetPointer(0xFD8A3F + 2, euCountryFlags);
        patch::SetPointer(0xFD8A6F + 2, euCountryFlags);
    }
}
