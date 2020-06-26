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
#include "ServerTrackedDeviceProvider.h"
namespace VMTDriver {
    //デバイスを管理する親
    //この下に子としてデバイスがぶら下がる

    vector<TrackedDeviceServerDriver>& ServerTrackedDeviceProvider::GetDevices()
    {
        return m_devices;
    }

    json ServerTrackedDeviceProvider::LoadJson()
    {
        if (m_pDriverContext == nullptr) {
            return json();
        }

        //ドライバのインストールパス取得
        string s = VRProperties()->GetStringProperty(m_pDriverContext->GetDriverHandle(), Prop_InstallPath_String);
        if (!s.empty()) {
            string filename = (s + "\\setting.json");
            try {
                std::ifstream inputStream(filename);
                string inputData((std::istreambuf_iterator<char>(inputStream)), std::istreambuf_iterator<char>());
                inputStream.close();

                json j = json::parse(inputData);
                Log::Output(("LoadJson:" + j.dump()).c_str());
                return j;
            }
            catch (...) {
                //パース失敗・ファイルなしなど
                Log::Output("LoadJson: Parse error or load faild");
                return json();
            }
        }
        Log::Output("LoadJson: No Path");
        return json();
    }

    void ServerTrackedDeviceProvider::SaveJson(json j)
    {
        if (m_pDriverContext == nullptr) {
            return;
        }

        //ドライバのインストールパス取得
        string s = VRProperties()->GetStringProperty(m_pDriverContext->GetDriverHandle(), Prop_InstallPath_String);
        if (!s.empty()) {
            string filename = (s + "\\setting.json");

            std::ofstream outputStream(filename);
            outputStream << j.dump();
            outputStream.close();

            Log::Output(("SaveJson:" + j.dump()).c_str());
            return;
        }
        Log::Output("SaveJson: No Path");
        return;
    }

    //初期化
    EVRInitError ServerTrackedDeviceProvider::Init(IVRDriverContext* pDriverContext)
    {
        VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext)
        m_pDriverContext = pDriverContext;
        Log::Open(VRDriverLog());
        CommunicationManager::GetInstance()->Open(this);

        //デバイスを準備
        m_devices.resize(58); //64デバイス

        //デバイスを初期化
        for (int i = 0; i < m_devices.size(); i++)
        {
            string name = "VMT_";
            name.append(std::to_string(i));

            m_devices[i].SetDeviceSerial(name);
            m_devices[i].SetObjectIndex(i);
            //m_devices[i].RegisterToVRSystem(); //登録は必要になったらやる
            m_devicesNum++;
        }

        Log::Output("Startup OK");
        return EVRInitError::VRInitError_None;
    }

    //終了
    void ServerTrackedDeviceProvider::Cleanup()
    {
        VR_CLEANUP_SERVER_DRIVER_CONTEXT()
        m_pDriverContext = nullptr;
        CommunicationManager::GetInstance()->Close();
        Log::Close();
    }

    //インターフェースバージョン(ライブラリに依存)
    const char* const* ServerTrackedDeviceProvider::GetInterfaceVersions()
    {
        return k_InterfaceVersions;
    }

    //毎フレーム処理
    void ServerTrackedDeviceProvider::RunFrame()
    {
        //通信処理をする
        CommunicationManager::GetInstance()->Process();
        for (int i = 0; i < m_devicesNum; i++)
        {
            m_devices[i].UpdatePoseToVRSystem();
        }
    }

    //スタンバイをブロックするか
    bool ServerTrackedDeviceProvider::ShouldBlockStandbyMode()
    {
        return false;
    }

    //スタンバイに入った
    void ServerTrackedDeviceProvider::EnterStandby()
    {
    }

    //スタンバイから出た
    void ServerTrackedDeviceProvider::LeaveStandby()
    {
    }
}