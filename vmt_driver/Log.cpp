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

    //ログをオープンする
    void Log::Open(IVRDriverLog* hnd)
    {
        handle = hnd;
        LogInfo("[Log Open] build: " __DATE__ " " __TIME__);
    }

    //ログをクローズする
    void Log::Close()
    {
        handle = nullptr;
    }

    //ログに出力する
    void Log::Output(std::string msg)
    {
        if (handle != nullptr) {
            handle->Log(("[VMT] " + msg).c_str());
        }
    }

    //ログに書式付きで出力する
    int Log::printf(const char* fmt, ...)
    {
        if (handle == nullptr) {
            return -1;
        }

        const int size = 8192;
        char output[size] = { 0 };

        //書式月出力
        va_list arg;
        va_start(arg, fmt);
        int ret = vsnprintf(output, size, fmt, arg);
        va_end(arg);

        //文字数オーバー時に確実に出力するためのフェールセーフ
        output[size - 1] = '\0';
        Log::Output(output);

        return ret;
    }

    const char* Log::ETrackedPropertyErrorToString(ETrackedPropertyError status) {
        switch (status)
        {
        case TrackedProp_Success: return "TrackedProp_Success";
        case TrackedProp_WrongDataType: return "TrackedProp_WrongDataType";
        case TrackedProp_WrongDeviceClass: return "TrackedProp_WrongDeviceClass";
        case TrackedProp_BufferTooSmall: return "TrackedProp_BufferTooSmall";
        case TrackedProp_UnknownProperty: return "TrackedProp_UnknownProperty";
        case TrackedProp_InvalidDevice: return "TrackedProp_InvalidDevice";
        case TrackedProp_CouldNotContactServer: return "TrackedProp_CouldNotContactServer";
        case TrackedProp_ValueNotProvidedByDevice: return "TrackedProp_ValueNotProvidedByDevice";
        case TrackedProp_StringExceedsMaximumLength: return "TrackedProp_StringExceedsMaximumLength";
        case TrackedProp_NotYetAvailable: return "TrackedProp_NotYetAvailable";
        case TrackedProp_PermissionDenied: return "TrackedProp_PermissionDenied";
        case TrackedProp_InvalidOperation: return "TrackedProp_InvalidOperation";
        case TrackedProp_CannotWriteToWildcards: return "TrackedProp_CannotWriteToWildcards";
        case TrackedProp_IPCReadFailure: return "TrackedProp_IPCReadFailure";
        case TrackedProp_OutOfMemory: return "TrackedProp_OutOfMemory";
        case TrackedProp_InvalidContainer: return "TrackedProp_InvalidContainer";
        default: return "Unknown";
        }
    }
    const char* Log::EVRInputErrorToString(EVRInputError status) {
        switch (status)
        {
        case VRInputError_None: return "VRInputError_None ";
        case VRInputError_NameNotFound: return "VRInputError_NameNotFound ";
        case VRInputError_WrongType: return "VRInputError_WrongType ";
        case VRInputError_InvalidHandle: return "VRInputError_InvalidHandle ";
        case VRInputError_InvalidParam: return "VRInputError_InvalidParam ";
        case VRInputError_NoSteam: return "VRInputError_NoSteam ";
        case VRInputError_MaxCapacityReached: return "VRInputError_MaxCapacityReached ";
        case VRInputError_IPCError: return "VRInputError_IPCError ";
        case VRInputError_NoActiveActionSet: return "VRInputError_NoActiveActionSet ";
        case VRInputError_InvalidDevice: return "VRInputError_InvalidDevice ";
        case VRInputError_InvalidSkeleton: return "VRInputError_InvalidSkeleton ";
        case VRInputError_InvalidBoneCount: return "VRInputError_InvalidBoneCount ";
        case VRInputError_InvalidCompressedData: return "VRInputError_InvalidCompressedData ";
        case VRInputError_NoData: return "VRInputError_NoData ";
        case VRInputError_BufferTooSmall: return "VRInputError_BufferTooSmall ";
        case VRInputError_MismatchedActionManifest: return "VRInputError_MismatchedActionManifest ";
        case VRInputError_MissingSkeletonData: return "VRInputError_MissingSkeletonData ";
        case VRInputError_InvalidBoneIndex: return "VRInputError_InvalidBoneIndex ";
        case VRInputError_InvalidPriority: return "VRInputError_InvalidPriority ";
        case VRInputError_PermissionDenied: return "VRInputError_PermissionDenied ";
        case VRInputError_InvalidRenderModel: return "VRInputError_InvalidRenderModel ";
        default: return "Unknown";
        }
    }
}