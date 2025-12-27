#include "PlayerAppearance.h"
#include "PlayerAppearanceIDs.h"
#include "RendererHook.h"
#include "UcpSettings.h"
#include "FifamTypes.h"
#include <cwctype>

unsigned int METHOD GetAppearanceAssetCount(void *t, DUMMY_ARG, int type) {
    switch (type) {
    case 0:
        return std::size(headTypes);
    case 1:
        return  std::size(skinColors);
    case 2:
        return std::size(skinTypes);
    case 3:
        return std::size(hairTypes);
    case 4:
        return std::size(hairColors);
    case 5:
        return std::size(eyeColors);
    case 6:
        return 2;
    case 7:
        return std::size(beardTypes);
    case 8:
        return 5;
    }
    return 0;
}

void *gPlayerPlayerFaceDialog = nullptr;

int METHOD OnGetPlayerSpecialFace(void *player) {
    gPlayerPlayerFaceDialog = player;
    return CallMethodAndReturn<int, 0x51C410>(player);
}

void StorePlayerEyeColorForStarhead(void *data) {
    *raw_ptr<UInt>(data, 0x74) = 2;
    if (gPlayerPlayerFaceDialog) {
        UChar eyeColor = CallMethodAndReturn<UChar, 0x51C3D0>(gPlayerPlayerFaceDialog);
        if (eyeColor < std::size(eyeColors))
            *raw_ptr<UInt>(data, 0x74) = eyeColors[eyeColor];
    }
}

void __declspec(naked) OnGetPlayerEyeColorForStarhead() {
    __asm push ebp
    __asm call StorePlayerEyeColorForStarhead
    __asm pop ebp
    __asm mov eax, 0x4B6B34
    __asm jmp eax
}

void METHOD CPlayer_SetHairStyle(void *t, DUMMY_ARG, UShort id) {
    *raw_ptr<UChar>(t, 0x1DA) = UChar(id & 0xFF);
    *raw_ptr<UChar>(t, 0x1DC) = UChar((id >> 8) & 0xFF);
}

UInt METHOD CPlayer_GetHairStyle(void *t) {
    UInt low = UInt(*raw_ptr<UChar>(t, 0x1DA));
    UInt high = UInt(*raw_ptr<UChar>(t, 0x1DC));
    return (high << 8) | low;
}

UChar METHOD CPlayer_GetHairStyle_LowPart(void *t) {
    return *raw_ptr<UChar>(t, 0x1DA);
}

UChar METHOD CPlayer_GetHairStyle_HighPart(void *t) {
    return *raw_ptr<UChar>(t, 0x1DC);
}

void METHOD CPlayer_SetSideburnsType(void *t, DUMMY_ARG, UChar) {}

UChar METHOD CPlayer_GetSideburnsType(void *t) {
    return 0;
}

void __declspec(naked) CDBFM09ToFIFAFaceParamMapper_GenerateAppearance1() {
    __asm {
        mov edi, [esp + 0x44]
        mov[edi + 2], al
        mov dl, ah
        and dl, 0xF
        mov cl, [edi + 6]
        and cl, 0xF0
        or cl, dl
        mov[edi + 6], cl
        mov eax, 0x5A016C
        jmp eax
    }
}

void __declspec(naked) CDBFM09ToFIFAFaceParamMapper_GenerateAppearance2() {
    __asm {
        mov edi, [esp + 0x44]
        mov[edi + 2], al
        mov dl, ah
        and dl, 0xF
        mov cl, [edi + 6]
        and cl, 0xF0
        or cl, dl
        mov[edi + 6], cl
        mov eax, 0x5A0005
        jmp eax
    }
}

bool METHOD OnReadSavPlayerHairStyle(void *file, DUMMY_ARG, UChar *out, WideChar character) {
    UChar *player = out - 0x1DA;
    UShort hairId = 0;
    CallMethod<0x5134B0>(file, &hairId, character); // read UShort
    CPlayer_SetHairStyle(player, 0, hairId);
    return true;
}

bool METHOD OnReadSavPlayerSideburns(void *file, DUMMY_ARG, UChar *out, WideChar character) {
    UChar dummy = 0;
    CallMethod<0x5133D0>(file, &dummy, character); // read UChar
    return true;
}

bool METHOD OnWriteSavPlayerHairStyle(void *file, DUMMY_ARG, UChar *data, WideChar character) {
    UChar *player = data - 0x1DA;
    UShort hairId = (UShort)CPlayer_GetHairStyle(player);
    return CallMethodAndReturn<Bool, 0x5147B0>(file, &hairId, character); // write UShort
}

bool METHOD OnWriteSavPlayerSideburns(void *file, DUMMY_ARG, UChar *data, WideChar character) {
    UChar zero = 0;
    return CallMethodAndReturn<Bool, 0x514670>(file, &zero, character); // write UChar
}

struct AppearanceComboBoxItem {
    String text;
    Int id;

    AppearanceComboBoxItem() {
        id = 0;
    }

    AppearanceComboBoxItem(String const &_text, Int _id) {
        text = _text;
        id = _id;
    }
};

Vector<AppearanceComboBoxItem> &AppearanceComboBoxItems() {
    static Vector<AppearanceComboBoxItem> items;
    return items;
}

void METHOD OnAppearanceComboBoxAddItem(void *t, DUMMY_ARG, WideChar const *text, Int id) {
    AppearanceComboBoxItems().push_back(AppearanceComboBoxItem(text, id));
}

Int AppearanceComboBoxSorting(String const &a, const String &b) {
    UInt i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (iswdigit(a[i]) && iswdigit(b[j])) {
            UInt startA = i;
            while (i < a.size() && iswdigit(a[i]))
                i++;
            Int numA = std::stoi(a.substr(startA, i - startA));
            UInt startB = j;
            while (j < b.size() && iswdigit(b[j]))
                j++;
            Int numB = std::stoi(b.substr(startB, j - startB));
            if (numA != numB)
                return (numA < numB) ? -1 : 1;
            continue;
        }
        WideChar c1 = std::towlower(a[i]);
        WideChar c2 = std::towlower(b[j]);
        if (c1 != c2)
            return (c1 < c2) ? -1 : 1;
        i++;
        j++;
    }
    if (a.size() == b.size())
        return 0;
    return (a.size() < b.size()) ? -1 : 1;
}

void __stdcall OnAppearanceComboBoxReset(HWND hCombo) {
    AppearanceComboBoxItems().clear();
    SendMessageW(hCombo, CB_RESETCONTENT, 0, 0);
}

void FillAppearanceComboBox(HWND hCombo, Vector<AppearanceComboBoxItem> &items) {
    std::sort(items.begin(), items.end(), [](const AppearanceComboBoxItem &A, const AppearanceComboBoxItem &B) {
        auto aLowered = ToLower(A.text);
        auto bLowered = ToLower(B.text);
        bool aMale = StartsWith(aLowered, L"male ");
        bool bMale = StartsWith(bLowered, L"male ");
        bool aFemale = StartsWith(aLowered, L"female ");
        bool bFemale = StartsWith(bLowered, L"female ");
        if (aMale && bFemale)
            return true;
        if (aFemale && bMale)
            return false;
        return AppearanceComboBoxSorting(A.text, B.text) < 0;
    });
    SendMessageW(hCombo, CB_RESETCONTENT, 0, 0);
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        int idx = (int)SendMessageW(hCombo, CB_INSERTSTRING, 0, (LPARAM)it->text.c_str());
        SendMessageW(hCombo, CB_SETITEMDATA, idx, it->id);
    }
}

void __stdcall OnAppearanceComboBoxRedraw(HWND hCombo) {
    FillAppearanceComboBox(hCombo, AppearanceComboBoxItems());
    AppearanceComboBoxItems().clear();
    SendMessageW(hCombo, CB_SETCURSEL, 0, 0);
    SendMessageW(hCombo, WM_SETREDRAW, 1, 0);
}

void ResortAppearanceComboBox(HWND hCombo) {
    Int count = (Int)SendMessageW(hCombo, CB_GETCOUNT, 0, 0);
    if (count <= 1)
        return;
    Vector<AppearanceComboBoxItem> items;
    items.reserve(count);
    for (Int i = 0; i < count; i++) {
        WideChar buffer[512];
        SendMessageW(hCombo, CB_GETLBTEXT, i, (LPARAM)buffer);
        LPARAM data = SendMessageW(hCombo, CB_GETITEMDATA, i, 0);
        items.push_back(AppearanceComboBoxItem(buffer, data));
    }
    SendMessageW(hCombo, CB_RESETCONTENT, 0, 0);
    FillAppearanceComboBox(hCombo, items);
}

void __stdcall OnAppearanceFilterComboBoxSetCurrSel(HWND hCombo) {
    ResortAppearanceComboBox(hCombo);
    SendMessageW(hCombo, CB_SETCURSEL, 0, 0);
}

void METHOD ConvertPlayerAppearanceFromFM09(void *app, DUMMY_ARG, UChar hairStyle, UInt face, UChar hairColor, UChar beard) {
    memset(app, 0, 8);
}

void PatchPlayerAppearance(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        // sideburns
        patch::SetUInt(0x681C14 + 4 * 1, 0); // sideburns 1

        // beard types
        patch::SetUInt(0x681C40, std::size(beardTypes));
        patch::SetPointer(0x418720 + 0xA8 + 2, beardTypes); // mov     ecx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0xC9 + 2, beardTypes); // mov     edx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0xF3 + 1, beardTypes); // mov     eax, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x11D + 2, beardTypes); // mov     ecx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x13E + 2, beardTypes); // mov     edx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x15C + 2, beardTypes); // mov     edx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x17A + 2, beardTypes); // mov     edx, ds:gFifaBeardTypes
        patch::SetPointer(0x418720 + 0x191 + 3, beardTypes); // mov     ecx, ds:gFifaBeardTypes[eax*4]
        patch::SetPointer(0x41FE10 + 0x110 + 3, beardTypes); // mov     edx, ds:gFifaBeardTypes[ecx*4]
        patch::SetPointer(0x486E50 + 0x17C + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[edx*4]
        patch::SetPointer(0x4B6AE0 + 0x11E + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[edx*4]
        patch::SetPointer(0x418720 + 0x48 + 1, beardTypes); // mov     eax, ds:gFifaBeardTypes+4
        patch::SetPointer(0x486DCC + 3, beardTypes); // mov     edx, ds:gFifaBeardTypes[eax*4]; jumptable 00486D5C case 7
        patch::SetPointer(0x6D0A84, beardTypes); // .data:006D0A84	dd offset gFifaBeardTypes
        patch::SetPointer(0x5A04CD + 1, &beardTypes[0]);
        patch::SetPointer(0x5A04C8 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A04DE + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A04F7 + 1, &beardTypes[0]);
        patch::SetPointer(0x5A054D + 1, &beardTypes[0]);
        patch::SetPointer(0x5A0548 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A055E + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A0575 + 1, &beardTypes[0]);
        patch::SetPointer(0x5A0570 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A058A + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x5A059C + 1, &beardTypes[0]);
        patch::SetPointer(0x59F19C + 1, &beardTypes[0]);
        patch::SetPointer(0x59F197 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x59F1A9 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0x59F1B8 + 1, &beardTypes[0]);

        // skin colors
        patch::SetUInt(0x681C44, std::size(skinColors));
        patch::SetPointer(0x418720 + 0xC2 + 3, skinColors); // mov     ecx, ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x41FE10 + 0xB6 + 3, skinColors); // mov     edx, ds:gFifaSkinColors[ecx*4]
        patch::SetPointer(0x486E50 + 0xDA + 3, skinColors); // mov     eax, ds:gFifaSkinColors[edx*4]
        patch::SetPointer(0x4B6AE0 + 0x9D + 3, skinColors); // mov     eax, ds:gFifaSkinColors[edx*4]
        patch::SetPointer(0x418720 + 0x2A + 1, skinColors); // mov     eax, ds:gFifaSkinColors+4
        patch::SetPointer(0x486D72 + 3, skinColors); // mov     edx, ds:gFifaSkinColors[eax*4]; jumptable 00486D5C case 1
        patch::SetPointer(0x6D0A74, skinColors); // .data:006D0A74	dd offset gFifaSkinColors
        patch::SetPointer(0x59FB63 + 1, &skinColors[0]);
        patch::SetPointer(0x59FB5E + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FB74 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FB7F + 1, &skinColors[0]);
        patch::SetPointer(0x59FDE0 + 1, &skinColors[0]);
        patch::SetPointer(0x59FDDB + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FDF1 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FE07 + 1, &skinColors[0]);
        patch::SetPointer(0x59FE02 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FE18 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59FE23 + 1, &skinColors[0]);
        patch::SetPointer(0x59F21C + 1, &skinColors[0]);
        patch::SetPointer(0x59F217 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59F229 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0x59F238 + 1, &skinColors[0]);

        // hair colors
        patch::SetUInt(0x681C50, std::size(hairColors));
        patch::SetPointer(0x418720 + 0x155 + 3, hairColors); // mov     ecx, ds:gFifaHairColors[eax*4]
        patch::SetPointer(0x41FE10 + 0xF2 + 3, hairColors); // mov     ecx, ds:gFifaHairColors[eax*4]
        patch::SetPointer(0x486E50 + 0x140 + 3, hairColors); // mov     edx, ds:gFifaHairColors[ecx*4]
        patch::SetPointer(0x4B6AE0 + 0xEE + 3, hairColors); // mov     edx, ds:gFifaHairColors[ecx*4]
        patch::SetPointer(0x418720 + 0x5B + 2, hairColors); // mov     ecx, ds:gFifaHairColors+4
        patch::SetPointer(0x486D9F + 3, hairColors); // mov     edx, ds:gFifaHairColors[eax*4]; jumptable 00486D5C case 4
        patch::SetPointer(0x6D0A8C, hairColors); // .data:006D0A8C	dd offset gFifaHairColors
        patch::SetPointer(0x5A0269 + 1, &hairColors[0]);
        patch::SetPointer(0x5A0264 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A027A + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0285 + 1, &hairColors[0]);
        patch::SetPointer(0x5A036F + 1, &hairColors[0]);
        patch::SetPointer(0x5A036A + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0380 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0397 + 1, &hairColors[0]);
        patch::SetPointer(0x5A0392 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A03AC + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A03B8 + 1, &hairColors[0]);
        patch::SetPointer(0x59F11C + 1, &hairColors[0]);
        patch::SetPointer(0x59F117 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x59F129 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x59F138 + 1, &hairColors[0]);

        // hair models
        patch::SetUInt(0x681C3C, std::size(hairTypes));
        patch::SetPointer(0x418720 + 0xCF + 1, hairTypes); // mov     eax, ds:gFifaHairModels
        patch::SetPointer(0x418720 + 0xF8 + 2, hairTypes); // mov     ecx, ds:gFifaHairModels
        patch::SetPointer(0x418720 + 0x137 + 3, hairTypes); // mov     ecx, ds:gFifaHairModels[eax*4]
        patch::SetPointer(0x41FE10 + 0xE3 + 3, hairTypes); // mov     edx, ds:gFifaHairModels[ecx*4]
        patch::SetPointer(0x486E50 + 0x12B + 3, hairTypes); // mov     eax, ds:gFifaHairModels[edx*4]
        patch::SetPointer(0x4B6AE0 + 0xDC + 3, hairTypes); // mov     eax, ds:gFifaHairModels[edx*4]
        patch::SetPointer(0x4188D0 + 2, hairTypes); // mov     edx, ds:gFifaHairModels
        patch::SetPointer(0x486D90 + 3, hairTypes); // mov     ecx, ds:gFifaHairModels[eax*4]; jumptable 00486D5C case 3
        patch::SetPointer(0x6D0A70, hairTypes); // .data:006D0A70	dd offset gFifaHairModels
        patch::SetPointer(0x59FFD1 + 1, &hairTypes[0]);
        patch::SetPointer(0x59FFCC + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x59FFE2 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x59FFED + 1, &hairTypes[0]);
        patch::SetPointer(0x5A0110 + 1, &hairTypes[0]);
        patch::SetPointer(0x5A010B + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x5A0121 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x5A0138 + 1, &hairTypes[0]);
        patch::SetPointer(0x5A0133 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x5A014D + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x5A0159 + 1, &hairTypes[0]);
        patch::SetPointer(0x59F0DC + 1, &hairTypes[0]);
        patch::SetPointer(0x59F0D7 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x59F0E9 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0x59F0F8 + 1, &hairTypes[0]);

        // head models
        patch::SetUInt(0x681C38, std::size(headTypes));
        patch::SetPointer(0x418720 + 0xA1 + 3, headTypes); // mov     eax, ds:gFifaHeadModels[edx*4]
        patch::SetPointer(0x41FE10 + 0xAB + 3, headTypes); // mov     eax, ds:gFifaHeadModels[edx*4]
        patch::SetPointer(0x486E50 + 0xB9 + 3, headTypes); // mov     ecx, ds:gFifaHeadModels[eax*4]
        patch::SetPointer(0x4B6AE0 + 0x85 + 3, headTypes); // mov     ecx, ds:gFifaHeadModels[eax*4]
        patch::SetPointer(0x418720 + 0x2F + 2, headTypes); // mov     edx, ds:gFifaHeadModels+4
        patch::SetPointer(0x486D63 + 3, headTypes); // mov     ecx, ds:gFifaHeadModels[eax*4]; jumptable 00486D5C case 0
        patch::SetPointer(0x6D0A6C, headTypes); // .data:off_6D0A6C	dd offset gFifaHeadModels
        patch::SetPointer(0x59FAF1 + 1, &headTypes[0]);
        patch::SetPointer(0x59FAEC + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FB02 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FB0D + 1, &headTypes[0]);
        patch::SetPointer(0x59FD31 + 1, &headTypes[0]);
        patch::SetPointer(0x59FD2C + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FD42 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FD4D + 1, &headTypes[0]);
        patch::SetPointer(0x59FD6C + 1, &headTypes[0]);
        patch::SetPointer(0x59FD67 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FD7D + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59FD88 + 1, &headTypes[0]);
        patch::SetPointer(0x59F09C + 1, &headTypes[0]);
        patch::SetPointer(0x59F097 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59F0A9 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0x59F0B8 + 1, &headTypes[0]);

        patch::SetUInt(0x681C58, std::size(eyeColors));
        patch::SetPointer(0x418720 + 0x51 + 2, eyeColors); // mov     edx, ds:gFifaEyeColors
        patch::SetPointer(0x418720 + 0x116 + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0x41FE10 + 0xD4 + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0x486E50 + 0x10A + 3, eyeColors); // mov     ecx, ds:gFifaEyeColors[eax*4]
        patch::SetPointer(0x4B6AE0 + 0xC4 + 3, eyeColors); // mov     ecx, ds:gFifaEyeColors[eax*4]
        patch::SetPointer(0x486DAE + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[eax*4]; jumptable 00486D5C case 5
        patch::SetPointer(0x6D0A80, eyeColors); // .data:006D0A80	dd offset gFifaEyeColors
        patch::SetPointer(0x59FC73 + 1, &eyeColors[0]);
        patch::SetPointer(0x59FC6E + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FC84 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FC8F + 1, &eyeColors[0]);
        patch::SetPointer(0x59FCAF + 1, &eyeColors[0]);
        patch::SetPointer(0x59FCAA + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FCC4 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FCD0 + 1, &eyeColors[0]);
        patch::SetPointer(0x59FF19 + 1, &eyeColors[0]);
        patch::SetPointer(0x59FF14 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FF2A + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FF40 + 1, &eyeColors[0]);
        patch::SetPointer(0x59FF3B + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FF51 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59FF5C + 1, &eyeColors[0]);
        patch::SetPointer(0x59F29C + 1, &eyeColors[0]);
        patch::SetPointer(0x59F297 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59F2A9 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0x59F2B8 + 1, &eyeColors[0]);

        // skin types
        patch::SetUInt(0x681C48, std::size(skinTypes));
        patch::SetPointer(0x418755 + 2, skinTypes);
        patch::SetPointer(0x41880C + 3, skinTypes);
        patch::SetPointer(0x41FED5 + 3, skinTypes);
        patch::SetPointer(0x486D81 + 3, skinTypes);
        patch::SetPointer(0x486F3F + 3, skinTypes);
        patch::SetPointer(0x4B6B8F + 3, skinTypes);
        patch::SetPointer(0x6D0A78, skinTypes); // .data:006D0A78 dd offset gFifaVariationFaceTypes
        patch::SetPointer(0x59F25C + 1, &skinTypes[0]);
        patch::SetPointer(0x59F257 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0x59F269 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0x59F278 + 1, &skinTypes[0]);
        patch::SetPointer(0x59FBD9 + 1, &skinTypes[0]);
        patch::SetPointer(0x59FBD4 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0x59FBEA + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0x59FBF5 + 1, &skinTypes[0]);
        patch::SetPointer(0x59FE76 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0x59FE8C + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0x59FE7B + 1, &skinTypes[0]);
        patch::SetPointer(0x59FEA2 + 1, &skinTypes[0]);
        patch::SetPointer(0x59FE9D + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0x59FEB3 + 1, &skinTypes[std::size(skinTypes)]);
        patch::SetPointer(0x59FEBE + 1, &skinTypes[0]);

        patch::RedirectJump(0x4FB290, GetAppearanceAssetCount);
        
        // starheads eye colors
        patch::RedirectCall(0x4B6B1D, OnGetPlayerSpecialFace);
        patch::RedirectJump(0x4B6B2D, OnGetPlayerEyeColorForStarhead);

        // increase 3D face limit - 6000 => 12000
        patch::SetUInt(0x6C9560 + 4, Settings::GetInstance().Max3dFaces);
        patch::SetUInt(0x6C9560 + 8, Settings::GetInstance().Max3dFaces);

        // -- Hair ID increase --
        patch::RedirectJump(0x51C360, CPlayer_SetHairStyle);
        patch::RedirectJump(0x51C350, CPlayer_GetHairStyle);
        patch::RedirectJump(0x51C390, CPlayer_GetSideburnsType);
        patch::RedirectJump(0x51C3A0, CPlayer_SetSideburnsType);
        patch::SetUChar(0x486F78 + 1, 0xB7); // movzx edx, al => movzx edx, ax
        patch::SetUChar(0x48873A, 0x89); // mov byte ptr [esp+0x34], al => mov dword ptr [esp+0x34], eax
        patch::SetUChar(0x488947, 0x39); // cmp byte ptr [esp+0x34], al => cmp dword ptr [esp+0x34], eax
        // CPlayer::SetAppearance
        patch::SetUChar(0x51C265, 0x8B); // mov dl, [esp+8] => mov edx, [esp+8]
        patch::Nop(0x51C2BA, 6); // sideburns set
        //
        patch::Nop(0x488CD6, 3); // movzx eax, al
        patch::Nop(0x4B59C6, 3); // movzx eax, al
        patch::SetUChar(0x4B6BB9 + 1, 0xB7); // movzx edx, al => movzx edx, ax
        patch::SetUShort(0x4F7A80, 0xD88B); // mov bl, al => mov ebx, eax
        patch::SetUShort(0x4F7A87, 0xC33B); // cmp al, bl => cmp eax, ebx
        patch::RedirectCall(0x53456F, CPlayer_GetHairStyle_LowPart);
        patch::RedirectCall(0x53458D, CPlayer_GetHairStyle_HighPart);
        // CDBFM09ToFIFAFaceParamMapper::GenerateAppearance
        patch::Nop(0x5A020B, 3);
        patch::Nop(0x5A00BB, 3);
        patch::Nop(0x5A0098, 3);
        patch::SetUChar(0x5A0163, 0x33); // xor al, al => xor eax, eax
        patch::SetUChar(0x59FFFC, 0x33); // xor al, al => xor eax, eax
        patch::RedirectJump(0x5A0165, CDBFM09ToFIFAFaceParamMapper_GenerateAppearance1);
        patch::RedirectJump(0x59FFFE, CDBFM09ToFIFAFaceParamMapper_GenerateAppearance2);
        //
        patch::Nop(0x4B5DAA, 6);
        // .sav reading
        patch::RedirectCall(0x52720E, OnReadSavPlayerHairStyle);
        patch::RedirectCall(0x52723E, OnReadSavPlayerSideburns);
        // .sav writing
        patch::RedirectCall(0x51F90F, OnWriteSavPlayerHairStyle);
        patch::RedirectCall(0x51F93F, OnWriteSavPlayerSideburns);

        // sorting in appearance comboboxes
        patch::SetUChar(0x4B6244, 0x51); // push ecx
        patch::RedirectCall(0x4B6244 + 1, OnAppearanceComboBoxReset);
        patch::Nop(0x4B6244 + 1 + 5, 6);
        patch::RedirectCall(0x4B628A, OnAppearanceComboBoxAddItem);
        patch::Nop(0x4B62A2, 21);
        patch::SetUChar(0x4B62BA, 0x52); // push edx
        patch::RedirectCall(0x4B62BA + 1, OnAppearanceComboBoxRedraw);
        patch::Nop(0x4B62BA + 1 + 5, 3);
        patch::SetUChar(0x41F3CB, 0x52); // push edx
        patch::RedirectCall(0x41F3CB + 1, OnAppearanceFilterComboBoxSetCurrSel);
        patch::Nop(0x41F3CB + 1 + 5, 6);
        patch::SetUChar(0x486C98, 0x51); // push ecx
        patch::RedirectCall(0x486C98 + 1, OnAppearanceFilterComboBoxSetCurrSel);
        patch::Nop(0x486C98 + 1 + 5, 6);

        // remove FM09 generator
        patch::RedirectJump(0x5A36C0, ConvertPlayerAppearanceFromFM09);
    }
}
