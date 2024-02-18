#include "UcpSettings.h"
#include "shared.h"
#include "WinHeader.h"
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
