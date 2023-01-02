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
#pragma once
#include "dllmain.h"

//OpenVRデバイス
//サーバーにぶら下がる子である
namespace VMTDriver {
    const HmdQuaternion_t HmdQuaternion_Identity{ 1,0,0,0 };
    const int skeletonBoneCount{ 31 };
    const int buttonCount{ 8 };
    const int triggerCount{ 2 };
    const int joystickCount{ 2 };

    struct RawPose {
        bool roomToDriver{};
        int idx{};
        int enable{};
        double x{};
        double y{};
        double z{};
        double qx{};
        double qy{};
        double qz{};
        double qw{};
        double timeoffset{};
        ReferMode_t mode{};
        std::string root_sn{};
        std::chrono::system_clock::time_point time{};
    };

    enum class ControllerRole {
        None,
        Left,
        Right
    };

    //手骨格ボーン
    enum class SkeletonBone {
        //手の原点
        Root = 0,
        //手のボーン
        Wrist,
        Thumb0,
        Thumb1,
        Thumb2,
        Thumb3,
        IndexFinger0,
        IndexFinger1,
        IndexFinger2,
        IndexFinger3,
        IndexFinger4,
        MiddleFinger0,
        MiddleFinger1,
        MiddleFinger2,
        MiddleFinger3,
        MiddleFinger4,
        RingFinger0,
        RingFinger1,
        RingFinger2,
        RingFinger3,
        RingFinger4,
        PinkyFinger0,
        PinkyFinger1,
        PinkyFinger2,
        PinkyFinger3,
        PinkyFinger4,
        //補助ボーン(ルートの子)
        Aux_Thumb,
        Aux_IndexFinger,
        Aux_MiddleFinger,
        Aux_RingFinger,
        Aux_PinkyFinger,
        //番兵
        ENUM_MAX
    };

    //個々のデバイス
    class TrackedDeviceServerDriver : public ITrackedDeviceServerDriver
    {
    private:
        bool m_alreadyRegistered = false;
        bool m_registrationInProgress = false;
        string m_serial{ "" };
        TrackedDeviceIndex_t m_deviceIndex{ 0 };
        PropertyContainerHandle_t m_propertyContainer{ 0 };
        uint32_t m_index = k_unTrackedDeviceIndexInvalid;

        DriverPose_t m_pose{ 0 };
        RawPose m_rawPose{ 0 };
        RawPose m_lastRawPose{ 0 };

        VRBoneTransform_t m_boneTransform[skeletonBoneCount]{ 0 };

        VRInputComponentHandle_t ButtonComponent[buttonCount]{ 0 };
        VRInputComponentHandle_t TriggerComponent[triggerCount]{ 0 };
        VRInputComponentHandle_t JoystickComponent[joystickCount]{ 0 };
        VRInputComponentHandle_t HapticComponent{ 0 };
        VRInputComponentHandle_t SkeletonComponent{ 0 };

        ETrackedDeviceClass m_deviceClass{ ETrackedDeviceClass::TrackedDeviceClass_Invalid };
        ControllerRole m_controllerRole{ ControllerRole::None };
        bool m_poweron{ false };

        static bool s_autoUpdate;
    public:
        //内部向け
        TrackedDeviceServerDriver();
        ~TrackedDeviceServerDriver();

        void SetDeviceSerial(string);
        void SetObjectIndex(uint32_t);
        void SetPose(DriverPose_t pose);
        void SetRawPose(RawPose rawPose);
        DriverPose_t RawPoseToPose();
        void RegisterToVRSystem(int type);
        void UpdatePoseToVRSystem();
        void UpdateButtonInput(uint32_t index, bool value, double timeoffset);
        void UpdateTriggerInput(uint32_t index, float value, double timeoffset);
        void UpdateJoystickInput(uint32_t index, float x, float y, double timeoffset);

        void WriteSkeletonInputBuffer(uint32_t index, VRBoneTransform_t bone);
        void UpdateSkeletonInput(double timeoffset);
        void Reset();

        void CalcVelocity(DriverPose_t& pose);
        void CalcJoint(DriverPose_t& pose, string serial, ReferMode_t mode, Eigen::Affine3d& RoomToDriverAffin);
        int SearchDevice(vr::TrackedDevicePose_t* poses, string serial);
        void RejectTracking(DriverPose_t& pose);
        void ProcessEvent(VREvent_t &VREvent);

        static void SetAutoUpdate(bool enable);

        //OpenVR向け
        virtual EVRInitError Activate(uint32_t unObjectId) override;
        virtual void Deactivate() override;
        virtual void EnterStandby() override;
        virtual void* GetComponent(const char* pchComponentNameAndVersion) override;
        virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
        virtual DriverPose_t GetPose() override;
    };
}