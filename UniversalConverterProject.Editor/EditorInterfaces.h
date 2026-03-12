#pragma once
#include "FifamTypes.h"
#include "plugin.h"

using namespace plugin;

class CKLFile {
public:
    Int m_nOpenState;
    Int m_nEncodingType;
    UInt m_nVersion;
    UInt m_nFlags;
    WideChar m_szFilePath[260];
    void *m_pFileData;
    UInt m_nFileDataSize;
    UInt m_nAlignment;
    WideChar *m_pCurrentPos;
    void *m_pEof;
    UInt m_nNumExecutedOperations;
    void *m_pFileIO;
    void *m_pEncryption;
    void *m_pszErrorText;

    Bool IsVersionGreaterOrEqual(UInt year, UShort build);
    Bool ReadChar(Char &out);
    Bool ReadUChar(UChar &out);
    Bool ReadChar(Char *out);
    Bool ReadUChar(UChar *out);
    Bool ReadShort(Short &out);
    Bool ReadUShort(UShort &out);
    Bool ReadShort(Short *out);
    Bool ReadUShort(UShort *out);
    Bool ReadInt(Int &out);
    Bool ReadUInt(UInt &out);
    Bool ReadInt(Int *out);
    Bool ReadUInt(UInt *out);
    Bool ReadBool(Bool *out);
    Bool ReadBool(Bool &out);
    Bool ReadCharAndDelimiter(Char &out, WideChar symbol);
    Bool ReadUCharAndDelimiter(UChar &out, WideChar symbol);
    Bool ReadCharAndDelimiter(Char *out, WideChar symbol);
    Bool ReadUCharAndDelimiter(UChar *out, WideChar symbol);
    Bool ReadShortAndDelimiter(Short &out, WideChar symbol);
    Bool ReadUShortAndDelimiter(UShort &out, WideChar symbol);
    Bool ReadShortAndDelimiter(Short *out, WideChar symbol);
    Bool ReadUShortAndDelimiter(UShort *out, WideChar symbol);
    Bool ReadIntAndDelimiter(Int &out, WideChar symbol);
    Bool ReadUIntAndDelimiter(UInt &out, WideChar symbol);
    Bool ReadIntAndDelimiter(Int *out, WideChar symbol);
    Bool ReadUIntAndDelimiter(UInt *out, WideChar symbol);
    Bool ReadBoolAndDelimiter(Bool *out, WideChar symbol);
    Bool ReadBoolAndDelimiter(Bool &out, WideChar symbol);
    Bool ReadString(WideChar *out, UInt maxLen);
    Bool ReadStringArray(void *out, UInt count, UInt maxLen);
    Bool WriteChar(Char value);
    Bool WriteUChar(UChar value);
    Bool WriteChar(Char *pValue);
    Bool WriteUChar(UChar *pValue);
    Bool WriteShort(Short value);
    Bool WriteUShort(UShort value);
    Bool WriteShort(Short *pValue);
    Bool WriteUShort(UShort *pValue);
    Bool WriteInt(Int value);
    Bool WriteUInt(UInt value);
    Bool WriteInt(Int *pValue);
    Bool WriteUInt(UInt *pValue);
    Bool WriteCharAndDelimiter(Char value, WideChar symbol);
    Bool WriteUCharAndDelimiter(UChar value, WideChar symbol);
    Bool WriteCharAndDelimiter(Char *pValue, WideChar symbol);
    Bool WriteUCharAndDelimiter(UChar *pValue, WideChar symbol);
    Bool WriteShortAndDelimiter(Short value, WideChar symbol);
    Bool WriteUShortAndDelimiter(UShort value, WideChar symbol);
    Bool WriteShortAndDelimiter(Short *pValue, WideChar symbol);
    Bool WriteUShortAndDelimiter(UShort *pValue, WideChar symbol);
    Bool WriteIntAndDelimiter(Int value, WideChar symbol);
    Bool WriteUIntAndDelimiter(UInt value, WideChar symbol);
    Bool WriteIntAndDelimiter(Int *pValue, WideChar symbol);
    Bool WriteUIntAndDelimiter(UInt *pValue, WideChar symbol);
    Bool WriteBool(Bool *pValue);
    Bool WriteBool(Bool value);
    Bool WriteString(WideChar const *str);
};

class CCountry;
class CClub;
class CPlayer;

class CBinaryFile {
public:
    UInt m_nCurrByte;
    void *m_pFile;
    void *m_pData;
    UInt m_acType;
    UInt m_nVersion;
    UInt m_nDataSize;
    UInt m_nMagic;
    UInt m_nNumTranslations;
    UInt dword20;
    CCountry *m_pCountry;
    CClub *m_pClub;
    CPlayer *m_pPlayer;

    void WriteString(WideChar const *str);
    void WriteString(Char const *str);
    void WriteTranslations(WideChar const *strings, UInt length);
    void WriteChar(Char value);
    void WriteUChar(UChar value);
    void WriteData(void const *data, UInt size);
    void WriteCharArray(Char *values, UInt count);
    void WriteUCharArray(UChar *values, UInt count);
    void WriteShort(Short value);
    void WriteUShort(UShort value);
    void WriteShortArray(Short *values, UInt count);
    void WriteUShortArray(UShort *values, UInt count);
    void WriteInt(Int value);
    void WriteUInt(UInt value);
    void WriteIntArray(Int *values, UInt count);
    void WriteUIntArray(UInt *values, UInt count);
    void WriteInt64(Int64 value);
    void WriteUInt64(UInt64 value);
    void WriteInt64Array(Int64 *values, UInt count);
    void WriteUInt64Array(UInt64 *values, UInt count);
    void WriteFloat(Float value);
    void WriteFloatArray(Float *values, UInt count);
    void WriteFourcc(UInt value);
    void WriteChunk(void const *data, UInt size);
};

class CCountry {
public:
    UChar GetID();
    WideChar const *GetName(UInt languageId = 1);
};

class CCompleteWorld {
public:
    WideChar const *GetDatabasePath();
    void SetDatabasePath(WideChar const *dbPath);
    Int GetCurrentLanguage();
    CCountry *GetCountry(Int countryId);
};

class CPlayer {
public:
};

class CStaff {
public:
    CPlayer *m_pPlayer;

    Bool32 IsValid();
};

class CClub {
public:
    CCompleteWorld *GetWorld();
    WideChar const *GetName(UInt languageId = 1);
};

class CDBCompetition;
class CComboBox;

HWND ComboBoxHWND(void *t);
LRESULT ComboBoxAddItem(void *t, const WideChar *itemName, LPARAM itemID);
LRESULT ComboBoxSetCurrentItem(void *t, Int itemID);
Int ComboBoxGetCurrentItem(void *t, Int defaultValue = 0);
wchar_t const *GetText(char const *key);
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
