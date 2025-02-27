#pragma once
#include "FifamTypes.h"
#include "WinHeader.h"

class FifamDebug {
    HMODULE hDebugModule = 0;
    unsigned int gFifamDebugPrint = 0;
public:
    FifamDebug();
    ~FifamDebug();
    void Print(Char const *text);
    static FifamDebug &GetInstance();
};

void DebugPrint(StringA const &message);
void DebugPrint(String const &message);
