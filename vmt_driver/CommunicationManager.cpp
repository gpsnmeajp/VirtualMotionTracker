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
#include "CommunicationManager.h"
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"
#include "DirectOSC.h"

namespace VMTDriver {
	//別スレッド
	void OSCReceiver::SetPose(bool roomToDriver,int idx, int enable, double x, double y, double z, double qx, double qy, double qz, double qw, double timeoffset)
	{
		DriverPose_t pose{ 0 };
		pose.poseTimeOffset = timeoffset;
		pose.qRotation = VMTDriver::HmdQuaternion_Identity;
		pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
		pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;

		if (enable) {
			pose.deviceIsConnected = true;
			pose.poseIsValid = true;
			pose.result = TrackingResult_Running_OK;
		}
		else {
			pose.deviceIsConnected = false;
			pose.poseIsValid = false;
			pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
		}

		Eigen::Affine3d RoomToDriverAffin;
		RoomToDriverAffin = CommunicationManager::GetInstance()->GetRoomToDriverMatrix();

		//ワールド・ドライバ変換行列を設定
		Eigen::Translation3d pos(RoomToDriverAffin.translation());
		Eigen::Quaterniond rot(RoomToDriverAffin.rotation());
		pose.vecWorldFromDriverTranslation[0] = pos.x();
		pose.vecWorldFromDriverTranslation[1] = pos.y();
		pose.vecWorldFromDriverTranslation[2] = pos.z();
		pose.qWorldFromDriverRotation.x = rot.x();
		pose.qWorldFromDriverRotation.y = rot.y();
		pose.qWorldFromDriverRotation.z = rot.z();
		pose.qWorldFromDriverRotation.w = rot.w();

		//座標を設定
		pose.vecPosition[0] = x;
		pose.vecPosition[1] = y;
		pose.vecPosition[2] = z;
		pose.qRotation.x = qx;
		pose.qRotation.y = qy;
		pose.qRotation.z = qz;
		pose.qRotation.w = qw;


		ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
		if (idx >= 0 && idx <= sever->GetDevices().size())
		{
			sever->GetDevices()[idx].RegisterToVRSystem();
			sever->GetDevices()[idx].SetPose(pose);
		}
	}

	//別スレッド
	void OSCReceiver::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint)
	{
		try {
			string adr = m.AddressPattern();
			if (adr == "/TrackerPoseRoomUnity")
			{
				int idx, enable;
				float x, y, z, qx, qy, qz, qw, timeoffset;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> x >> y >> z >> qx >> qy >> qz >> qw >> timeoffset >> osc::EndMessage;

				SetPose(true, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset);
			}
			else if (adr == "/TrackerPoseRoomDriver")
			{
				int idx, enable;
				float x, y, z, qx, qy, qz, qw, timeoffset;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> x >> y >> z >> qx >> qy >> qz >> qw >> timeoffset >> osc::EndMessage;

				SetPose(true, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset);
			}
			else if (adr == "/TrackerPoseRawUnity")
			{
				int idx, enable;
				float x, y, z, qx, qy, qz, qw, timeoffset;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> x >> y >> z >> qx >> qy >> qz >> qw >> timeoffset >> osc::EndMessage;

				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset);
			}
			else if (adr == "/TrackerPoseRawDriver")
			{
				int idx, enable;
				float x, y, z, qx, qy, qz, qw, timeoffset;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> x >> y >> z >> qx >> qy >> qz >> qw >> timeoffset >> osc::EndMessage;

				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset);
			}
			else if (adr == "/Reset")
			{
				//全トラッカーを0にする
				ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
				for (int i = 0; i < sever->GetDevices().size(); i++)
				{
					DriverPose_t pose{ 0 };
					pose.qRotation = VMTDriver::HmdQuaternion_Identity;
					pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
					pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;
					pose.deviceIsConnected = false;
					pose.poseIsValid = false;
					pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
					sever->GetDevices()[i].SetPose(pose); //すでにVRシステムに登録済みのものだけ通知される
				}
			}
			else if (adr == "/LoadSetting")
			{
				CommunicationManager::GetInstance()->LoadSetting();
			}
			else if (adr == "/SetRoomMatrix")
			{
				float m1, m2, m3, m4, 
					m5, m6, m7, m8, 
					m9, m10, m11, m12;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> m1 >> m2 >> m3 >> m4 >> m5 >> m6 >> m7 >> m8 >> m9 >> m10 >> m11 >> m12 >> osc::EndMessage;

				CommunicationManager::GetInstance()->GetRoomToDriverMatrix()
					<< m1, m2, m3, m4
					, m5, m6, m7, m8
					, m9, m10, m11, m12
					, 0, 0, 0, 1;

				json j = CommunicationManager::GetInstance()->GetServer()->LoadJson();
				if (!j.contains("RoomMatrix"))
				{
					j["RoomMatrix"] = {};
				}
				j["RoomMatrix"] = {m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12};
				CommunicationManager::GetInstance()->GetServer()->SaveJson(j);
			}
			else {
				Log::printf("Unkown: %s", adr.c_str());
			}
		}
		catch (osc::Exception& e)
		{
			Log::printf("Exp: %s\n", e.what());
		}
	}
	/*
	void sendXYZ(DirectOSC::OSC* osc, float x, float y, float z) {
		const size_t bufsize = 1024;
		char buf[bufsize];
		osc::OutboundPacketStream packet(buf, bufsize);
		packet << osc::BeginMessage("/test1")
			<< x
			<< y
			<< z
			<< osc::EndMessage;
		osc->GetSocketTx()->Send(packet.Data(), packet.Size());
	}
	*/
}
namespace VMTDriver {

	CommunicationManager* CommunicationManager::GetInstance()
	{
		static CommunicationManager cm;
		return &cm;
	}

	ServerTrackedDeviceProvider* CommunicationManager::GetServer()
	{
		return m_server;
	}

	Eigen::Matrix4d& CommunicationManager::GetRoomToDriverMatrix()
	{
		return m_RoomToDriverMatrix;
	}

	void CommunicationManager::Open(ServerTrackedDeviceProvider* server)
	{
		if (m_opened) {
			return;
		}
		m_server = server;
		DirectOSC::OSC::GetInstance()->Open(&m_rcv, 39570, 39571);
		m_opened = true;

		LoadSetting();
		/*
		m_RoomToDriverMatrix << -0.9998478, 0,          -0.01745246,     0,
                     			0,          1,           0,              0,
			                    0.01745246, 0,          -0.9998478,      0,
			                    0.5270745, -2.244383,   -0.778713,       1;
								*/
		/*
		m_RoomToDriverMatrix << -0.9998478, 0, 0.01745246, 0.5270745,
			0, 1, 0, -2.244383,
			-0.01745246, 0, -0.9998478, -0.778713,
			0, 0, 0, 1;
			*/
	}
	void CommunicationManager::Close()
	{
		DirectOSC::OSC::GetInstance()->Close();
		m_opened = false;
	}
	void CommunicationManager::Process()
	{
		//特に処理がない
	}
	void CommunicationManager::LoadSetting()
	{
		try {
			json j = CommunicationManager::GetInstance()->GetServer()->LoadJson();
			if (j.contains("RoomMatrix"))
			{
				m_RoomToDriverMatrix
					<< j["RoomMatrix"][0], j["RoomMatrix"][1], j["RoomMatrix"][2], j["RoomMatrix"][3]
					, j["RoomMatrix"][4], j["RoomMatrix"][5], j["RoomMatrix"][6], j["RoomMatrix"][7]
					, j["RoomMatrix"][8], j["RoomMatrix"][9], j["RoomMatrix"][10], j["RoomMatrix"][11]
					, 0, 0, 0, 1;
			}
		}
		catch (...) {
			m_RoomToDriverMatrix = Eigen::Matrix4d::Identity();
		}
	}
}