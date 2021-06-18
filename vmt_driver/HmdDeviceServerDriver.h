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

    //個々のデバイス
    class HmdDeviceServerDriver : public ITrackedDeviceServerDriver, IVRDisplayComponent
    {
    private:
        bool m_alreadyRegistered = false;
        string m_serial = "";
        TrackedDeviceIndex_t m_deviceIndex{ 0 };
        PropertyContainerHandle_t m_propertyContainer{ 0 };
        uint32_t m_index = k_unTrackedDeviceIndexInvalid;

        DriverPose_t m_pose{ 0 };
        RawPose m_rawPose{ 0 };
        RawPose m_lastRawPose{ 0 };

        static bool s_autoUpdate;
    public:
        //内部向け
        HmdDeviceServerDriver();
        ~HmdDeviceServerDriver();

        void SetDeviceSerial(string);
        void SetObjectIndex(uint32_t);
        void SetPose(DriverPose_t pose);
        void SetRawPose(RawPose rawPose);
        DriverPose_t RawPoseToPose();
        void RegisterToVRSystem();
        void UpdatePoseToVRSystem();
        void Reset();

        void ProcessEvent(VREvent_t &VREvent);

        static void SetAutoUpdate(bool enable);

        //OpenVR向け
        virtual EVRInitError Activate(uint32_t unObjectId) override;
        virtual void Deactivate() override;
        virtual void EnterStandby() override;
        virtual void* GetComponent(const char* pchComponentNameAndVersion) override;
        virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
        virtual DriverPose_t GetPose() override;

        // IVRDisplayComponent を介して継承されました
        virtual void GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) override;
        virtual bool IsDisplayOnDesktop() override;
        virtual bool IsDisplayRealDisplay() override;
        virtual void GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight) override;
        virtual void GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) override;
        virtual void GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom) override;
        virtual DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) override;
    };
}