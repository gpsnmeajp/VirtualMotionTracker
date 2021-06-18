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
#include "framework.h"
#include <sstream>
#include <fstream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include "openvr_driver.h"
#include <thread>
#include <mutex>
#include <deque>
#include "Eigen/Dense"
#include "json.hpp"

//DLL定義
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
using Eigen::MatrixXd;

using namespace vr;
using std::string;
using std::vector;
using json = nlohmann::json;

namespace VMTDriver {
	class ServerTrackedDeviceProvider;
    class TrackedDeviceServerDriver;
    class HmdDeviceServerDriver;
    class VRWatchdogProvider;
	class Log;

    const string Version = "VMT_014";

	enum class ReferMode_t {
		None = 0,   // ルーム座標を使用する
		Joint = 1,  // Position, Rotation共に他のトラッカーデバイスの座標系を参照する
		Follow = 2, // Positionは他のトラッカーデバイスを参照するが、Rotationはルーム座標を参照する
	};

    const HmdQuaternion_t HmdQuaternion_Identity{ 1,0,0,0 };

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

	ServerTrackedDeviceProvider* GetServer();
	VRWatchdogProvider* GetWatchdog();
}

namespace DirectOSC {
	class OSC;
}

#include "DirectOSC.h"
#include "ServerTrackedDeviceProvider.h"
#include "TrackedDeviceServerDriver.h"
#include "HmdDeviceServerDriver.h"
#include "VRWatchdogProvider.h"
#include "Log.h"
#include "CommunicationManager.h"
#include "Config.h"