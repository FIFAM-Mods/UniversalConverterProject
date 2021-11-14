#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"

String &GetCustomInterfaceFolderW();
StringA &GetCustomInterfaceFolderA();
String GetUIScreenFilenameW(String const &filename);
StringA GetUIScreenFilenameA(StringA const &filename);
void PatchInterfaceTheme(FM::Version v);
