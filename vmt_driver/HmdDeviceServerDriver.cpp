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
#include "HmdDeviceServerDriver.h"

namespace VMTDriver {
    //** 内部向け関数群 **

    //自動更新を有効にするか
    bool HmdDeviceServerDriver::s_autoUpdate = false;

    //仮想デバイスのコンストラクタ。(Listから暗黙的にコールされる)
    HmdDeviceServerDriver::HmdDeviceServerDriver()
    {
        m_deviceIndex = k_unTrackedDeviceIndexInvalid;
        m_propertyContainer = k_ulInvalidPropertyContainer;

        m_rawPose.enable = 1;
        m_rawPose.x = 0;
        m_rawPose.y = 0;
        m_rawPose.z = 0;
        m_rawPose.qx = 0;
        m_rawPose.qy = 0;
        m_rawPose.qz = 0;
        m_rawPose.qw = 1;
        m_rawPose.time = std::chrono::system_clock::now();
    }

    //仮想デバイスのデストラクタ。(Listから暗黙的にコールされる)
    HmdDeviceServerDriver::~HmdDeviceServerDriver()
    {
    }

    //仮想デバイスにシリアル番号を設定
    void HmdDeviceServerDriver::SetDeviceSerial(string serial)
    {
        m_serial = serial;
    }

    //仮想デバイスに内部Indexを設定
    void HmdDeviceServerDriver::SetObjectIndex(uint32_t idx)
    {
        m_index = idx;
    }

    //仮想デバイスにOpenVR姿勢を設定
    void HmdDeviceServerDriver::SetPose(DriverPose_t pose)
    {
        m_pose = pose;
    }

    //仮想デバイスに内部姿勢を設定
    void HmdDeviceServerDriver::SetRawPose(RawPose rawPose)
    {
        if (s_autoUpdate) {
            //自動更新が有効なら内部姿勢を保存するのみ。(OpenVR姿勢は自動更新されるため)
            m_rawPose = rawPose;
        }
        else {
            //自動更新が無効ならば内部姿勢を保存し、OpenVR姿勢を更新する
            m_lastRawPose = m_rawPose; //差分を取るために前回値を取っておく
            m_rawPose = rawPose;
            SetPose(RawPoseToPose());
        }
    }

    DriverPose_t HmdDeviceServerDriver::RawPoseToPose()
    {
        DriverPose_t pose{ 0 };

        //ルーム変換行列の変換
        Eigen::Affine3d RoomToDriverAffin;
        RoomToDriverAffin = Config::GetInstance()->GetRoomToDriverMatrix();

        Eigen::Translation3d pos(RoomToDriverAffin.translation());
        Eigen::Quaterniond rot(RoomToDriverAffin.rotation());

        //OpenVR姿勢へ、一旦通常のデータを書き込む
        pose.poseTimeOffset = m_rawPose.timeoffset;

        pose.qWorldFromDriverRotation.x = rot.x();
        pose.qWorldFromDriverRotation.y = rot.y();
        pose.qWorldFromDriverRotation.z = rot.z();
        pose.qWorldFromDriverRotation.w = rot.w();

        pose.vecWorldFromDriverTranslation[0] = pos.x();
        pose.vecWorldFromDriverTranslation[1] = pos.y();
        pose.vecWorldFromDriverTranslation[2] = pos.z();

        pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;

        pose.vecDriverFromHeadTranslation[0] = 0.0f;
        pose.vecDriverFromHeadTranslation[1] = 0.0f;
        pose.vecDriverFromHeadTranslation[2] = 0.0f;

        pose.vecPosition[0] = m_rawPose.x;
        pose.vecPosition[1] = m_rawPose.y;
        pose.vecPosition[2] = m_rawPose.z;

        pose.vecVelocity[0] = 0.0f;
        pose.vecVelocity[1] = 0.0f;
        pose.vecVelocity[2] = 0.0f;

        pose.vecAcceleration[0] = 0.0f;
        pose.vecAcceleration[1] = 0.0f;
        pose.vecAcceleration[2] = 0.0f;

        pose.qRotation.x = m_rawPose.qx;
        pose.qRotation.y = m_rawPose.qy;
        pose.qRotation.z = m_rawPose.qz;
        pose.qRotation.w = m_rawPose.qw;

        pose.vecAngularVelocity[0] = 0.0f;
        pose.vecAngularVelocity[1] = 0.0f;
        pose.vecAngularVelocity[2] = 0.0f;

        pose.vecAngularAcceleration[0] = 0.0f;
        pose.vecAngularAcceleration[1] = 0.0f;
        pose.vecAngularAcceleration[2] = 0.0f;

        pose.result = TrackingResult_Running_OK;

        pose.poseIsValid = true;
        pose.willDriftInYaw = false;
        pose.shouldApplyHeadModel = false;

        pose.deviceIsConnected = true;

        //デバイスが有効でない場合、ステータスを無効で更新し、ここで返却
        if (m_rawPose.enable == 0) {
            pose.deviceIsConnected = false;
            pose.poseIsValid = false;
            pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
            //デバイス = 非接続・無効
            return pose;
        }

        return pose;
    }

    //仮想デバイスからOpenVRへデバイスの登録を依頼する
    void HmdDeviceServerDriver::RegisterToVRSystem()
    {
        VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_HMD, this);
        m_alreadyRegistered = true;
    }

    //仮想デバイスの状態をリセットする
    void HmdDeviceServerDriver::Reset()
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
    }

    //仮想デバイスからOpenVRへデバイスの姿勢の更新を通知する(サーバーから毎フレームコールされる)
    void HmdDeviceServerDriver::UpdatePoseToVRSystem()
    {
        if (!m_alreadyRegistered) { return; }
        //姿勢を更新
        VRServerDriverHost()->TrackedDevicePoseUpdated(m_deviceIndex, GetPose(), sizeof(DriverPose_t));
    }

    //仮想デバイスでOpenVRイベントを処理する(サーバーからイベントがあるタイミングでコールされる)
    void HmdDeviceServerDriver::ProcessEvent(VREvent_t& VREvent)
    {
    }

    //仮想デバイスの姿勢を、OpenVRに転送するたびに自動更新するか
    void HmdDeviceServerDriver::SetAutoUpdate(bool enable)
    {
        s_autoUpdate = enable;
    }


    //** OpenVR向け関数群 **

    //OpenVRからのデバイス有効化コール
    EVRInitError HmdDeviceServerDriver::Activate(uint32_t unObjectId)
    {
        //OpenVR Indexの記録
        m_deviceIndex = unObjectId;

        //OpenVR プロパティコンテナの保持
        m_propertyContainer = VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);

        //OpenVR デバイスプロパティの設定
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_IsOnDesktop_Bool, false);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_UserIpdMeters_Float, 0.058f);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_UserHeadToEyeDepthMeters_Float, 0.0f);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_DisplayFrequency_Float, 90);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_SecondsFromVsyncToPhotons_Float, 0.011f);


        VRProperties()->SetStringProperty(m_propertyContainer, Prop_TrackingSystemName_String, "VirtualMotionTracker");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ModelNumber_String, m_serial.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_SerialNumber_String, m_serial.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_RenderModelName_String, "{vmt}vmt_rendermodel");
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_CurrentUniverseId_Uint64, 2);

        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_WillDriftInYaw_Bool, false);
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ManufacturerName_String, "VirtualMotionTracker");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_TrackingFirmwareVersion_String, Version.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_HardwareRevision_String, Version.c_str());

        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ConnectedWirelessDongle_String, Version.c_str());
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DeviceIsWireless_Bool, true);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DeviceIsCharging_Bool, false);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_DeviceBatteryPercentage_Float, 1.0f);

        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Firmware_UpdateAvailable_Bool, false);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Firmware_ManualUpdate_Bool, true);
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_Firmware_ManualUpdateURL_String, "https://github.com/gpsnmeajp/VirtualMotionTracker");
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_HardwareRevision_Uint64, 0);
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_FirmwareVersion_Uint64, 0);
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_FPGAVersion_Uint64, 0);
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_VRCVersion_Uint64, 0);
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_RadioVersion_Uint64, 0);
        VRProperties()->SetUint64Property(m_propertyContainer, Prop_DongleVersion_Uint64, 0);



        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DeviceProvidesBatteryStatus_Bool, true);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DeviceCanPowerOff_Bool, true);
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_Firmware_ProgrammingTarget_String, Version.c_str());


        
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Firmware_ForceUpdateRequired_Bool, false);

        VRProperties()->SetUint64Property(m_propertyContainer, Prop_ParentDriver_Uint64, 0);
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ResourceRoot_String, "vmt");
        std::string RegisteredDeviceType_String = std::string("vmt/");
        RegisteredDeviceType_String += m_serial.c_str();
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_RegisteredDeviceType_String, RegisteredDeviceType_String.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_InputProfilePath_String, "{vmt}/input/vmt_profile.json"); //vmt_profile.jsonに影響する
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_NeverTracked_Bool, false);


        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Identifiable_Bool, true);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_CanWirelessIdentify_Bool, true);

        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Firmware_RemindUpdate_Bool, false);

        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_ReportsTimeSinceVSync_Bool, false);

        VRProperties()->SetUint64Property(m_propertyContainer, Prop_PreviousUniverseId_Uint64, 2);

        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DisplaySupportsRuntimeFramerateChange_Bool, false);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DisplaySupportsAnalogGain_Bool, false);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_DisplayMinAnalogGain_Float, 1.0f);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_DisplayMaxAnalogGain_Float, 1.0f);


        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceOff_String, "{vmt}/icons/Off32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceSearching_String, "{vmt}/icons/Searching32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{vmt}/icons/SearchingAlert32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceReady_String, "{vmt}/icons/Ready32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{vmt}/icons/ReadyAlert32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{vmt}/icons/NotReady32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceStandby_String, "{vmt}/icons/Standby32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{vmt}/icons/StandbyAlert32x32.png");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{vmt}/icons/AlertLow32x32.png");

        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasDisplayComponent_Bool, true);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasCameraComponent_Bool, false);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasDriverDirectModeComponent_Bool, false);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasVirtualDisplayComponent_Bool, true);

        //VRProperties()->SetStringProperty(m_propertyContainer, vmt_profile.json, "NO_SETTING"); //設定不可
        VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerHandSelectionPriority_Int32, 0);
        return EVRInitError::VRInitError_None;
    }

    //OpenVRからのデバイス無効化コール
    void HmdDeviceServerDriver::Deactivate()
    {
        m_deviceIndex = k_unTrackedDeviceIndexInvalid;
        m_propertyContainer = k_ulInvalidPropertyContainer;
    }

    //OpenVRからのデバイス電源オフコール
    void HmdDeviceServerDriver::EnterStandby()
    {
        //電源オフ要求が来た
        Reset();
    }

    //OpenVRからのデバイス固有機能の取得(ない場合はnullptrを返す)
    void* HmdDeviceServerDriver::GetComponent(const char* pchComponentNameAndVersion)
    {
        if (pchComponentNameAndVersion != nullptr) {
            string input(pchComponentNameAndVersion);
            string ivrdisplay(vr::IVRDisplayComponent_Version);

            if (input == ivrdisplay) {
                return (vr::IVRDisplayComponent*)this;
            }
        }
        return nullptr;
    }

    //OpenVRからのデバイスのデバッグリクエスト
    void HmdDeviceServerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
    {
        //デバッグ用
        //Log::printf("DebugRequest: %s", pchRequest);
        if (unResponseBufferSize > 0) {
            pchResponseBuffer[0] = '\0';
        }
    }

    //OpenVRからのデバイス姿勢取得
    DriverPose_t HmdDeviceServerDriver::GetPose()
    {
        //自動更新が有効 AND デバイス登録済み AND 電源オン状態の場合
        if (s_autoUpdate && m_alreadyRegistered) {
            //加速度計算の自動更新を行う
            m_lastRawPose = m_rawPose;
            m_rawPose.time = std::chrono::system_clock::now();
            //姿勢情報の更新(他デバイス連動時に効果あり)
            SetPose(RawPoseToPose());
        }
        //現在のOpenVR向け姿勢を返却する
        return m_pose;
    }
    void HmdDeviceServerDriver::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
    {
        *pnX = 0;
        *pnY = 0;
        *pnWidth = 800;
        *pnHeight = 400;
    }
    bool HmdDeviceServerDriver::IsDisplayOnDesktop()
    {
        return true;
    }
    bool HmdDeviceServerDriver::IsDisplayRealDisplay()
    {
        return false;
    }
    void HmdDeviceServerDriver::GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight)
    {
        *pnWidth = 800;
        *pnHeight = 400;
    }
    void HmdDeviceServerDriver::GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
    {
        *pnY = 0;
        *pnWidth = 400;
        *pnHeight = 400;

        if (eEye == Eye_Left) {
            *pnX = 0;
        }
        else {
            *pnX = 400;
        }
    }
    void HmdDeviceServerDriver::GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
    {
        *pfLeft = -1.0;
        *pfRight = 1.0;
        *pfTop = -1.0;
        *pfBottom = 1.0;
    }
    DistortionCoordinates_t HmdDeviceServerDriver::ComputeDistortion(EVREye eEye, float fU, float fV)
    {
        DistortionCoordinates_t cood{};
        cood.rfBlue[0] = fU;
        cood.rfBlue[1] = fV;
        cood.rfGreen[0] = fU;
        cood.rfGreen[1] = fV;
        cood.rfRed[0] = fU;
        cood.rfRed[1] = fV;

        return cood;
    }
}