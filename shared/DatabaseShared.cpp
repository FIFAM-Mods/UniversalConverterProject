#include "DatabaseShared.h"
#include "FifamReadWrite.h"
#include "UcpSettings.h"
#include "Random.h"
#include "CustomTranslation.h"
#include "shared.h"

void DatabaseInfo::Clear() {
    id.clear();
    parentDatabaseId.clear();
    isWomenDatabase = false;
    hasEditorDatabase = false;
    isEditorDatabase = false;
    index = -1;
}

Bool IsWomenDatabase() {
    return CurrentDatabase().isWomenDatabase;
}

Map<StringA, DatabaseInfo> &Databases() {
    static Map<StringA, DatabaseInfo> dbsMap;
    return dbsMap;
}

DatabaseInfo *GetDatabaseInfo(StringA const &id) {
    if (Utils::Contains(Databases(), id))
        return &Databases()[id];
    return nullptr;
}

Vector<DatabaseInfo> &DatabasesVec() {
    static Vector<DatabaseInfo> dbsVec;
    return dbsVec;
}

DatabaseInfo &CurrentDatabase() {
    static DatabaseInfo info;
    return info;
}

void ReadDatabaseIDs(Bool onlyEditorDatabases) {
    Databases().clear();
    DatabasesVec().clear();
    FifamReader r(FM::GameDirPath(L"plugins\\ucp\\database_options.txt"));
    if (r.Available()) {
        r.SkipLine();
        while (!r.IsEof()) {
            if (!r.EmptyLine()) {
                DatabaseInfo d;
                r.ReadLineWithSeparator(L'\t', d.id, d.isWomenDatabase, d.parentDatabaseId);
                if (!d.id.empty()) {
                    Path dbFolder = Path(FM::GetGameDir()) / ("database_" + d.id);
                    Bool hasEditorDatabase = exists(dbFolder) && exists(dbFolder / "Master.dat");
                    if (!onlyEditorDatabases || hasEditorDatabase) {
                        d.hasEditorDatabase = hasEditorDatabase;
                        d.index = DatabasesVec().size();
                        DatabasesVec().push_back(d);
                        Databases()[d.id] = d;
                    }
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
        if (databaseIndex < DatabasesVec().size())
            return L"database_" + AtoW(DatabasesVec()[databaseIndex].id);
    }
    return L"database";
}

void SetKLFilePath_Shared(WideChar *out, WideChar const *in) {
    wcscpy(out, in);
    auto &db = CurrentDatabase();
    if (db.id.empty())
        return;
    String p = in;
    for (auto &c : p) {
        if (c == L'/')
            c = L'\\';
    }
    auto lastSlash = p.find_last_of(L'\\');
    if (lastSlash == String::npos)
        return;
    auto parentStart = (lastSlash == 0) ? String::npos : p.rfind(L'\\', lastSlash - 1);
    String parentName;
    if (parentStart == String::npos)
        parentName = p.substr(0, lastSlash);
    else
        parentName = p.substr(parentStart + 1, lastSlash - parentStart - 1);
    String parentLower = ToLower(parentName);
    static String targets[] = { L"script", L"parameterfiles", L"historic", L"configfiles", L"towndata" };
    for (auto t : targets) {
        if (parentLower == t) {
            UInt insertPos = (UInt)(lastSlash + 1);
            {
                String withId = p;
                withId.insert(insertPos, AtoW(db.id) + L"\\");
                if (exists(withId)) {
                    wcscpy(out, withId.c_str());
                    ::Message(L"Loading database-specific file:\n" + withId); // TODO: testing, remove this
                    return;
                }
            }
            if (!db.parentDatabaseId.empty()) {
                String withParent = p;
                withParent.insert(insertPos, AtoW(db.parentDatabaseId) + L"\\");
                if (exists(withParent)) {
                    wcscpy(out, withParent.c_str());
                    ::Message(L"Loading database-specific file (from parent db):\n" + withParent); // TODO: testing, remove this
                    return;
                }
            }
            return;
        }
    }
}
