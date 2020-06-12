#include "Translation.h"
#include "Utils.h"
#include "FifamReadWrite.h"

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
    if (Utils::StartsWith(originalPath, L"fmdata\\")) {
        auto fileName = originalPath.substr(7);
        auto modifiedPath = Utils::Format(L"fmdata\\%s\\%s", GameLanguage().c_str(), fileName.c_str());
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
    OnReadLocalisationFile(loc, 0, L"fmdata\\DatabaseEditor.de.txt", 8);
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

void OnGetFaceIDsPath(WideChar *dst, WideChar const *format, WideChar const *arg) {
    static Path faceIDsPath = FM::GetGameDir() + L"fmdata\\eng\\faceIDs.txt";
    wcscpy(dst, faceIDsPath.c_str());
}

void OnGetClubIDsPath(WideChar *dst, WideChar const *format, WideChar const *arg) {
    static Path clubIDsPath = FM::GetGameDir() + L"fmdata\\eng\\ClubIDs.txt";
    wcscpy(dst, clubIDsPath.c_str());
}

void OnGetStadiumListPath(WideChar *dst, WideChar const *format, WideChar const *arg) {
    static Path clubIDsPath = FM::GetGameDir() + L"fmdata\\eng\\StadiumList.txt";
    wcscpy(dst, clubIDsPath.c_str());
}

void PatchTranslation(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        wchar_t gameLanguageStr[MAX_PATH];
        GetPrivateProfileStringW(L"OPTIONS", L"TEXT_LANGUAGE", L"eng", gameLanguageStr, MAX_PATH, L".\\locale.ini");
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
        patch::RedirectCall(0x5100BA, OnGetFaceIDsPath);
        patch::RedirectCall(0x510141, OnGetClubIDsPath);
        patch::RedirectCall(0x548F5B, OnGetStadiumListPath);
    }
}
