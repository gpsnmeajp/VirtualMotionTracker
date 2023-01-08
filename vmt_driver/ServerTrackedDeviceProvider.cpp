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
    //** 内部向け関数群 **

    //全仮想デバイスを返す
    vector<TrackedDeviceServerDriver>& ServerTrackedDeviceProvider::GetDevices()
    {
        return m_devices;
    }

    //特定仮想デバイスを返す
    TrackedDeviceServerDriver& ServerTrackedDeviceProvider::GetDevice(int index)
    {
        return m_devices[index];
    }

    //全仮想デバイスにリセットを指示する
    void ServerTrackedDeviceProvider::DeviceResetAll()
    {
        LogMarker();
        for (int i = 0; i < m_devices.size(); i++)
        {
            m_devices[i].Reset();
        }
    }

    //仮想デバイス内部インデックスの範囲内に収まっているかをチェックする
    bool ServerTrackedDeviceProvider::IsVMTDeviceIndex(int index)
    {
        return (index >= 0 && index <= m_devices.size());
    }

    //インストールパスの取得
    std::string ServerTrackedDeviceProvider::GetInstallPath()
    {
        return m_installPath;
    }

    //全デバイス情報文字列の取得
    std::string ServerTrackedDeviceProvider::GetOpenVRDevicesString()
    {
        std::string result("");

        IVRProperties* props = VRPropertiesRaw();
        CVRPropertyHelpers* helper = VRProperties();
        vr::TrackedDevicePose_t poses[k_unMaxTrackedDeviceCount]{};

        //OpenVRから全トラッキングデバイスの情報を取得する
        VRServerDriverHost()->GetRawTrackedDevicePoses(0.0f, poses, k_unMaxTrackedDeviceCount);

        //デバイスをOpenVR index順に調べる
        for (uint32_t i = 0; i < k_unMaxTrackedDeviceCount; i++) {
            //そのデバイスがつながっていないなら次のデバイスへ
            if (poses[i].bDeviceIsConnected != true) {
                continue;
            }

            //デバイスがつながっているので、シリアルナンバーを取得する
            PropertyContainerHandle_t h = props->TrackedDeviceToPropertyContainer(i);
            string SerialNumber = helper->GetStringProperty(h, ETrackedDeviceProperty::Prop_SerialNumber_String);

            //取得できた情報を文字列に格納する
            result = result + std::to_string(i) + ":" + SerialNumber + "\n";
        }
        return result;
    }

    //デバイス購読
    void ServerTrackedDeviceProvider::SubscribeDevice(std::string serial) {

        //登録済みなら受け付けない
        for (auto& s : m_subscribeDevices) {
            if (s == serial) {
                LogInfo("Already subscribed : %s", serial.c_str());
                return;
            }
        }
        m_subscribeDevices.push_back(serial);

        //変更後の購読リストを吐き出す
        if (Log::s_diag) {
            LogInfo("*** Begin ***");
            for (auto& s : m_subscribeDevices) {
                LogInfo("%s", s.c_str());
            }
            LogInfo("*** End ***");
        }
    }

    //デバイス購読解除
    void ServerTrackedDeviceProvider::UnsubscribeDevice(std::string serial) {
        for (auto it = m_subscribeDevices.begin(), e = m_subscribeDevices.end(); it != e; ++it) {
            if (*it == serial) {
                m_subscribeDevices.erase(it);

                //変更後の購読リストを吐き出す
                if (Log::s_diag) {
                    LogInfo("*** Begin ***");
                    for (auto& s : m_subscribeDevices) {
                        LogInfo("%s", s.c_str());
                    }
                    LogInfo("*** End ***");
                }
                return;
            }
        }
        LogInfo("Not found : %s", serial.c_str());
    }

    //デバイスをシリアル番号から探す
    int ServerTrackedDeviceProvider::SearchDevice(vr::TrackedDevicePose_t* poses, string serial)
    {
        IVRProperties* props = VRPropertiesRaw();
        CVRPropertyHelpers* helper = VRProperties();

        //デバイスシリアルが空白
        if (serial.empty()) {
            //探索エラーを返す
            return k_unTrackedDeviceIndexInvalid;
        }

        //デバイスシリアルがHMDなら
        if (serial == "HMD") {
            //HMDが接続OKなら
            if (poses[k_unTrackedDeviceIndex_Hmd].bDeviceIsConnected) {
                //HMDのインデックスを返す
                return k_unTrackedDeviceIndex_Hmd;
            }
            else {
                //(HMDがつながっていないのは普通ありえないが)探索エラーを返す
                return k_unTrackedDeviceIndexInvalid;
            }
        }

        //デバイスをOpenVR index順に調べる
        for (uint32_t i = 0; i < k_unMaxTrackedDeviceCount; i++) {
            //そのデバイスがつながっていないなら次のデバイスへ
            if (poses[i].bDeviceIsConnected != true) {
                continue;
            }

            //デバイスがつながっているので、シリアルナンバーを取得する
            PropertyContainerHandle_t h = props->TrackedDeviceToPropertyContainer(i);
            string SerialNumber = helper->GetStringProperty(h, ETrackedDeviceProperty::Prop_SerialNumber_String);

            //対象シリアルナンバーと比較し、違うデバイスなら、次のデバイスへ
            if (serial != SerialNumber) {
                continue;
            };

            //目的のデバイスを見つけたので返却
            return i;
        }
        //最後まで探したが、目的のデバイスは見つからなかった
        return k_unTrackedDeviceIndexInvalid;
    }

    //再起動要求
    void ServerTrackedDeviceProvider::RequestRestart() {
        VRServerDriverHost()->RequestRestart("*** RESTART REQUIRED ***","","","");
    }

    //** OpenVR向け関数群 **

    //OpenVRからのデバイスサーバー初期化
    EVRInitError ServerTrackedDeviceProvider::Init(IVRDriverContext* pDriverContext)
    {
        //OpenVR定形の初期化処理を実行
        VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext)

        //デバイスコンテキストを保持
        m_pDriverContext = pDriverContext;

        //セットアップへプロセス起動中を伝達するMutex
        ::CreateMutexA(nullptr, FALSE, "VMT_Mutex");

        //ログをオープン
        Log::Open(VRDriverLog());

        //ドライバのインストールパス取得
        m_installPath = VRProperties()->GetStringProperty(m_pDriverContext->GetDriverHandle(), Prop_InstallPath_String);
        LogInfo("Install Path: %s", m_installPath.c_str());

        //起動時設定読み込み(パス取得済みである必要あり)
        Config::GetInstance()->LoadSetting();

        //起動時診断ログの反映
        Log::s_diag = Config::GetInstance()->GetDiagLogOnStartup();

        //起動時自動更新の反映
        TrackedDeviceServerDriver::SetAutoUpdate(Config::GetInstance()->GetAutoPoseUpdateOnStartup());

        //通信のオープン
        CommunicationManager::GetInstance()->Open();

        //仮想デバイスを準備
        m_devices.resize(58); //58デバイス(全合計64に満たないくらい)

        //仮想デバイスを初期化
        for (int i = 0; i < m_devices.size(); i++)
        {
            //シリアル番号を準備
            string name = "VMT_";
            name.append(std::to_string(i));
            m_devices[i].SetDeviceSerial(name);

            //仮想デバイス内部インデックス(OpenVRのindexではなく、Listのindex)をセット
            m_devices[i].SetObjectIndex(i);
        }


        //起動時登録処理のための初期値
        RawPose pose{};
        pose.roomToDriver = false;
        pose.idx = 0;
        pose.enable = 0;
        pose.x = 0;
        pose.y = 0;
        pose.z = 0;
        pose.qx = 0;
        pose.qy = 0;
        pose.qz = 0;
        pose.qw = 1;
        pose.timeoffset = 0;
        pose.mode = ReferMode_t::None;
        pose.root_sn = "";
        pose.time = std::chrono::system_clock::now();

        //起動時に既定の互換性コントローラとして登録する処理
        if (Config::GetInstance()->GetAddCompatibleControllerOnStartup())
        {
            pose.idx = 1;
            pose.enable = 5;//VMT_1 = Compatible(Knuckles) Controller Left
            m_devices[pose.idx].RegisterToVRSystem(pose.enable);
            m_devices[pose.idx].SetRawPose(pose);

            pose.idx = 2;
            pose.enable = 6;//VMT_2 = Compatible(Knuckles) Controller Right
            m_devices[pose.idx].RegisterToVRSystem(pose.enable);
            m_devices[pose.idx].SetRawPose(pose);
        }
        else {
            //起動時に既定のコントローラとして登録する処理
            if (Config::GetInstance()->GetAddControllerOnStartup()) {
                pose.idx = 1;
                pose.enable = 2; //VMT_1 = Controller Left
                m_devices[pose.idx].RegisterToVRSystem(pose.enable);
                m_devices[pose.idx].SetRawPose(pose);

                pose.idx = 2;
                pose.enable = 3; //VMT_2 = Controller Right
                m_devices[pose.idx].RegisterToVRSystem(pose.enable);
                m_devices[pose.idx].SetRawPose(pose);
            }
        }

        //起動完了
        LogInfo("Startup OK");
        return EVRInitError::VRInitError_None;
    }

    //OpenVRからのデバイスサーバー停止処理
    void ServerTrackedDeviceProvider::Cleanup()
    {
        LogMarker();

        //OpenVR定形の開放処理を実行
        VR_CLEANUP_SERVER_DRIVER_CONTEXT()
        m_pDriverContext = nullptr;

        //通信をクローズ
        CommunicationManager::GetInstance()->Close();

        //ログをクローズ
        Log::Close();
    }

    //OpenVRからのOpenVRインターフェースバージョン問い合わせ
    const char* const* ServerTrackedDeviceProvider::GetInterfaceVersions()
    {
        return k_InterfaceVersions;
    }

    //OpenVRからのフレーム処理
    void ServerTrackedDeviceProvider::RunFrame()
    {
        //通信の毎フレーム処理をする
        CommunicationManager::GetInstance()->Process();

        //各仮想デバイスの姿勢情報のアップデート
        size_t size = m_devices.size();
        for (int i = 0; i < size; i++)
        {
            m_devices[i].UpdatePoseToVRSystem();
        }

        //OpenVRイベントの取得
        VREvent_t VREvent{};
        VRServerDriverHost()->PollNextEvent(&VREvent, sizeof(VREvent_t));

        //各仮想デバイスのイベントを処理
        for (int i = 0; i < size; i++)
        {
            m_devices[i].ProcessEvent(VREvent);
        }

        //OpenVRから全トラッキングデバイスの情報を取得する
        vr::TrackedDevicePose_t devicePoses[k_unMaxTrackedDeviceCount]{};
        VRServerDriverHost()->GetRawTrackedDevicePoses(0.0f, devicePoses, k_unMaxTrackedDeviceCount);

        //購読デバイス処理開始
        for (auto& s : m_subscribeDevices) {
            int index = SearchDevice(devicePoses, s);
            if (index == k_unTrackedDeviceIndexInvalid) { continue; }

            vr::TrackedDevicePose_t& devicePose = devicePoses[index];

            //デバイスのトラッキング状態が正常なら
            if (devicePose.bPoseIsValid) {
                //デバイスの変換行列を取得し、Eigenの行列に変換
                float* m = (float*)(devicePose.mDeviceToAbsoluteTracking.m);

                Eigen::Affine3d rootDeviceToAbsoluteTracking;
                rootDeviceToAbsoluteTracking.matrix() <<
                    m[0 * 4 + 0], m[0 * 4 + 1], m[0 * 4 + 2], m[0 * 4 + 3],
                    m[1 * 4 + 0], m[1 * 4 + 1], m[1 * 4 + 2], m[1 * 4 + 3],
                    m[2 * 4 + 0], m[2 * 4 + 1], m[2 * 4 + 2], m[2 * 4 + 3],
                    0.0, 0.0, 0.0, 1.0;

                Eigen::Translation3d pos(rootDeviceToAbsoluteTracking.translation());
                Eigen::Quaterniond rot = Eigen::Quaterniond(rootDeviceToAbsoluteTracking.rotation());

                //返送する
                OSCReceiver::SendSubscribedDevicePose(s, (float)pos.x(), (float)pos.y(), (float)pos.z(), (float)rot.x(), (float)rot.y(), (float)rot.z(), (float)rot.w());
            }
        }
    }

    //OpenVRからのスタンバイブロック問い合わせ
    bool ServerTrackedDeviceProvider::ShouldBlockStandbyMode()
    {
        return false;
    }

    //OpenVRからのスタンバイ開始通知
    void ServerTrackedDeviceProvider::EnterStandby()
    {
    }

    //OpenVRからのスタンバイ終了通知
    void ServerTrackedDeviceProvider::LeaveStandby()
    {
    }
}