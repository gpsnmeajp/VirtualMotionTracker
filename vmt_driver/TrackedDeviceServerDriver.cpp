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
    bool TrackedDeviceServerDriver::s_autoUpdate = false;

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

    void TrackedDeviceServerDriver::SetObjectIndex(uint32_t idx)
    {
        m_index = idx;
    }

    void TrackedDeviceServerDriver::SetPose(DriverPose_t pose)
    {
        m_pose = pose;
    }

    void TrackedDeviceServerDriver::SetRawPose(RawPose rawPose)
    {
        m_rawPose = rawPose;
        RawPoseToPose();
    }

    void TrackedDeviceServerDriver::RawPoseToPose()
    {
        DriverPose_t pose{ 0 };
        pose.poseTimeOffset = m_rawPose.timeoffset;
        pose.qRotation = VMTDriver::HmdQuaternion_Identity;
        pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
        pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;

        if (m_rawPose.enable != 0) {
            pose.deviceIsConnected = true;
            pose.poseIsValid = true;
            pose.result = TrackingResult_Running_OK;
        }
        else {
            pose.deviceIsConnected = false;
            pose.poseIsValid = false;
            pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
        }

        Eigen::Affine3d RoomToDriverAffin;
        RoomToDriverAffin = CommunicationManager::GetInstance()->GetRoomToDriverMatrix();

        //座標を設定
        pose.vecPosition[0] = m_rawPose.x;
        pose.vecPosition[1] = m_rawPose.y;
        pose.vecPosition[2] = m_rawPose.z;
        pose.qRotation.x = m_rawPose.qx;
        pose.qRotation.y = m_rawPose.qy;
        pose.qRotation.z = m_rawPose.qz;
        pose.qRotation.w = m_rawPose.qw;

        //Jointでない場合
        if (m_rawPose.mode == ReferMode_t::None || m_rawPose.root_sn == nullptr) {
            //ワールド・ドライバ変換行列を設定
            Eigen::Translation3d pos(RoomToDriverAffin.translation());
            Eigen::Quaterniond rot(RoomToDriverAffin.rotation());
            pose.vecWorldFromDriverTranslation[0] = pos.x();
            pose.vecWorldFromDriverTranslation[1] = pos.y();
            pose.vecWorldFromDriverTranslation[2] = pos.z();
            pose.qWorldFromDriverRotation.x = rot.x();
            pose.qWorldFromDriverRotation.y = rot.y();
            pose.qWorldFromDriverRotation.z = rot.z();
            pose.qWorldFromDriverRotation.w = rot.w();
        } else {
            //Joint時
            // 既存のトラッキングデバイスの座標系を参照する

            // 参照元のPoseを取得
            vr::TrackedDevicePose_t poses[k_unMaxTrackedDeviceCount];
            IVRServerDriverHost* host = VRServerDriverHost();
            host->GetRawTrackedDevicePoses(0.0f, poses, k_unMaxTrackedDeviceCount);
            IVRProperties* props = VRPropertiesRaw();
            CVRPropertyHelpers* helper = VRProperties();

            bool deviceFound = false;

            for (uint32_t i = 0; i < k_unMaxTrackedDeviceCount; i++) {
                TrackedDevicePose_t* const p = poses + i;
                if (!p->bDeviceIsConnected) break;

                PropertyContainerHandle_t h = props->TrackedDeviceToPropertyContainer(i);
                string SerialNumber = helper->GetStringProperty(h, ETrackedDeviceProperty::Prop_SerialNumber_String);

                if (SerialNumber.compare(m_rawPose.root_sn) != 0) continue;

                // 参照元のトラッキングステータスを継承させる
                if (m_rawPose.enable != 0) {
                    pose.result = p->eTrackingResult;
                }

                if (p->eTrackingResult == ETrackingResult::TrackingResult_Running_OK) {
                    float* m = (float*)p->mDeviceToAbsoluteTracking.m;

                    Eigen::Affine3d rootDeviceToAbsoluteTracking;
                    rootDeviceToAbsoluteTracking.matrix() <<
                        m[0 * 4 + 0], m[0 * 4 + 1], m[0 * 4 + 2], m[0 * 4 + 3],
                        m[1 * 4 + 0], m[1 * 4 + 1], m[1 * 4 + 2], m[1 * 4 + 3],
                        m[2 * 4 + 0], m[2 * 4 + 1], m[2 * 4 + 2], m[2 * 4 + 3],
                        0.0, 0.0, 0.0, 1.0;

                    Eigen::Translation3d pos(rootDeviceToAbsoluteTracking.translation());
                    pose.vecWorldFromDriverTranslation[0] = pos.x();
                    pose.vecWorldFromDriverTranslation[1] = pos.y();
                    pose.vecWorldFromDriverTranslation[2] = pos.z();

                    Eigen::Quaterniond rot;
                    switch (m_rawPose.mode) {
                    case ReferMode_t::Follow:
                        rot = Eigen::Quaterniond(RoomToDriverAffin.rotation());
                        break;
                    case ReferMode_t::Joint:
                    default:
                        rot = Eigen::Quaterniond(rootDeviceToAbsoluteTracking.rotation());
                        break;
                    }

                    pose.qWorldFromDriverRotation.x = rot.x();
                    pose.qWorldFromDriverRotation.y = rot.y();
                    pose.qWorldFromDriverRotation.z = rot.z();
                    pose.qWorldFromDriverRotation.w = rot.w();

                    deviceFound = true;
                }

                break;
            }

            if (!deviceFound) {
                pose.vecWorldFromDriverTranslation[0] = 0.0;
                pose.vecWorldFromDriverTranslation[1] = 0.0;
                pose.vecWorldFromDriverTranslation[2] = 0.0;
                pose.qWorldFromDriverRotation.x = 0.0;
                pose.qWorldFromDriverRotation.y = 0.0;
                pose.qWorldFromDriverRotation.z = 0.0;
                pose.qWorldFromDriverRotation.w = 1.0;
            }
        }
        SetPose(pose);
    }

    void TrackedDeviceServerDriver::RegisterToVRSystem(int type)
    {
        if (!m_alreadyRegistered)
        {
            switch (type)
            {
            case 3://Controller Right
                VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_RightHand);
                VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_Controller, this);
                m_alreadyRegistered = true;
                break;
            case 2://Controller Left
                VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_LeftHand);
                VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_Controller, this);
                m_alreadyRegistered = true;
                break;
            case 1://Tracker
                VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, this);
                m_alreadyRegistered = true;
            default:
                break;
            }
        }
    }
    void TrackedDeviceServerDriver::UpdatePoseToVRSystem()
    {
        if (!m_alreadyRegistered) { return; }
        VRServerDriverHost()->TrackedDevicePoseUpdated(m_deviceIndex, GetPose(), sizeof(DriverPose_t));
    }
    void TrackedDeviceServerDriver::UpdateButtonInput(uint32_t index, bool value, double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (0 <= index && index <= 7)
        {
            VRDriverInput()->UpdateBooleanComponent(ButtonComponent[index], value, timeoffset);
        }
    }
    void TrackedDeviceServerDriver::UpdateTriggerInput(uint32_t index, float value, double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (value > 1.0) {
            value = 1.0;
        }
        if (value < 0) {
            value = 0;
        }
        if (isnan(value)) {
            value = 0;
        }

        if (0 <= index && index <= 1)
        {
            VRDriverInput()->UpdateScalarComponent(TriggerComponent[index], value, timeoffset);
        }
    }
    void TrackedDeviceServerDriver::UpdateJoystickInput(uint32_t index, float x, float y, double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (index == 0)
        {
            VRDriverInput()->UpdateScalarComponent(JoystickComponent[index + 0], x, timeoffset);
            VRDriverInput()->UpdateScalarComponent(JoystickComponent[index + 1], y, timeoffset);
        }
    }
    void TrackedDeviceServerDriver::Reset()
    {
        if (!m_alreadyRegistered) { return; }
        DriverPose_t pose{ 0 };
        pose.qRotation = VMTDriver::HmdQuaternion_Identity;
        pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
        pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;
        pose.deviceIsConnected = false;
        pose.poseIsValid = false;
        pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
        SetPose(pose);

        //全状態を初期化する
        for (int i = 0; i < 16; i++) {
            UpdateButtonInput(i, false, 0);
            UpdateTriggerInput(i, 0, 0);
            UpdateJoystickInput(i, 0, 0, 0);
        }
    }
    void TrackedDeviceServerDriver::ProcessEvent(VREvent_t& VREvent)
    {
        switch (VREvent.eventType)
        {
        case EVREventType::VREvent_Input_HapticVibration:
            //バイブレーション
            if (VREvent.data.hapticVibration.componentHandle == HapticComponent) {
                OSCReceiver::SendHaptic(m_index, VREvent.data.hapticVibration.fFrequency, VREvent.data.hapticVibration.fAmplitude, VREvent.data.hapticVibration.fDurationSeconds);
            }
            break;
        default:
            break;
        }
    }
    void TrackedDeviceServerDriver::SetAutoUpdate(bool enable)
    {
        s_autoUpdate = enable;
    }
    EVRInitError TrackedDeviceServerDriver::Activate(uint32_t unObjectId)
    {
        m_deviceIndex = unObjectId;
        m_propertyContainer = VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ModelNumber_String, m_serial.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_RenderModelName_String, "{vmt}vmt_rendermodel");
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_CurrentUniverseId_Uint64, 2);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_NeverTracked_Bool, false);

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_InputProfilePath_String, "{vmt}/input/vmt_profile.json");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ControllerType_String, "vmt");

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceReady_String, "{vmt}/icons/Ready32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{vmt}/icons/Ready32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{vmt}/icons/Ready32x32.png");

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceSearching_String, "{vmt}/icons/Ready32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{vmt}/icons/Ready32x32.png");

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceStandby_String, "{vmt}/icons/Ready32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{vmt}/icons/Ready32x32.png");

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{vmt}/icons/Ready32x32.png");

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceOff_String, "{vmt}/icons/Ready32x32.png");

        VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button0/click", &ButtonComponent[0]);
        VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button1/click", &ButtonComponent[1]);
        VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button2/click", &ButtonComponent[2]);
        VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button3/click", &ButtonComponent[3]);
        VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button4/click", &ButtonComponent[4]);
        VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button5/click", &ButtonComponent[5]);
        VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button6/click", &ButtonComponent[6]);
        VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button7/click", &ButtonComponent[7]);

        VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/Trigger0/value", &TriggerComponent[0], EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedOneSided);
        VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/Trigger1/value", &TriggerComponent[1], EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedOneSided);

        VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/Joystick0/x", &JoystickComponent[0], EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided);
        VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/Joystick0/y", &JoystickComponent[1], EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided);

        VRDriverInput()->CreateHapticComponent(m_propertyContainer, "/output/haptic", &HapticComponent);

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
        if (s_autoUpdate) {
            if (m_alreadyRegistered) {
                RawPoseToPose();
            }
        }
        return m_pose;
    }

}