#include "TranslationShared.h"

const Char *TranslationLanguages[NUM_TRANSLATION_LANGUAGES] = {
    "eng", "fre", "ger", "ita", "spa", "pol", "rus", "ukr", "cze", "hun", "por", "tur", "kor", "chi"
};

const WideChar *TranslationLanguagesW[NUM_TRANSLATION_LANGUAGES] = {
    L"eng", L"fre", L"ger", L"ita", L"spa", L"pol", L"rus", L"ukr", L"cze", L"hun", L"por", L"tur", L"kor", L"chi"
};

Bool IsDefaultTranslationLanguage(Int langID) {
    return langID >= 0 && langID < NUM_DEFAULT_TRANSLATION_LANGUAGES;
}

Bool IsCustomTranslationLanguage(Int langID) {
    return langID >= NUM_DEFAULT_TRANSLATION_LANGUAGES && langID < NUM_TRANSLATION_LANGUAGES;
}

Int TranslationLanguageIdToCustomLanguageId(Int langID) {
    if (IsCustomTranslationLanguage(langID))
        return langID - NUM_DEFAULT_TRANSLATION_LANGUAGES;
    return -1;
}

Char const *GetTranslationLanguageCode(Int langID) {
    if (langID >= 0 && langID < NUM_TRANSLATION_LANGUAGES)
        return TranslationLanguages[langID];
    return "";
}

WideChar const *GetTranslationLanguageCodeW(Int langID) {
    if (langID >= 0 && langID < NUM_TRANSLATION_LANGUAGES)
        return TranslationLanguagesW[langID];
    return L"";
}

Int GetTranslationLanguageID(StringA const &name) {
    for (Int i = 0; i < NUM_TRANSLATION_LANGUAGES; i++) {
        if (name == TranslationLanguages[i])
            return i;
    }
    return -1;
}

Int GetTranslationLanguageID(String const &name) {
    for (Int i = 0; i < NUM_TRANSLATION_LANGUAGES; i++) {
        if (name == TranslationLanguagesW[i])
            return i;
    }
    return -1;
}
