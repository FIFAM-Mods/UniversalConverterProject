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

Bool IsCJKUnifiedIdeograph(WideChar c) {
    return (c >= 0x4E00 && c <= 0x9FFF);
}

String AddSpacesBetweenCJK(String const &src) {
    String out;
    out.reserve(src.size() + 16);
    UInt n = src.size();
    for (UInt i = 0; i < n; ++i) {
        WideChar c = src[i];
        out.push_back(c);
        if (i + 1 < n && IsCJKUnifiedIdeograph(c) && IsCJKUnifiedIdeograph(src[i + 1])) {
            if (src[i + 1] != L' ')
                out.push_back(L' ');
        }
    }
    return out;
}

enum TranslationFileKeyType {
    TRANSLATIONFILE_KEY_ID,
    TRANSLATIONFILE_KEY_HASH
};

void LoadTranslationFromFile(Path const &fileName, String const &langName, TranslationTable &out, TranslationFileKeyType keyType,
    Function<void(Path const &)> callback)
{
    TextFileTable file;
    if (!file.ReadUnicodeText(fileName))
        return; // failed to read file
    if (file.Rows().size() > 1) {
        for (size_t i = 1; i < file.Rows().size(); i++) {
            if (file.Rows()[i - 1].size() != file.Rows()[i].size()) {
                ::Error(L"Error in translation file: " + fileName.wstring() + Utils::Format(L"at row %u (%u/%u)", i,
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
    Bool addSpacing = false;
    if (langColumn == 0)
        langColumn = defaultColumn;
    else
        addSpacing = langName == L"chi";
    for (UInt row = 1; row < file.NumRows(); row++) {
        String const &keyStr = file.Cell(0, row);
        if (!keyStr.empty()) {
            UInt key = (keyType == TRANSLATIONFILE_KEY_ID) ? Utils::SafeConvertInt<UInt>(keyStr) : GetTranslationHash(keyStr);
            String const &text = file.Cell(langColumn, row);
            if (!text.empty()) {
                if (text.size() == 1 && text[0] == L' ')
                    out[key] = L"";
                else if (addSpacing)
                    out[key] = AddSpacesBetweenCJK(text);
                else
                    out[key] = text;
            }
            else if (defaultColumn != 0 && defaultColumn != langColumn)
                out[key] = file.Cell(defaultColumn, row);
        }
    }
}

void LoadTranslationFilesInFolder(Path const &folderPath, String const &langName, TranslationTable &out, Function<void(Path const &)> callback) {
    if (exists(folderPath) && is_directory(folderPath)) {
        for (const auto &i : directory_iterator(folderPath)) {
            path const &p = i.path();
            String ext = Utils::ToLower(p.extension().c_str());
            if (ext == L".txt")
                LoadTranslationFromFile(p, langName, out, TRANSLATIONFILE_KEY_HASH, callback);
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
