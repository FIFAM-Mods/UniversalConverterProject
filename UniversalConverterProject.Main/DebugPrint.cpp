#include "DebugPrint.h"
#include "plugin-std.h"

FifamDebug::FifamDebug() {
    hDebugModule = LoadLibraryA("plugins\\FifamDebug.asi");
    if (hDebugModule)
        gFifamDebugPrint = (unsigned int)GetProcAddress(hDebugModule, "FifamDebugPrint");
}

FifamDebug::~FifamDebug() {
    if (hDebugModule)
        FreeLibrary(hDebugModule);
}

void FifamDebug::Print(Char const *text) {
    if (gFifamDebugPrint)
        plugin::CallDynGlobal(gFifamDebugPrint, text);
}

FifamDebug &FifamDebug::GetInstance() {
    static FifamDebug fifamDebug;
    return fifamDebug;
}

void DebugPrint(StringA const &message) {
    FifamDebug::GetInstance().Print(message.c_str());
}

void DebugPrint(String const &message) {
    StringA str;
    if (!message.empty()) {
        Int size_needed = WideCharToMultiByte(CP_UTF8, 0, &message[0], (int)message.size(), NULL, 0, NULL, NULL);
        str.resize(size_needed);
        WideCharToMultiByte(CP_UTF8, 0, &message[0], (int)message.size(), &str[0], size_needed, NULL, NULL);
    }
    FifamDebug::GetInstance().Print(str.c_str());
}
