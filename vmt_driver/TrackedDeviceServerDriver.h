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

namespace VMTDriver {
    const HmdQuaternion_t HmdQuaternion_Identity{ 1,0,0,0 };

    //個々のデバイス
    class TrackedDeviceServerDriver : public ITrackedDeviceServerDriver
    {
    private:
        bool m_alreadyRegistered = false;
        string m_serial;
        TrackedDeviceIndex_t m_deviceIndex;
        PropertyContainerHandle_t m_propertyContainer;
        int m_index;

        DriverPose_t m_pose{ 0 };

        VRInputComponentHandle_t ButtonComponent[8];
        VRInputComponentHandle_t TriggerComponent[2];
        VRInputComponentHandle_t JoystickComponent[2];

    public:
        TrackedDeviceServerDriver();
        ~TrackedDeviceServerDriver();

        void SetDeviceSerial(string);
        void SetObjectIndex(int);
        void SetPose(DriverPose_t pose);
        void RegisterToVRSystem();
        void UpdatePoseToVRSystem();
        void UpdateButtonInput(int index, bool value, double timeoffset);
        void UpdateTriggerInput(int index, float value, double timeoffset);
        void UpdateJoystickInput(int index, float x, float y, double timeoffset);
        void Reset();

        //---------------

        virtual EVRInitError Activate(uint32_t unObjectId) override;
        virtual void Deactivate() override;
        virtual void EnterStandby() override;
        virtual void* GetComponent(const char* pchComponentNameAndVersion) override;
        virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
        virtual DriverPose_t GetPose() override;
    };
}