#pragma once
#ifdef _USING_V110_SDK71_
#ifndef _WIN_OBJBASE_UNKNOWN_DEFINED
typedef struct IUnknown IUnknown;
#define _WIN_OBJBASE_UNKNOWN_DEFINED
#endif
#endif
#include <Windows.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include "Error.h"
#include "Other.h"
#include "Patch.h"
#include "LicenseCheck.h"

enum ePluginApp {
    APP_UNKNOWN = 0,
    APP_MANAGER = 1,
    APP_EDITOR = 2,
    APP_RESOLUTOOL = 3,
    APP_COUNT
};

#define VERSION_UNKNOWN 0

#define FM_VERSION(game, v1, v2, v3, v4, prot, app) \
    ( ((unsigned int)(game) << 24) | \
      ((unsigned int)(v1)   << 20) | \
      ((unsigned int)(v2)   << 16) | \
      ((unsigned int)(v3)   << 12)  | \
      ((unsigned int)(v4)   << 8)  | \
      ((unsigned int)(prot) << 4)  | \
      ((unsigned int)(app)) )

#define ID_FM_14_1000     FM_VERSION(14, 1,0,0,0, 1, APP_MANAGER)
#define VERSION_FM_14 ID_FM_14_1000

#define ID_FM_13_1030_RLD FM_VERSION(13, 1,0,3,0, 2, APP_MANAGER)
#define VERSION_FM_13 ID_FM_13_1030_RLD

#define ID_FM_12_1003_RZ  FM_VERSION(12, 1,0,0,3, 2, APP_MANAGER)
#define VERSION_FM_12 ID_FM_12_1003_RZ

#define ID_FM_11_1003     FM_VERSION(11, 1,0,0,3, 0, APP_MANAGER)
#define ID_FM_11_1002     FM_VERSION(11, 1,0,0,2, 0, APP_MANAGER)
#define ID_FM_11_1000_RLD FM_VERSION(11, 1,0,0,0, 2, APP_MANAGER)
#define VERSION_FM_11 ID_FM_11_1000_RLD

#define ID_FM_10_1000_C   FM_VERSION(10, 1,0,0,0, 2, APP_MANAGER)
#define VERSION_FM_10 ID_FM_10_1000_C

#define ID_FM_09_1000_C   FM_VERSION(9, 1,0,0,0, 2, APP_MANAGER)
#define VERSION_FM_09 ID_FM_09_1000_C

#define ID_FM_08_1020_C   FM_VERSION(8, 1,0,2,0, 2, APP_MANAGER)
#define ID_FM_08_1020     FM_VERSION(8, 1,0,2,0, 1, APP_MANAGER)
#define ID_FM_08_1000_FL  FM_VERSION(8, 1,0,0,0, 2, APP_MANAGER)
#define ID_FM_08_1000     FM_VERSION(8, 1,0,0,0, 1, APP_MANAGER)
#define VERSION_FM_08 ID_FM_08_1020_C

#define ID_FM_07_1000_C   FM_VERSION(7, 1,0,0,0, 2, APP_MANAGER)
#define VERSION_FM_07 ID_FM_07_1000_C

#define ID_FM_06_1000_C   FM_VERSION(6, 1,0,0,0, 2, APP_MANAGER)
#define VERSION_FM_06 ID_FM_06_1000_C

#define ID_FM_05_1000_C   FM_VERSION(5, 1,0,0,0, 2, APP_MANAGER)
#define ID_FM_05_1010_C   FM_VERSION(5, 1,0,1,0, 2, APP_MANAGER)
#define VERSION_FM_05 ID_FM_05_1010_C

#define ID_FM_04_1000_C   FM_VERSION(4, 1,0,0,0, 2, APP_MANAGER)
#define VERSION_FM_04 ID_FM_04_1000_C

#define ID_FM_03_1000_C   FM_VERSION(3, 1,0,0,0, 2, APP_MANAGER)
#define VERSION_FM_03 ID_FM_03_1000_C

#define ID_ED_14_1000     FM_VERSION(14, 1,0,0,0, 1, APP_EDITOR)
#define VERSION_ED_14 ID_ED_14_1000

#define ID_ED_13_1000     FM_VERSION(13, 1,0,0,0, 1, APP_EDITOR)
#define VERSION_ED_13 ID_ED_13_1000

#define ID_ED_12_1000     FM_VERSION(12, 1,0,0,0, 1, APP_EDITOR)
#define VERSION_ED_12 ID_ED_12_1000

#define ID_ED_11_1003     FM_VERSION(11, 1,0,0,3, 1, APP_EDITOR)
#define ID_ED_11_1000     FM_VERSION(11, 1,0,0,0, 1, APP_EDITOR)
#define VERSION_ED_11 ID_ED_11_1000

#define ID_ED_10_9010     FM_VERSION(10, 9,0,1,0, 1, APP_EDITOR)
#define VERSION_ED_10 ID_ED_10_9010

#define ID_ED_09_9010     FM_VERSION(9, 9,0,1,0, 1, APP_EDITOR)
#define VERSION_ED_09 ID_ED_09_9010

#define ID_ED_08_8071     FM_VERSION(8, 8,0,7,1, 1, APP_EDITOR)
#define VERSION_ED_08 ID_ED_08_8071

#define ID_ED_07_7020     FM_VERSION(7, 7,0,2,0, 1, APP_EDITOR)
#define VERSION_ED_07 ID_ED_07_7020

#define ID_ED_06_5001     FM_VERSION(6, 5,0,0,1, 1, APP_EDITOR)
#define VERSION_ED_06 ID_ED_06_5001

#define ID_ED_05_4000     FM_VERSION(5, 4,0,0,0, 1, APP_EDITOR)
#define VERSION_ED_05 ID_ED_05_4000

#define ID_ED_04_1016     FM_VERSION(4, 1,0,1,6, 1, APP_EDITOR)
#define ID_ED_04_1020     FM_VERSION(4, 1,0,2,0, 1, APP_EDITOR)
#define VERSION_ED_04 ID_ED_04_1020

#define ID_ED_03_1002     FM_VERSION(3, 1,0,0,2, 1, APP_EDITOR)
#define VERSION_ED_03 ID_ED_03_1002

#define ID_RESOLUTOOL_13_1000 FM_VERSION(13, 1,0,0,0, 1, APP_RESOLUTOOL)
#define VERSION_RESOLUTOOL_13 ID_RESOLUTOOL_13_1000

namespace FM {

class Version {
    union {
        struct {
            unsigned int mApp : 4;
            unsigned int mProtection : 4;
            unsigned int mVersion4 : 4;
            unsigned int mVersion3 : 4;
            unsigned int mVersion2 : 4;
            unsigned int mVersion1 : 4;
            unsigned int mGame : 8;
        };
        unsigned int mId;
    };
public:
    inline Version() { mId = 0; }
    inline Version(unsigned int _id) { mId = _id; }
    inline unsigned char app() { return mApp; }
    inline unsigned int id() { return mId; }
    inline unsigned char game() { return mGame; }
    inline unsigned char version1() { return mVersion1; }
    inline unsigned char version2() { return mVersion2; }
    inline unsigned char version3() { return mVersion3; }
    inline unsigned char version4() { return mVersion4; }
    inline unsigned char protection() { return mProtection; }

    inline std::wstring name() {
        if (mApp == APP_UNKNOWN || mApp >= APP_COUNT)
            return L"Unknown";
        static wchar_t const *AppName[] = {
            L"Unknown",
            L"Manager",
            L"Editor",
            L"Resolutool"
        };
        static wchar_t const *Protection[] = { L"", L" (license)" };
        return plugin::Format(L"%s %02d v.%d.%d.%d.%d%s", AppName[mApp], mGame,
            mVersion1, mVersion2, mVersion3, mVersion4, Protection[mProtection == 1]);
    }
};

inline unsigned long DetectAppEntryPoint() {
    uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)(base);
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    auto entryPointAddr = base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base);
    return entryPointAddr;
}

inline unsigned long GetEntryPoint() {
    static unsigned long ep = DetectAppEntryPoint();
    return ep;
}

inline Version DetectAppVersion() {
    switch (GetEntryPoint()) {

    // Manager

    case 0x3C8D830: // ?
        return ID_FM_14_1000;
    case 0x1203AB4: // decrypted
        return ID_FM_12_1003_RZ;
    case 0x1743049: // encrypted
        return ID_FM_11_1003;
    case 0x179B049: // encrypted
        return ID_FM_11_1002;
    case 0x1736049: // decrypted
        return ID_FM_11_1000_RLD;
    case 0x153C4C0:
        return ID_FM_10_1000_C;
    case 0x3140A52:
        return ID_FM_09_1000_C;
    case 0x1C03990:
        return ID_FM_08_1020_C;
    case 0x17F6CF0:
        return ID_FM_08_1020;
    case 0xE5F311:
        return ID_FM_08_1000_FL;
    case 0x1789B30:
        return ID_FM_08_1000;
    case 0xBC0B69:
        return ID_FM_07_1000_C;
    case 0xA26F90:
        return ID_FM_06_1000_C;
    case 0xB71005:
        return ID_FM_05_1010_C;
    case 0xB6E005:
        return ID_FM_05_1000_C;
    case 0xCF2865:
        return ID_FM_04_1000_C;
    case 0x938B91:
        return ID_FM_03_1000_C;

    // Editor

    case 0x5FA5A2:
        return ID_ED_14_1000;
    case 0x5F8712:
        return ID_ED_13_1000;
    case 0x5728A1:
        return ID_ED_12_1000;
    case 0x560DB0:
        return ID_ED_11_1003;
    case 0x55A8AC:
        return ID_ED_11_1000;
    case 0x503D2C:
        return ID_ED_10_9010;
    case 0x4E9A06:
        return ID_ED_09_9010;
    case 0x4D5526:
        return ID_ED_08_8071;
    case 0x4A7B57:
        return ID_ED_07_7020;
    case 0x4D1703:
        return ID_ED_06_5001;
    case 0x4D76C9:
        return ID_ED_05_4000;
    case 0x493128:
        return ID_ED_04_1016;
    case 0x493148:
        return ID_ED_04_1020; // 1.0.1.6 version 2
    case 0x46CA70:
        return ID_ED_03_1002;

    // Resolutool

    case 0x502B92:
        return ID_RESOLUTOOL_13_1000;

    }

    return VERSION_UNKNOWN;
}

inline Version GetAppVersion() {
    static Version v = DetectAppVersion();
    return v;
}

inline Version GetManagerVersion() {
    Version v = GetAppVersion();
    if (v.app() == APP_MANAGER)
        return v;
    return VERSION_UNKNOWN;
}

inline Version GetEditorVersion() {
    Version v = GetAppVersion();
    if (v.app() == APP_EDITOR)
        return v;
    return VERSION_UNKNOWN;
}

inline std::wstring GetProcessName(bool withExtension = true) {
    std::wstring result;
    wchar_t filepath[MAX_PATH + 1];
    wchar_t outname[MAX_PATH + 1];
    wchar_t ext[MAX_PATH + 1];
    DWORD length = GetModuleFileNameW(NULL, filepath, MAX_PATH);
    if (length > 0) {
        _wsplitpath(filepath, NULL, NULL, outname, ext);
        result = outname;
        if (withExtension)
            result += ext;
    }
    return result;
}

inline std::wstring GetProcessDir() {
    wchar_t filepath[MAX_PATH + 1];
    wchar_t outname[MAX_PATH + 1];
    DWORD length = GetModuleFileNameW(NULL, filepath, MAX_PATH);
    if (length > 0) {
        _wsplitpath(filepath, NULL, outname, NULL, NULL);
        return outname;
    }
    return std::wstring();
}

inline std::wstring GetModulePath(HMODULE hmodule) {
    static constexpr auto INITIAL_BUFFER_SIZE = MAX_PATH;
    static constexpr auto MAX_ITERATIONS = 7;
    std::wstring ret;
    auto bufferSize = INITIAL_BUFFER_SIZE;
    for (size_t iterations = 0; iterations < MAX_ITERATIONS; ++iterations) {
        ret.resize(bufferSize);
        auto charsReturned = GetModuleFileNameW(hmodule, &ret[0], bufferSize);
        if (charsReturned < ret.length()) {
            ret.resize(charsReturned);
            return ret;
        }
        else
            bufferSize *= 2;
    }
    return std::wstring();
}

inline std::wstring GetGameDir() {
    std::wstring modulePath = GetModulePath(NULL);
    if (!modulePath.empty()) {
        auto lastSlash = modulePath.find_last_of(L"/\\");
        if (lastSlash != std::wstring::npos) {
            modulePath.resize(lastSlash + 1);
            return modulePath;
        }
    }
    return std::wstring();
}

inline std::wstring GameDirPath(std::wstring const &subPath) {
    auto gameDir = GetGameDir();
    if (!gameDir.empty())
        return gameDir + subPath;
    return subPath;
}

inline std::wstring GameDirPath(std::wstring const &subPath, std::wstring extension) {
    return GameDirPath(subPath) + extension;
}

inline std::wstring GetPluginName() {
    HMODULE hModule = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(&GetGameDir), &hModule)) {
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

inline bool UnknownVersionMessage() {
    return plugin::Error(L"Your game version is not supported by this plugin.\nPlugin: %s\nGame process: %s\nEntryPoint: %llX",
        GetPluginName().c_str(), GetProcessName().c_str(), GetEntryPoint());
}

}
