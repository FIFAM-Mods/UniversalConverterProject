#include "DatabaseName.h"

using namespace plugin;

void SetDatabaseName(wchar_t *dst, wchar_t const *a, wchar_t const *b) {
    static wchar_t dbname[MAX_PATH];
    wcscpy(dbname, b);
    wcscpy(&wcsrchr(dbname, L'\\')[1], L"fmdata\\UniversalConverterProjectDatabase.ucpdb");
    wcscpy(dst, dbname);
}

void PatchDatabaseName(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        patch::SetUInt(0x5504D5 + 3, 0x20190001);
        patch::RedirectCall(0x4D9854, SetDatabaseName);
    }
    else if (v.id() == ID_ED_11_1003) {
        patch::SetUInt(0x50CCB1 + 3, 0x20190001);
        patch::RedirectCall(0x4B13C0, SetDatabaseName);
    }
}
