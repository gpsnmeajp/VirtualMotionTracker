#include "CommunicationManager.h"
namespace VMTDriver {
	CommunicationManager* CommunicationManager::GetInstance()
	{
		static CommunicationManager cm;
		return &cm;
	}
	void CommunicationManager::Open()
	{
		m_sm = new SharedMemory::SharedMemory();
		if (!m_sm->open()) {
			//オープンに失敗
			delete m_sm;
			m_sm = nullptr;
		}
	}
	void CommunicationManager::Close()
	{
		m_sm->close();
		delete m_sm;
		m_sm = nullptr;
	}
	void CommunicationManager::Process()
	{
		//通信の準備ができていない
		if (m_sm == nullptr) {
			return;
		}

		string r = m_sm->readM2D();
		if (!r.empty()) {
			json j = json::parse(r);
			string type = j["type"];

			printf("%s\n", type.c_str());
		}

		json jw;

		jw["type"] = "Hello";
		jw["json"] = json{ {"msg","Hello from cpp"} }.dump();
		m_sm->writeD2M(jw.dump());
	}
}