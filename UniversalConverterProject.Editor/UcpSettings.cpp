#include "UcpSettings.h"
#ifdef _USING_V110_SDK71_
#ifndef _WIN_OBJBASE_UNKNOWN_DEFINED
typedef struct IUnknown IUnknown;
#define _WIN_OBJBASE_UNKNOWN_DEFINED
#endif
#endif
#include <ShlObj.h>

void Settings::Save() {
    static path p = GetPath();
    if (!p.empty())
        Write(p, true);
}

void Settings::Load() {
    static path p = GetPath();
    if (!p.empty())
        Read(GetPath());
    TeamControlDisabledAtGameStart = DisableTeamControl;
    WindowedModeStartValue = WindowedMode;
    WindowsMousePointerStartValue = WindowsMousePointer;
    WindowPositionStartValue = WindowPosition;
    DisableResourcePathsCachingStartValue = DisableResourcePathsCaching;
}

Settings &Settings::GetInstance() {
    static Settings settings;
    return settings;
}

path Settings::GetPath() {
    static wchar_t documentsDir[MAX_PATH];
    static bool foundDocuments = SHGetSpecialFolderPathW(NULL, documentsDir, CSIDL_MYDOCUMENTS, FALSE);
    if (foundDocuments) {
        static path result = path(documentsDir) / "FM" / "Config" / "ucp.ini";
        return result;
    }
    return path();
}
