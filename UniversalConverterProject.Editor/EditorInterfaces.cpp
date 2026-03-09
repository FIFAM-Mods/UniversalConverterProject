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

Bool CKLFile::ReadIntAndSymbol(Int &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513530>(this, &out, symbol);
}

Bool CKLFile::ReadUIntAndSymbol(UInt &out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513530>(this, &out, symbol);
}

Bool CKLFile::ReadIntAndSymbol(Int *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513530>(this, out, symbol);
}

Bool CKLFile::ReadUIntAndSymbol(UInt *out, WideChar symbol) {
    return CallMethodAndReturn<Bool, 0x513530>(this, out, symbol);
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

Bool CKLFile::WriteString(WideChar const *str) {
    return CallMethodAndReturn<Bool, 0x513810>(this, str);
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
