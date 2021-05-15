#include "Talent6Stars.h"
//#include <Windows.h>

using namespace plugin;

/*
template<unsigned int OpDelete, unsigned int OpNewArray, unsigned int BitmapFromTGA>
void METHOD MySetTalentImages(int _this, DUMMY_ARG, const wchar_t *resType, const wchar_t **resNames, int resCount) {
    void *oldData = *(void **)(_this + 84);
    if (oldData)
        Call<OpDelete>(oldData);
    *(DWORD *)(_this + 84) = CallAndReturn<DWORD, OpNewArray>(4 * resCount);
    *(DWORD *)(_this + 88) = resCount;
    HMODULE hModule = GetModuleHandleW(0);
    for (int i = 0; i < resCount; i++) {
        void *resData = nullptr;
        unsigned char *tgaData = nullptr;
        if (i == 10) {
            FILE *file = _wfopen(L"plugins\\EditorTalent11.tga", L"rb");
            if (file) {
                fseek(file, 0, SEEK_END);
                auto fileSize = ftell(file);
                fseek(file, 0, SEEK_SET);
                tgaData = new unsigned char[fileSize];
                fread(tgaData, fileSize, 1, file);
                fclose(file);
                resData = tgaData;
            }
        }
        else {
            HRSRC hRes = FindResourceW(hModule, resNames[i], resType);
            HGLOBAL hGlobal = LoadResource(hModule, hRes);
            SizeofResource(hModule, hRes);
            resData = LockResource(hGlobal);
        }
        *(DWORD *)(*(DWORD *)(_this + 84) + 4 * i) = CallAndReturn<DWORD, BitmapFromTGA>(resData);
        if (i == 10 && tgaData)
            delete[] tgaData;
    }
    if (*(BYTE *)(_this + 92))
        SendMessageW(*(HWND *)(_this + 132), 1135, 0, *(DWORD *)(_this + 88) - 1);
}
*/

void PatchTalentStars(FM::Version v) {
    static const wchar_t *pStarsTextFmt = L"%d / 11";

    if (v.id() == ID_ED_13_1000) {
        //patch::SetUChar(0x46D166 + 1, 11);
        //patch::SetUChar(0x51C664 + 1, 10);
        //patch::SetUChar(0x51C66B + 1, 10);
        //patch::RedirectCall(0x46D17A, MySetTalentImages<0x5B04B9, 0x5B04AE, 0x54EFB0>);
        //patch::SetPointer(0x46D640 + 1, (void *)pStarsTextFmt);
    }
}
