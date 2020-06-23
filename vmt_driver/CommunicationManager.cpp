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

	//送受信スレッド
	std::mutex CommunicationWorkerMutex;
	std::deque<string> CommunicationWorkerReadQue;
	std::deque<string> CommunicationWorkerWriteQue;
	bool CommunicationWorkerExit = false;
	std::thread* CommunicationWorkerThread;

	class OSCReceiver : public osc::OscPacketListener {
		virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint) override
		{
			try {
				string adr = m.AddressPattern();
				if (adr == "/test1")
				{
					float x;
					float y;
					float z;
					osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
					args >> x >> y >> z >> osc::EndMessage;

					printf("X:%lf, Y:%lf, Z:%lf\n", x, y, z);
				}
			}
			catch (osc::Exception& e)
			{
				printf("Exp: %s\n", e.what());
			}
		}
	};

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

	void CommunicationWorker()
	{
		while (!CommunicationWorkerExit)
		{
			//クリティカルセクション
			{
				std::lock_guard<std::mutex> lock(CommunicationWorkerMutex);

				//受信データがあったらひたすら読み込む
				string r = CommunicationManager::GetInstance()->GetSM()->readM2D();
				//受信データが有る場合(かつリミッター以下の場合)
				if (r != "" && CommunicationWorkerReadQue.size() < 1024) {
					CommunicationWorkerReadQue.push_back(r);
					//空になるまで待ち時間最短で読み込む
					continue;
				}
				//送信データがあったらひたすら書き込む
				if (!CommunicationWorkerWriteQue.empty())
				{
					if (CommunicationManager::GetInstance()->GetSM()->writeD2M(CommunicationWorkerWriteQue.front()) == true)
					{
						//成功したら引き抜いて続ける
						CommunicationWorkerWriteQue.pop_front();
						continue;
					}
					//書き込み失敗時は次の周期まで待つ(相手が起動していないか、相手がいっぱいいっぱい)
				}
			}
			//ノンクリティカルセクション
			Sleep(4); //240fps
		}
	}

	string CommunicationRead() {
		string r = "";
		//クリティカルセクション
		{
			std::lock_guard<std::mutex> lock(CommunicationWorkerMutex);
			if (!CommunicationWorkerReadQue.empty())
			{
				//受信キューから取り出し
				r = CommunicationWorkerReadQue.front();
				CommunicationWorkerReadQue.pop_front();
			}
		}
		return r;
	}
	void CommunicationWrite(string s) {
		std::lock_guard<std::mutex> lock(CommunicationWorkerMutex);
		if (CommunicationWorkerWriteQue.size() < 1024) { //1024件以上は捨てる(異常時、通信不良時)
			CommunicationWorkerWriteQue.push_back(s);
		}
	}
}

namespace VMTDriver {

	CommunicationManager* CommunicationManager::GetInstance()
	{
		static CommunicationManager cm;
		return &cm;
	}
	SharedMemory::SharedMemory* CommunicationManager::GetSM()
	{
		return m_sm;
	}
	void CommunicationManager::Open()
	{
		if (m_opened) {
			return;
		}
		m_sm = new SharedMemory::SharedMemory();
		if (!m_sm->open()) {
			//オープンに失敗
			delete m_sm;
			m_sm = nullptr;
			return;
		}
		
		CommunicationWorkerThread = new std::thread(CommunicationWorker);
		m_opened = true;
	}
	void CommunicationManager::Close()
	{
		CommunicationWorkerExit = true;
		CommunicationWorkerThread->join();
		m_sm->close();
		delete m_sm;
		m_sm = nullptr;
		m_opened = false;
	}
	void CommunicationManager::Process(ServerTrackedDeviceProvider* server)
	{
		//通信の準備ができていない
		if (m_sm == nullptr) {
			return;
		}
		try {
			//受信(バッファが溜まっている場合高速に処理する)
			do {
				string r = CommunicationRead();
				if (r.empty()) {
					break;
				}
				//Log::printf("CommunicationWorkerReadQue:%d", CommunicationWorkerReadQue.size());

				json j = json::parse(r);
				string type = j["type"];
				if (type == "Pos") {
					string j2s = j["json"];
					json j2 = json::parse(j2s);


					DriverPose_t pose{ 0 };

					pose.qRotation = VMTDriver::HmdQuaternion_Identity;
					pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
					pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;

					int idx = j2["idx"];
					if (j2["en"]) {
						pose.deviceIsConnected = true;
						pose.poseIsValid = true;
						pose.result = TrackingResult_Running_OK;
					}
					else {
						pose.deviceIsConnected = false;
						pose.poseIsValid = false;
						pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
					}
					Eigen::Translation3d pos((double)j2["x"], (double)j2["y"], (double)j2["z"]);
					Eigen::Quaterniond rot((double)j2["qw"],(double)j2["qx"], (double)j2["qy"], (double)j2["qz"]);
					Eigen::Affine3d affin(pos * rot);
					//{ {M11:-0.9998478 M12:0 M13:-0.01745246 M14:0} 
					//  {M21:0 M22:1 M23:0 M24:0}
					//  {M31:0.01745246 M32:0 M33:-0.9998478 M34:0}
					//  {M41:0.5270745 M42:-2.244383 M43:-0.778713 M44:1} }

					Eigen::Matrix4d RoomToDriver;
					RoomToDriver << -0.9998478, 0, -0.01745246, 0,
						0, 1, 0, 0,
						0.01745246, 0, -0.9998478, 0,
						0.5270745, -2.244383, -0.778713, 1;
					Eigen::Affine3d RoomToDriverAffin;
					RoomToDriverAffin = RoomToDriver.transpose();

					Eigen::Affine3d outputAffin(RoomToDriverAffin * affin);
					Eigen::Translation3d outpos(outputAffin.translation());
					Eigen::Quaterniond outrot(outputAffin.rotation());

					Log::printf("Origin: %10lf,%10lf,%10lf\n", pos.x(), pos.y(), pos.z());
					Log::printf("Trans : %10lf,%10lf,%10lf\n", outpos.x(), outpos.y(), outpos.z());
					Log::printf("Origin: %10lf,%10lf,%10lf,%10lf\n", rot.x(),rot.y(),rot.z(),rot.w());
					Log::printf("Trans : %10lf,%10lf,%10lf,%10lf\n", outrot.x(), outrot.y(), outrot.z(), outrot.w());

					pose.vecPosition[0] = outpos.x();
					pose.vecPosition[1] = outpos.y();
					pose.vecPosition[2] = outpos.z();
					pose.qRotation.x = outrot.x();
					pose.qRotation.y = outrot.y();
					pose.qRotation.z = outrot.z();
					pose.qRotation.w = outrot.w();

					if (idx >= 0 && idx <= server->GetDevices().size())
					{
						server->GetDevices()[idx].RegisterToVRSystem();
						server->GetDevices()[idx].SetPose(pose);
					}
				}
				printf("%s\n", type.c_str());
			} while (true);

			//送信
			/*
			json jw;
			jw["type"] = "Hello";
			jw["json"] = json{ {"msg","Hello from cpp"} }.dump();
			CommunicationWrite(jw.dump());
			*/
		}
		catch (...) {
			m_sm->logError("Exception in CommunicationManager::Process");
		}
	}
}