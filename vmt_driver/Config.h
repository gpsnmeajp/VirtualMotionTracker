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
	class Config {
	private:
		Config();

		//Status
		bool m_RoomMatrixStatus = false;

		//Config
		Eigen::Matrix4d m_RoomToDriverMatrix = Eigen::Matrix4d::Identity();
		bool m_velocityEnable = false;
		int m_receivePort = 39570;
		int m_sendPort = 39571;
		bool m_optoutTrackingRole = true;
		bool m_HMDisIndex0 = true;
		bool m_RejectWhenCannotTracking = true;
		bool m_DefaultAutoPoseUpdateOn = true;

	public:
		static Config* GetInstance();

		void InitSetting();
		void LoadSetting();

		void SetRoomMatrixStatus(bool ok);
		bool GetRoomMatrixStatus();

		Eigen::Matrix4d& GetRoomToDriverMatrix();
		bool GetVelocityEnable();
		int GetReceivePort();
		int GetSendPort();
		bool GetOptoutTrackingRole();
		bool GetHMDisIndex0();
		bool GetRejectWhenCannotTracking();
		bool GetDefaultAutoPoseUpdateOn();
	};
}