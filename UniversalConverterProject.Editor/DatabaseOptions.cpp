#include "DatabaseOptions.h"
#include "FifamReadWrite.h"
#include "Editor.h"
#include <Windows.h>

using namespace plugin;

Vector<Pair<StringA, String>> &DatabaseIDs() {
    static Vector<Pair<StringA, String>> dbs;
    return dbs;
}

wchar_t DatabaseID[256];
wchar_t DatabaseFolderName[256];
wchar_t DatabaseFolderPath[256];
wchar_t RestoreBigPath[256];
wchar_t RestoreBigSubPath[256];
//wchar_t ScriptFolderName[256];
//wchar_t ScriptPath[256];
//wchar_t ScriptLeagueSubPath[256];
//wchar_t ParameterFolderName[256];
//wchar_t ParameterStaffGenerationPath[256];
//wchar_t ParameterPlayerLevelPath[256];
//wchar_t ParameterPlayerStylesPath[256];
//wchar_t ParameterClubJobsPath[256];
//wchar_t ParameterClubPositionsPath[256];
//wchar_t ParameterTrainingCampNewPath[256];
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

void ReadDatabaseIDs() {
    DatabaseIDs().clear();
    FifamReader r(FM::GameDirPath(L"plugins\\ucp\\database_options.txt"));
    if (r.Available()) {
        r.SkipLine();
        while (!r.IsEof()) {
            if (!r.EmptyLine()) {
                StringA id;
                String description;
                bool isWomen = false;
                r.ReadLineWithSeparator(L'\t', id, isWomen, description);
                if (!id.empty()) {
                    wchar_t const *text = GetText(FormatStatic("DATABASE_TITLE_%s", id.c_str()));
                    if (text)
                        DatabaseIDs().emplace_back(id, text);
                }
            }
            else
                r.SkipLine();
        }
    }
}

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

    //// script
    //if (!id.empty()) {
    //    wcscpy(ScriptFolderName, L"Script_");
    //    wcscat(ScriptPath, DatabaseID);
    //}
    //else
    //    wcscpy(ScriptFolderName, L"Script");
    //if (exists(ScriptFolderName) && is_directory(ScriptFolderName)) {
    //    wcscpy(ScriptPath, L"%s\\");
    //    wcscat(ScriptPath, ScriptFolderName);
    //    wcscpy(ScriptLeagueSubPath, ScriptFolderName);
    //    wcscat(ScriptLeagueSubPath, L"/lg%d.txt");
    //    wcscpy(RestoreFolderScriptPath, ScriptFolderName);
    //    patch::SetPointer(0x44911B + 1, ScriptLeagueSubPath);
    //    patch::SetPointer(0x4E1252 + 1, ScriptLeagueSubPath);
    //    patch::SetPointer(0x4BE234 + 1, ScriptPath);
    //}
    //else
    //    wcscpy(RestoreFolderScriptPath, L"script");
    //// parameter files
    //if (!id.empty()) {
    //    wcscpy(ParameterFolderName, L"ParameterFiles_");
    //    wcscat(ParameterFolderName, DatabaseID);
    //}
    //else
    //    wcscpy(ParameterFolderName, L"ParameterFiles");
    //if (exists(ParameterFolderName) && is_directory(ParameterFolderName)) {
    //    wcscpy(ParameterStaffGenerationPath, L"%s\\fmdata\\");
    //    wcscat(ParameterStaffGenerationPath, ParameterFolderName);
    //    wcscat(ParameterStaffGenerationPath, L"\\Staff Generation.txt");
    //    wcscpy(ParameterPlayerLevelPath, L"fmdata\\");
    //    wcscat(ParameterPlayerLevelPath, ParameterFolderName);
    //    wcscat(ParameterPlayerLevelPath, L"\\Player Level.txt");
    //    wcscpy(ParameterPlayerStylesPath, L"fmdata\\");
    //    wcscat(ParameterPlayerStylesPath, ParameterFolderName);
    //    wcscat(ParameterPlayerStylesPath, L"\\Player Styles.txt");
    //    wcscpy(ParameterClubJobsPath, ParameterFolderName);
    //    wcscat(ParameterClubJobsPath, L"\\Club Jobs.txt");
    //    wcscpy(ParameterClubPositionsPath, ParameterFolderName);
    //    wcscat(ParameterClubPositionsPath, L"\\Club Positions.txt");
    //    wcscpy(ParameterTrainingCampNewPath, ParameterFolderName);
    //    wcscat(ParameterTrainingCampNewPath, L"\\Training Camp New.txt");
    //    patch::SetPointer(0x4BE4BF + 1, ParameterStaffGenerationPath);
    //    patch::SetPointer(0x4DB140 + 1, ParameterPlayerLevelPath);
    //    patch::SetPointer(0x4DB160 + 1, ParameterPlayerLevelPath);
    //    patch::SetPointer(0x4DB70D + 1, ParameterPlayerStylesPath);
    //    patch::SetPointer(0x4DB72D + 1, ParameterPlayerStylesPath);
    //    patch::SetPointer(0x56F38B + 1, ParameterClubJobsPath);
    //    patch::SetPointer(0x56F68F + 1, ParameterClubPositionsPath);
    //    patch::SetPointer(0x6D0BA0 + 1, ParameterTrainingCampNewPath);
    //}

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
}

int METHOD EULACloseApp(void *) {
    return 0;
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
        SendMessageW(cbDatabase, CB_ADDSTRING, 0, (LPARAM)GetText("DATABASE_TITLE"));
        for (auto const &[id, text] : DatabaseIDs())
            SendMessageW(cbDatabase, CB_ADDSTRING, 0, (LPARAM)text.c_str());
        SendMessageW(cbDatabase, 334, 0, 0);
        return TRUE;
    }
    else if (uMsg == WM_COMMAND) {
        unsigned short itemId = LOWORD(wParam);
        unsigned short notifyCode = HIWORD(wParam);
        if (itemId == 1) { // OK
            if (notifyCode == BN_CLICKED) {
                HWND cbDatabase = GetDlgItem(hwndDlg, 2068);
                int selectedIndex = SendMessageW(cbDatabase, CB_GETCURSEL, 0, 0);
                if (selectedIndex > 0) {
                    int vecIndex = selectedIndex - 1;
                    if (vecIndex < (int)DatabaseIDs().size())
                        SetDatabaseID(AtoW(DatabaseIDs()[vecIndex].first));
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
    ReadDatabaseIDs();
    if (DatabaseIDs().size() > 0)
        DialogBoxParamA(NULL, (LPCSTR)30735, NULL, DatabaseOptionsDialog, 0);
    //else
    //    ::Warning("%d", DatabaseIDs().size());
    return CallAndReturn<void *, 0x575200>( );
}

void PatchDatabaseOptions(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        DatabaseID[0] = L'\0';
        wcscpy(DatabaseFolderName, L"database");
        patch::RedirectCall(0x4C1307, CreateDatabaseOptionsDialog);
    }
}
