#pragma once
#include "framework.h"
#include "..\openvr\headers\openvr_driver.h"
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

using namespace vr;

#include "ServerTrackedDeviceProvider.h"
#include "TrackedDeviceServerDriver.h"
#include "VRWatchdogProvider.h"
#include "Log.h"
