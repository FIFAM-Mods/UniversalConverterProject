#include "Translation.h"
#include "Utils.h"
#include "FifamReadWrite.h"
#include "CustomTranslation.h"
#include "TranslationShared.h"
#include "TextFileTable.h"
#include "Editor.h"
#include "shared.h"

using namespace plugin;

using CountryNamesArray = Array<Array<String, 207>, NUM_CUSTOM_TRANSLATION_LANGUAGES>;

CountryNamesArray &CountryNames() {
    static CountryNamesArray countryNames;
    return countryNames;
}

Int CustomLanguageIndex = -1;

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
    if (CustomLanguageIndex != -1) {
        auto countryId = *raw_ptr<unsigned char>(country, 0x7C);
        if (countryId >= 1 && countryId <= 207)
            name = CountryNames()[CustomLanguageIndex][countryId - 1].c_str();
    }
    CallMethod<0xFD67C0>(country, name);
}

void METHOD OnSetCountryNameEditor(void *country, DUMMY_ARG, Int langId, const WideChar *name) {
    if (CustomLanguageIndex != -1) {
        auto countryId = *raw_ptr<Int>(country, 0x1A4);
        if (countryId >= 1 && countryId <= 207)
            name = CountryNames()[CustomLanguageIndex][countryId - 1].c_str();
    }
    CallMethod<0x4DCE80>(country, langId, name);
}

WideChar const *METHOD OnGetCountryNameEditor(void *country) {
    if (CustomLanguageIndex != -1) {
        auto countryId = *raw_ptr<Int>(country, 0x1A4);
        if (countryId >= 1 && countryId <= 207)
            return CountryNames()[CustomLanguageIndex][countryId - 1].c_str();
    }
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

void LoadCustomCountryNames(Path const &dbFolder) {
    TextFileTable file;
    Path fileName = dbFolder / "CountryNames.txt";
    if (!file.ReadUnicodeText(fileName))
        return; // failed to read file
    if (file.Rows().size() > 1) {
        for (size_t i = 1; i < file.Rows().size(); i++) {
            if (file.Rows()[i - 1].size() != file.Rows()[i].size()) {
                ::Error(L"Error in CountryNames file: " + fileName.wstring() + Utils::Format(L"at row %u (%u/%u)", i,
                    file.Rows()[i - 1].size(), file.Rows()[i].size()));
                return;
            }
        }
    }
    if (!file.IsConsistent() || file.NumRows() < 1 || file.NumColumns(0) < 2)
        return; // wrong file format
    if (file.NumRows() < 2)
        return; // empty file
    auto header = file.Row(0);
    Int languageColumnIndexes[NUM_CUSTOM_TRANSLATION_LANGUAGES] = {};
    for (UInt i = 0; i < NUM_CUSTOM_TRANSLATION_LANGUAGES; i++)
        languageColumnIndexes[i] = -1;
    for (UInt columnIndex = 1; columnIndex < header.size(); columnIndex++) {
        auto columnName = ToLower(header[columnIndex]);
        Int languageId = GetTranslationLanguageID(columnName);
        if (languageId != -1 && IsCustomTranslationLanguage(languageId)) {
            Int customLanguageId = TranslationLanguageIdToCustomLanguageId(languageId);
            languageColumnIndexes[customLanguageId] = columnIndex;
        }
    }
    for (UInt row = 1; row < file.NumRows(); row++) {
        String const &keyStr = file.Cell(0, row);
        if (!keyStr.empty()) {
            UInt countryId = Utils::SafeConvertInt<UInt>(keyStr);
            if (countryId >= 1 && countryId <= 207) {
                for (UInt customLanguageId = 0; customLanguageId < NUM_CUSTOM_TRANSLATION_LANGUAGES; customLanguageId++) {
                    Int colIndex = languageColumnIndexes[customLanguageId];
                    if (colIndex != -1) {
                        String name = file.Cell(colIndex, row);
                        if (customLanguageId == TranslationLanguageIdToCustomLanguageId(TRANSLATIONLANGUAGE_CHI))
                            AddSpacesBetweenCJK(name);
                        if (name.size() > 29)
                            name.resize(29);
                        CountryNames()[customLanguageId][countryId - 1] = name;
                    }
                }
            }
        }
    }
}

void METHOD WriteCountryNameToMasterDatabase(void *file, DUMMY_ARG, UInt iterationIndex, WideChar const *name) {
    UInt languageId = NUM_TRANSLATION_LANGUAGES - iterationIndex;
    if (IsDefaultTranslationLanguage(languageId))
        BinaryFileWriteString(file, name + languageId * 30);
    else if (IsCustomTranslationLanguage(languageId)) {
        void *country = (void *)((UInt)name - 4);
        UInt countryId = *raw_ptr<UInt>(country, 0x1A4);
        Int customLanguageId = TranslationLanguageIdToCustomLanguageId(languageId);
        BinaryFileWriteString(file, CountryNames()[customLanguageId][countryId - 1].c_str());
    }
    else
        BinaryFileWriteString(file, L"");
}

void PatchTranslation(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        wchar_t gameLanguageStr[MAX_PATH];
        GetPrivateProfileStringW(L"OPTIONS", L"TEXT_LANGUAGE", L"eng", gameLanguageStr, MAX_PATH, L".\\locale.ini");
        GameLanguage() = Utils::ToLower(gameLanguageStr);
        CustomLanguageIndex = -1;
        if (!GameLanguage().empty()) {
            LoadCustomTranslation(GameLanguage(), true);
            Int languageId = GetTranslationLanguageID(GameLanguage());
            if (languageId != -1 && IsCustomTranslationLanguage(languageId))
                CustomLanguageIndex = TranslationLanguageIdToCustomLanguageId(languageId);
        }
        patch::RedirectCall(0x4BF101, OnSetLocaleCurrentLanguage);
        patch::RedirectCall(0x4BF0C9, OnReadLocalisationFile);
        patch::RedirectCall(0x4BF0E0, OnReadLocalisationFile);
        patch::RedirectCall(0x4BF0F7, OnReadLocalisationFile);
        patch::RedirectJump(0x4DCEE0, OnGetCountryNameEditor);
        patch::SetUInt(0x4E3816 + 1, NUM_TRANSLATION_LANGUAGES);
        patch::SetUChar(0x4E3823, 0x57); // push edi
        patch::RedirectCall(0x4E3823 + 1, WriteCountryNameToMasterDatabase);
        patch::Nop(0x4E3823 + 1 + 5, 2);
        patch::RedirectCall(0x5100BA, OnGetFaceIDsPath);
        patch::RedirectCall(0x510141, OnGetClubIDsPath);
        patch::RedirectCall(0x548F5B, OnGetStadiumListPath);
        patch::SetPointer(0x4BE25A + 1, L"%s\\fmdata\\translation\\languages\\%s");
        patch::RedirectCall(0x57571D, FindRangeByHash);
        patch::RedirectCall(0x575774, FindRangeByHash);
        patch::SetPointer(0x4427E3 + 1, L"fmdata/translation/languages/%s/Tool-EULA.rtf");

    }
}
