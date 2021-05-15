#include "WinHeader.h"
#include "Exception.h"
#include "shared.h"
#include "Utils.h"

unsigned int gAddress = 0;
wchar_t gMessage[512];

unsigned int gPreviousAddress = 0;

wchar_t const *GetSafePatchName() {
    static String str = GetPatchNameWithVersion() + L" Exception";
    return str.c_str();
}

void __declspec(naked) ShowExceptionError() {
    __asm mov eax, dword ptr[esp]
        __asm mov gAddress, eax
    if (gPreviousAddress != 0) {
        swprintf(gMessage, L"Unhandled exception at 0x%X/0x%X\n\nPlease make a screensot of this error\nand show to patch developers.\n\nMachen Sie einen Screenshot dieses Fehlers\nund zeigen Sie ihn den Patch-Entwicklern.\n\nПожалуйста, сделайте скриншот этого\nсообщения и покажите его разработичкам.", gAddress, gPreviousAddress);
        gPreviousAddress = 0;
    }
    else
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
