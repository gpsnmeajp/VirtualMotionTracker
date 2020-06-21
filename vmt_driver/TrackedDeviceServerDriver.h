#pragma once
#include "dllmain.h"

using namespace vr;
using std::string;
namespace VMTDriver {
    //個々のデバイス
    class TrackedDeviceServerDriver : public ITrackedDeviceServerDriver
    {
    public:
        const HmdQuaternion_t HmdQuaternion_Identity{ 1,0,0,0 };
        int index;
        TrackedDeviceIndex_t m_deviceIndex;
        PropertyContainerHandle_t m_propertyContainer;
        string m_serial;

        TrackedDeviceServerDriver();
        ~TrackedDeviceServerDriver();

        double x = 0;
        double y = 0;
        double z = 0;

        virtual EVRInitError Activate(uint32_t unObjectId) override;
        virtual void Deactivate() override;
        virtual void EnterStandby() override;
        virtual void* GetComponent(const char* pchComponentNameAndVersion) override;
        virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
        virtual DriverPose_t GetPose() override;
    };
}