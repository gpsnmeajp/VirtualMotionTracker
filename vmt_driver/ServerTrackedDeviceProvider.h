#pragma once
#include "dllmain.h"

using namespace vr;
namespace VMTDriver {
    //デバイスサーバー
    class ServerTrackedDeviceProvider : IServerTrackedDeviceProvider
    {
    public:
        virtual EVRInitError Init(IVRDriverContext* pDriverContext) override;
        virtual void Cleanup() override;
        virtual const char* const* GetInterfaceVersions() override;
        virtual void RunFrame() override;
        virtual bool ShouldBlockStandbyMode() override;
        virtual void EnterStandby() override;
        virtual void LeaveStandby() override;
    };
}