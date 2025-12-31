#pragma once
#include "plugin.h"
#include "FifamTypes.h"

enum ePortratsStyle {
    PORTRAITSTYLE_FOOM = 1,
    PORTRAITSTYLE_FIFAM = 2
};

extern Bool IsDarkTheme;
extern UInt PortraitsStyle;

String &GetCustomInterfaceFolderW();
StringA &GetCustomInterfaceFolderA();
String GetUIScreenFilenameW(String const &filename);
StringA GetUIScreenFilenameA(StringA const &filename);
void PatchInterfaceTheme(FM::Version v);
