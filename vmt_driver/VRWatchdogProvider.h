#pragma once
#include "dllmain.h"

using namespace vr;
namespace VMTDriver {
    //状況に関係なく物理デバイスなどを監視するためのWatchDog
    class VRWatchdogProvider : IVRWatchdogProvider
    {
    public:
        // IVRWatchdogProvider を介して継承されました
        virtual EVRInitError Init(IVRDriverContext* pDriverContext) override;
        virtual void Cleanup() override;
    };
}