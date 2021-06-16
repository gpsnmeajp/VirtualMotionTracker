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
    //** 内部向け関数群 **

    //自動更新を有効にするか
    bool TrackedDeviceServerDriver::s_autoUpdate = false;

    //仮想デバイスのコンストラクタ。(Listから暗黙的にコールされる)
    TrackedDeviceServerDriver::TrackedDeviceServerDriver()
    {
        m_deviceIndex = k_unTrackedDeviceIndexInvalid;
        m_propertyContainer = k_ulInvalidPropertyContainer;
    }

    //仮想デバイスのデストラクタ。(Listから暗黙的にコールされる)
    TrackedDeviceServerDriver::~TrackedDeviceServerDriver()
    {
    }

    //仮想デバイスにシリアル番号を設定
    void TrackedDeviceServerDriver::SetDeviceSerial(string serial)
    {
        m_serial = serial;
    }

    //仮想デバイスに内部Indexを設定
    void TrackedDeviceServerDriver::SetObjectIndex(uint32_t idx)
    {
        m_index = idx;
    }

    //仮想デバイスにOpenVR姿勢を設定
    void TrackedDeviceServerDriver::SetPose(DriverPose_t pose)
    {
        m_pose = pose;
    }

    //仮想デバイスに内部姿勢を設定
    void TrackedDeviceServerDriver::SetRawPose(RawPose rawPose)
    {
        m_poweron = true; //有効な姿勢なので電源オン状態にする

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

    //内部姿勢→OpenVR姿勢の変換と、相対座標計算処理を行う
    void TrackedDeviceServerDriver::CalcVelocity(DriverPose_t& pose) {
        //経過時間を計算
        double duration_sec = std::chrono::duration_cast<std::chrono::microseconds>((m_rawPose.time - m_lastRawPose.time)).count() / (1000.0 * 1000.0);
        //速度・角速度を計算
        if (duration_sec > std::numeric_limits<double>::epsilon())
        {
            pose.vecVelocity[0] = (m_rawPose.x - m_lastRawPose.x) / duration_sec;
            pose.vecVelocity[1] = (m_rawPose.y - m_lastRawPose.y) / duration_sec;
            pose.vecVelocity[2] = (m_rawPose.z - m_lastRawPose.z) / duration_sec;

            Eigen::Quaterniond q1(m_rawPose.qw, m_rawPose.qx, m_rawPose.qy, m_rawPose.qz);
            Eigen::Quaterniond q2(m_lastRawPose.qw, m_lastRawPose.qx, m_lastRawPose.qy, m_lastRawPose.qz);

            Eigen::Quaterniond dq = q1 * q2.inverse();
            Eigen::AngleAxisd dAAx(dq);
            double angle = dAAx.angle();
            double angularVelocity = angle / duration_sec;
            Eigen::Vector3d axis = dAAx.axis();
            Eigen::Vector3d vecAngularVelocity = axis * angularVelocity;

            pose.vecAngularVelocity[0] = vecAngularVelocity.x();
            pose.vecAngularVelocity[1] = vecAngularVelocity.y();
            pose.vecAngularVelocity[2] = vecAngularVelocity.z();
        }
        else {
            pose.vecVelocity[0] = 0.0f;
            pose.vecVelocity[1] = 0.0f;
            pose.vecVelocity[2] = 0.0f;
            pose.vecAngularVelocity[0] = 0.0f;
            pose.vecAngularVelocity[1] = 0.0f;
            pose.vecAngularVelocity[2] = 0.0f;
        }
    }

    //Joint計算を行う
    void TrackedDeviceServerDriver::CalcJoint(DriverPose_t& pose, string serial, ReferMode_t mode, Eigen::Affine3d& RoomToDriverAffin) {
        vr::TrackedDevicePose_t devicePoses[k_unMaxTrackedDeviceCount]{};

        //OpenVRから全トラッキングデバイスの情報を取得する
        VRServerDriverHost()->GetRawTrackedDevicePoses(0.0f, devicePoses, k_unMaxTrackedDeviceCount);

        //接続済みのデバイスの中から、シリアル番号でデバイスを検索する
        int index = SearchDevice(devicePoses, serial);

        //探索エラーが帰ってきたら
        if (index == k_unTrackedDeviceIndexInvalid) {
            //デバイス = 接続済み・無効
            RejectTracking(pose);
            return;
        }

        vr::TrackedDevicePose_t& devicePose = devicePoses[index];

        //参照元のトラッキングステータスを継承させる(Reject無効化時に意味あり)
        pose.poseIsValid = devicePose.bPoseIsValid;
        pose.result = devicePose.eTrackingResult;

        //デバイスのトラッキング状態が正常なら
        if (devicePose.bPoseIsValid) {
            //デバイスの変換行列を取得し、Eigenの行列に変換
            float* m = (float*)(devicePose.mDeviceToAbsoluteTracking.m);

            Eigen::Affine3d rootDeviceToAbsoluteTracking;
            rootDeviceToAbsoluteTracking.matrix() <<
                m[0 * 4 + 0], m[0 * 4 + 1], m[0 * 4 + 2], m[0 * 4 + 3],
                m[1 * 4 + 0], m[1 * 4 + 1], m[1 * 4 + 2], m[1 * 4 + 3],
                m[2 * 4 + 0], m[2 * 4 + 1], m[2 * 4 + 2], m[2 * 4 + 3],
                0.0, 0.0, 0.0, 1.0;

            //位置の座標系をデバイス基準にする
            Eigen::Translation3d pos(rootDeviceToAbsoluteTracking.translation());
            pose.vecWorldFromDriverTranslation[0] = pos.x();
            pose.vecWorldFromDriverTranslation[1] = pos.y();
            pose.vecWorldFromDriverTranslation[2] = pos.z();

            //回転の座標系をルーム基準にしたりデバイス基準にしたりする
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

            //デバイス = 接続済み・有効・特殊座標系
            return;
        }
        else {
            //デバイス = 接続済み・無効
            RejectTracking(pose);
            return;
        }
    }

    //デバイスをシリアル番号から探す
    int TrackedDeviceServerDriver::SearchDevice(vr::TrackedDevicePose_t* poses, string serial)
    {
        IVRProperties* props = VRPropertiesRaw();
        CVRPropertyHelpers* helper = VRProperties();

        //デバイスシリアルが空白
        if (serial.empty()) {
            //探索エラーを返す
            return k_unTrackedDeviceIndexInvalid;
        }

        //デバイスシリアルがHMD(でかつ、HMD特別処理が有効なら)
        if (serial == "HMD" && Config::GetInstance()->GetHMDisIndex0()) {
            //HMDが接続OKなら
            if (poses[k_unTrackedDeviceIndex_Hmd].bDeviceIsConnected) {
                //HMDのインデックスを返す
                return k_unTrackedDeviceIndex_Hmd;
            }
            else {
                //(HMDがつながっていないのは普通ありえないが)探索エラーを返す
                return k_unTrackedDeviceIndexInvalid;
            }
        }

        //デバイスをOpenVR index順に調べる
        for (uint32_t i = 0; i < k_unMaxTrackedDeviceCount; i++) {
            //そのデバイスがつながっていないなら次のデバイスへ
            if (poses[i].bDeviceIsConnected != true) {
                continue;
            }

            //デバイスがつながっているので、シリアルナンバーを取得する
            PropertyContainerHandle_t h = props->TrackedDeviceToPropertyContainer(i);
            string SerialNumber = helper->GetStringProperty(h, ETrackedDeviceProperty::Prop_SerialNumber_String);

            //対象シリアルナンバーと比較し、違うデバイスなら、次のデバイスへ
            if (serial != SerialNumber) {
                continue;
            };

            //目的のデバイスを見つけたので返却
            return i;
        }
        //最後まで探したが、目的のデバイスは見つからなかった
        return k_unTrackedDeviceIndexInvalid;
    }

    //デバイスをトラッキング失敗状態にする
    void TrackedDeviceServerDriver::RejectTracking(DriverPose_t& pose)
    {
        //(ただし、設定から有効な場合のみ。そうでない場合は無視してトラッキングを継続する)
        if (Config::GetInstance()->GetRejectWhenCannotTracking()) {
            //デバイス = 接続済み・無効
            pose.poseIsValid = false;
            pose.result = TrackingResult_Running_OutOfRange;
        }
    }

    DriverPose_t TrackedDeviceServerDriver::RawPoseToPose()
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

        //ルームマトリクスが設定されていないとき、ステータスを無効で更新し、ここで返却
        if (!Config::GetInstance()->GetRoomMatrixStatus()) {
            //デバイス = 接続済み・無効
            RejectTracking(pose);
            return pose;
        }

        //速度エミュレーションが有効な場合、速度・各速度の計算を行い、更新する
        if (Config::GetInstance()->GetVelocityEnable()) {
            CalcVelocity(pose);
        }

        //トラッキングモードに合わせて処理する
        switch (m_rawPose.mode) {
            case ReferMode_t::None: {
                //通常のトラッキングモードの場合、何もしない
                //デバイス = 接続済み・有効・ルーム座標系

                //do noting
                break;
            }

            case ReferMode_t::Follow: {
                //デバイス = 接続済み・有効・デバイス位置座標系
                CalcJoint(pose, m_rawPose.root_sn, ReferMode_t::Follow, RoomToDriverAffin);
                break;
            }

            case ReferMode_t::Joint: {
                //デバイス = 接続済み・有効・デバイス位置回転座標系
                CalcJoint(pose, m_rawPose.root_sn, ReferMode_t::Joint, RoomToDriverAffin);
                break;
            }
            default: {
                //デバイス = 接続済み・無効
                RejectTracking(pose);
                break;
            }
        }
        return pose;
    }

    //仮想デバイスからOpenVRへデバイスの登録を依頼する
    void TrackedDeviceServerDriver::RegisterToVRSystem(int type)
    {
        if (!m_alreadyRegistered)
        {
            switch (type)
            {
            case 5://HMD
                VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_HMD, this);
                m_alreadyRegistered = true;
                break;
            case 4://TrackingReference
                if (Config::GetInstance()->GetOptoutTrackingRole()) {
                    VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_OptOut); //手に割り当てないように
                }
                VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_TrackingReference, this);
                m_alreadyRegistered = true;
                break;
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
                if (Config::GetInstance()->GetOptoutTrackingRole()) {
                    VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_OptOut); //手に割り当てないように
                }
                VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, this);
                m_alreadyRegistered = true;
                break;
            default:
                break;
            }
        }
    }


    //仮想デバイスからOpenVRへデバイスのボタン状態の更新を通知する
    void TrackedDeviceServerDriver::UpdateButtonInput(uint32_t index, bool value, double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (0 <= index && index <= 7)
        {
            VRDriverInput()->UpdateBooleanComponent(ButtonComponent[index], value, timeoffset);
        }
    }

    //仮想デバイスからOpenVRへデバイスのトリガー(1軸)状態の更新を通知する
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

    //仮想デバイスからOpenVRへデバイスのジョイスティック(2軸)状態の更新を通知する
    void TrackedDeviceServerDriver::UpdateJoystickInput(uint32_t index, float x, float y, double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (index == 0)
        {
            VRDriverInput()->UpdateScalarComponent(JoystickComponent[index + 0], x, timeoffset);
            VRDriverInput()->UpdateScalarComponent(JoystickComponent[index + 1], y, timeoffset);
        }
    }

    //仮想デバイスの状態をリセットする
    void TrackedDeviceServerDriver::Reset()
    {
        m_poweron = false; //電源オフ状態にする

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

    //仮想デバイスからOpenVRへデバイスの姿勢の更新を通知する(サーバーから毎フレームコールされる)
    void TrackedDeviceServerDriver::UpdatePoseToVRSystem()
    {
        if (!m_alreadyRegistered) { return; }
        //姿勢を更新
        VRServerDriverHost()->TrackedDevicePoseUpdated(m_deviceIndex, GetPose(), sizeof(DriverPose_t));
    }

    //仮想デバイスでOpenVRイベントを処理する(サーバーからイベントがあるタイミングでコールされる)
    void TrackedDeviceServerDriver::ProcessEvent(VREvent_t& VREvent)
    {
        //未登録 or 電源オフ時は反応しない
        if (!m_alreadyRegistered || !m_poweron) {
            return;
        }

        //異常値を除去(なんで送られてくるんだ？)
        if (VREvent_VendorSpecific_Reserved_End < VREvent.eventType) {
            return;
        }

        switch (VREvent.eventType)
        {
        case EVREventType::VREvent_Input_HapticVibration:
            //バイブレーション
            if (VREvent.data.hapticVibration.componentHandle == HapticComponent) {
                OSCReceiver::SendHaptic(m_index, VREvent.data.hapticVibration.fFrequency, VREvent.data.hapticVibration.fAmplitude, VREvent.data.hapticVibration.fDurationSeconds);
            }
            break;
        default:
            //デバッグ用
            //if (m_serial == "VMT_0") {
            //    Log::printf("Event: %d\n",VREvent.eventType);
            //}
            break;
        }
    }

    //仮想デバイスの姿勢を、OpenVRに転送するたびに自動更新するか
    void TrackedDeviceServerDriver::SetAutoUpdate(bool enable)
    {
        s_autoUpdate = enable;
    }




    //** OpenVR向け関数群 **

    //OpenVRからのデバイス有効化コール
    EVRInitError TrackedDeviceServerDriver::Activate(uint32_t unObjectId)
    {
        //OpenVR Indexの記録
        m_deviceIndex = unObjectId;

        //OpenVR プロパティコンテナの保持
        m_propertyContainer = VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);

        //OpenVR デバイスプロパティの設定
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_TrackingSystemName_String, "VirtualMotionTracker");
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_ModelNumber_String, m_serial.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_SerialNumber_String, m_serial.c_str());
        VRProperties()->SetStringProperty(m_propertyContainer, Prop_RenderModelName_String, "{vmt}vmt_rendermodel");
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


        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_IsOnDesktop_Bool, false);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_UserIpdMeters_Float, 0.058f);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_UserHeadToEyeDepthMeters_Float, 0.0f);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_DisplayFrequency_Float, 90);
        VRProperties()->SetFloatProperty(m_propertyContainer, Prop_SecondsFromVsyncToPhotons_Float, 0.011f);


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

        VRProperties()->SetUint64Property(m_propertyContainer, Prop_CurrentUniverseId_Uint64, 2);
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

        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasDisplayComponent_Bool, false);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasCameraComponent_Bool, false);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasDriverDirectModeComponent_Bool, false);
        VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasVirtualDisplayComponent_Bool, false);

        //VRProperties()->SetStringProperty(m_propertyContainer, vmt_profile.json, "NO_SETTING"); //設定不可
        VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerHandSelectionPriority_Int32, 0);


        //OpenVR デバイス入力情報の定義
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

    //OpenVRからのデバイス無効化コール
    void TrackedDeviceServerDriver::Deactivate()
    {
        m_deviceIndex = k_unTrackedDeviceIndexInvalid;
        m_propertyContainer = k_ulInvalidPropertyContainer;
    }

    //OpenVRからのデバイス電源オフコール
    void TrackedDeviceServerDriver::EnterStandby()
    {
        //電源オフ要求が来た
        Reset();
    }

    //OpenVRからのデバイス固有機能の取得(ない場合はnullptrを返す)
    void* TrackedDeviceServerDriver::GetComponent(const char* pchComponentNameAndVersion)
    {
        return nullptr;
    }

    //OpenVRからのデバイスのデバッグリクエスト
    void TrackedDeviceServerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
    {
        //デバッグ用
        //Log::printf("DebugRequest: %s", pchRequest);
        if (unResponseBufferSize > 0) {
            pchResponseBuffer[0] = '\0';
        }
    }

    //OpenVRからのデバイス姿勢取得
    DriverPose_t TrackedDeviceServerDriver::GetPose()
    {
        //自動更新が有効 AND デバイス登録済み AND 電源オン状態の場合
        if (s_autoUpdate && m_alreadyRegistered && m_poweron) {
            //加速度計算の自動更新を行う
            m_lastRawPose = m_rawPose;
            m_rawPose.time = std::chrono::system_clock::now();
            //姿勢情報の更新(他デバイス連動時に効果あり)
            SetPose(RawPoseToPose());
        }
        //現在のOpenVR向け姿勢を返却する
        return m_pose;
    }

    void TrackedDeviceServerDriver::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
    {
        *pnX = 500;
        *pnY = 500;
        *pnWidth = 800;
        *pnHeight = 400;
    }
    bool TrackedDeviceServerDriver::IsDisplayOnDesktop()
    {
        return true;
    }
    bool TrackedDeviceServerDriver::IsDisplayRealDisplay()
    {
        return false;
    }
    void TrackedDeviceServerDriver::GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight)
    {
        *pnWidth = 800;
        *pnHeight = 400;
    }
    void TrackedDeviceServerDriver::GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
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
    void TrackedDeviceServerDriver::GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
    {
        *pfLeft = -1.0;
        *pfRight = -1.0;
        *pfTop = -1.0;
        *pfBottom = -1.0;
    }
    DistortionCoordinates_t TrackedDeviceServerDriver::ComputeDistortion(EVREye eEye, float fU, float fV)
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