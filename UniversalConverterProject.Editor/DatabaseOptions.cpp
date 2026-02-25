#include "DatabaseOptions.h"
#include "FifamReadWrite.h"
#include "Editor.h"
#include "CustomTranslation.h"
#include "Translation.h"
#include "shared.h"
#include <Windows.h>

using namespace plugin;

wchar_t DatabaseID[256];
wchar_t DatabaseFolderName[256];
wchar_t DatabaseFolderPath[256];
wchar_t RestoreBigPath[256];
wchar_t RestoreBigSubPath[256];
wchar_t RestoreFileCountryDataPath[256];
wchar_t RestoreFileCountryScriptPath[256];
wchar_t RestoreFileAppearanceDefsPath[256];
wchar_t RestoreFileAssessmentPath[256];
wchar_t RestoreFileCountriesPath[256];
wchar_t RestoreFileMasterPath[256];
wchar_t RestoreFileRulesPath[256];
wchar_t RestoreFileWithoutPath[256];
wchar_t RestoreFilePriorityClubsPath[256];
wchar_t RestoreFileFemaleNamesPath[256];
wchar_t RestoreFileMaleNamesPath[256];
wchar_t RestoreFileSurnamesPath[256];
//wchar_t RestoreFileExcludeCommonNames[256];
//wchar_t RestoreFileExcludeNames[256];
//wchar_t RestoreFileExcludeSurnames[256];
//wchar_t RestoreFolderScriptPath[256];
wchar_t RestoreFolderDatabasePath[256];
wchar_t RestoreFolderDatabaseDataPath[256];
wchar_t RestoreFolderDatabaseScriptPath[256];

static WideChar const *RestoreFiles[] = {
    RestoreFileCountryDataPath,
    RestoreFileCountryScriptPath,
    RestoreFileAppearanceDefsPath,
    RestoreFileAssessmentPath,
    RestoreFileCountriesPath,
    RestoreFileMasterPath,
    RestoreFileRulesPath,
    RestoreFileWithoutPath,
    RestoreFilePriorityClubsPath,
    RestoreFileFemaleNamesPath,
    RestoreFileMaleNamesPath,
    RestoreFileSurnamesPath,
    //RestoreFileExcludeCommonNames,
    //RestoreFileExcludeNames,
    //RestoreFileExcludeSurnames,
    nullptr
};

static WideChar const *RestoreFolders[] = {
    L"script" /*RestoreFolderScriptPath*/,
    RestoreFolderDatabasePath,
    RestoreFolderDatabaseDataPath,
    RestoreFolderDatabaseScriptPath,
    nullptr
};

void SetDatabaseID(String const &id) {
    if (!id.empty())
        wcscpy(DatabaseID, id.c_str());
    else
        DatabaseID[0] = L'\0';
    // database
    if (!id.empty()) {
        wcscpy(DatabaseFolderName, L"database_");
        wcscat(DatabaseFolderName, DatabaseID);
    }
    else
        wcscpy(DatabaseFolderName, L"database");
    wcscpy(DatabaseFolderPath, L"%s\\");
    wcscat(DatabaseFolderPath, DatabaseFolderName);
    patch::SetPointer(0x4BE2DE + 1, DatabaseFolderName);
    patch::SetPointer(0x4D2112 + 1, DatabaseFolderName);
    patch::SetPointer(0x6C57B0, DatabaseFolderName);
    patch::SetPointer(0x4BE222 + 1, DatabaseFolderPath);
    patch::SetPointer(0x4FAFDC + 1, DatabaseFolderName + 1); // substring compare
    // restore big
    if (!id.empty()) {
        wcscpy(RestoreBigSubPath, L"fmdata\\Restore_");
        wcscat(RestoreBigSubPath, DatabaseID);
    }
    else
        wcscpy(RestoreBigSubPath, L"fmdata\\Restore");
    wcscat(RestoreBigSubPath, L".big");
    wcscpy(RestoreBigPath, L"%s\\");
    wcscat(RestoreBigPath, RestoreBigSubPath);
    patch::SetPointer(0x4BAF55 + 1, RestoreBigPath);
    patch::SetPointer(0x50C7E0 + 1, RestoreBigPath);
    patch::SetPointer(0x4FA94F + 1, RestoreBigSubPath);

    // restore
    wcscpy(RestoreFileCountryDataPath, DatabaseFolderName);
    wcscpy(RestoreFileCountryScriptPath, DatabaseFolderName);
    wcscpy(RestoreFileAppearanceDefsPath, DatabaseFolderName);
    wcscpy(RestoreFileAssessmentPath, DatabaseFolderName);
    wcscpy(RestoreFileCountriesPath, DatabaseFolderName);
    wcscpy(RestoreFileMasterPath, DatabaseFolderName);
    wcscpy(RestoreFileRulesPath, DatabaseFolderName);
    wcscpy(RestoreFileWithoutPath, DatabaseFolderName);
    wcscpy(RestoreFilePriorityClubsPath, DatabaseFolderName);
    wcscpy(RestoreFileFemaleNamesPath, DatabaseFolderName);
    wcscpy(RestoreFileMaleNamesPath, DatabaseFolderName);
    wcscpy(RestoreFileSurnamesPath, DatabaseFolderName);
    //wcscpy(RestoreFileExcludeCommonNames, DatabaseFolderName);
    //wcscpy(RestoreFileExcludeNames, DatabaseFolderName);
    //wcscpy(RestoreFileExcludeSurnames, DatabaseFolderName);
    wcscat(RestoreFileCountryDataPath, L"\\data\\CountryData*.sav");
    wcscat(RestoreFileCountryScriptPath, L"\\script\\CountryScript*.sav");
    wcscat(RestoreFileAppearanceDefsPath, L"\\AppearanceDefs.sav");
    wcscat(RestoreFileAssessmentPath, L"\\Assessment.sav");
    wcscat(RestoreFileCountriesPath, L"\\Countries.sav");
    wcscat(RestoreFileMasterPath, L"\\Master.dat");
    wcscat(RestoreFileRulesPath, L"\\Rules.sav");
    wcscat(RestoreFileWithoutPath, L"\\Without.sav");
    wcscat(RestoreFilePriorityClubsPath, L"\\PriorityClubs.txt");
    wcscat(RestoreFileFemaleNamesPath, L"\\FemaleNames.txt");
    wcscat(RestoreFileMaleNamesPath, L"\\MaleNames.txt");
    wcscat(RestoreFileSurnamesPath, L"\\Surnames.txt");
    //wcscat(RestoreFileExcludeCommonNames, L"database\\ExcludeCommonNames.txt");
    //wcscat(RestoreFileExcludeNames, L"database\\ExcludeNames.txt");
    //wcscat(RestoreFileExcludeSurnames, L"database\\ExcludeSurnames.txt");
    wcscpy(RestoreFolderDatabasePath, DatabaseFolderName);
    wcscpy(RestoreFolderDatabaseDataPath, DatabaseFolderName);
    wcscat(RestoreFolderDatabaseDataPath, L"\\data");
    wcscpy(RestoreFolderDatabaseScriptPath, DatabaseFolderName);
    wcscat(RestoreFolderDatabaseScriptPath, L"\\script");
    patch::SetPointer(0x4FA87D + 2, RestoreFiles);
    patch::SetPointer(0x4FA886 + 3, RestoreFiles);
    patch::SetPointer(0x4FABBF + 3, RestoreFiles);
    patch::SetPointer(0x4FAF20 + 2, RestoreFolders);
    patch::SetUChar(0x4FAF4B + 2, UChar((std::size(RestoreFolders) - 1) * 4));

    // translation
    LoadCustomCountryNames(FM::GameDirPath(DatabaseFolderName));
    Vector<String> dbNames;
    DatabaseInfo *d = GetDatabaseInfo(Utils::WtoA(id));
    if (d && !d->parentDatabaseId.empty())
        dbNames.push_back(Utils::AtoW(d->parentDatabaseId));
    dbNames.push_back(id.empty() ? L"default" : id);
    LoadDatabaseCustomTranslation(dbNames, GameLanguage(), true);
}

int METHOD EULACloseApp(void *) {
    return 0;
}

String GetDatabaseTitle(StringA const &id) {
    StringA titleKey = id.empty() ? "DATABASE_TITLE" : Format("DATABASE_TITLE_%s", id.c_str());
    WideChar const *titleStr = GetText(titleKey.c_str());
    return titleStr ? titleStr : AtoW(titleKey);
}

INT_PTR CALLBACK DatabaseOptionsDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INITDIALOG) {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        RECT rcDlg;
        GetWindowRect(hwndDlg, &rcDlg);
        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;
        SetWindowPos(hwndDlg, HWND_TOP, screenWidth / 2 - dlgWidth / 2, screenHeight / 2 - dlgHeight / 2, 0, 0, SWP_NOSIZE);
        SetWindowTextW(hwndDlg, GetText("DATABASE_OPTIONS"));
        SetDlgItemTextW(hwndDlg, 1952, GetText("DATABASE_SELECT"));
        SetDlgItemTextW(hwndDlg, 1, GetText("Dialog.Ok"));
        SetDlgItemTextW(hwndDlg, 2, GetText("Dialog.Cancel"));
        HWND cbDatabase = GetDlgItem(hwndDlg, 2068);
        EnableWindow(GetDlgItem(hwndDlg, 1953), false);
        SendMessageW(cbDatabase, CB_RESETCONTENT, 0, 0);
        SendMessageW(cbDatabase, CB_ADDSTRING, 0, (LPARAM)GetDatabaseTitle(StringA()).c_str());
        for (auto const &d : DatabasesVec()) {
            String title = GetDatabaseTitle(d.id);
            SendMessageW(cbDatabase, CB_ADDSTRING, 0, (LPARAM)title.c_str());
        }
        SendMessageW(cbDatabase, 334, 0, 0);
        return TRUE;
    }
    else if (uMsg == WM_COMMAND) {
        CurrentDatabase().Clear();
        unsigned short itemId = LOWORD(wParam);
        unsigned short notifyCode = HIWORD(wParam);
        if (itemId == 1) { // OK
            if (notifyCode == BN_CLICKED) {
                HWND cbDatabase = GetDlgItem(hwndDlg, 2068);
                int selectedIndex = SendMessageW(cbDatabase, CB_GETCURSEL, 0, 0);
                Bool customDatabase = false;
                if (selectedIndex > 0) {
                    int vecIndex = selectedIndex - 1;
                    if (vecIndex < (int)DatabasesVec().size()) {
                        SetDatabaseID(AtoW(DatabasesVec()[vecIndex].id));
                        CurrentDatabase() = DatabasesVec()[vecIndex];
                        customDatabase = true;
                    }
                }
                if (!customDatabase) {
                    SetDatabaseID(L"");
                    CurrentDatabase().Clear();
                }
                EndDialog(hwndDlg, 1);
            }
            return TRUE;
        }
        else if (itemId == 2) { // Cancel
            if (notifyCode == BN_CLICKED) {
                patch::RedirectCall(0x4C1389, EULACloseApp);
                EndDialog(hwndDlg, 1);
            }
            return TRUE;
        }
    }
    return FALSE;
}

void *CreateDatabaseOptionsDialog() {
    ReadDatabaseIDs(true);
    if (DatabasesVec().size() > 0)
        DialogBoxParamW(NULL, (LPCWSTR)30735, NULL, DatabaseOptionsDialog, 0);
    else {
        SetDatabaseID(L"");
        CurrentDatabase().Clear();
    }
    return CallAndReturn<void *, 0x575200>( );
}

WideChar KLFilePathBuffer[2048];

void SetKLFilePath(WideChar const *filepath) {
    SetKLFilePath_Shared(KLFilePathBuffer, filepath);
}

void __declspec(naked) OnLoadKLFile() {
    __asm {
        pushad
        lea ebx, KLFilePathBuffer
        mov [esp + 0x38], ebx
        push ebx
        call SetKLFilePath
        add esp, 4
        popad
        mov edi, [esp + 0x38]
        xor ebx, ebx
        mov ecx, 0x5140F8
        jmp ecx
    }
}

WideChar const *METHOD GetEditorWindowNamePrefixWithDatabase(void *locale, DUMMY_ARG, Char const *textKey, Int langId) {
    static WideChar const *sep = L" - ";
    WideChar const *appPrefix = GetText(textKey);
    String appName = appPrefix ? appPrefix : (GetAppName() + L' ' + GetPatchName());
    if (!EndsWith(appName, sep))
        appName += sep;
    appName += GetDatabaseTitle(CurrentDatabase().id);
    appName += sep;
    static WideChar namePrefix[1024];
    wcscpy(namePrefix, appName.c_str());
    return namePrefix;
}

void PatchDatabaseOptions(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        DatabaseID[0] = L'\0';
        wcscpy(DatabaseFolderName, L"database");
        CurrentDatabase().Clear();
        patch::RedirectCall(0x4C1307, CreateDatabaseOptionsDialog);
        // dynamic folder path for text file loader
        patch::RedirectJump(0x5140F2, OnLoadKLFile);
        // window name
        patch::RedirectCall(0x41419D, GetEditorWindowNamePrefixWithDatabase);
    }
}
