#include "DatabaseOptions.h"
#include "FifamReadWrite.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"
#include "Random.h"
#include "Translation.h"

using namespace plugin;

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

struct DatabaseOption {
    StringA id;
    Bool isWomen = false;
};

Vector<DatabaseOption> &DatabaseOptions() {
    static Vector<DatabaseOption> dbs;
    return dbs;
}

void ReadDatabaseIDs() {
    DatabaseOptions().clear();
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
                    DatabaseOption databaseOption;
                    databaseOption.id = id;
                    databaseOption.isWomen = isWomen;
                    DatabaseOptions().push_back(databaseOption);
                }
            }
            else
                r.SkipLine();
        }
    }
}

String GetDatabaseFolder(unsigned int databaseIndex) {
    if (databaseIndex > 0) {
        databaseIndex -= 1;
        if (databaseIndex < DatabaseOptions().size())
            return L"database_" + AtoW(DatabaseOptions()[databaseIndex].id);
    }
    return L"database";
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
        if (databaseIndex < DatabaseOptions().size()) {
            StringA databaseDescKey = "DATABASE_DESC_" + DatabaseOptions()[databaseIndex].id;
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
    void *editorDatabaseCheckbox = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkEditorDatabase");
    *raw_ptr<void *>(screen, 0xAD0 + 8) = editorDatabaseCheckbox;
    CallVirtualMethod<84>(editorDatabaseCheckbox, Settings::GetInstance().UseEditorDatabase);
    void *databasePicture = CallMethodAndReturn<void *, 0xD44380>(screen, "Gamestart_Database1");
    *raw_ptr<void *>(screen, 0xAD0 + 12) = databasePicture;
    void *databaseComboBox = CallMethodAndReturn<void *, 0xD442C0>(screen, "CbDatabase");
    *raw_ptr<void *>(screen, 0xAD0 + 16) = databaseComboBox;
    void* databaseInfo = CreateTextBox(screen, "TbDatabaseInfo");
    *raw_ptr<void*>(screen, 0xAD0 + 20) = databaseInfo;
    CallVirtualMethod<83>(databaseComboBox, GetTranslation("DATABASE_TITLE"), 0, 0);
    for (unsigned int i = 0; i < DatabaseOptions().size(); i++) {
        StringA dbKey = "DATABASE_TITLE_" + DatabaseOptions()[i].id;
        CallVirtualMethod<83>(databaseComboBox, GetTranslation(dbKey.c_str()), i + 1, 0);
    }
    if (Settings::GetInstance().DatabaseOption > 0) {
        UInt optionIndex = Settings::GetInstance().DatabaseOption - 1;
        if (optionIndex >= DatabaseOptions().size())
            Settings::GetInstance().DatabaseOption = 0;
    }
    CallVirtualMethod<70>(databaseComboBox, Settings::GetInstance().DatabaseOption);
    SetDatabaseImage(databasePicture, Settings::GetInstance().DatabaseOption);
    SetDatabaseText(databaseInfo, Settings::GetInstance().DatabaseOption);
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

void METHOD ProcessGameStartDatabaseComboBoxes(void *screen, DUMMY_ARG, int *id, int unk1, int unk2) {
    void *databaseComboBox = *raw_ptr<void *>(screen, 0xAD0 + 16);
    void *databasePicture = *raw_ptr<void *>(screen, 0xAD0 + 12);
    void* databaseInfo = *raw_ptr<void*>(screen, 0xAD0 + 20);
    bool isPrediction = *raw_ptr<UInt>(screen, 0xAD0 + 24) != 0;
    if (*id == CallVirtualMethodAndReturn<int, 23>(databaseComboBox)) {
        int option = CallVirtualMethodAndReturn<int, 94>(databaseComboBox, 0, 0);
        if (!isPrediction)
            Settings::GetInstance().DatabaseOption = option;
        SetDatabaseImage(databasePicture, option);
        SetDatabaseText(databaseInfo, option);
    }
}

void *METHOD OnCreateCheckBoxPrediction(void *screen, DUMMY_ARG, char const *name) {
    void *originalChk = CallMethodAndReturn<void *, 0xD44260>(screen, name);
    *raw_ptr<UInt>(screen, 0xAD0 + 24) = 1;
    void *editorDatabaseCheckbox = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkEditorDatabase1");
    *raw_ptr<void *>(screen, 0xAD0 + 8) = editorDatabaseCheckbox;
    CallVirtualMethod<84>(editorDatabaseCheckbox, false);
    void *databaseComboBox = CallMethodAndReturn<void *, 0xD442C0>(screen, "CbDatabase1");
    *raw_ptr<void *>(screen, 0xAD0 + 16) = databaseComboBox;
    void* databaseInfo = CreateTextBox(screen, "TbDatabaseInfo1");
    *raw_ptr<void*>(screen, 0xAD0 + 20) = databaseInfo;
    CallVirtualMethod<83>(databaseComboBox, GetTranslation("DATABASE_TITLE"), 0, 0);
    for (unsigned int i = 0; i < DatabaseOptions().size(); i++) {
        StringA dbKey = "DATABASE_TITLE_" + DatabaseOptions()[i].id;
        CallVirtualMethod<83>(databaseComboBox, GetTranslation(dbKey.c_str()), i + 1, 0);
    }
    CallVirtualMethod<70>(databaseComboBox, 0);
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
        if (databaseIndex >= DatabaseOptions().size())
            option = 0;
    }
    else if (option < 0)
        option = 0;
    if (option == 0) {
        if (editorDb)
            CallMethod<0x10F48C0>(t, GetUserDbPath(String()));
        else
            CallMethod<0x10F48C0>(t, L"fmdata\\Restore.dat");
    }
    else {
        UInt databaseIndex = option - 1;
        if (editorDb) {
            String dbPath = GetUserDbPath(AtoW(DatabaseOptions()[databaseIndex].id));
            CallMethod<0x10F48C0>(t, dbPath.c_str());
        }
        else {
            String dbPath = L"fmdata\\Restore_" + AtoW(DatabaseOptions()[databaseIndex].id) + L".dat";
            CallMethod<0x10F48C0>(t, dbPath.c_str());
        }
    }
    //::Warning(L"Loading %s", raw_ptr<wchar_t>(t, 0x30));
}

void PatchDatabaseOptions(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadDatabaseIDs();
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
    }
}
