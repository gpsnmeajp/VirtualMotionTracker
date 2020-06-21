#include "TrackedDeviceServerDriver.h"
namespace VMTDriver {
    TrackedDeviceServerDriver::TrackedDeviceServerDriver()
    {
        m_deviceIndex = k_unTrackedDeviceIndexInvalid;
        m_propertyContainer = k_ulInvalidPropertyContainer;
    }
    TrackedDeviceServerDriver::~TrackedDeviceServerDriver()
    {
    }
    EVRInitError TrackedDeviceServerDriver::Activate(uint32_t unObjectId)
    {
        m_deviceIndex = unObjectId;
        m_propertyContainer = VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ModelNumber_String, m_serial.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_RenderModelName_String, m_serial.c_str());

        VRProperties()->SetUint64Property(m_propertyContainer, Prop_CurrentUniverseId_Uint64, 2);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_NeverTracked_Bool, false);

        return EVRInitError::VRInitError_None;
    }
    void TrackedDeviceServerDriver::Deactivate()
    {
        m_deviceIndex = k_unTrackedDeviceIndexInvalid;
        m_propertyContainer = k_ulInvalidPropertyContainer;
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
        if (unResponseBufferSize > 0) {
            pchResponseBuffer[0] = '\0';
        }
    }
    DriverPose_t TrackedDeviceServerDriver::GetPose()
    {
        DriverPose_t pose{ 0 };
        pose.deviceIsConnected = true;
        pose.poseIsValid = true;
        pose.result = TrackingResult_Running_OK;

        pose.qWorldFromDriverRotation = HmdQuaternion_Identity;
        pose.qDriverFromHeadRotation = HmdQuaternion_Identity;

        x += 0.0001f;
        y += index / 100000.0;

        pose.vecPosition[0] = x;
        pose.vecPosition[1] = y;
        pose.vecPosition[2] = z;
        return pose;
    }

}