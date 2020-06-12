#include "Translation.h"
#include "Utils.h"
#include "FifamReadWrite.h"
#include "license_check/license_check.h"

using namespace plugin;

String &GameLanguage() {
    static String gameLanguage;
    return gameLanguage;
}

Array<String, 207> &CountryNames() {
    static Array<String, 207> countryNames;
    return countryNames;
};

Bool countryNamesFileRead;

void METHOD OnSetCountryName(void *country, DUMMY_ARG, const WideChar *name) {
    if (countryNamesFileRead) {
        auto countryId = *raw_ptr<unsigned char>(country, 0x7C);
        if (countryId >= 1 && countryId <= 207 && !CountryNames()[countryId - 1].empty())
            name = CountryNames()[countryId - 1].c_str();
    }
    CallMethod<0xFD67C0>(country, name);
}

const WideChar *GetLocaleShortName(UInt) {
    if (!GameLanguage().empty()) {
        auto translationFile = path(L"fmdata") / GameLanguage() / L"Translations.huf";
        if (exists(translationFile))
            return GameLanguage().c_str();
    }
    return L"eng";
}

const WideChar * METHOD GetCurrentLocaleShortName() {
    return GetLocaleShortName(0);
}

template<UInt Addr>
void METHOD OnReadAdditionalTranslationFile(void *t, DUMMY_ARG, const WideChar *filepath) {
    if (!GameLanguage().empty()) {
        String filename = Path(filepath).filename().c_str();
        String newFilepath = L"fmdata\\" + GameLanguage() + L"\\" + filename;
        if (exists(newFilepath))
            CallMethod<Addr>(t, newFilepath.c_str());
    }
    CallMethod<Addr>(t, filepath);
}

void PatchTranslation(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        wchar_t gameLanguageStr[MAX_PATH];
        GetPrivateProfileStringW(Magic<'O', 'P', 'T', 'I', 'O', 'N', 'S'>(1224534890).c_str(), Magic<'T', 'E', 'X', 'T', '_', 'L', 'A', 'N', 'G', 'U', 'A', 'G', 'E'>(3562105574).c_str(), Magic<'e', 'n', 'g'>(3703889367).c_str(), gameLanguageStr, MAX_PATH, Magic<'.', '\\', 'l', 'o', 'c', 'a', 'l', 'e', '.', 'i', 'n', 'i'>(2393442148).c_str());
        GameLanguage() = Utils::ToLower(gameLanguageStr);
        if (!GameLanguage().empty()) {
            auto countryNamesFile = path(L"fmdata") / GameLanguage() / L"CountryNames.txt";
            if (exists(countryNamesFile)) {
                FifamReader countryNamesReader(countryNamesFile, 14);
                if (countryNamesReader.Available()) {
                    countryNamesFileRead = true;
                    int i = 0;
                    while (!countryNamesReader.IsEof()) {
                        countryNamesReader.ReadLine(CountryNames()[i++]);
                        if (i == 207)
                            break;
                    }
                }
            }
        }
        patch::RedirectCall(0xF9720D, OnSetCountryName);
        patch::RedirectCall(0x108F723, OnSetCountryName);
        patch::RedirectJump(0x14A950B, GetLocaleShortName);
        patch::RedirectCall(0x111920A, OnReadAdditionalTranslationFile<0x13B5820>);
        patch::RedirectCall(0x111924A, OnReadAdditionalTranslationFile<0x13B6C90>);
        patch::SetPointer(0x23AB37C, GetCurrentLocaleShortName);
    }
}
