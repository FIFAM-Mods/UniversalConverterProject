#include "Exception.h"
#include <Windows.h>
#include "shared.h"
#include "Utils.h"
#include "license_check/license_check.h"

unsigned int gAddress = 0;
wchar_t gMessage[512];

wchar_t const *GetSafePatchName() {
    
    String str;
    str = GetPatchNameWithVersion() + Magic<' ','E','x','c','p','e','t','i','o','n'>(2057051410);
    return str.c_str();
}

void __declspec(naked) ShowExceptionError() {
    __asm mov eax, dword ptr[esp]
        __asm mov gAddress, eax
    swprintf(gMessage, L"Unhandled exception at 0x%X\n\nPlease make a screensot of this error\nand show to patch developers.\n\nMachen Sie einen Screenshot dieses Fehlers\nund zeigen Sie ihn den Patch-Entwicklern.\n\nПожалуйста, сделайте скриншот этого\nсообщения и покажите его разработичкам.", gAddress);
    MessageBoxW(NULL, gMessage, GetSafePatchName(), MB_ICONERROR);
    __asm retn
}

unsigned int gAddress2 = 0;
wchar_t gMessage2[512];

void __declspec(naked) ShowExceptionError_GfxCore() {
    __asm mov eax, dword ptr[esp]
        __asm mov gAddress2, eax
    swprintf(gMessage2, L"Unhandled exception at 0x%X (GfxCore)\n\nPlease make a screensot of this error\nand show to patch developers.\n\nMachen Sie einen Screenshot dieses Fehlers\nund zeigen Sie ihn den Patch-Entwicklern.\n\nПожалуйста, сделайте скриншот этого\nсообщения и покажите его разработичкам.", gAddress2);
    MessageBoxW(NULL, gMessage2, GetSafePatchName(), MB_ICONERROR);
    __asm retn
}
