#include "TextFiles.h"

using namespace plugin;

void CopyStrMemEditor(unsigned int addr, wchar_t const *what) {
    unsigned int len = wcslen(what) + 1;
    DWORD oldP;
    VirtualProtect((void *)addr, len, PAGE_READWRITE, &oldP);
    wcscpy((wchar_t *)addr, what);
    VirtualProtect((void *)addr, len, oldP, &oldP);
}

void PatchTextFiles(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {
        // parameter files
        CopyStrMemEditor(0x668678, L"%s\\fmdata\\ParameterFiles\\Staff Generation.txt");
        CopyStrMemEditor(0x66AF00, L"fmdata\\ParameterFiles\\Player Level.txt");
        CopyStrMemEditor(0x66AF70, L"fmdata\\ParameterFiles\\Player Styles.txt");
        CopyStrMemEditor(0x67E948, L"ParameterFiles\\Club Jobs.txt");
        CopyStrMemEditor(0x67E988, L"ParameterFiles\\Club Positions.txt");
        CopyStrMemEditor(0x683508, L"ParameterFiles\\Training Camp New.txt");
        // config files
        CopyStrMemEditor(0x64F3D8, L"ConfigFiles\\Face Mapping.txt");
        CopyStrMemEditor(0x681D20, L"ConfigFiles\\Face Mapping.txt");
    }
}
