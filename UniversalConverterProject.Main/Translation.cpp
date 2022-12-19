﻿#include "Translation.h"
#include "GameInterfaces.h"
#include "Utils.h"
#include "FifamReadWrite.h"
#include "license_check/license_check.h"

using namespace plugin;

String &GameLanguage() {
    static String gameLanguage;
    return gameLanguage;
}

Bool IsRussianLanguage = false;
Bool IsUkrainianLanguage = false;

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

void *FormatNumberTh(void *t, UInt value, Bool masculine) {
    String s;
    if (value) {
        s = Format(L"%d", value);
        if (IsRussianLanguage) {
            if (masculine)
                s += L"-й";
            else
                s += L"-я";
        }
        else if (IsUkrainianLanguage) {
            if (masculine)
                s += L"-й";
            else {
                UInt m10 = value % 10;
                UInt m100 = value % 100;
                if (m10 == 3 && m100 != 13)
                    s += L"-я";
                else
                    s += L"-а";
            }
        }
        else {
            UInt metric = GetCurrentMetric();
            if (metric == METRIC_FRANCE) {
                if (value == 1) {
                    if (masculine)
                        s += L"er";
                    else
                        s += L"re";
                }
                else
                    s += L"e";
            }
            else if (metric == METRIC_SPAIN || metric == METRIC_ITALY) {
                if (masculine)
                    s += L"º";
                else
                    s += L"ª";
            }
            else if (metric == METRIC_UNITED_KINGDOM) {
                UInt m10 = value % 10;
                UInt m100 = value % 100;
                if (m10 == 1 && m100 != 11)
                    s += L"st";
                else if (m10 == 2 && m100 != 12)
                    s += L"nd";
                else if (m10 == 3 && m100 != 13)
                    s += L"rd";
                else
                    s += L"th";
            }
            else
                s += L".";
        }
    }
    else
        s = L"-";
    CallMethod<0x14978B3>(t, s.c_str());
    return t;
}

void PatchTranslation(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        wchar_t gameLanguageStr[MAX_PATH];
        GetPrivateProfileStringW(Magic<'O', 'P', 'T', 'I', 'O', 'N', 'S'>(1224534890).c_str(), Magic<'T', 'E', 'X', 'T', '_', 'L', 'A', 'N', 'G', 'U', 'A', 'G', 'E'>(3562105574).c_str(), Magic<'e', 'n', 'g'>(3703889367).c_str(), gameLanguageStr, MAX_PATH, Magic<'.', '\\', 'l', 'o', 'c', 'a', 'l', 'e', '.', 'i', 'n', 'i'>(2393442148).c_str());
        GameLanguage() = Utils::ToLower(gameLanguageStr);
        if (!GameLanguage().empty()) {
            IsRussianLanguage = GameLanguage() == L"rus";
            IsUkrainianLanguage = GameLanguage() == L"ukr";
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

        patch::RedirectJump(0x14AD82F, FormatNumberTh);
    }
}
