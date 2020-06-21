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
        //VRServerDriverHost()->TrackedDeviceAdded(serial, ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, deriver);

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