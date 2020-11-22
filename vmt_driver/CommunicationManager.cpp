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
	//�ʃX���b�h
	void OSCReceiver::SetPose(bool roomToDriver,int idx, int enable, double x, double y, double z, double qx, double qy, double qz, double qw, double timeoffset, int root_sn)
	{
		DriverPose_t pose{ 0 };
		pose.poseTimeOffset = timeoffset;
		pose.qRotation = VMTDriver::HmdQuaternion_Identity;
		pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
		pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;

		if (enable != 0) {
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

		//���W��ݒ�
		pose.vecPosition[0] = x;
		pose.vecPosition[1] = y;
		pose.vecPosition[2] = z;
		pose.qRotation.x = qx;
		pose.qRotation.y = qy;
		pose.qRotation.z = qz;
		pose.qRotation.w = qw;

		if (root_sn == 0) {
			//���[���h�E�h���C�o�ϊ��s���ݒ�
			Eigen::Translation3d pos(RoomToDriverAffin.translation());
			Eigen::Quaterniond rot(RoomToDriverAffin.rotation());
			pose.vecWorldFromDriverTranslation[0] = pos.x();
			pose.vecWorldFromDriverTranslation[1] = pos.y();
			pose.vecWorldFromDriverTranslation[2] = pos.z();
			pose.qWorldFromDriverRotation.x = rot.x();
			pose.qWorldFromDriverRotation.y = rot.y();
			pose.qWorldFromDriverRotation.z = rot.z();
			pose.qWorldFromDriverRotation.w = rot.w();


			ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
			if (idx >= 0 && idx <= sever->GetDevices().size())
			{
				sever->GetDevices()[idx].RegisterToVRSystem(enable); //1=Tracker, 2=controller
				sever->GetDevices()[idx].SetPose(pose);
			}
		}
		else {
			// �����̃g���b�L���O�f�o�C�X�̍��W�n�łԂ牺����

			// �Ԃ牺���錳��Pose���擾
			vr::TrackedDevicePose_t poses[64];
			IVRServerDriverHost* host = VRServerDriverHost();
			host->GetRawTrackedDevicePoses(0.0f, poses, 64);
			IVRProperties* props = VRPropertiesRaw();
			CVRPropertyHelpers* helper = VRProperties();

			// �f�o�C�X����肷��V���A���i���o�[
			char sn[16];
			sprintf(sn, "LHR-%8x", root_sn);


			bool deviceFound = false;

			for (int i = 0; i < 64; i++) {
				TrackedDevicePose_t* p = poses + i;
				if (!p->bDeviceIsConnected) break;

				PropertyContainerHandle_t h = props->TrackedDeviceToPropertyContainer(i);
				string SerialNumber = helper->GetStringProperty(h, ETrackedDeviceProperty::Prop_SerialNumber_String);

				if (SerialNumber.compare(sn) != 0) continue;

				pose.result = p->eTrackingResult;

				if (p->eTrackingResult == ETrackingResult::TrackingResult_Running_OK) {
					float* m = (float*)p->mDeviceToAbsoluteTracking.m;

					Eigen::Affine3d mm;
					mm.matrix() << m[0 * 4 + 0], m[0 * 4 + 1], m[0 * 4 + 2], m[0 * 4 + 3],
						m[1 * 4 + 0], m[1 * 4 + 1], m[1 * 4 + 2], m[1 * 4 + 3],
						m[2 * 4 + 0], m[2 * 4 + 1], m[2 * 4 + 2], m[2 * 4 + 3],
						0.0, 0.0, 0.0, 1.0;

					Eigen::Translation3d pos(mm.translation());
					Eigen::Quaterniond rot(mm.rotation());
					pose.vecWorldFromDriverTranslation[0] = pos.x();
					pose.vecWorldFromDriverTranslation[1] = pos.y();
					pose.vecWorldFromDriverTranslation[2] = pos.z();
					pose.qWorldFromDriverRotation.x = rot.x();
					pose.qWorldFromDriverRotation.y = rot.y();
					pose.qWorldFromDriverRotation.z = rot.z();
					pose.qWorldFromDriverRotation.w = rot.w();

					deviceFound = true;
				}

				break;
			}

			if (!deviceFound) {
				pose.vecWorldFromDriverTranslation[0] = 0.0;
				pose.vecWorldFromDriverTranslation[1] = 0.0;
				pose.vecWorldFromDriverTranslation[2] = 0.0;
				pose.qWorldFromDriverRotation.x = 0.0;
				pose.qWorldFromDriverRotation.y = 0.0;
				pose.qWorldFromDriverRotation.z = 0.0;
				pose.qWorldFromDriverRotation.w = 1.0;
			}

			ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
			if (idx >= 0 && idx <= sever->GetDevices().size())
			{
				sever->GetDevices()[idx].RegisterToVRSystem(enable); //1=Tracker, 2=controller
				sever->GetDevices()[idx].SetPose(pose);
			}
		}
	}

	void OSCReceiver::SendLog(int stat, string msg) {
		const size_t bufsize = 8192;
		char buf[bufsize];
		osc::OutboundPacketStream packet(buf, bufsize);
		packet << osc::BeginMessage("/VMT/Out/Log")
			<< stat
			<< msg.c_str()
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance()->GetSocketTx()->Send(packet.Data(), packet.Size());
	}

	void OSCReceiver::SendAlive() {
		const size_t bufsize = 8192;
		char buf[bufsize];
		osc::OutboundPacketStream packet(buf, bufsize);
		packet << osc::BeginMessage("/VMT/Out/Alive")
			<< Version.c_str()
			<< CommunicationManager::GetInstance()->GetInstallPath().c_str()
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance()->GetSocketTx()->Send(packet.Data(), packet.Size());
	}

	void OSCReceiver::SendHaptic(int index, float frequency, float amplitude, float duration)
	{
		const size_t bufsize = 8192;
		char buf[bufsize];
		osc::OutboundPacketStream packet(buf, bufsize);
		packet << osc::BeginMessage("/VMT/Out/Haptic")
			<< index
			<< frequency
			<< amplitude
			<< duration
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance()->GetSocketTx()->Send(packet.Data(), packet.Size());
	}


	//�ʃX���b�h
	void OSCReceiver::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint)
	{
		try {
			string adr = m.AddressPattern();
			if (adr == "/VMT/Room/Unity")
			{
				int idx, enable;
				float timeoffset;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;

				SetPose(true, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset);
			}
			else if (adr == "/VMT/Room/Driver")
			{
				int idx, enable;
				float timeoffset;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;

				SetPose(true, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset);
			}
			else if (adr == "/VMT/Raw/Unity")
			{
				int idx, enable;
				float timeoffset;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;

				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset);
			}
			else if (adr == "/VMT/Raw/Driver")
			{
				int idx, enable;
				float timeoffset;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;

				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset);
			}
			else if (adr == "/VMT/Joint/Unity")
			{
				int root_sn, idx, enable;
				float timeoffset;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;

				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset, root_sn);
			}
			else if (adr == "/VMT/Joint/Driver")
			{
				int root_sn, idx, enable;
				float timeoffset;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;

				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset, root_sn);
			}
			else if (adr == "/VMT/Input/Button")
			{
				int idx, ButtonIndex;
				float timeoffset;
				int value;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> ButtonIndex >> timeoffset >> value >> osc::EndMessage;

				ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
				if (idx >= 0 && idx <= sever->GetDevices().size())
				{
					sever->GetDevices()[idx].UpdateButtonInput(ButtonIndex, value != 0, timeoffset);
				}
			}
			else if (adr == "/VMT/Input/Trigger")
			{
				int idx, ButtonIndex;
				float timeoffset;
				float value;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> ButtonIndex >> timeoffset >> value >> osc::EndMessage;

				ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
				if (idx >= 0 && idx <= sever->GetDevices().size())
				{
					sever->GetDevices()[idx].UpdateTriggerInput(ButtonIndex, value, timeoffset);
				}
			}
			else if (adr == "/VMT/Input/Joystick")
			{
				int idx, ButtonIndex;
				float timeoffset;
				float x,y;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> ButtonIndex >> timeoffset >> x >> y >> osc::EndMessage;

				ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
				if (idx >= 0 && idx <= sever->GetDevices().size())
				{
					sever->GetDevices()[idx].UpdateJoystickInput(ButtonIndex, x, y, timeoffset);
				}
			}
			else if (adr == "/VMT/Reset")
			{
				//�S�g���b�J�[��0�ɂ���
				ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
				for (int i = 0; i < sever->GetDevices().size(); i++)
				{
					sever->GetDevices()[i].Reset(); //���ł�VR�V�X�e���ɓo�^�ς݂̂��̂����ʒm�����
				}
			}
			else if (adr == "/VMT/LoadSetting")
			{
				CommunicationManager::GetInstance()->LoadSetting();
				SendLog(0, "Setting Loaded");
			}
			else if (adr == "/VMT/SetRoomMatrix")
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
				j["RoomMatrix"] = { m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12 };
				CommunicationManager::GetInstance()->GetServer()->SaveJson(j);

				SendLog(0, "Set Room Matrix Done.");
			}
			else if (adr == "/VMT/SetRoomMatrix/Temporary")
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

				SendLog(0, "Set Room Matrix Done.(Temporary)");
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

	string CommunicationManager::GetInstallPath()
	{
		return m_installPath;
	}

	void CommunicationManager::SetInstallPath(string path)
	{
		m_installPath = path;
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
		//����I�ɐ����M���𑗐M
		if (m_frame > frameCycle) {
			OSCReceiver::SendAlive();
			m_frame = 0;
		}
		m_frame++;
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