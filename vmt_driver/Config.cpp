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
#include "Config.h"

namespace VMTDriver {
	Config::Config()
	{
	}
	Config* VMTDriver::Config::GetInstance()
	{
		static Config c;
		return &c;
	}

	void Config::InitSetting()
	{
		json j = VMTDriver::GetServer()->LoadJson();
		if (!j.contains("RoomMatrix"))
		{
			j["RoomMatrix"] = {};
		}
		if (!j.contains("VelocityEnable"))
		{
			j["VelocityEnable"] = false;
		}
		if (!j.contains("ReceivePort"))
		{
			j["ReceivePort"] = -1;
		}
		if (!j.contains("SendPort"))
		{
			j["SendPort"] = -1;
		}
		if (!j.contains("OptoutTrackingRole"))
		{
			j["OptoutTrackingRole"] = true;
		}
		if (!j.contains("HMDisIndex0"))
		{
			j["HMDisIndex0"] = true;
		}
		if (!j.contains("RejectWhenCannotTracking"))
		{
			j["RejectWhenCannotTracking"] = true;
		}
		if (!j.contains("DefaultAutoPoseUpdateOn"))
		{
			j["DefaultAutoPoseUpdateOn"] = true;
		}
		VMTDriver::GetServer()->SaveJson(j);
	}

	void Config::LoadSetting()
	{
		try {
			SetRoomMatrixStatus(false); //ルーム行列セット状態をクリア

			json j = VMTDriver::GetServer()->LoadJson();
			if (j.contains("RoomMatrix"))
			{
				m_RoomToDriverMatrix
					<< j["RoomMatrix"][0], j["RoomMatrix"][1], j["RoomMatrix"][2], j["RoomMatrix"][3]
					, j["RoomMatrix"][4], j["RoomMatrix"][5], j["RoomMatrix"][6], j["RoomMatrix"][7]
					, j["RoomMatrix"][8], j["RoomMatrix"][9], j["RoomMatrix"][10], j["RoomMatrix"][11]
					, 0, 0, 0, 1;
				SetRoomMatrixStatus(true); //ルーム行列がセットされた
			}
			if (j.contains("VelocityEnable"))
			{
				m_velocityEnable = j["VelocityEnable"];
			}
			if (j.contains("ReceivePort"))
			{
				if (j["ReceivePort"] > 0) {
					m_receivePort = j["ReceivePort"];
				}
			}
			if (j.contains("SendPort"))
			{
				if (j["SendPort"] > 0) {
					m_sendPort = j["SendPort"];
				}
			}
			if (j.contains("OptoutTrackingRole"))
			{
				m_optoutTrackingRole = j["OptoutTrackingRole"];
			}
			if (j.contains("HMDisIndex0"))
			{
				m_HMDisIndex0 = j["HMDisIndex0"];
			}
			if (j.contains("RejectWhenCannotTracking"))
			{
				m_RejectWhenCannotTracking = j["RejectWhenCannotTracking"];
			}
			if (j.contains("DefaultAutoPoseUpdateOn"))
			{
				m_DefaultAutoPoseUpdateOn = j["DefaultAutoPoseUpdateOn"];
			}
		}
		catch (...) {
			m_RoomToDriverMatrix = Eigen::Matrix4d::Identity();
		}
	}
	void Config::SetRoomMatrixStatus(bool ok)
	{
		m_RoomMatrixStatus = ok;
	}

	Eigen::Matrix4d& Config::GetRoomToDriverMatrix()
	{
		return m_RoomToDriverMatrix;
	}

	bool Config::GetVelocityEnable()
	{
		return m_velocityEnable;
	}
	int Config::GetReceivePort()
	{
		return m_receivePort;
	}
	int Config::GetSendPort()
	{
		return m_sendPort;
	}
	bool Config::GetOptoutTrackingRole()
	{
		return m_optoutTrackingRole;
	}
	bool Config::GetHMDisIndex0()
	{
		return m_HMDisIndex0;
	}
	bool Config::GetRoomMatrixStatus()
	{
		return m_RoomMatrixStatus;
	}
	bool Config::GetRejectWhenCannotTracking()
	{
		return m_RejectWhenCannotTracking;
	}
	bool Config::GetDefaultAutoPoseUpdateOn()
	{
		return m_DefaultAutoPoseUpdateOn;
	}
}