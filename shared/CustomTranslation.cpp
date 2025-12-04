#include "CustomTranslation.h"
#include "TextFileTable.h"
#include "Utils.h"

UInt GetTranslationHash(String const &text) {
    UInt result = 0;
    for (WideChar c : text)
        result = (Char)c + 65599 * result;
    return result;
}

unsigned int GetTranslationHashOld(char const *text) {
    char const *curr = text;
    unsigned int result = 0;
    char letter = *text;
    if (*text) {
        do {
            ++curr;
            result = letter + 65599 * result;
            letter = *curr;
        } while (*curr);
    }
    return result;
}

TranslationTable &GetTranslationTable(TranslationTableType type) {
    static TranslationTable tables[NUM_TRANSLATION_TABLES];
    return tables[type];
}

void LoadTranslationFromFile(Path const &fileName, String const &langName, TranslationTable &out, Function<void(Path const &)> callback) {
    TextFileTable file;
    if (!file.ReadUnicodeText(fileName))
        return; // failed to read file
    if (file.Rows().size() > 1) {
        for (size_t i = 1; i < file.Rows().size(); i++) {
            if (file.Rows()[i - 1].size() != file.Rows()[i].size()) {
                ::Error(L"Error in custom translation file: " + fileName.wstring() + Utils::Format(L"at row %u (%u/%u)", i,
                    file.Rows()[i - 1].size(), file.Rows()[i].size()));
                return;
            }
        }
    }
    if (!file.IsConsistent() || file.NumRows() < 1 || file.NumColumns(0) < 2)
        return; // wrong file format
    if (file.NumRows() < 2)
        return; // empty file
    if (callback)
        callback(fileName);
    auto header = file.Row(0);
    UInt langColumn = 0;
    UInt defaultColumn = 0;
    for (UInt col = 1; col < header.size(); col++) {
        auto columnName = ToLower(header[col]);
        if (langColumn == 0 && columnName == langName)
            langColumn = col;
        if (defaultColumn == 0 && (columnName == L"def" || columnName == L"default"))
            defaultColumn = col;
    }
    if (langColumn == 0 && defaultColumn == 0)
        return; // language is not present in the file
    if (langColumn == 0)
        langColumn = defaultColumn;
    for (UInt row = 1; row < file.NumRows(); row++) {
        String const &key = file.Cell(0, row);
        if (!key.empty()) {
            String const &text = file.Cell(langColumn, row);
            if (!text.empty() || defaultColumn == 0)
                out[GetTranslationHash(key)] = text;
            else if (defaultColumn != 0 && defaultColumn != langColumn)
                out[GetTranslationHash(key)] = file.Cell(1, row);
        }
    }
}

void LoadTranslationFilesInFolder(Path const &folderPath, String const &langName, TranslationTable &out, Function<void(Path const &)> callback) {
    if (exists(folderPath) && is_directory(folderPath)) {
        for (const auto &i : directory_iterator(folderPath)) {
            path const &p = i.path();
            String ext = Utils::ToLower(p.extension().c_str());
            if (ext == L".txt")
                LoadTranslationFromFile(p, langName, out, callback);
        }
    }
}

Path TranslationFolderPath() {
    static Path p = Path(FM::GetGameDir()) / "fmdata" / "translation" / "text";
    return p;
}

void LoadCustomTranslation(String const &langName, Bool editor, Function<void(Path const &)> callback) {
    auto &table = GetTranslationTable(TRANSLATION_DEFAULT);
    table.clear();
    Path const &translationDir = TranslationFolderPath();
    LoadTranslationFilesInFolder(translationDir, langName, table, callback);
    LoadTranslationFilesInFolder(translationDir / (editor ? "editor" : "game"), langName, table, callback);
    
}

void LoadDatabaseCustomTranslation(Vector<String> const &dbIds, String const &langName, Bool editor, Function<void(Path const &)> callback) {
    auto &table = GetTranslationTable(TRANSLATION_DATABASE);
    table.clear();
    Path const &translationDir = TranslationFolderPath() / "databases";
    if (exists(translationDir) && is_directory(translationDir)) {
        for (auto const &dbId : dbIds) {
            LoadTranslationFilesInFolder(translationDir / dbId, langName, table, callback);
            LoadTranslationFilesInFolder(translationDir / dbId / (editor ? "editor" : "game"), langName, table, callback);
        }
    }
}
