#pragma once
#include <string>
#include <vector>
#include "Other.h"

#define AUTHOR_INFO(str) \
struct _Plugin_AuthorInfo { \
    virtual const char *GetName() { \
        return str; \
    } \
} info

namespace PluginLicenseCheckNamespace {

inline void DummyFunc() {}

inline std::wstring GetPluginName() {
    HMODULE hModule = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(&DummyFunc), &hModule)) {
        wchar_t filepath[MAX_PATH];
        wchar_t outname[MAX_PATH];
        wchar_t outext[MAX_PATH];
        DWORD len = GetModuleFileNameW(hModule, filepath, MAX_PATH);
        if (len > 0) {
            _wsplitpath(filepath, NULL, NULL, outname, outext);
            return std::wstring(outname) + outext;
        }
    }
    return std::wstring();
}

}

namespace plugin {

template<unsigned int ...chars>
inline std::wstring Magic() {
    std::wstring out;
    (out.push_back(static_cast<wchar_t>(chars)),...);
    return out;
}

inline bool CheckPluginName(std::wstring const &pluginName) {
    auto name = PluginLicenseCheckNamespace::GetPluginName();
    if (plugin::ToLower(name) != plugin::ToLower(pluginName)) {
        MessageBoxW(NULL, Magic<
            'P','l','u','g','i','n',' ','i','n','s','t','a','l','l','a','t','i','o','n',' ','i','s',' ',
            'w','r','o','n','g','.',0xA,'P','l','e','a','s','e',' ','r','e','i','n','s','t','a','l','l',
            ' ','t','h','e',' ','p','l','u','g','i','n','.'
            >().c_str(), name.c_str(), MB_ICONERROR);
        return false;
    }
    return true;
}

};
