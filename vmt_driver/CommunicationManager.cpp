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
	//別スレッドからのコール

	//Unityオイラー回転→クォータニオン
	Eigen::Quaterniond OSCReceiver::UnityEulerToQuaternion(double x, double y, double z)
	{
		return
			Eigen::AngleAxisd(y / 180.0 * M_PI, Eigen::Vector3d::UnitY())
			* Eigen::AngleAxisd(x / 180.0 * M_PI, Eigen::Vector3d::UnitX())
			* Eigen::AngleAxisd(z / 180.0 * M_PI, Eigen::Vector3d::UnitZ());
	}

	//姿勢を各仮想デバイスに設定する
	void OSCReceiver::SetPose(bool roomToDriver, int deviceIndex, int enable,
	                          double x, double y, double z,
	                          double qx, double qy, double qz, double qw,
	                          double timeoffset,
	                          const char* root_sn, ReferMode_t mode)
	{
		RawPose pose{};
		pose.roomToDriver = roomToDriver;
		pose.idx = deviceIndex;
		pose.enable = enable;
		pose.x = x;
		pose.y = y;
		pose.z = z;
		pose.qx = qx;
		pose.qy = qy;
		pose.qz = qz;
		pose.qw = qw;
		pose.timeoffset = timeoffset;
		pose.mode = mode;
		pose.root_sn = root_sn ? root_sn : "";
		pose.time = std::chrono::system_clock::now();

		//範囲チェック
		if (GetServer()->IsVMTDeviceIndex(deviceIndex))
		{
			GetServer()->GetDevice(deviceIndex).RegisterToVRSystem(enable); //1=Tracker, 2=Controller Left, 3=Controller Right, 4=Tracking Reference
			GetServer()->GetDevice(deviceIndex).SetRawPose(pose);
		}
	}

	//骨格をバッファに書き込み
	void OSCReceiver::WriteSkeletonBone(int deviceIndex, int boneIndex,
		float x, float y, float z,
		float qx, float qy, float qz, float qw)
	{
		VRBoneTransform_t bone{};
		bone.position.v[0] = x;
		bone.position.v[1] = y;
		bone.position.v[2] = z;
		bone.position.v[3] = 0;
		bone.orientation.x = qx;
		bone.orientation.y = qy;
		bone.orientation.z = qz;
		bone.orientation.w = qw;

		//LogInfo("%d %lf %lf %lf %lf %lf %lf %lf ", boneIndex, x, y, z, qx, qy, qz, qw);
		//範囲チェック
		if (GetServer()->IsVMTDeviceIndex(deviceIndex))
		{
			GetServer()->GetDevice(deviceIndex).WriteSkeletonInputBuffer(boneIndex, bone);
		}
	}

	//ログ情報を送信する(ダイアログを表示する)
	void OSCReceiver::SendLog(int stat, string msg) {
		const size_t bufsize = 8192;
		char buf[bufsize]{};
		osc::OutboundPacketStream packet(buf, bufsize);
		LogIfDiag("/VMT/Out/Log : %d : %s", stat, msg.c_str());
		packet << osc::BeginMessage("/VMT/Out/Log")
			<< stat
			<< msg.c_str()
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance().GetSocketTx().Send(packet.Data(), packet.Size());
	}

	//生存信号を送信する
	void OSCReceiver::SendAlive() {
		const size_t bufsize = 8192;
		char buf[bufsize]{};
		osc::OutboundPacketStream packet(buf, bufsize);
		LogIfDiag("/VMT/Out/Alive : %s : %s", Version.c_str(), GetServer()->GetInstallPath().c_str());
		packet << osc::BeginMessage("/VMT/Out/Alive")
			<< Version.c_str()
			<< GetServer()->GetInstallPath().c_str()
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance().GetSocketTx().Send(packet.Data(), packet.Size());
	}

	//振動情報を送信する
	void OSCReceiver::SendHaptic(int index, float frequency, float amplitude, float duration)
	{
		const size_t bufsize = 8192;
		char buf[bufsize]{};
		osc::OutboundPacketStream packet(buf, bufsize);
		LogIfDiag("/VMT/Out/Haptic : %d : %f %f %f", index, frequency, amplitude, duration);
		packet << osc::BeginMessage("/VMT/Out/Haptic")
			<< index
			<< frequency
			<< amplitude
			<< duration
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance().GetSocketTx().Send(packet.Data(), packet.Size());
	}

	//エラー情報を送信する
	void OSCReceiver::SendUnavailable(int code, std::string reason) {
		const size_t bufsize = 8192;
		char buf[bufsize]{};
		osc::OutboundPacketStream packet(buf, bufsize);
		LogIfDiag("/VMT/Out/Unavailable : %d : %s", code, reason.c_str());
		packet << osc::BeginMessage("/VMT/Out/Unavailable")
			<< code
			<< reason.c_str()
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance().GetSocketTx().Send(packet.Data(), packet.Size());
	}

	//受信処理
	void OSCReceiver::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint)
	{
		int idx{};
		int enable{};
		float timeoffset{};
		float x{};
		float y{};
		float z{};
		float qx{};
		float qy{};
		float qz{};
		float qw{};
		float rx{};
		float ry{};
		float rz{};
		int ButtonIndex{};
		int ButtonValue{};
		float TriggerValue{};
		const char* root_sn = nullptr;

		int i{};
		int mode{};

		float m1{};
		float m2{};
		float m3{};
		float m4{};
		float m5{};
		float m6{};
		float m7{};
		float m8{};
		float m9{};
		float m10{};
		float m11{};
		float m12{};
		const char* command = nullptr;

		try {
			string adr = m.AddressPattern();
			osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
			
			//姿勢情報の受信
			if (adr == "/VMT/Room/Unity")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f %f", adr.c_str(), idx, enable, timeoffset, x, y, z, qx, qy, qz, qw);
				SetPose(true, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset);
			}
			else if (adr == "/VMT/Room/UEuler")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> rx >> ry >> rz >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f", adr.c_str(), idx, enable, timeoffset, x, y, z, rx, ry, rz);

				Eigen::Quaterniond q = UnityEulerToQuaternion(rx, ry, rz);
				LogIfDiag("%lf %lf %lf %lf", q.x(), q.y(), q.z(), q.w());
				SetPose(true, idx, enable, x, y, -z, q.x(), q.y(), -q.z(), -q.w(), timeoffset);
			}
			else if (adr == "/VMT/Room/Driver")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f %f", adr.c_str(), idx, enable, timeoffset, x, y, z, qx, qy, qz, qw);
				SetPose(true, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset);
			}
			else if (adr == "/VMT/Raw/Unity")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f %f", adr.c_str(), idx, enable, timeoffset, x, y, z, qx, qy, qz, qw);
				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset);
			}
			else if (adr == "/VMT/Raw/UEuler")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> rx >> ry >> rz  >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f", adr.c_str(), idx, enable, timeoffset, x, y, z, rx, ry, rz);

				Eigen::Quaterniond q = UnityEulerToQuaternion(rx, ry, rz);
				LogIfDiag("%lf %lf %lf %lf", q.x(), q.y(), q.z(), q.w());
				SetPose(false, idx, enable, x, y, -z, q.x(), q.y(), -q.z(), -q.w(), timeoffset);
			}
			else if (adr == "/VMT/Raw/Driver")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f %f", adr.c_str(), idx, enable, timeoffset, x, y, z, qx, qy, qz, qw);
				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset);
			}
			else if (adr == "/VMT/Joint/Unity")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f %f : %s", adr.c_str(), idx, enable, timeoffset, x, y, z, qx, qy, qz, qw, root_sn);
				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset, root_sn, ReferMode_t::Joint);
			}
			else if (adr == "/VMT/Joint/UEuler")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> rx >> ry >> rz >> root_sn >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f : %s", adr.c_str(), idx, enable, timeoffset, x, y, z, rx, ry, rz, root_sn);

				Eigen::Quaterniond q = UnityEulerToQuaternion(rx, ry, rz);
				LogIfDiag("%lf %lf %lf %lf", q.x(), q.y(), q.z(), q.w());
				SetPose(false, idx, enable, x, y, -z, q.x(), q.y(), -q.z(), -q.w(), timeoffset, root_sn, ReferMode_t::Joint);
			}
			else if (adr == "/VMT/Joint/Driver")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f %f : %s", adr.c_str(), idx, enable, timeoffset, x, y, z, qx, qy, qz, qw, root_sn);
				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset, root_sn, ReferMode_t::Joint);
			}
			else if (adr == "/VMT/Follow/Unity")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f %f : %s", adr.c_str(), idx, enable, timeoffset, x, y, z, qx, qy, qz, qw, root_sn);
				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset, root_sn, ReferMode_t::Follow);
			}
			else if (adr == "/VMT/Follow/UEuler")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> rx >> ry >> rz >> root_sn >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f : %s", adr.c_str(), idx, enable, timeoffset, x, y, z, rx, ry, rz, root_sn);

				Eigen::Quaterniond q = UnityEulerToQuaternion(rx, ry, rz);
				LogIfDiag("%lf %lf %lf %lf", q.x(), q.y(), q.z(), q.w());
				SetPose(false, idx, enable, x, y, -z, q.x(), q.y(), -q.z(), -q.w(), timeoffset, root_sn, ReferMode_t::Follow);
			}
			else if (adr == "/VMT/Follow/Driver")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f %f : %f %f %f %f : %s", adr.c_str(), idx, enable, timeoffset, x, y, z, qx, qy, qz, qw, root_sn);
				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset, root_sn, ReferMode_t::Follow);
			}
			else if (adr == "/VMT/Skeleton/Unity")
			{
				args >> idx >> i >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f %f %f : %f %f %f %f", adr.c_str(), idx, i, x, y, z, qx, qy, qz, qw);
				WriteSkeletonBone(idx, i, x, y, -z, qx, qy, -qz, -qw);
			}
			else if (adr == "/VMT/Skeleton/UEuler")
			{
				args >> idx >> i >> x >> y >> z >> rx >> ry >> rz >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f %f %f : %f %f %f", adr.c_str(), idx, i, x, y, z, rx, ry, rz);

				Eigen::Quaterniond q = UnityEulerToQuaternion(rx, ry, rz);
				LogIfDiag("%lf %lf %lf %lf", q.x(), q.y(), q.z(), q.w());
				WriteSkeletonBone(idx, i, x, y, -z, (float)q.x(), (float)q.y(), (float)-q.z(), (float)-q.w());
			}
			else if (adr == "/VMT/Skeleton/Driver")
			{
				args >> idx >> i >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f %f %f : %f %f %f %f", adr.c_str(), idx, i, x, y, z, qx, qy, qz, qw);
				WriteSkeletonBone(idx, i, x, y, z, qx, qy, qz, qw);
			}
			else if (adr == "/VMT/Skeleton/Scalar")
			{
				//デバイスインデックス、ボーンインデックス、モード(ブレンド先)、軸連動有効可否
				args >> idx >> i >> x >> mode >> enable >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %d : %d", adr.c_str(), idx, i, x, mode, enable);
				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					GetServer()->GetDevice(idx).WriteSkeletonInputBufferStaticLerpFinger(i, x, mode);

					//軸連動が有効な場合
					if (enable > 0) {
						//TODO: 軸の連動処理をコールする
					}
				}
			}
			else if (adr == "/VMT/Skeleton/Apply")
			{
				args >> idx >> timeoffset >> osc::EndMessage;
				LogIfDiag("%s : %d : %f", adr.c_str(), idx, timeoffset);
				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					GetServer()->GetDevice(idx).UpdateSkeletonInput(timeoffset);
				}
			}
			//デバイス入力系の受信
			else if (adr == "/VMT/Input/Button")
			{
				args >> idx >> ButtonIndex >> timeoffset >> ButtonValue >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %d", adr.c_str(), idx, ButtonIndex, timeoffset, ButtonValue);
				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					GetServer()->GetDevice(idx).UpdateButtonInput(ButtonIndex, ButtonValue != 0, timeoffset);
				}
			}
			else if (adr == "/VMT/Input/Trigger")
			{
				args >> idx >> ButtonIndex >> timeoffset >> TriggerValue >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f", adr.c_str(), idx, ButtonIndex, timeoffset, TriggerValue);
				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					GetServer()->GetDevice(idx).UpdateTriggerInput(ButtonIndex, TriggerValue, timeoffset);
				}
			}
			else if (adr == "/VMT/Input/Joystick")
			{
				args >> idx >> ButtonIndex >> timeoffset >> x >> y >> osc::EndMessage;
				LogIfDiag("%s : %d : %d : %f : %f %f", adr.c_str(), idx, ButtonIndex, timeoffset, x, y);
				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					GetServer()->GetDevice(idx).UpdateJoystickInput(ButtonIndex, x, y, timeoffset);
				}
			}
			//すべてのデバイスのリセット
			else if (adr == "/VMT/Reset")
			{
				LogIfDiag("%s", adr.c_str());
				GetServer()->DeviceResetAll();
			}
			//設定の読み込み
			else if (adr == "/VMT/LoadSetting")
			{
				LogIfDiag("%s", adr.c_str());
				Config::GetInstance()->LoadSetting();
				SendLog(0, "Setting Loaded");
			}
			//ルーム変換行列の設定
			else if (adr == "/VMT/SetRoomMatrix")
			{
				args >> m1 >> m2 >> m3 >> m4 >> m5 >> m6 >> m7 >> m8 >> m9 >> m10 >> m11 >> m12 >> osc::EndMessage;
				LogIfDiag("%s : %f %f %f %f %f %f %f %f %f %f %f %f", adr.c_str(), m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
				Config::GetInstance()->SetRoomMatrix(true, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
				SendLog(0, "Set Room Matrix Done.");
			}
			else if (adr == "/VMT/SetRoomMatrix/Temporary")
			{
				args >> m1 >> m2 >> m3 >> m4 >> m5 >> m6 >> m7 >> m8 >> m9 >> m10 >> m11 >> m12 >> osc::EndMessage;
				LogIfDiag("%s : %f %f %f %f %f %f %f %f %f %f %f %f", adr.c_str(), m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
				Config::GetInstance()->SetRoomMatrix(false, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
				SendLog(0, "Set Room Matrix Done.(Temporary)");
			}
			//自動姿勢更新の設定
			else if (adr == "/VMT/SetAutoPoseUpdate")
			{
				args >> enable >> osc::EndMessage;
				LogIfDiag("%s : %d", adr.c_str(), enable);
				TrackedDeviceServerDriver::SetAutoUpdate(enable != 0);
			}
			//デバッグコマンド
			else if (adr == "/VMT/Debug")
			{
				args >> idx >> command >> osc::EndMessage;
				LogIfDiag("%s : %d : %s", adr.c_str(), idx, command);
				std::string report(command);
				report += std::string(" -> ");

				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					report += GetServer()->GetDevice(idx).VMTDebugCommand(command);
				}

				OSCReceiver::SendUnavailable(0, report.c_str());
			}
			//不明なパケット
			else {
				LogError("Unkown: %s", adr.c_str());
			}
		}
		catch (osc::Exception& e)
		{
			LogError("Exp: %s\n", e.what());
		}
	}
}
namespace VMTDriver {
	//通信クラスのコンストラクタ
	CommunicationManager::CommunicationManager()
	{
	}

	//通信クラスのシングルトンインスタンスの取得
	CommunicationManager* CommunicationManager::GetInstance()
	{
		static CommunicationManager cm;
		return &cm;
	}

	//通信のオープン
	void CommunicationManager::Open()
	{
		//すでにオープン済みなら何もしない
		if (m_opened) {
			return;
		}
		LogMarker();

		Config* config = Config::GetInstance();
		//起動時設定読み込み
		config->LoadSetting();
		TrackedDeviceServerDriver::SetAutoUpdate(config->GetDefaultAutoPoseUpdateOn());

		//通信ポートオープン
		DirectOSC::OSC::GetInstance().Open(&m_rcv, config->GetReceivePort(), config->GetSendPort());
		m_opened = true;
	}
	//通信のクローズ
	void CommunicationManager::Close()
	{
		LogMarker();

		DirectOSC::OSC::GetInstance().Close();
		m_opened = false;
	}
	//フレーム単位の処理
	void CommunicationManager::Process()
	{
		if (m_frame > frameCycle) {
			//定期的に生存信号を送信
			OSCReceiver::SendAlive();

			//エラー状態を送信
			if (!Config::GetInstance()->GetRoomMatrixStatus()) {
				OSCReceiver::SendUnavailable(1, "Room Matrix has not been set.");
			}
			else {
				OSCReceiver::SendUnavailable(0, "OK");
			}
			//ログカウンタをリセットする
			Log::s_logCounter = 0;

			m_frame = 0;
		}
		m_frame++;
	}
}