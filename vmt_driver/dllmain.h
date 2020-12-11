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


#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
using Eigen::MatrixXd;

using namespace vr;
using std::string;
using std::vector;
using json = nlohmann::json;

namespace VMTDriver {
	class ServerTrackedDeviceProvider;
	class TrackedDeviceServerDriver;
	class VRWatchdogProvider;
	class Log;

	enum ReferMode_t : int {
		None = 0,   // ���[�����W���g�p����
		Joint = 1,  // Position, Rotation���ɑ��̃g���b�J�[�f�o�C�X�̍��W�n���Q�Ƃ���
		Follow = 2, // Position�͑��̃g���b�J�[�f�o�C�X���Q�Ƃ��邪�ARotation�̓��[�����W���Q�Ƃ���
	};
}

namespace DirectOSC {
	class OSC;
}

#include "DirectOSC.h"
#include "ServerTrackedDeviceProvider.h"
#include "TrackedDeviceServerDriver.h"
#include "VRWatchdogProvider.h"
#include "Log.h"
#include "CommunicationManager.h"
