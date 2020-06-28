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

    void TrackedDeviceServerDriver::SetDeviceSerial(string serial)
    {
        m_serial = serial;
    }

    void TrackedDeviceServerDriver::SetObjectIndex(int idx)
    {
        m_index = idx;
    }

    void TrackedDeviceServerDriver::SetPose(DriverPose_t pose)
    {
        m_pose = pose;
    }

    void TrackedDeviceServerDriver::RegisterToVRSystem()
    {
        if (!m_alreadyRegistered)
        {
            VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, this);
            m_alreadyRegistered = true;
        }
    }
    void TrackedDeviceServerDriver::UpdatePoseToVRSystem()
    {
        if (!m_alreadyRegistered) { return; }
        VRServerDriverHost()->TrackedDevicePoseUpdated(m_deviceIndex, GetPose(), sizeof(DriverPose_t));
    }
    EVRInitError TrackedDeviceServerDriver::Activate(uint32_t unObjectId)
    {
        m_deviceIndex = unObjectId;
        m_propertyContainer = VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ModelNumber_String, m_serial.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_RenderModelName_String, "lh_basestation_vive"); //とりあえずベースステーションの姿
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_CurrentUniverseId_Uint64, 2);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_NeverTracked_Bool, false);


        VRProperties()->SetStringProperty(m_propertyContainer, Prop_InputProfilePath_String, "{vmt}/input/inputprofile.json");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceReady_String, "{vmt}/icons/Ready32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceOff_String, "{vmt}/icons/Off32x32.png");

        VRProperties()->SetUint64Property(m_propertyContainer, Prop_SupportedButtons_Uint64, 0xFF); //8bit buttons
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_Axis0Type_Int32, EVRControllerAxisType::k_eControllerAxis_Trigger);
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_Axis1Type_Int32, EVRControllerAxisType::k_eControllerAxis_TrackPad);
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_Axis2Type_Int32, EVRControllerAxisType::k_eControllerAxis_Joystick);

        //VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/a/click", &a);
        //VRDriverInput()->UpdateBooleanComponent(m_propertyContainer, true,0);

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
        return m_pose;
    }

}