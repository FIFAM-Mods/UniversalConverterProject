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
}

Bool countryNamesFileRead;

String GetTranslationFileModifiedPath(String const &originalPath) {
    if (Utils::StartsWith(originalPath, Magic<'f', 'm', 'd', 'a', 't', 'a', '\\'>(3512122331))) {
        auto fileName = originalPath.substr(7);
        auto modifiedPath = Utils::Format(Magic<'f', 'm', 'd', 'a', 't', 'a', '\\', '%', 's', '\\', '%', 's'>(2333809814), GameLanguage().c_str(), fileName.c_str());
        if (exists(modifiedPath))
            return modifiedPath;
    }
    return originalPath;
}

void METHOD OnReadLocalisationFile(void *loc, DUMMY_ARG, WideChar const *originalPath, Int flags) {
    auto modified = GetTranslationFileModifiedPath(originalPath);
    CallMethod<0x577A70>(loc, modified.c_str(), flags);
}

void METHOD OnSetLocaleCurrentLanguage(void *loc, DUMMY_ARG, Int langId) {
    CallMethod<0x574700>(loc, langId);
    static auto filepath = Magic<'f', 'm', 'd', 'a', 't', 'a', '\\', 'D', 'a', 't', 'a', 'b', 'a', 's', 'e', 'E', 'd', 'i', 't', 'o', 'r', '.', 'd', 'e', '.', 't', 'x', 't'>(2498057817);
    OnReadLocalisationFile(loc, 0, filepath.c_str(), 8);
}

void METHOD OnSetCountryName(void *country, DUMMY_ARG, const WideChar *name) {
    if (countryNamesFileRead) {
        auto countryId = *raw_ptr<unsigned char>(country, 0x7C);
        if (countryId >= 1 && countryId <= 207 && !CountryNames()[countryId - 1].empty())
            name = CountryNames()[countryId - 1].c_str();
    }
    CallMethod<0xFD67C0>(country, name);
}

void METHOD OnSetCountryNameEditor(void *country, DUMMY_ARG, Int langId, const WideChar *name) {
    if (countryNamesFileRead) {
        auto countryId = *raw_ptr<Int>(country, 0x1A4);
        if (countryId >= 1 && countryId <= 207 && !CountryNames()[countryId - 1].empty())
            name = CountryNames()[countryId - 1].c_str();
    }
    CallMethod<0x4DCE80>(country, langId, name);
}

WideChar const *METHOD OnGetCountryNameEditor(void *country) {
    auto countryId = *raw_ptr<Int>(country, 0x1A4);
    if (countryId >= 1 && countryId <= 207 && !CountryNames()[countryId - 1].empty())
        return CountryNames()[countryId - 1].c_str();
    Int currLang = CallMethodAndReturn<Int, 0x4C9F20>(*raw_ptr<void *>(country, 0));
    return (WideChar const *)((UInt)country + 4 + 60 * currLang);
}

void PatchTranslation(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        wchar_t gameLanguageStr[MAX_PATH];
        GetPrivateProfileStringW(Magic<'O', 'P', 'T', 'I', 'O', 'N', 'S'>(1224534890).c_str(), Magic<'T', 'E', 'X', 'T', '_', 'L', 'A', 'N', 'G', 'U', 'A', 'G', 'E'>(3562105574).c_str(), Magic<'e', 'n', 'g'>(3703889367).c_str(), gameLanguageStr, MAX_PATH, Magic<'.', '\\', 'l', 'o', 'c', 'a', 'l', 'e', '.', 'i', 'n', 'i'>(2393442148).c_str());
        GameLanguage() = Utils::ToLower(gameLanguageStr);
        if (!GameLanguage().empty()) {
            // TODO
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
        patch::RedirectCall(0x4BF101, OnSetLocaleCurrentLanguage);
        patch::RedirectCall(0x4BF0C9, OnReadLocalisationFile);
        patch::RedirectCall(0x4BF0E0, OnReadLocalisationFile);
        patch::RedirectCall(0x4BF0F7, OnReadLocalisationFile);
        patch::RedirectJump(0x4DCEE0, OnGetCountryNameEditor);
    }
}
