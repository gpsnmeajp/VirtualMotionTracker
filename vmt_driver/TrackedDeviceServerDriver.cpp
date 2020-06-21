#include "TrackedDeviceServerDriver.h"
namespace VMTDriver {
    EVRInitError TrackedDeviceServerDriver::Activate(uint32_t unObjectId)
    {
        return EVRInitError();
    }
    void TrackedDeviceServerDriver::Deactivate()
    {
    }
    void TrackedDeviceServerDriver::EnterStandby()
    {
    }
    void* TrackedDeviceServerDriver::GetComponent(const char* pchComponentNameAndVersion)
    {
        return nullptr;
    }
    void TrackedDeviceServerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
    {
    }
    DriverPose_t TrackedDeviceServerDriver::GetPose()
    {
        return DriverPose_t();
    }

}