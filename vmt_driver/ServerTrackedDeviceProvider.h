#pragma once
#include "dllmain.h"

using namespace vr;
using std::vector;
namespace VMTDriver {
    //デバイスサーバー
    class ServerTrackedDeviceProvider : public IServerTrackedDeviceProvider
    {
    private:
        vector<TrackedDeviceServerDriver> m_devices;

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