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
	//コンストラクタ
	Config::Config()
	{
	}
	//シングルトンインスタンスの取得
	Config* VMTDriver::Config::GetInstance()
	{
		static Config c;
		return &c;
	}

	//Jsonファイルの読み込み
	json Config::LoadJson()
	{
		//ドライバのインストールパス取得
		string installPath = GetServer()->GetInstallPath();

		if (!installPath.empty()) {
			string filename = (installPath + "\\setting.json");
			try {
				//テキストの読み込み
				std::ifstream inputStream(filename);
				string inputData((std::istreambuf_iterator<char>(inputStream)), std::istreambuf_iterator<char>());
				inputStream.close();

				//Jsonパース
				json j = json::parse(inputData);
				LogInfo("LoadJson: %s", j.dump().c_str());
				return ErrorCheck(j);
			}
			catch (...) {
				//パース失敗・ファイルなしなど
				LogError("LoadJson: Parse error or load faild");
				return ErrorCheck(json());
			}
		}

		//インストールパスが取得できない
		LogError("LoadJson: No Path");
		return ErrorCheck(json());
	}

	//Jsonファイルの読み込み
	void Config::SaveJson(json j)
	{
		//ドライバのインストールパス取得
		string installPath = GetServer()->GetInstallPath();
		json output = ErrorCheck(j);

		if (!installPath.empty()) {
			try {
				//テキストの書き込み
				string filename = (installPath + "\\setting.json");

				std::ofstream outputStream(filename);
				outputStream << output.dump(3, ' ');
				outputStream.close();

				LogInfo("SaveJson: %s", output.dump().c_str());
				return;
			}
			catch (...) {
				//書き込みエラーなど
				LogError("SaveJson: Save faild");
				return;
			}
		}

		//インストールパスが取得できない
		LogError("SaveJson: No Path");
		return;
	}

	//json内の不足する要素を初期値に設定する
	json Config::ErrorCheck(json j)
	{
		if (!j.contains("RoomMatrix"))
		{
			j["RoomMatrix"] = {};
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
		if (!j.contains("RejectWhenCannotTracking"))
		{
			j["RejectWhenCannotTracking"] = true;
		}
		if (!j.contains("DefaultAutoPoseUpdateOn"))
		{
			j["DefaultAutoPoseUpdateOn"] = true;
		}
		if (!j.contains("DefaultControllerDeviceRegistration"))
		{
			j["DefaultControllerDeviceRegistration"] = false;
		}
		if (!j.contains("SkeletonInput"))
		{
			j["SkeletonInput"] = true;
		}
		if (!j.contains("CompatibleMode"))
		{
			j["CompatibleMode"] = true;
		}
		return j;
	}

	//jsonからの設定の読み込み
	void Config::LoadSetting()
	{
		try {
			SetRoomMatrixStatus(false); //ルーム行列セット状態をクリア

			json j = LoadJson();
			if (j.contains("RoomMatrix"))
			{
				m_RoomToDriverMatrix
					<< j["RoomMatrix"][0], j["RoomMatrix"][1], j["RoomMatrix"][2], j["RoomMatrix"][3]
					, j["RoomMatrix"][4], j["RoomMatrix"][5], j["RoomMatrix"][6], j["RoomMatrix"][7]
					, j["RoomMatrix"][8], j["RoomMatrix"][9], j["RoomMatrix"][10], j["RoomMatrix"][11]
					, 0, 0, 0, 1;
				SetRoomMatrixStatus(true); //ルーム行列がセットされた
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
			if (j.contains("RejectWhenCannotTracking"))
			{
				m_RejectWhenCannotTracking = j["RejectWhenCannotTracking"];
			}
			if (j.contains("DefaultAutoPoseUpdateOn"))
			{
				m_DefaultAutoPoseUpdateOn = j["DefaultAutoPoseUpdateOn"];
			}
			if (j.contains("DefaultControllerDeviceRegistration"))
			{
				m_DefaultControllerDeviceRegistration = j["DefaultControllerDeviceRegistration"];
			}
			if (j.contains("SkeletonInput"))
			{
				m_SkeletonInput = j["SkeletonInput"];
			}
			if (j.contains("CompatibleMode"))
			{
				m_CompatibleMode = j["CompatibleMode"];
			}
			SaveJson(j);
		}
		catch (...) {
			m_RoomToDriverMatrix = Eigen::Matrix4d::Identity();
		}
	}

	//ルーム変換行列をjsonに保存する
	void Config::SaveJsonRoomToDriverMatrix(float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8, float m9, float m10, float m11, float m12)
	{
		json j = LoadJson();
		j["RoomMatrix"] = { m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12 };
		SaveJson(j);
	}

	//ルーム変換行列の設定状態を設定する
	void Config::SetRoomMatrixStatus(bool ok)
	{
		m_RoomMatrixStatus = ok;
	}

	//ルーム変換行列の取得
	Eigen::Matrix4d& Config::GetRoomToDriverMatrix()
	{
		return m_RoomToDriverMatrix;
	}

	//受信ポートの取得
	int Config::GetReceivePort()
	{
		return m_receivePort;
	}

	//送信ポートの取得
	int Config::GetSendPort()
	{
		return m_sendPort;
	}

	//ロールのオプトアウトの取得
	bool Config::GetOptoutTrackingRole()
	{
		return m_optoutTrackingRole;
	}

	//ルーム変換行列の状態の取得
	bool Config::GetRoomMatrixStatus()
	{
		return m_RoomMatrixStatus;
	}

	//ルーム変換行列の設定
	void Config::SetRoomMatrix(bool save, float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8, float m9, float m10, float m11, float m12)
	{
		//セットする
		m_RoomToDriverMatrix
			<< m1, m2, m3, m4
			, m5, m6, m7, m8
			, m9, m10, m11, m12
			, 0, 0, 0, 1;
		SetRoomMatrixStatus(true); //ルーム行列がセットされた

		//保存するなら保存する
		if (save) {
			SaveJsonRoomToDriverMatrix(m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
		}
	}

	//エラー時にトラッキングを停止するかを取得する
	bool Config::GetRejectWhenCannotTracking()
	{
		return m_RejectWhenCannotTracking;
	}

	//自動更新をデフォルトでオンにするかを取得する
	bool Config::GetDefaultAutoPoseUpdateOn()
	{
		return m_DefaultAutoPoseUpdateOn;
	}

	//起動直後にコントローラとして登録するかを取得する
	bool Config::GetDefaultControllerDeviceRegistration()
	{
		return m_DefaultControllerDeviceRegistration;
	}

	//骨格入力が有効かを取得する
	bool Config::GetSkeletonInput()
	{
		return m_SkeletonInput;
	}

	//Knuckles互換モードが有効かを取得する
	bool Config::GetCompatibleMode()
	{
		return m_CompatibleMode;
	}
}