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
	const string Version = "VMT_005";

	class OSCReceiver : public osc::OscPacketListener {
	private:
		void SetPose(bool roomToDriver, int idx, int enable, double x, double y, double z, double qx, double qy, double qz, double qw, double timeoffset, int jointMode = 0, const char* root_sn = nullptr);
		virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint);
	public:
		static void OSCReceiver::SendLog(int stat, string msg);
		static void OSCReceiver::SendAlive();
		static void OSCReceiver::SendHaptic(int index, float frequency, float amplitude, float duration);
	};

	class CommunicationManager {
	private:
		const int frameCycle = 45;
		int m_frame = 0;

		string m_installPath = "";

		bool m_opened = false;
		OSCReceiver m_rcv;
		ServerTrackedDeviceProvider* m_server;
		Eigen::Matrix4d m_RoomToDriverMatrix = Eigen::Matrix4d::Identity();
	public:
		static CommunicationManager* GetInstance();
		ServerTrackedDeviceProvider* GetServer();
		Eigen::Matrix4d& GetRoomToDriverMatrix();
		string GetInstallPath();
		void SetInstallPath(string);

		void Open(ServerTrackedDeviceProvider* server);
		void Close();
		void Process();
		void LoadSetting();
	};
}