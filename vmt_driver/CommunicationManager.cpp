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

	//姿勢を各仮想デバイスに設定する
	void OSCReceiver::SetPose(bool roomToDriver, int idx, int enable,
	                          double x, double y, double z,
	                          double qx, double qy, double qz, double qw,
	                          double timeoffset,
	                          const char* root_sn, ReferMode_t mode)
	{
		RawPose pose{};
		pose.roomToDriver = roomToDriver;
		pose.idx = idx;
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
		if (GetServer()->IsVMTDeviceIndex(idx))
		{
			GetServer()->GetDevice(idx).RegisterToVRSystem(enable); //1=Tracker, 2=Controller Left, 3=Controller Right, 4=Tracking Reference, 5=HMD
			GetServer()->GetDevice(idx).SetRawPose(pose);
		}
	}

	//ログ情報を送信する(ダイアログを表示する)
	void OSCReceiver::SendLog(int stat, string msg) {
		const size_t bufsize = 8192;
		char buf[bufsize]{};
		osc::OutboundPacketStream packet(buf, bufsize);
		packet << osc::BeginMessage("/VMT/Out/Log")
			<< stat
			<< msg.c_str()
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance()->GetSocketTx()->Send(packet.Data(), packet.Size());
	}

	//生存信号を送信する
	void OSCReceiver::SendAlive() {
		const size_t bufsize = 8192;
		char buf[bufsize]{};
		osc::OutboundPacketStream packet(buf, bufsize);
		packet << osc::BeginMessage("/VMT/Out/Alive")
			<< Version.c_str()
			<< GetServer()->GetInstallPath().c_str()
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance()->GetSocketTx()->Send(packet.Data(), packet.Size());
	}

	//振動情報を送信する
	void OSCReceiver::SendHaptic(int index, float frequency, float amplitude, float duration)
	{
		const size_t bufsize = 8192;
		char buf[bufsize]{};
		osc::OutboundPacketStream packet(buf, bufsize);
		packet << osc::BeginMessage("/VMT/Out/Haptic")
			<< index
			<< frequency
			<< amplitude
			<< duration
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance()->GetSocketTx()->Send(packet.Data(), packet.Size());
	}

	//エラー情報を送信する
	void OSCReceiver::SendUnavailable(int code, std::string reason) {
		const size_t bufsize = 8192;
		char buf[bufsize]{};
		osc::OutboundPacketStream packet(buf, bufsize);
		packet << osc::BeginMessage("/VMT/Out/Unavailable")
			<< code
			<< reason.c_str()
			<< osc::EndMessage;
		DirectOSC::OSC::GetInstance()->GetSocketTx()->Send(packet.Data(), packet.Size());
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
		int ButtonIndex{};
		int ButtonValue{};
		float TriggerValue{};
		const char* root_sn = nullptr;

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

		try {
			string adr = m.AddressPattern();
			osc::ReceivedMessageArgumentStream args = m.ArgumentStream();

			//姿勢情報の受信
			if (adr == "/VMT/Room/Unity")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				SetPose(true, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset);
			}
			else if (adr == "/VMT/Room/Driver")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				SetPose(true, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset);
			}
			else if (adr == "/VMT/Raw/Unity")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset);
			}
			else if (adr == "/VMT/Raw/Driver")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;
				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset);
			}
			else if (adr == "/VMT/Joint/Unity")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;
				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset, root_sn, ReferMode_t::Joint);
			}
			else if (adr == "/VMT/Joint/Driver")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;
				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset, root_sn, ReferMode_t::Joint);
			}
			else if (adr == "/VMT/Follow/Unity")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;
				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw, timeoffset, root_sn, ReferMode_t::Follow);
			}
			else if (adr == "/VMT/Follow/Driver")
			{
				args >> idx >> enable >> timeoffset >> x >> y >> z >> qx >> qy >> qz >> qw >> root_sn >> osc::EndMessage;
				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw, timeoffset, root_sn, ReferMode_t::Follow);
			}
			//デバイス入力系の受信
			else if (adr == "/VMT/Input/Button")
			{
				args >> idx >> ButtonIndex >> timeoffset >> ButtonValue >> osc::EndMessage;
				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					GetServer()->GetDevice(idx).UpdateButtonInput(ButtonIndex, ButtonValue != 0, timeoffset);
				}
			}
			else if (adr == "/VMT/Input/Trigger")
			{
				args >> idx >> ButtonIndex >> timeoffset >> TriggerValue >> osc::EndMessage;
				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					GetServer()->GetDevice(idx).UpdateTriggerInput(ButtonIndex, TriggerValue, timeoffset);
				}
			}
			else if (adr == "/VMT/Input/Joystick")
			{
				args >> idx >> ButtonIndex >> timeoffset >> x >> y >> osc::EndMessage;
				if (GetServer()->IsVMTDeviceIndex(idx))
				{
					GetServer()->GetDevice(idx).UpdateJoystickInput(ButtonIndex, x, y, timeoffset);
				}
			}
			//すべてのデバイスのリセット
			else if (adr == "/VMT/Reset")
			{
				GetServer()->DeviceResetAll();
			}
			//設定の読み込み
			else if (adr == "/VMT/LoadSetting")
			{
				Config::GetInstance()->LoadSetting();
				SendLog(0, "Setting Loaded");
			}
			//ルーム変換行列の設定
			else if (adr == "/VMT/SetRoomMatrix")
			{
				args >> m1 >> m2 >> m3 >> m4 >> m5 >> m6 >> m7 >> m8 >> m9 >> m10 >> m11 >> m12 >> osc::EndMessage;
				Config::GetInstance()->SetRoomMatrix(true, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
				SendLog(0, "Set Room Matrix Done.");
			}
			else if (adr == "/VMT/SetRoomMatrix/Temporary")
			{
				args >> m1 >> m2 >> m3 >> m4 >> m5 >> m6 >> m7 >> m8 >> m9 >> m10 >> m11 >> m12 >> osc::EndMessage;
				Config::GetInstance()->SetRoomMatrix(false, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
				SendLog(0, "Set Room Matrix Done.(Temporary)");
			}
			//自動姿勢更新の設定
			else if (adr == "/VMT/SetAutoPoseUpdate")
			{
				args >> enable >> osc::EndMessage;
				TrackedDeviceServerDriver::SetAutoUpdate(enable != 0);
			}
			//不明なパケット
			else {
				Log::printf("Unkown: %s", adr.c_str());
			}
		}
		catch (osc::Exception& e)
		{
			Log::printf("Exp: %s\n", e.what());
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

		Config* config = Config::GetInstance();
		//起動時設定読み込み
		config->LoadSetting();
		TrackedDeviceServerDriver::SetAutoUpdate(config->GetDefaultAutoPoseUpdateOn());

		//通信ポートオープン
		DirectOSC::OSC::GetInstance()->Open(&m_rcv, config->GetReceivePort(), config->GetSendPort());
		m_opened = true;
	}
	//通信のクローズ
	void CommunicationManager::Close()
	{
		DirectOSC::OSC::GetInstance()->Close();
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
			m_frame = 0;
		}
		m_frame++;
	}
}