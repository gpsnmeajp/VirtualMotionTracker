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

		void ErrorCheck();
		void SaveJsonRoomToDriverMatrix(float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8, float m9, float m10, float m11, float m12);
		void SetRoomMatrixStatus(bool ok);
	public:
		static Config* GetInstance();

		void LoadSetting();

		bool GetRoomMatrixStatus();

		void SetRoomMatrix(bool save, float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8, float m9, float m10, float m11, float m12);

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