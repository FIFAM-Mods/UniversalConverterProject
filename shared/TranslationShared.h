#pragma once
#include "plugin.h"
#include "FifamTypes.h"

// Note: adding new language will require database version change
const Int NUM_DEFAULT_TRANSLATION_LANGUAGES = 6;
const Int NUM_CUSTOM_TRANSLATION_LANGUAGES = 8;
const Int NUM_TRANSLATION_LANGUAGES = NUM_DEFAULT_TRANSLATION_LANGUAGES + NUM_CUSTOM_TRANSLATION_LANGUAGES;
extern const Char *TranslationLanguages[NUM_TRANSLATION_LANGUAGES];
extern const WideChar *TranslationLanguagesW[NUM_TRANSLATION_LANGUAGES];

enum TranslationLanguageID {
    TRANSLATIONLANGUAGE_ENG = 0,
    TRANSLATIONLANGUAGE_FRE = 1,
    TRANSLATIONLANGUAGE_GER = 2,
    TRANSLATIONLANGUAGE_ITA = 3,
    TRANSLATIONLANGUAGE_SPA = 4,
    TRANSLATIONLANGUAGE_POL = 5,
    TRANSLATIONLANGUAGE_RUS = 6,
    TRANSLATIONLANGUAGE_UKR = 7,
    TRANSLATIONLANGUAGE_CZE = 8,
    TRANSLATIONLANGUAGE_HUN = 9,
    TRANSLATIONLANGUAGE_POR = 10,
    TRANSLATIONLANGUAGE_TUR = 11,
    TRANSLATIONLANGUAGE_KOR = 12,
    TRANSLATIONLANGUAGE_CHI = 13
};

Bool IsDefaultTranslationLanguage(Int langID);
Bool IsCustomTranslationLanguage(Int langID);
Int TranslationLanguageIdToCustomLanguageId(Int langID);
Char const *GetTranslationLanguageCode(Int langID);
WideChar const *GetTranslationLanguageCodeW(Int langID);
Int GetTranslationLanguageID(StringA const &name);
Int GetTranslationLanguageID(String const &name);
