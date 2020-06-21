#include "CommunicationManager.h"
namespace VMTDriver {
	CommunicationManager* CommunicationManager::GetInstance()
	{
		static CommunicationManager cm;
		return &cm;
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
		m_opened = true;
	}
	void CommunicationManager::Close()
	{
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
			//受信
			string r = m_sm->readM2D();
			if (!r.empty()) {
				json j = json::parse(r);
				string type = j["type"];
				if (type == "Pos") {
					string j2s = j["json"];
					json j2 = json::parse(j2s);

					
					DriverPose_t pose{ 0 };
					pose.deviceIsConnected = true;
					pose.poseIsValid = true;
					pose.result = TrackingResult_Running_OK;

					pose.qRotation = VMTDriver::HmdQuaternion_Identity;
					pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
					pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;

					pose.vecPosition[0] = j2["x"];
					pose.vecPosition[1] = j2["y"];
					pose.vecPosition[2] = j2["z"];
					pose.qRotation.x = j2["qx"];
					pose.qRotation.y = j2["qy"];
					pose.qRotation.z = j2["qz"];
					pose.qRotation.w = j2["qw"];
					server->GetDevices()[0].SetPose(pose);
				}

				printf("%s\n", type.c_str());
			}

			json jw;

			jw["type"] = "Hello";
			jw["json"] = json{ {"msg","Hello from cpp"} }.dump();
			m_sm->writeD2M(jw.dump());
		}
		catch (...) {
			m_sm->logError("Exception in CommunicationManager::Process");
		}
	}
}