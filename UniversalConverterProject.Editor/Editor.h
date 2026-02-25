#pragma once
#include "plugin.h"
#include "FifamTypes.h"

wchar_t const *GetText(char const *key);
void BinaryFileWriteString(void *binaryFile, WideChar const *str);
void BinaryFileWriteInt(void *binaryFile, UInt value);
bool ReaderIsVersionGreaterOrEqual(void *reader, UInt year, UShort build);
void PatchEditor(FM::Version v);
