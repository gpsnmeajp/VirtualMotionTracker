#pragma once
#include "dllmain.h"

using namespace vr;
namespace VMTDriver {
    //個々のデバイス
    class TrackedDeviceServerDriver :ITrackedDeviceServerDriver
    {
    public:
        virtual EVRInitError Activate(uint32_t unObjectId) override;
        virtual void Deactivate() override;
        virtual void EnterStandby() override;
        virtual void* GetComponent(const char* pchComponentNameAndVersion) override;
        virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
        virtual DriverPose_t GetPose() override;
    };
}