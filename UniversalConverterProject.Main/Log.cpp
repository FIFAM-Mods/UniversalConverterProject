#include "Log.h"

Log::Log() {
    mWriter = new FifamWriter(L"ucp.log", 14, 0, 0);
}

Log::~Log() {
    delete mWriter;
}

Log &Log::Instance() {
    static Log log;
    return log;
}

void Log::Write(String const & str) {
    Instance().mWriter->Write(str);
}

void Log::WriteLine(String const & str) {
    Instance().mWriter->WriteLine(str);
}
