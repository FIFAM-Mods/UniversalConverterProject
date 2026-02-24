#include "Translation.h"
#include "GameInterfaces.h"
#include "Utils.h"
#include "FifamReadWrite.h"
#include "CustomTranslation.h"
#include "shared.h"

using namespace plugin;

Bool IsRussianLanguage = false;
Bool IsUkrainianLanguage = false;
eCurrency DefaultCurrency = CURRENCY_EUR;
Bool IsDefaultImperialUnits = false;

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

String GetLocaleFilename() {
    return L"fmdata\\translation\\languages\\" + GameLanguage() + L"\\Translations.huf";
}

const WideChar *GetLocaleShortName(UInt) {
    static wchar_t localeShortName[32];
    wcscpy(localeShortName, GameLanguage().c_str());
    return localeShortName;
}

const WideChar * METHOD GetCurrentLocaleShortName() {
    return GetLocaleShortName(0);
}

template<UInt Addr>
void METHOD OnReadAdditionalTranslationFile(void *t, DUMMY_ARG, const WideChar *filepath) {
    if (!GameLanguage().empty()) {
        String filename = Path(filepath).filename().c_str();
        String newFilepath = L"fmdata\\translation\\languages\\" + GameLanguage() + L"\\" + filename;
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

void GetWeekDayNameForCustomLanguage(WideChar const *&out) {
    if (IsRussianLanguage) {
        if (!wcscmp(out, L"Mo"))
            out = L"Пн";
        else if (!wcscmp(out, L"Tu"))
            out = L"Вт";
        else if (!wcscmp(out, L"We"))
            out = L"Ср";
        else if (!wcscmp(out, L"Th"))
            out = L"Чт";
        else if (!wcscmp(out, L"Fr"))
            out = L"Пт";
        else if (!wcscmp(out, L"Sa"))
            out = L"Сб";
        else if (!wcscmp(out, L"Su"))
            out = L"Вс";
    }
    else if (IsUkrainianLanguage) {
        if (!wcscmp(out, L"Mo"))
            out = L"Пн";
        else if (!wcscmp(out, L"Tu"))
            out = L"Вт";
        else if (!wcscmp(out, L"We"))
            out = L"Ср";
        else if (!wcscmp(out, L"Th"))
            out = L"Чт";
        else if (!wcscmp(out, L"Fr"))
            out = L"Пт";
        else if (!wcscmp(out, L"Sa"))
            out = L"Сб";
        else if (!wcscmp(out, L"Su"))
            out = L"Нд";
    }
}

void OnFormatDateStr_222(WideChar *dst, WideChar const *format, Int d, Int m, Int y) {
    if (IsRussianLanguage || IsUkrainianLanguage)
        Call<0x1494136>(dst, L"%02d.%02d.%02d", d, m, y);
    else
        Call<0x1494136>(dst, format, d, m, y);
}

void OnCopyDateStr(WideChar *dst, WideChar const *src, UInt len) {
    GetWeekDayNameForCustomLanguage(src);
    Call<0x1493F41>(dst, src, len);
}

void OnFormatDateStr_224(WideChar *dst, WideChar const *format, Int d, Int m, Int y) {
    if (IsRussianLanguage || IsUkrainianLanguage)
        Call<0x1494136>(dst, L"%02d.%02d.%04d", d, m, y);
    else
        Call<0x1494136>(dst, format, d, m, y);
}

void OnFormatDateStr_22(WideChar *dst, WideChar const *format, Int d, Int m) {
    if (IsRussianLanguage || IsUkrainianLanguage)
        Call<0x1494136>(dst, L"%02d.%02d", d, m);
    else
        Call<0x1494136>(dst, format, d, m);
}

// not sure about calling convention
void OnSprintfDateStr(void *str, WideChar const *format, WideChar const *a, WideChar const *b) {
    GetWeekDayNameForCustomLanguage(a);
    Call<0x1497B06>(str, format, a, b);
}

void ProcessStringUkrainianLanguage(String &str) {
    static WideChar table[] =
    { 0,  0,  0,  0,  8,  0,  12, 13, 0,  0,  0,  0,  0,  0,  0,  0,
      1,  2,  3,  4,  6,  7,  9,  10, 11, 14, 15, 16, 17, 18, 19, 20,
      21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  0,  31, 0,  32, 33,
      34, 35, 36, 37, 39, 40, 42, 43, 44, 47, 48, 49, 50, 51, 52, 53,
      54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0,  0,  64, 0,  65, 66,
      0,  0,  0,  0,  41, 0,  45, 46, 0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      5,  38 };
    for (auto &c : str) {
        if (c >= 0x400 && c <= 0x492) {
            c = table[c - 0x400];
            if (c == 0)
                c = L' ';
            else
                c += 0x3FF;
        }
    }
}

void ProcessStringUkrainianLanguageNoCase(String &str) {
    static WideChar table[] =
    { 0,  0,  0,  0,  8,  0,  12, 13, 0,  0,  0,  0,  0,  0,  0,  0,
      1,  2,  3,  4,  6,  7,  9,  10, 11, 14, 15, 16, 17, 18, 19, 20,
      21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  0,  31, 0,  32, 33,
      1,  2,  3,  4,  6,  7,  9,  10, 11, 14, 15, 16, 17, 18, 19, 20,
      21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  0,  31, 0,  32, 33,
      0,  0,  0,  0,  8,  0,  12, 13, 0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      5,  5 };
    for (auto &c : str) {
        if (c >= 0x400 && c <= 0x492) {
            c = table[c - 0x400];
            if (c == 0)
                c = L' ';
            else
                c += 0x3FF;
        }
    }
}

Int CompareStringNoCase(WideChar const *str1, WideChar const *str2) {
    String myStr1 = str1;
    String myStr2 = str2;
    ProcessStringUkrainianLanguageNoCase(myStr1);
    ProcessStringUkrainianLanguageNoCase(myStr2);
    return CallAndReturn<Int, 0x1580597>(myStr1.c_str(), myStr2.c_str());
}

void METHOD OnSetDefaultCurrencyRate(void *t, DUMMY_ARG, Double rate) {
    switch (DefaultCurrency) {
    case CURRENCY_EUR:
        CallMethod<0x14AB9D1>(t, 1.0);
        break;
    case CURRENCY_USD:
        CallMethod<0x14AB9D1>(t, 1.25);
        break;
    case CURRENCY_GBP:
        CallMethod<0x14AB9D1>(t, 0.8);
        break;
    case CURRENCY_PLN:
        CallMethod<0x14AB9D1>(t, 3.85);
        break;
    }
    *(UInt *)0x31264C4 = DefaultCurrency;
}

void METHOD OnSetDefaultCurrencySymbol(void *t, DUMMY_ARG, WideChar const *symbol) {
    switch (DefaultCurrency) {
    case CURRENCY_EUR:
        CallMethod<0x14AB9B2>(t, L"€");
        break;
    case CURRENCY_USD:
        CallMethod<0x14AB9B2>(t, L"$");
        break;
    case CURRENCY_GBP:
        CallMethod<0x14AB9B2>(t, L"£");
        break;
    case CURRENCY_PLN:
        CallMethod<0x14AB9B2>(t, L"zł");
        break;
    }
}

void METHOD OnSetDefaultUnits(CDBGameOptions *t, DUMMY_ARG, UInt flag, Bool set) {
    CallMethod<0x1041110>(t, flag, !IsDefaultImperialUnits);
}

Bool METHOD OnLoadTranslationsFile(void *t, DUMMY_ARG, WideChar const *filePath) {
    String translationFilePath = GetLocaleFilename();
    //::Message(L"Loading translation from " + translationFilePath);
    Bool result = CallMethodAndReturn<Bool, 0x14A9811>(t, translationFilePath.c_str());
    LoadCustomTranslation(GameLanguage(), false, [](Path const &filename) {
        SafeLog::Write(L"Loading custom translation file: " + filename.wstring());
    });
    return result;
}

WideChar const *METHOD GetGameTextByHashKey(void *t, DUMMY_ARG, UInt key) {
    for (UInt i = 0; i < NUM_TRANSLATION_TABLES; i++) {
        auto &table = GetTranslationTable((TranslationTableType)i);
        auto entry = table.find(key);
        if (entry != table.end())
            return (*entry).second.c_str();
    }
    return CallMethodAndReturn<wchar_t const *, 0x14A9671>(t, key);
}

void PatchTranslation(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        auto iniPath = FM::GameDirPath(L"locale.ini");
        GameLanguage() = GetIniOption(L"OPTIONS", L"TEXT_LANGUAGE", L"eng", iniPath);
        if (!GameLanguage().empty()) {
            IsRussianLanguage = GameLanguage() == L"rus";
            IsUkrainianLanguage = GameLanguage() == L"ukr";
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
        }
        else
            GameLanguage() = L"eng";
        String currency = GetIniOption(L"OPTIONS", L"LANGUAGE_CURRENCY", L"", iniPath);
        if (currency == L"usd")
            DefaultCurrency = CURRENCY_USD;
        else if (currency == L"gbp")
            DefaultCurrency = CURRENCY_GBP;
        else if (currency == L"pln")
            DefaultCurrency = CURRENCY_PLN;
        else
            DefaultCurrency = CURRENCY_EUR;
        String units = GetIniOption(L"OPTIONS", L"LANGUAGE_UNITS", L"", iniPath);
        IsDefaultImperialUnits = units == L"imperial";

        patch::RedirectCall(0xF9720D, OnSetCountryName);
        patch::RedirectCall(0x108F723, OnSetCountryName);
        patch::RedirectJump(0x14A950B, GetLocaleShortName);
        patch::RedirectCall(0x111920A, OnReadAdditionalTranslationFile<0x13B5820>);
        patch::RedirectCall(0x111924A, OnReadAdditionalTranslationFile<0x13B6C90>);
        patch::SetPointer(0x23AB37C, GetCurrentLocaleShortName);

        patch::RedirectJump(0x14AD82F, FormatNumberTh);

        patch::RedirectCall(0x14951FE, OnFormatDateStr_222);
        patch::RedirectCall(0x1495433, OnFormatDateStr_224);
        patch::RedirectCall(0x1495654, OnFormatDateStr_22);
        patch::RedirectCall(0x1495174, OnCopyDateStr);
        patch::RedirectCall(0x1495394, OnCopyDateStr);
        patch::RedirectCall(0x14955D0, OnCopyDateStr);
        //patch::RedirectCall(0x1495B8B, OnSprintfDateStr);

        if (IsUkrainianLanguage)
            patch::RedirectCall(0x1493FD4, CompareStringNoCase);

        patch::RedirectCall(0xF65591, OnSetDefaultCurrencyRate);
        patch::RedirectCall(0xF655D2, OnSetDefaultCurrencyRate);
        patch::RedirectCall(0xF6560B, OnSetDefaultCurrencyRate);
        patch::RedirectCall(0xF6559E, OnSetDefaultCurrencySymbol);
        patch::RedirectCall(0xF65619, OnSetDefaultCurrencySymbol);
        patch::RedirectCall(0xF65624, OnSetDefaultUnits);

        patch::RedirectCall(0x14A9C1C, OnLoadTranslationsFile);
        patch::RedirectCall(0x14A9C43, OnLoadTranslationsFile);

        patch::RedirectCall(0x14A9AF8, GetGameTextByHashKey);
        patch::RedirectCall(0x14A9B9B, GetGameTextByHashKey);
        patch::RedirectCall(0x14A9BB8, GetGameTextByHashKey);

        patch::SetPointer(0x7B979B + 1, L"fmdata\\Credits.txt");
        patch::SetPointer(0x12F9883 + 1, L"fmdata/translation/languages/%s/Bedingungen.txt");
        patch::SetPointer(0x4DA4DE + 1, L"fmdata\\translation\\languages\\"); // language folder (type 12) in CFMResolver::GetGlobalPath()
    }
}
