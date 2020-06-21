#pragma once
#include "dllmain.h"
using namespace vr;
namespace VMTDriver {
    class Log {
    private:
        static IVRDriverLog* handle;
        Log();
        ~Log();
    public:

        static void Open(IVRDriverLog*);
        static void Close();
        static void Output(char*);

        static int printf(const char* fmt, ...);
    };
}