/*
MIT License

Copyright (c) 2020 gpsnmeajp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "Log.h"
namespace VMTDriver {
    IVRDriverLog* Log::handle = nullptr;

    void Log::Open(IVRDriverLog* hnd)
    {
        handle = hnd;
    }
    void Log::Close()
    {
        handle = nullptr;
    }
    void Log::Output(const char* msg)
    {
        if (handle != nullptr) {
            handle->Log(msg);
        }
    }

    int Log::printf(const char* fmt, ...)
    {
        if (handle == nullptr) {
            return -1;
        }

        const int size = 8192;
        char output[size] = { 0 };

        //ƒƒOo—Í(ó‹µ‚É‡‚í‚¹‚ÄØ‚è‘Ö‚¦‚ç‚ê‚é‚æ‚¤‚É)
        va_list arg;
        va_start(arg, fmt);
        int ret = vsnprintf(output, size, fmt, arg);
        va_end(arg);

        output[size - 1] = '\0';
        handle->Log(output);

        return ret;
    }
}