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
    const int buttonCount{ 18 };
    const int triggerCount{ 9 };
    const int joystickCount{ 4 };

    //生姿勢
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

    //コントローラの役割
    enum class ControllerRole {
        None,
        Left,
        Right
    };

    //コントローラの役割
    enum class SkeletonBonePoseStatic {
        BindHand,
        OpenHand,
        Fist
    };

    //手骨格補間指
    enum class SkeletonLerpFinder {
        RootAndWrist = 0,
        Thumb = 1,
        Index = 2,
        Middle = 3,
        Ring = 4,
        PinkyLittle = 5,
    };

    //手骨格ボーン
    enum class SkeletonBone {
        //手の原点
        Root = 0,
        //手のボーン
        Wrist = 1,
        Thumb0_ThumbProximal = 2,
        Thumb1_ThumbIntermediate = 3,
        Thumb2_ThumbDistal = 4,
        Thumb3_ThumbEnd = 5,
        IndexFinger0_IndexProximal = 6,
        IndexFinger1_IndexIntermediate = 7,
        IndexFinger2_IndexDistal = 8,
        IndexFinger3_IndexDistal2 = 9,
        IndexFinger4_IndexEnd = 10,
        MiddleFinger0_MiddleProximal = 11,
        MiddleFinger1_MiddleIntermediate = 12,
        MiddleFinger2_MiddleDistal = 13,
        MiddleFinger3_MiddleDistal2 = 14,
        MiddleFinger4_MiddleEnd = 15,
        RingFinger0_RingProximal = 16,
        RingFinger1_RingIntermediate = 17,
        RingFinger2_RingDistal = 18,
        RingFinger3_RingDistal2 = 19,
        RingFinger4_RingEnd = 20,
        PinkyFinger0_LittleProximal = 21,
        PinkyFinger1_LittleIntermediate = 22,
        PinkyFinger2_LittleDistal = 23,
        PinkyFinger3_LittleDistal2 = 24,
        PinkyFinger4_LittleEnd = 25,
        //補助ボーン(ルートの子)
        Aux_Thumb_ThumbHelper = 26,
        Aux_IndexFinger_IndexHelper = 27,
        Aux_MiddleFinger_MiddleHelper = 28,
        Aux_RingFinger_RingHelper = 29,
        Aux_PinkyFinger_LittleHelper = 30,
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
        VRInputComponentHandle_t JoystickXComponent[joystickCount]{ 0 };
        VRInputComponentHandle_t JoystickYComponent[joystickCount]{ 0 };
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
        void WriteSkeletonInputBufferStaticLerpFinger(uint32_t finger, double t, uint32_t mode);
        void WriteSkeletonInputBufferStaticLerpBone(const VRBoneTransform_t a[], const VRBoneTransform_t b[], uint32_t index, double t);
        void WriteSkeletonInputBufferStatic(SkeletonBonePoseStatic type);
        void UpdateSkeletonInput(double timeoffset);
        void Reset();

        std::string VMTDebugCommand(std::string command);

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