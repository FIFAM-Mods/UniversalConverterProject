#include "UcpSettings.h"
#include "shared.h"
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
        Read(p);
    TeamControlDisabledAtGameStart = DisableTeamControl;
    WindowedModeStartValue = WindowedMode;
    WindowsMousePointerStartValue = WindowsMousePointer;
    WindowPositionStartValue = WindowPosition;
    ThemeAtGameStart = Theme;
}

Settings &Settings::GetInstance() {
    static Settings settings;
    return settings;
}

path Settings::GetPath() {
    path documentsPath = GetDocumentsPath();
    if (!documentsPath.empty())
        return GetDocumentsPath() / "Config" / "ucp.ini";
    return path();
}
