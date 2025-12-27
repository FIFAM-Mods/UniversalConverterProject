#pragma once
#include "plugin.h"
#include "FifamTypes.h"

extern Bool IsDarkTheme;

String &GetCustomInterfaceFolderW();
StringA &GetCustomInterfaceFolderA();
String GetUIScreenFilenameW(String const &filename);
StringA GetUIScreenFilenameA(StringA const &filename);
void PatchInterfaceTheme(FM::Version v);
