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
#pragma once
#include "dllmain.h"

#define LogMarker()  Log::printf("Marker:L%d:%s", __LINE__ , __FUNCTION__)
#define LogInfo(fmt, ...)  Log::printf("Info:L%d:%s> " fmt , __LINE__ , __FUNCTION__, __VA_ARGS__)
#define LogError(fmt, ...) Log::printf(" !!!! Error !!!! :L%d:%s> " fmt , __LINE__ , __FUNCTION__, __VA_ARGS__)

#define LogIfDiag(fmt, ...)  do{ if(Log::s_diag){Log::printf(" **** Diag **** :L%d:%s> " fmt , __LINE__ , __FUNCTION__, __VA_ARGS__);} }while(0)
#define LogIfFalse(status) do{ if(!status){Log::printf(" !!!! Error !!!! :L%d:%s", __LINE__ , __FUNCTION__);} }while(0)
#define LogIfETrackedPropertyError(status) do{ if(status != ETrackedPropertyError::TrackedProp_Success){Log::printf(" !!!! Error !!!! :L%d:%s:%s", __LINE__ , __FUNCTION__, Log::ETrackedPropertyErrorToString(status));} }while(0)
#define LogIfEVRInputError(status) do{ if(status != EVRInputError::VRInputError_None){Log::printf(" !!!! Error !!!! :L%d:%s:%s", __LINE__ , __FUNCTION__, Log::EVRInputErrorToString(status));} }while(0)

//OpenVRの開発者コンソールに表示するログを担うstaticクラス
namespace VMTDriver {
    class Log {
    private:
        static IVRDriverLog* handle;
        Log();
    public:
        static int s_logCounter;
        static bool s_diag;

        static void Open(IVRDriverLog*);
        static void Close();
        static void Output(std::string msg);

        static int printf(const char* fmt, ...);

        static const char* ETrackedPropertyErrorToString(ETrackedPropertyError status);
        static const char* EVRInputErrorToString(EVRInputError status);
    };
}