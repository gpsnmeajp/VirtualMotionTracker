#pragma once
#include "framework.h"
#include <string>
#include <vector>
#include "..\openvr\headers\openvr_driver.h"


#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

using namespace vr;
namespace VMTDriver {
	class ServerTrackedDeviceProvider;
	class TrackedDeviceServerDriver;
	class VRWatchdogProvider;
	class Log;
}

#include "ServerTrackedDeviceProvider.h"
#include "TrackedDeviceServerDriver.h"
#include "VRWatchdogProvider.h"
#include "Log.h"
