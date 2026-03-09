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
    WideChar *m_pCurrentCharacter;
    void *m_pEof;
    UInt m_nNumExecutedOperations;
    void *m_pFileIO;
    void *m_pEncryption;
    void *m_pszErrorText;

    Bool IsVersionGreaterOrEqual(UInt year, UShort build);
    Bool ReadString(WideChar *out, UInt maxLen);
    Bool ReadStringArray(void *out, UInt count, UInt maxLen);
    Bool ReadInt(Int &out);
    Bool ReadUInt(UInt &out);
    Bool ReadInt(Int *out);
    Bool ReadUInt(UInt *out);
    Bool ReadIntAndSymbol(Int &out, WideChar symbol);
    Bool ReadUIntAndSymbol(UInt &out, WideChar symbol);
    Bool ReadIntAndSymbol(Int *out, WideChar symbol);
    Bool ReadUIntAndSymbol(UInt *out, WideChar symbol);
    Bool WriteInt(Int value);
    Bool WriteUInt(UInt value);
    Bool WriteInt(Int *pValue);
    Bool WriteUInt(UInt *pValue);
    Bool WriteString(WideChar const *str);
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
