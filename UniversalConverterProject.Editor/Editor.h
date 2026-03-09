#pragma once
#include "plugin.h"
#include "FifamTypes.h"

wchar_t const *GetText(char const *key);
void BinaryFileWriteString(void *binaryFile, WideChar const *str);
void BinaryFileWriteInt(void *binaryFile, UInt value);
bool ReaderIsVersionGreaterOrEqual(void *reader, UInt year, UShort build);
void DDX_Control(void *pDX, int nIDC, void *rControl);
void *ComboBoxConstruct(void *t);
void ComboBoxDestruct(void *t);
void *CheckBoxConstruct(void *t);
void CheckBoxDestruct(void *t);
void *GroupBoxConstruct(void *t);
void GroupBoxDestruct(void *t);
void CheckBoxSetIsChecked(void *t, Bool checked);
Bool CheckBoxGetIsChecked(void *t);
int WndShowWindow(void *t, int nCmdShow);
void PatchEditor(FM::Version v);
