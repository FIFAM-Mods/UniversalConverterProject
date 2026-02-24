#include "Translation.h"
#include "Utils.h"
#include "FifamReadWrite.h"
#include "CustomTranslation.h"
#include "shared.h"

using namespace plugin;

Array<String, 207> &CountryNames() {
    static Array<String, 207> countryNames;
    return countryNames;
}

Bool countryNamesFileRead;

String GetTranslationFileModifiedPath(String const &originalPath) {
    if (Utils::StartsWith(originalPath, L"fmdata\\")) {
        auto fileName = originalPath.substr(7);
        auto modifiedPath = Utils::Format(L"fmdata\\translation\\languages\\%s\\%s", GameLanguage().c_str(), fileName.c_str());
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
    static Path faceIDsPath = FM::GetGameDir() + L"fmdata\\faceIDs.txt";
    wcscpy(dst, faceIDsPath.c_str());
}

void OnGetClubIDsPath(WideChar *dst, WideChar const *format, WideChar const *arg) {
    static Path clubIDsPath = FM::GetGameDir() + L"fmdata\\ClubIDs.txt";
    wcscpy(dst, clubIDsPath.c_str());
}

void OnGetStadiumListPath(WideChar *dst, WideChar const *format, WideChar const *arg) {
    static Path clubIDsPath = FM::GetGameDir() + L"fmdata\\StadiumList.txt";
    wcscpy(dst, clubIDsPath.c_str());
}

struct TranslationData {
    UInt key;
    WideChar const *langs[6];
    Int field_1C;
};

struct TextContainerRange {
    TranslationData *begin;
    TranslationData *end;
};

TextContainerRange const *FindRangeByHash(TextContainerRange *ret_out, void *begin, void *end, UInt *pHash) {
    static TranslationData translationData;
    ret_out->begin = nullptr;
    ret_out->end = nullptr;
    for (UInt i = 0; i < NUM_TRANSLATION_TABLES; i++) {
        auto &table = GetTranslationTable((TranslationTableType)i);
        auto entry = table.find(*pHash);
        if (entry != table.end()) {
            translationData.key = *pHash;
            for (size_t i = 0; i < 6; i++)
                translationData.langs[i] = (*entry).second.c_str();
            ret_out->begin = &translationData;
            return ret_out;
        }
    }
    return CallAndReturn<TextContainerRange const *, 0x575560>(ret_out, begin, end, pHash);
}

void PatchTranslation(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        wchar_t gameLanguageStr[MAX_PATH];
        GetPrivateProfileStringW(L"OPTIONS", L"TEXT_LANGUAGE", L"eng", gameLanguageStr, MAX_PATH, L".\\locale.ini");
        GameLanguage() = Utils::ToLower(gameLanguageStr);
        if (!GameLanguage().empty()) {
            // TODO
            auto countryNamesFile = path(L"fmdata") / L"translation" / L"languages" / GameLanguage() / L"CountryNames.txt";
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
            LoadCustomTranslation(GameLanguage(), true);
        }
        patch::RedirectCall(0x4BF101, OnSetLocaleCurrentLanguage);
        patch::RedirectCall(0x4BF0C9, OnReadLocalisationFile);
        patch::RedirectCall(0x4BF0E0, OnReadLocalisationFile);
        patch::RedirectCall(0x4BF0F7, OnReadLocalisationFile);
        patch::RedirectJump(0x4DCEE0, OnGetCountryNameEditor);
        patch::RedirectCall(0x5100BA, OnGetFaceIDsPath);
        patch::RedirectCall(0x510141, OnGetClubIDsPath);
        patch::RedirectCall(0x548F5B, OnGetStadiumListPath);
        patch::SetPointer(0x4BE25A + 1, L"%s\\fmdata\\translation\\languages\\%s");
        patch::RedirectCall(0x57571D, FindRangeByHash);
        patch::RedirectCall(0x575774, FindRangeByHash);
        patch::SetPointer(0x4427E3 + 1, L"fmdata/translation/languages/%s/Tool-EULA.rtf");

    }
}
