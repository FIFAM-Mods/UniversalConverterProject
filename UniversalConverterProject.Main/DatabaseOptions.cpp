#include "DatabaseOptions.h"
#include "FifamReadWrite.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"
#include "Random.h"
#include "Translation.h"
#include "CustomTranslation.h"
#include "LeagueSelection.h"
#include "shared.h"

using namespace plugin;

int DatabaseOption = 0;

Char RandomTalent(Char talent, Int a, Int b, Int c) {
    Int rnd = Random::Get(1, a + b + c);
    if (talent == 0) {
        if (rnd > b)
            return talent + 1;
    }
    else if (talent == 9) {
        if (rnd <= a)
            return talent - 1;
    }
    else {
        if (rnd <= a)
            return  talent - 1;
        else if (rnd > (a + b))
            return  talent + 1;
    }
    return talent;
}

void *METHOD OnGetPlayerContract_RandomizeTalent(CDBPlayer *player) {
    void *contract = CallMethodAndReturn<void *, 0xF98890>(player);
    if (Settings::GetInstance().RandomizeTalents) {
        if (player->GetAge() <= 18) {
            //static FifamWriter w("test_talents.csv");
            auto talent = player->GetTalent();
            if (talent <= 0)
                talent = 0;
            else if (talent >= 9)
                talent = 9;
            if (talent == 9) // 5 - 1
                talent = RandomTalent(talent, 50, 50, 0);
            else if (talent == 8) // 4.5 - 8
                talent = RandomTalent(talent, 45, 45, 10);
            else if (talent == 7) // 4 - 26
                talent = RandomTalent(talent, 42, 42, 16);
            else if (talent == 6) // 3.5 - 123
                talent = RandomTalent(talent, 45, 45, 10);
            else if (talent == 5) // 3 - 1704
                talent = RandomTalent(talent, 48, 48, 4);
            else if (talent == 4) // 2.5 - 3976
                talent = RandomTalent(talent, 39, 39, 22);
            else if (talent == 3) // 2 - 5582
                talent = RandomTalent(talent, 23, 49, 28);
            else if (talent == 2) // 1.5 - 3776
                talent = RandomTalent(talent, 33, 33, 33);
            else if (talent == 1) // 1 - 2157
                talent = RandomTalent(talent, 33, 33, 33);
            else if (talent == 0) // 0.5 - 403
                talent = RandomTalent(talent, 33, 33, 33);
            //w.WriteLine(CallMethodAndReturn<WideChar *, 0xFA2010>(player, 0), *raw_ptr<Char>(player, 0x10), talent);
            *raw_ptr<Char>(player, 0x10) = talent;
        }
    }
    return contract;
}

UInt64 savedSalary = 0;

void METHOD OnClearContractAtStartOfTheSeason(void *contract) {
    if (Settings::GetInstance().UseRealSalaries)
        savedSalary = *raw_ptr<UInt64>(contract, 0x20);
    CallMethod<0x1014E90>(contract);
}

void *METHOD OnPlayerGetConract_SalaryCalculation(CDBPlayer *player) {
    void *contract = CallMethodAndReturn<void *, 0xF98890>(player);
    if (Settings::GetInstance().UseRealSalaries) {
        if (savedSalary > 0) {
            CallMethod<0x10167F0>(contract, savedSalary, false);
            savedSalary = 0;
        }
    }
    return contract;
}

void SetDatabaseImage(void *img, unsigned int databaseIndex) {
    auto dbFolder = GetDatabaseFolder(databaseIndex);
    String fileName = dbFolder + L"\\picture_" + GameLanguage() + L".tga";
    if (!FmFileExists(fileName)) {
        fileName = dbFolder + L"\\picture.tga";
        if (!FmFileExists(fileName))
            fileName.clear();
    }
    if (!fileName.empty())
        SetImageFilename(img, fileName);
}

void SetDatabaseText(void *textBox, unsigned int databaseIndex) {
    if (databaseIndex > 0) {
        databaseIndex -= 1;
        if (databaseIndex < DatabasesVec().size()) {
            StringA databaseDescKey = "DATABASE_DESC_" + DatabasesVec()[databaseIndex].id;
            SetText(textBox, GetTranslation(databaseDescKey.c_str()));
        }
        else
            SetText(textBox, L"");
    }
    else
        SetText(textBox, GetTranslation("DATABASE_DESC"));
}

void *METHOD OnCreateWinterTransfersCheckbox(void *screen, DUMMY_ARG, char const *name) {
    void *originalChk = CallMethodAndReturn<void *, 0xD44260>(screen, name);
    void *randomizeTalentsCheckbox = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkRandomizeTalents");
    *raw_ptr<void *>(screen, 0xAD0) = randomizeTalentsCheckbox;
    CallVirtualMethod<84>(randomizeTalentsCheckbox, Settings::GetInstance().RandomizeTalents);
    void *useRealSalaries = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkUseRealSalaries");
    *raw_ptr<void *>(screen, 0xAD0 + 4) = useRealSalaries;
    CallVirtualMethod<84>(useRealSalaries, Settings::GetInstance().UseRealSalaries);
    CXgCheckBox *editorDatabaseCheckbox = CallMethodAndReturn<CXgCheckBox *, 0xD44260>(screen, "ChkEditorDatabase");
    *raw_ptr<void *>(screen, 0xAD0 + 8) = editorDatabaseCheckbox;
    void *databasePicture = CallMethodAndReturn<void *, 0xD44380>(screen, "Gamestart_Database1");
    *raw_ptr<void *>(screen, 0xAD0 + 12) = databasePicture;
    CXgComboBox *databaseComboBox = CallMethodAndReturn<CXgComboBox *, 0xD442C0>(screen, "CbDatabase");
    *raw_ptr<void *>(screen, 0xAD0 + 16) = databaseComboBox;
    void* databaseInfo = CreateTextBox(screen, "TbDatabaseInfo");
    *raw_ptr<void*>(screen, 0xAD0 + 20) = databaseInfo;
    databaseComboBox->AddItem(GetTranslation("DATABASE_TITLE"), 0);
    for (unsigned int i = 0; i < DatabasesVec().size(); i++) {
        StringA dbKey = "DATABASE_TITLE_" + DatabasesVec()[i].id;
        databaseComboBox->AddItem(GetTranslation(dbKey.c_str()), i + 1);
    }
    DatabaseOption = 0;
    Bool enableEditorDatabase = true;
    for (UInt i = 0; i < DatabasesVec().size(); i++) {
        if (DatabasesVec()[i].parentDatabaseId == "default") {
            DatabaseOption = i + 1;
            enableEditorDatabase = DatabasesVec()[i].hasEditorDatabase;
        }
    }
    editorDatabaseCheckbox->SetEnabled(enableEditorDatabase);
    Bool editorDatabaseChecked = enableEditorDatabase && Settings::GetInstance().UseEditorDatabase;
    editorDatabaseCheckbox->SetIsChecked(editorDatabaseChecked);
    Settings::GetInstance().UseEditorDatabase = editorDatabaseChecked;
    databaseComboBox->SetCurrentIndex(DatabaseOption);
    SetDatabaseImage(databasePicture, DatabaseOption);
    SetDatabaseText(databaseInfo, DatabaseOption);
    *raw_ptr<UInt>(screen, 0xAD0 + 24) = 0;
    return originalChk;
}

void METHOD OnProcessGameStartCheckboxes(void *screen, DUMMY_ARG, int *data, int unk) {
    void *randomizeTalentsCheckbox = *raw_ptr<void *>(screen, 0xAD0);
    void *useRealSalaries = *raw_ptr<void *>(screen, 0xAD0 + 4);
    void *editorDatabaseCheckbox = *raw_ptr<void *>(screen, 0xAD0 + 8);
    bool isPrediction = *raw_ptr<UInt>(screen, 0xAD0 + 24) != 0;
    if (*data == CallVirtualMethodAndReturn<int, 23>(randomizeTalentsCheckbox)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(randomizeTalentsCheckbox) != 0;
        Settings::GetInstance().RandomizeTalents = checked;
        return;
    }
    else if (*data == CallVirtualMethodAndReturn<int, 23>(useRealSalaries)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(useRealSalaries) != 0;
        Settings::GetInstance().UseRealSalaries = checked;
        return;
    }
    else if (*data == CallVirtualMethodAndReturn<int, 23>(editorDatabaseCheckbox)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(editorDatabaseCheckbox) != 0;
        if (!isPrediction)
            Settings::GetInstance().UseEditorDatabase = checked;
        return;
    }
    CallMethod<0x5247D0>(screen, data, unk);
}

void METHOD ProcessGameStartDatabaseComboBoxes(void *screen, DUMMY_ARG, Int *id, Int unk1, Int unk2) {
    void *databaseComboBox = *raw_ptr<void *>(screen, 0xAD0 + 16);
    void *databasePicture = *raw_ptr<void *>(screen, 0xAD0 + 12);
    void* databaseInfo = *raw_ptr<void*>(screen, 0xAD0 + 20);
    Bool isPrediction = *raw_ptr<UInt>(screen, 0xAD0 + 24) != 0;
    if (*id == CallVirtualMethodAndReturn<Int, 23>(databaseComboBox)) {
        Int option = CallVirtualMethodAndReturn<Int, 94>(databaseComboBox, 0, 0);
        if (!isPrediction)
            DatabaseOption = option;
        SetDatabaseImage(databasePicture, option);
        SetDatabaseText(databaseInfo, option);
        CXgCheckBox *editorDatabaseCheckbox = *raw_ptr<CXgCheckBox *>(screen, 0xAD0 + 8);
        Bool enableEditorDatabase = true;
        if (option > 0) {
            UInt databaseIndex = option - 1;
            if (databaseIndex < DatabasesVec().size())
                enableEditorDatabase = DatabasesVec()[databaseIndex].hasEditorDatabase;
        }
        editorDatabaseCheckbox->SetEnabled(enableEditorDatabase);
        if (!enableEditorDatabase) {
            editorDatabaseCheckbox->SetIsChecked(false);
            if (!isPrediction)
                Settings::GetInstance().EditorDatabase = enableEditorDatabase;
        }
    }
}

void *METHOD OnCreateCheckBoxPrediction(void *screen, DUMMY_ARG, char const *name) {
    void *originalChk = CallMethodAndReturn<void *, 0xD44260>(screen, name);
    *raw_ptr<UInt>(screen, 0xAD0 + 24) = 1;
    CXgCheckBox *editorDatabaseCheckbox = CallMethodAndReturn<CXgCheckBox *, 0xD44260>(screen, "ChkEditorDatabase1");
    *raw_ptr<void *>(screen, 0xAD0 + 8) = editorDatabaseCheckbox;
    editorDatabaseCheckbox->SetIsChecked(false);
    CXgComboBox *databaseComboBox = CallMethodAndReturn<CXgComboBox *, 0xD442C0>(screen, "CbDatabase1");
    *raw_ptr<void *>(screen, 0xAD0 + 16) = databaseComboBox;
    void* databaseInfo = CreateTextBox(screen, "TbDatabaseInfo1");
    *raw_ptr<void*>(screen, 0xAD0 + 20) = databaseInfo;
    databaseComboBox->AddItem(GetTranslation("DATABASE_TITLE"), 0);
    for (unsigned int i = 0; i < DatabasesVec().size(); i++) {
        StringA dbKey = "DATABASE_TITLE_" + DatabasesVec()[i].id;
        databaseComboBox->AddItem(GetTranslation(dbKey.c_str()), i + 1);
    }
    databaseComboBox->SetCurrentIndex(0);
    void *databasePicture = *raw_ptr<void *>(screen, 0xAD0 + 12);
    SetDatabaseImage(databasePicture, 0);
    SetDatabaseText(databaseInfo, 0);
    return originalChk;
}

DWORD GetFileLastWriteTime(const wchar_t *filePath) {
    HANDLE h = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return 0;
    FILETIME LastWriteTime;
    DWORD time = 0;
    if (GetFileTime(h, 0, 0, &LastWriteTime))
        time = LastWriteTime.dwLowDateTime;
    CloseHandle(h);
    return time;
}

String GetUserDbPath(String const &dbName) {
    String dbLocalPath = dbName.empty() ? L"Database\\Master.dat" : (L"Database_" + dbName + L"\\Master.dat");
    void *resolver = CallAndReturn<void *, 0x40BF10>();
    WideChar dbDocumentsPath[260];
    CallVirtualMethod<5>(resolver, dbDocumentsPath, 10, dbLocalPath.c_str(), 0, 0, 1, 0); // resolver->GetFilePath()
    if (dbDocumentsPath[0] && GetFileLastWriteTime(dbDocumentsPath) > GetFileLastWriteTime(dbLocalPath.c_str()))
        return dbDocumentsPath;
    return dbLocalPath;
}

void METHOD OnDatabaseLoaderSetDatabasePath(void *t, DUMMY_ARG, void *screen) {
    void *editorDatabaseCheckbox = *raw_ptr<void *>(screen, 0xAD0 + 8);
    void *databaseComboBox = *raw_ptr<void *>(screen, 0xAD0 + 16);
    int option = CallVirtualMethodAndReturn<int, 94>(databaseComboBox, 0, 0);
    bool editorDb = CallVirtualMethodAndReturn<unsigned char, 85>(editorDatabaseCheckbox) != 0;
    if (option > 0) {
        UInt databaseIndex = option - 1;
        if (databaseIndex >= DatabasesVec().size())
            option = 0;
    }
    else if (option < 0)
        option = 0;
    CurrentDatabase().Clear();
    if (option == 0) {
        if (editorDb)
            CallMethod<0x10F48C0>(t, GetUserDbPath(String()));
        else
            CallMethod<0x10F48C0>(t, L"fmdata\\Restore.dat");
        LoadDatabaseCustomTranslation({ L"default" }, GameLanguage(), false, [](Path const &filename) {
            SafeLog::Write(L"Loading custom translation file: " + filename.wstring());
        });
        CurrentDatabase().isEditorDatabase = false;
    }
    else {
        UInt databaseIndex = option - 1;
        CurrentDatabase() = DatabasesVec()[databaseIndex];
        CurrentDatabase().isEditorDatabase = true;
        String dbId = AtoW(DatabasesVec()[databaseIndex].id);
        if (editorDb) {
            String dbPath = GetUserDbPath(dbId);
            CallMethod<0x10F48C0>(t, dbPath.c_str());
        }
        else {
            String dbPath = L"fmdata\\Restore_" + dbId + L".dat";
            CallMethod<0x10F48C0>(t, dbPath.c_str());
        }
        Vector<String> dbNames;
        DatabaseInfo *d = GetDatabaseInfo(Utils::WtoA(dbId));
        if (d && !d->parentDatabaseId.empty())
            dbNames.push_back(Utils::AtoW(d->parentDatabaseId));
        dbNames.push_back(dbId);
        LoadDatabaseCustomTranslation(dbNames, GameLanguage(), false, [](Path const &filename) {
            SafeLog::Write(L"Loading custom translation file: " + filename.wstring());
        });
    }
    DecidePreferredLeagues();
    //::Warning(L"Loading %s", raw_ptr<wchar_t>(t, 0x30));
}

WideChar KLFilePathBuffer[2048];

Bool SetKLFilePath(WideChar const *filepath) {
    SetKLFilePath_Shared(KLFilePathBuffer, filepath);
    return CallAndReturn<Bool, 0x1494558>(KLFilePathBuffer, L".enc");
}

void __declspec(naked) OnLoadKLFile() {
    __asm {
        push [ebp+8]
        lea ecx, KLFilePathBuffer
        mov [ebp+8], ecx
        call SetKLFilePath
        pop ecx
        mov ecx, 0x14B2C90
        jmp ecx
    }
}

void METHOD OnLoadDBLoadInfo(void *loadInfo) {
    CurrentDatabase().Clear();
    void *save = *(void **)0x3179DD8;
    if (SaveGameLoadGetVersion(save) >= 48) {
        auto &db = CurrentDatabase();
        WideChar bufId[256];
        bufId[0] = L'\0';
        SaveGameReadString(save, bufId, std::size(bufId));
        db.id = WtoA(bufId);
        bufId[0] = L'\0';
        SaveGameReadString(save, bufId, std::size(bufId));
        db.parentDatabaseId = WtoA(bufId);
        db.isWomenDatabase = SaveGameReadInt32(save);
        db.hasEditorDatabase = SaveGameReadInt32(save);
        db.isEditorDatabase = SaveGameReadInt32(save);
        db.index = (Int)SaveGameReadInt32(save);
    }
    Vector<String> dbNames;
    if (!CurrentDatabase().parentDatabaseId.empty())
        dbNames.push_back(Utils::AtoW(CurrentDatabase().parentDatabaseId));
    dbNames.push_back(CurrentDatabase().id.empty() ? L"default" : Utils::AtoW(CurrentDatabase().id));
    LoadDatabaseCustomTranslation(dbNames, GameLanguage(), false, [](Path const &filename) {
        SafeLog::Write(L"Loading custom translation file: " + filename.wstring());
    });
    CallMethod<0x14F29AB>(loadInfo);
}

void METHOD OnSaveDBLoadInfo(void *loadInfo) {
    void *save = *(void **)0x3179DD4;
    auto const &db = CurrentDatabase();
    SaveGameWriteString(save, Utils::AtoW(db.id).c_str());
    SaveGameWriteString(save, Utils::AtoW(db.parentDatabaseId).c_str());
    SaveGameWriteInt32(save, db.isWomenDatabase);
    SaveGameWriteInt32(save, db.hasEditorDatabase);
    SaveGameWriteInt32(save, db.isEditorDatabase);
    SaveGameWriteInt32(save, (UInt)db.index);
    CallMethod<0x14F291E>(loadInfo);
}

Int METHOD OnLoadDatabaseTownDataUniques(void *file, DUMMY_ARG, WideChar const *filename) {
    if (!CurrentDatabase().id.empty()) {
        static WideChar buf[1024];
        wcscpy(buf, L"Database_");
        wcscat(buf, AtoW(CurrentDatabase().id).c_str());
        wcscat(buf, L"\\TownDataUniques.txt");
        filename = buf;
    }
    return CallMethodAndReturn<Int, 0x14B2C35>(file, filename);
}

void __declspec(naked) OnResolveGraphicsFilePath() {
    __asm {
        mov eax, [esp + 0xFF8]
        cmp eax, 0 // badge
        je badge_case
        cmp eax, 4 // trophy
        je trophy_case
        cmp eax, 18 // xml screen
        jne done
//  xml_screen_case:
        mov ecx, [esp + 0x1000] // ID
        mov edx, 0x1000
        jmp check_id
    badge_case:
        mov edx, [esp + 0x1000]   // badge subtype
        test edx, edx
        jz done // club badge
        mov ecx, [esp + 0xFFC] // ID
        mov edx, 0xFFC
        jmp check_id
    trophy_case:
        mov ecx, [esp + 0xFFC] // ID
        mov edx, 0xFFC
    check_id:
        cmp ecx, 0x10000 // ID >= 0x10000
        jb done
        call IsWomenDatabase
        test eax, eax
        jz done
        mov ecx, [esp + edx]
        or ecx, 0x800000 // ID |= 0x800000
        mov [esp + edx], ecx
    done:
        mov eax, [esp + 0xFF8]
        mov ecx, 0x4DC5FD
        jmp ecx
    }
}

void PatchDatabaseOptions(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadDatabaseIDs(false);
        patch::SetUInt(0x47F7A4 + 1, 0xAD0 + 28);
        patch::SetUInt(0x47F7AB + 1, 0xAD0 + 28);
        patch::RedirectCall(0xFD31F6, OnGetPlayerContract_RandomizeTalent);
        patch::RedirectCall(0xF24DFC, OnClearContractAtStartOfTheSeason);
        patch::RedirectCall(0xF24E5A, OnPlayerGetConract_SalaryCalculation);
        patch::RedirectCall(0x524E7A, OnCreateWinterTransfersCheckbox);
        patch::SetPointer(0x23BA8F4, OnProcessGameStartCheckboxes);
        patch::SetPointer(0x23BA8E0, ProcessGameStartDatabaseComboBoxes);
        patch::RedirectCall(0x524CAF, OnCreateCheckBoxPrediction);
        patch::RedirectCall(0x52486C, OnDatabaseLoaderSetDatabasePath);
        patch::SetUChar(0x524869, 0x57); // push eax => push edi

        // dynamic folder path for text file loader
        patch::RedirectJump(0x14B2C81, OnLoadKLFile);

        // save & load db info
        patch::RedirectCall(0x108303C, OnSaveDBLoadInfo);
        patch::RedirectCall(0x1080DFC, OnLoadDBLoadInfo);

        // TownDataUniques
        patch::RedirectCall(0x12F01E2, OnLoadDatabaseTownDataUniques);

        // special league IDs for women's database
        patch::RedirectJump(0x4DC5F6, OnResolveGraphicsFilePath);
    }
}
