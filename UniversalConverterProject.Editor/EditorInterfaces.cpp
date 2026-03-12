#include "EditorInterfaces.h"

Bool CKLFile::IsVersionGreaterOrEqual(UInt year, UShort build) {
    return CallMethodAndReturn<Bool, 0x511C70>(this, year, build);
}

Bool CKLFile::ReadString(WideChar *out, UInt maxLen) {
    return CallMethodAndReturn<Bool, 0x5123B0>(this, out, maxLen);
}

Bool CKLFile::ReadStringArray(void *out, UInt count, UInt maxLen) {
    return CallMethodAndReturn<Bool, 0x512510>(this, out, count, maxLen);
}

Bool CKLFile::ReadInt(Int &out) {
    return CallMethodAndReturn<Bool, 0x513560>(this, &out);
}

Bool CKLFile::ReadUInt(UInt &out) {
    return CallMethodAndReturn<Bool, 0x513560>(this, &out);
}

Bool CKLFile::ReadInt(Int *out) {
    return CallMethodAndReturn<Bool, 0x513560>(this, out);
}

Bool CKLFile::ReadUInt(UInt *out) {
    return CallMethodAndReturn<Bool, 0x513560>(this, out);
}

Bool CKLFile::ReadChar(Char &out) {
    return CallMethodAndReturn<Bool, 0x513340>(this, &out);
}

Bool CKLFile::ReadUChar(UChar &out) {
    return CallMethodAndReturn<Bool, 0x513340>(this, &out);
}

Bool CKLFile::ReadChar(Char *out) {
    return CallMethodAndReturn<Bool, 0x513340>(this, out);
}

Bool CKLFile::ReadUChar(UChar *out) {
    return CallMethodAndReturn<Bool, 0x513340>(this, out);
}

Bool CKLFile::ReadShort(Short &out) {
    return CallMethodAndReturn<Bool, 0x513480>(this, &out);
}

Bool CKLFile::ReadUShort(UShort &out) {
    return CallMethodAndReturn<Bool, 0x513480>(this, &out);
}

Bool CKLFile::ReadShort(Short *out) {
    return CallMethodAndReturn<Bool, 0x513480>(this, out);
}

Bool CKLFile::ReadUShort(UShort *out) {
    return CallMethodAndReturn<Bool, 0x513480>(this, out);
}

Bool CKLFile::ReadBool(Bool *out) {
    return CallMethodAndReturn<Bool, 0x5137A0>(this, out);
}

Bool CKLFile::ReadBool(Bool &out) {
    return CallMethodAndReturn<Bool, 0x5137A0>(this, &out);
}

Bool CKLFile::ReadIntAndDelimiter(Int &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513530>(this, &out, symbol);
}

Bool CKLFile::ReadUIntAndDelimiter(UInt &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513530>(this, &out, symbol);
}

Bool CKLFile::ReadIntAndDelimiter(Int *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513530>(this, out, symbol);
}

Bool CKLFile::ReadUIntAndDelimiter(UInt *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513530>(this, out, symbol);
}

Bool CKLFile::ReadShortAndDelimiter(Short &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513450>(this, &out, symbol);
}

Bool CKLFile::ReadUShortAndDelimiter(UShort &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513450>(this, &out, symbol);
}

Bool CKLFile::ReadShortAndDelimiter(Short *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513450>(this, out, symbol);
}

Bool CKLFile::ReadUShortAndDelimiter(UShort *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513450>(this, out, symbol);
}

Bool CKLFile::ReadCharAndDelimiter(Char &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513370>(this, &out, symbol);
}

Bool CKLFile::ReadUCharAndDelimiter(UChar &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513370>(this, &out, symbol);
}

Bool CKLFile::ReadCharAndDelimiter(Char *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513370>(this, out, symbol);
}

Bool CKLFile::ReadUCharAndDelimiter(UChar *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513370>(this, out, symbol);
}

Bool CKLFile::ReadBoolAndDelimiter(Bool *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x5137D0>(this, out, symbol);
}

Bool CKLFile::ReadBoolAndDelimiter(Bool &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x5137D0>(this, &out, symbol);
}

Bool CKLFile::WriteChar(Char value) {
    return WriteChar(&value);
}

Bool CKLFile::WriteUChar(UChar value) {
    return WriteUChar(&value);
}

Bool CKLFile::WriteChar(Char *pValue) {
    return CallMethodAndReturn<Bool, 0x5145A0>(this, pValue);
}

Bool CKLFile::WriteUChar(UChar *pValue) {
    return CallMethodAndReturn<Bool, 0x5145A0>(this, pValue);
}

Bool CKLFile::WriteShort(Short value) {
    return WriteShort(&value);
}

Bool CKLFile::WriteUShort(UShort value) {
    return WriteUShort(&value);
}

Bool CKLFile::WriteShort(Short *pValue) {
    return CallMethodAndReturn<Bool, 0x5146E0>(this, pValue);
}

Bool CKLFile::WriteUShort(UShort *pValue) {
    return CallMethodAndReturn<Bool, 0x5146E0>(this, pValue);
}

Bool CKLFile::WriteInt(Int value) {
    return CallMethodAndReturn<Bool, 0x514A10>(this, value);
}

Bool CKLFile::WriteUInt(UInt value) {
    return CallMethodAndReturn<Bool, 0x514A10>(this, value);
}

Bool CKLFile::WriteInt(Int *pValue) {
    return CallMethodAndReturn<Bool, 0x514510>(this, pValue);
}

Bool CKLFile::WriteUInt(UInt *pValue) {
    return CallMethodAndReturn<Bool, 0x514510>(this, pValue);
}

Bool CKLFile::WriteBool(Bool value) {
    return WriteBool(&value);
}

Bool CKLFile::WriteBool(Bool *pValue) {
    return CallMethodAndReturn<Bool, 0x513940>(this, pValue);
}

Bool CKLFile::WriteString(WideChar const *str) {
    return CallMethodAndReturn<Bool, 0x513810>(this, str);
}

Bool CKLFile::WriteCharAndDelimiter(Char value, WideChar symbol) {
    return WriteCharAndDelimiter(&value, symbol);
}

Bool CKLFile::WriteUCharAndDelimiter(UChar value, WideChar symbol) {
    return WriteUCharAndDelimiter(&value, symbol);
}

Bool CKLFile::WriteCharAndDelimiter(Char *pValue, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x514670>(this, pValue, symbol);
}

Bool CKLFile::WriteUCharAndDelimiter(UChar *pValue, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x514670>(this, pValue, symbol);
}

Bool CKLFile::WriteShortAndDelimiter(Short value, WideChar symbol) {
    return WriteShortAndDelimiter(&value, symbol);
}

Bool CKLFile::WriteUShortAndDelimiter(UShort value, WideChar symbol) {
    return WriteUShortAndDelimiter(&value, symbol);
}

Bool CKLFile::WriteShortAndDelimiter(Short *pValue, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x5147B0>(this, pValue, symbol);
}

Bool CKLFile::WriteUShortAndDelimiter(UShort *pValue, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x5147B0>(this, pValue, symbol);
}

Bool CKLFile::WriteIntAndDelimiter(Int value, WideChar symbol) {
    return WriteIntAndDelimiter(&value, symbol);
}

Bool CKLFile::WriteUIntAndDelimiter(UInt value, WideChar symbol) {
    return WriteUIntAndDelimiter(&value, symbol);
}

Bool CKLFile::WriteIntAndDelimiter(Int *pValue, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x514550>(this, pValue, symbol);
}

Bool CKLFile::WriteUIntAndDelimiter(UInt *pValue, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x514550>(this, pValue, symbol);
}


UChar CCountry::GetID() {
    return CallMethodAndReturn<UChar, 0x4DCE70>(this);
}

WideChar const *CCountry::GetName(UInt languageId) {
    return CallMethodAndReturn<WideChar const *, 0x4DCEC0>(this, languageId);
}

WideChar const *CCompleteWorld::GetDatabasePath() {
    return CallMethodAndReturn<WideChar const *, 0x4C9F40>(this);
}

void CCompleteWorld::SetDatabasePath(WideChar const *dbPath) {
    CallMethod<0x4C9F50>(this, dbPath);
}

Int CCompleteWorld::GetCurrentLanguage() {
    return CallMethodAndReturn<Int, 0x4C9F20>(this);
}

CCountry *CCompleteWorld::GetCountry(Int countryId) {
    return CallMethodAndReturn<CCountry *, 0x4C9DC0>(this, countryId);
}

HWND ComboBoxHWND(void *t) {
    return *raw_ptr<HWND>(t, 0x20);
}

LRESULT ComboBoxAddItem(void *t, const WideChar *itemName, LPARAM itemID) {
    return CallMethodAndReturn<LRESULT, 0x4138A0>(0, t, itemName, itemID);
}

LRESULT ComboBoxSetCurrentItem(void *t, Int itemID) {
    return CallMethodAndReturn<LRESULT, 0x4138E0>(0, t, itemID);
}

Int ComboBoxGetCurrentItem(void *t, Int defaultValue) {
    return CallMethodAndReturn<Int, 0x413990>(0, t, defaultValue);
}

CCompleteWorld *CClub::GetWorld() {
    return CallMethodAndReturn<CCompleteWorld *, 0x4C1B70>(this);
}

WideChar const *CClub::GetName(UInt languageId) {
    return CallMethodAndReturn<WideChar const *, 0x4C1CB0>(this, languageId);
}

Bool32 CStaff::IsValid() {
    return CallMethodAndReturn<Bool32, 0x572990>(this);
}

wchar_t const *GetText(char const *key) {
    void *locale = CallAndReturn<void *, 0x575200>();
    return CallMethodAndReturn<wchar_t const *, 0x5756F0>(locale, key, *raw_ptr<unsigned int>(locale, 0x18));
}

void DDX_Control(void *pDX, int nIDC, void *rControl) {
    CallMethod<0x5B13D5>(0, pDX, nIDC, rControl);
}

void *ComboBoxConstruct(void *t) {
    return CallMethodAndReturn<void *, 0x402710>(t);
}

void ComboBoxDestruct(void *t) {
    CallMethod<0x5BDD37>(t);
}

void *CheckBoxConstruct(void *t) {
    return CallMethodAndReturn<void *, 0x4B5010>(t);
}

void CheckBoxDestruct(void *t) {
    CallMethod<0x4B4990>(t);
}

void *GroupBoxConstruct(void *t) {
    return CallMethodAndReturn<void *, 0x4B3B30>(t);
}

void GroupBoxDestruct(void *t) {
    CallMethod<0x4B3B00>(t);
}

void CheckBoxSetIsChecked(void *t, Bool checked) {
    CallMethod<0x414090>(0, t, checked);
}

Bool CheckBoxGetIsChecked(void *t) {
    return CallMethodAndReturn<Bool32, 0x414070>(0, t);
}

int WndShowWindow(void *t, int nCmdShow) {
    return CallMethodAndReturn<int, 0x5B9D6A>(t, nCmdShow);
}

void CBinaryFile::WriteString(WideChar const *str) { CallMethod<0x550D10>(this, str); }

void CBinaryFile::WriteString(Char const *str) { CallMethod<0x550D60>(this, str); }

void CBinaryFile::WriteTranslations(WideChar const *strings, UInt length) { CallMethod<0x550DB0>(this, strings, length); }

void CBinaryFile::WriteChar(Char value) { CallMethod<0x550E20>(this, value); }

void CBinaryFile::WriteUChar(UChar value) { CallMethod<0x550E20>(this, value); }

void CBinaryFile::WriteData(void const *data, UInt size) { CallMethod<0x550E40>(this, data, size); }

void CBinaryFile::WriteCharArray(Char *values, UInt count) { CallMethod<0x550EA0>(this, values, count); }

void CBinaryFile::WriteUCharArray(UChar *values, UInt count) { CallMethod<0x550EA0>(this, values, count); }

void CBinaryFile::WriteShort(Short value) { CallMethod<0x550F40>(this, value); }

void CBinaryFile::WriteUShort(UShort value) { CallMethod<0x550F40>(this, value); }

void CBinaryFile::WriteShortArray(Short *values, UInt count) { CallMethod<0x550F60>(this, values, count); }

void CBinaryFile::WriteUShortArray(UShort *values, UInt count) { CallMethod<0x550F60>(this, values, count); }

void CBinaryFile::WriteInt(Int value) { CallMethod<0x551000>(this, value); }

void CBinaryFile::WriteUInt(UInt value) { CallMethod<0x551000>(this, value); }

void CBinaryFile::WriteIntArray(Int *values, UInt count) { CallMethod<0x551020>(this, values, count); }

void CBinaryFile::WriteUIntArray(UInt *values, UInt count) { CallMethod<0x551020>(this, values, count); }

void CBinaryFile::WriteInt64(Int64 value) { CallMethod<0x5510C0>(this, value); }

void CBinaryFile::WriteUInt64(UInt64 value) { CallMethod<0x5510C0>(this, value); }

void CBinaryFile::WriteInt64Array(Int64 *values, UInt count) { CallMethod<0x551120>(this, values, count); }

void CBinaryFile::WriteUInt64Array(UInt64 *values, UInt count) { CallMethod<0x551120>(this, values, count); }

void CBinaryFile::WriteFloat(Float value) { CallMethod<0x551160>(this, value); }

void CBinaryFile::WriteFloatArray(Float *values, UInt count) { CallMethod<0x551180>(this, values, count); }

void CBinaryFile::WriteFourcc(UInt value) { CallMethod<0x5511C0>(this, value); }

void CBinaryFile::WriteChunk(void const *data, UInt size) { CallMethod<0x551210>(this, data, size); }
