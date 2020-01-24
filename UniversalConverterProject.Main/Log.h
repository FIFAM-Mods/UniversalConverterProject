#pragma once
#include "FifamReadWrite.h"

class Log {
    FifamWriter *mWriter = nullptr;
    Log();
    ~Log();
    static Log &Instance();
public:
    static FifamWriter *GetWriter();
    static void Write(String const &str);
    static void WriteLine(String const &str);
};
