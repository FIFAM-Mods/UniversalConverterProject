#include "Log.h"

Log::Log() {
    mWriter = new FifamWriter(L"ucp.log", 14, FifamVersion());
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
