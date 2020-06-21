#include "ServerTrackedDeviceProvider.h"
namespace VMTDriver {
    //デバイスを管理する親
    //この下に子としてデバイスがぶら下がる

    //初期化
    EVRInitError ServerTrackedDeviceProvider::Init(IVRDriverContext* pDriverContext)
    {
        VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext)
        Log::Open(VRDriverLog());
        Log::Output("HelloWorld");

        //16デバイスを準備
        m_devices.resize(16);

        //16デバイスを登録
        for (int i = 0; i < m_devices.size(); i++)
        {
            m_devices[i].m_serial = "Hello Device";
            m_devices[i].m_serial.append(std::to_string(i));
            m_devices[i].index = i;
            VRServerDriverHost()->TrackedDeviceAdded(m_devices[i].m_serial.c_str(), ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, &m_devices[i]);
        }

        return EVRInitError::VRInitError_None;
    }

    //終了
    void ServerTrackedDeviceProvider::Cleanup()
    {
        VR_CLEANUP_SERVER_DRIVER_CONTEXT()
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
        for (int i = 0; i < m_devices.size(); i++)
        {
            VRServerDriverHost()->TrackedDevicePoseUpdated(m_devices[i].m_deviceIndex, m_devices[i].GetPose(), sizeof(DriverPose_t));
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