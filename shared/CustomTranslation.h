#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"

using namespace plugin;

enum TranslationTableType {
    TRANSLATION_DATABASE = 0,
    TRANSLATION_DEFAULT = 1,
    NUM_TRANSLATION_TABLES
};

using TranslationTable = Map<UInt, String>; // key hash => text

TranslationTable &GetTranslationTable(TranslationTableType type);
void LoadCustomTranslation(String const &langName, Bool editor, Function<void(Path const &)> callback = {});
void LoadDatabaseCustomTranslation(Vector<String> const &dbNames, String const &langName, Bool editor, Function<void(Path const &)> callback = {});
