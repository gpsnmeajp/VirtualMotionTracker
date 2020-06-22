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
/**
 * Based on EasyOpenVRUtil by gpsnmeajp v0.05
 */

using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using Valve.VR;
using System.Numerics;

namespace EasyLazyLibrary
{
    public class EasyOpenVRUtil
    {
        //定数定義
        public const uint InvalidDeviceIndex = OpenVR.k_unTrackedDeviceIndexInvalid;

        //VRハンドル
        CVRSystem openvr = null;

        //内部保持用全デバイス姿勢
        TrackedDevicePose_t[] allDevicePose;
        TrackedDevicePose_t[] allDevicePoseRaw;

        //デバイス姿勢を常にアップデートするか
        bool autoupdate = true;

        //光子遅延補正予測時間(0=補正なし or 予測時間取得失敗)
        float PredictedTime = 0f;

        //最終更新フレームカウント
        long LastFrameCount = 0;

        //姿勢クラス
        public class Transform
        {
            public uint deviceid = InvalidDeviceIndex;
            public Vector3 position = Vector3.Zero;
            public Quaternion rotation = Quaternion.Identity;
            public Vector3 velocity = Vector3.Zero;
            public Vector3 angularVelocity = Vector3.Zero;
            public Matrix4x4 matrix4X4;

            //デバッグ用
            public override string ToString()
            {
                return "deviceid: " + deviceid + " position:" + position.ToString() + " rotation:" + rotation.ToString() + " velocity:"+ velocity.ToString() + " angularVelocity:" + angularVelocity.ToString();
            }
        }

        public EasyOpenVRUtil()
        {
            //とりあえず初期化する
            Init();
        }

        public uint GetHMDIndex()
        {
            if (!IsReady()) { return InvalidDeviceIndex; }
            return OpenVR.k_unTrackedDeviceIndex_Hmd;
        }

        public uint GetLeftControllerIndex()
        {
            if (!IsReady()) { return InvalidDeviceIndex; }
            return openvr.GetTrackedDeviceIndexForControllerRole(ETrackedControllerRole.LeftHand);
        }

        public uint GetRightControllerIndex()
        {
            if (!IsReady()) { return InvalidDeviceIndex; }
            return openvr.GetTrackedDeviceIndexForControllerRole(ETrackedControllerRole.RightHand);
        }

        public TrackedDevicePose_t[] GetAllDevicePose()
        {
            if (autoupdate)
            {
                Update();
            }
            return allDevicePose;
        }
        public TrackedDevicePose_t[] GetAllDevicePoseRaw()
        {
            if (autoupdate)
            {
                Update();
            }
            return allDevicePoseRaw;
        }

        public TrackedDevicePose_t GetDevicePose(uint i)
        {
            if (!IsDeviceValid(i))
            {
                return new TrackedDevicePose_t();
            }
            return allDevicePose[i];
        }
        public TrackedDevicePose_t GetDevicePoseRaw(uint i)
        {
            if (!IsDeviceValid(i))
            {
                return new TrackedDevicePose_t();
            }
            return allDevicePoseRaw[i];
        }

        public ETrackingResult GetDeviceTrackingResult(uint i)
        {
            if (!IsDeviceValid(i))
            {
                return ETrackingResult.Uninitialized;
            }
            return allDevicePose[i].eTrackingResult;
        }

        public void SetAutoUpdate(bool autoupdate)
        {
            this.autoupdate = autoupdate;
        }

        //初期化。失敗したらfalse
        public bool Init()
        {
            openvr = OpenVR.System;
            return IsReady();
        }

        //OpenVRを初期化する
        public bool StartOpenVR(EVRApplicationType type = EVRApplicationType.VRApplication_Overlay)
        {
            //すでに利用可能な場合は初期化しない(衝突する)
            if (Init())
            {
                return true;
            }

            //初期化する
            var openVRError = EVRInitError.None;
            openvr = OpenVR.Init(ref openVRError, type);
            if (openVRError != EVRInitError.None)
            {
                return false;
            }

            //本ライブラリも初期化
            return Init();
        }


        //終了イベントをキャッチした時に戻す
        public bool ProcessEventAndCheckQuit()
        {
            if (!IsReady()) { return false; }
            //イベント構造体のサイズを取得
            uint uncbVREvent = (uint)System.Runtime.InteropServices.Marshal.SizeOf(typeof(VREvent_t));

            //イベント情報格納構造体
            VREvent_t Event = new VREvent_t();
            //イベントを取り出す
            while (openvr.PollNextEvent(ref Event, uncbVREvent))
            {
                //イベント情報で分岐
                switch ((EVREventType)Event.eventType)
                {
                    case EVREventType.VREvent_Quit:
                        return true;
                }
            }
            return false;
        }

        public void AutoExitOnQuit()
        {
            if (ProcessEventAndCheckQuit())
            {
                ApplicationQuit();
            }
        }

        //アプリケーションを終了させる
        public void ApplicationQuit()
        {

        }

        //本ライブラリが利用可能か確認する
        public bool IsReady()
        {
            return openvr != null;
        }

        //VRシステムが使えるか確認する
        public bool CanUseOpenVR()
        {
            return OpenVR.System != null;
        }

        //全デバイス情報を更新
        public void Update(ETrackingUniverseOrigin origin = ETrackingUniverseOrigin.TrackingUniverseStanding)
        {
            allDevicePose = new TrackedDevicePose_t[OpenVR.k_unMaxTrackedDeviceCount];
            allDevicePoseRaw = new TrackedDevicePose_t[OpenVR.k_unMaxTrackedDeviceCount];
            if (!IsReady()) { return; }
            //すべてのデバイスの情報を取得
            openvr.GetDeviceToAbsoluteTrackingPose(origin, PredictedTime, allDevicePose);
            openvr.GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin.TrackingUniverseRawAndUncalibrated, PredictedTime, allDevicePoseRaw);
            //最終更新フレームを更新
            LastFrameCount = DateTime.Now.Ticks;
        }

        //デバイスが有効か
        public bool IsDeviceValid(uint index)
        {
            //自動更新処理
            if (autoupdate)
            {
                //前回と違うフレームの場合のみ更新
                if (LastFrameCount != DateTime.Now.Ticks)
                {
                    UpdatePredictedTime(); //光子遅延時間のアップデート追加
                    Update();
                }
            }
            //情報が有効でないなら更新
            if (allDevicePose == null)
            {
                Update();
            }
            //それでも情報が有効でないなら失敗
            if (allDevicePose == null)
            {
                return false;
            }

            //device indexが有効
            if (index != OpenVR.k_unTrackedDeviceIndexInvalid)
            {
                //接続されていて姿勢情報が有効
                if (allDevicePose[index].bDeviceIsConnected && allDevicePose[index].bPoseIsValid)
                {
                    return true;
                }
            }
            return false;
        }

        public Transform GetHMDTransform()
        {
            return GetTransform(GetHMDIndex());
        }

        public Transform GetLeftControllerTransform()
        {
            return GetTransform(GetLeftControllerIndex());
        }

        public Transform GetRightControllerTransform()
        {
            return GetTransform(GetRightControllerIndex());
        }

        public Transform GetTransformBySerialNumber(string serial)
        {
            return GetTransform(GetDeviceIndexBySerialNumber(serial));
        }
        public Transform GetHMDTransformRaw()
        {
            return GetTransformRaw(GetHMDIndex());
        }

        public Transform GetLeftControllerTransformRaw()
        {
            return GetTransformRaw(GetLeftControllerIndex());
        }

        public Transform GetRightControllerTransformRaw()
        {
            return GetTransformRaw(GetRightControllerIndex());
        }

        public Transform GetTransformBySerialNumberRaw(string serial)
        {
            return GetTransformRaw(GetDeviceIndexBySerialNumber(serial));
        }

        //指定デバイスの姿勢情報を取得
        public Transform GetTransform(uint index)
        {
            //有効なデバイスか
            if (!IsDeviceValid(index))
            {
                return null;
            }

            TrackedDevicePose_t Pose = allDevicePose[index];
            var m = Pose.mDeviceToAbsoluteTracking;
            Matrix4x4 mat = new Matrix4x4(
                m.m0, m.m4, m.m8, 0,
                m.m1, m.m5, m.m9, 0,
                m.m2, m.m6, m.m10, 0,
                m.m3, m.m7, m.m11, 1
            );

            Transform res = new Transform();
            res.matrix4X4 = mat;
            res.deviceid = index;

            res.velocity.X = Pose.vVelocity.v0;
            res.velocity.Y = Pose.vVelocity.v1;
            res.velocity.Z = Pose.vVelocity.v2;
            res.angularVelocity.X = Pose.vAngularVelocity.v0;
            res.angularVelocity.Y = Pose.vAngularVelocity.v1;
            res.angularVelocity.Z = Pose.vAngularVelocity.v2;

            res.position = mat.Translation;
            res.rotation = Quaternion.CreateFromRotationMatrix(mat);
            //res.rotation.W = -res.rotation.W;

            return res;
        }
        public Transform Matrix4x4ToTransform(Matrix4x4 mat)
        {
            Transform res = new Transform();
            res.matrix4X4 = mat;
            res.position = mat.Translation;
            res.rotation = Quaternion.CreateFromRotationMatrix(mat);
            //res.rotation.W = -res.rotation.W;
            return res;
        }

        public Matrix4x4 HmdMatrix34ToMatrix4x4(HmdMatrix34_t m)
        { 
            return new Matrix4x4(
                m.m0, m.m4, m.m8, 0,
                m.m1, m.m5, m.m9, 0,
                m.m2, m.m6, m.m10, 0,
                m.m3, m.m7, m.m11, 1
            );
        }

        public Transform GetTransformRaw(uint index)
        {
            //有効なデバイスか
            if (!IsDeviceValid(index))
            {
                return null;
            }

            TrackedDevicePose_t Pose = allDevicePoseRaw[index];
            var m = Pose.mDeviceToAbsoluteTracking;
            Matrix4x4 mat = new Matrix4x4(
                m.m0, m.m4, m.m8, 0,
                m.m1, m.m5, m.m9, 0,
                m.m2, m.m6, m.m10, 0,
                m.m3, m.m7, m.m11, 1
            );

            Transform res = new Transform();
            res.matrix4X4 = mat;
            res.deviceid = index;

            res.velocity.X = Pose.vVelocity.v0;
            res.velocity.Y = Pose.vVelocity.v1;
            res.velocity.Z = Pose.vVelocity.v2;
            res.angularVelocity.X = Pose.vAngularVelocity.v0;
            res.angularVelocity.Y = Pose.vAngularVelocity.v1;
            res.angularVelocity.Z = Pose.vAngularVelocity.v2;

            res.position = mat.Translation;
            res.rotation = Quaternion.CreateFromRotationMatrix(mat);
            //res.rotation.W = -res.rotation.W;

            return res;
        }

        //指定デバイスの姿勢情報を取得
        public bool GetPose(uint index, out Vector3 pos, out Quaternion rot)
        {
            pos = Vector3.Zero;
            rot = Quaternion.Identity;

            Transform t = GetTransform(index);
            if (t == null)
            {
                return false;
            }

            pos = t.position;
            rot = t.rotation;

            return true;
        }
        public bool GetPoseRaw(uint index, out Vector3 pos, out Quaternion rot)
        {
            pos = Vector3.Zero;
            rot = Quaternion.Identity;

            Transform t = GetTransformRaw(index);
            if (t == null)
            {
                return false;
            }

            pos = t.position;
            rot = t.rotation;

            return true;
        }

        //指定デバイスの速度情報を取得
        public bool GetVelocity(uint index, out Vector3 velocity, out Vector3 angularVelocity)
        {
            velocity = Vector3.Zero;
            angularVelocity = Vector3.Zero;

            Transform t = GetTransform(index);
            if (t == null)
            {
                return false;
            }

            velocity = t.velocity;
            angularVelocity = t.angularVelocity;

            return true;
        }

        //device情報を取得する
        public bool GetPropertyString(uint idx, ETrackedDeviceProperty prop, out string result)
        {
            result = null;
            ETrackedPropertyError error = new ETrackedPropertyError();
            //device情報を取得するのに必要な文字数を取得
            uint size = openvr.GetStringTrackedDeviceProperty(idx, prop, null, 0, ref error);
            if (error != ETrackedPropertyError.TrackedProp_BufferTooSmall)
            {
                return false;
            }

            StringBuilder s = new StringBuilder();
            s.Length = (int)size; //文字長さ確保
                                  //device情報を取得する
            openvr.GetStringTrackedDeviceProperty(idx, prop, s, size, ref error);

            result = s.ToString();
            return (error == ETrackedPropertyError.TrackedProp_Success);
        }

        //device情報を取得する
        public bool GetPropertyFloat(uint idx, ETrackedDeviceProperty prop, out float result)
        {
            ETrackedPropertyError error = new ETrackedPropertyError();
            result = openvr.GetFloatTrackedDeviceProperty(idx, prop, ref error);
            return (error == ETrackedPropertyError.TrackedProp_Success);
        }

        //device情報を取得する
        public bool GetPropertyBool(uint idx, ETrackedDeviceProperty prop, out bool result)
        {
            ETrackedPropertyError error = new ETrackedPropertyError();
            result = openvr.GetBoolTrackedDeviceProperty(idx, prop, ref error);
            return (error == ETrackedPropertyError.TrackedProp_Success);
        }

        //device情報を取得する
        public bool GetPropertyUint64(uint idx, ETrackedDeviceProperty prop, out ulong result)
        {
            ETrackedPropertyError error = new ETrackedPropertyError();
            result = openvr.GetUint64TrackedDeviceProperty(idx, prop, ref error);
            return (error == ETrackedPropertyError.TrackedProp_Success);
        }

        //device情報を取得する
        public bool GetPropertyInt32(uint idx, ETrackedDeviceProperty prop, out int result)
        {
            ETrackedPropertyError error = new ETrackedPropertyError();
            result = openvr.GetInt32TrackedDeviceProperty(idx, prop, ref error);
            return (error == ETrackedPropertyError.TrackedProp_Success);
        }

        public bool IsDeviceConnected(uint idx)
        {
            if (!IsReady()) { return false; }
            return openvr.IsTrackedDeviceConnected(idx);
        }

        public bool GetControllerButtonPressed(uint index, out ulong ulButtonPressed)
        {
            ulButtonPressed = 0;
            VRControllerState_t state;
            bool r = GetControllerState(index, out state);
            if (!r)
            {
                return false;
            }
            ulButtonPressed = state.ulButtonPressed;
            return true;
        }

        public bool GetControllerState(uint index, out VRControllerState_t state)
        {
            state = new VRControllerState_t();

            //有効なデバイスか
            if (!IsDeviceValid(index))
            {
                return false;
            }

            uint size = (uint)System.Runtime.InteropServices.Marshal.SizeOf(typeof(VRControllerState_t));
            return openvr.GetControllerState(index, ref state, size);
        }

        public bool TriggerHapticPulse(uint index, ushort us = 3000)
        {
            //有効なデバイスか
            if (!IsDeviceValid(index))
            {
                return false;
            }

            openvr.TriggerHapticPulse(index, 1, us);
            return true;
        }

        //
        public string GetPropertyStringWhenConnected(uint idx, ETrackedDeviceProperty prop)
        {
            if (!IsDeviceConnected(idx))
            {
                return null;
            }

            string result = null;
            if (!GetPropertyString(idx, prop, out result))
            {
                return null;
            }
            return result;
        }

        //
        public float GetPropertyFloatWhenConnected(uint idx, ETrackedDeviceProperty prop)
        {
            if (!IsDeviceConnected(idx))
            {
                return float.NaN;
            }

            float result = float.NaN;
            if (!GetPropertyFloat(idx, prop, out result))
            {
                return float.NaN;
            }
            return result;
        }


        //シリアル番号を取得する
        public string GetSerialNumber(uint idx)
        {
            return GetPropertyStringWhenConnected(idx, ETrackedDeviceProperty.Prop_SerialNumber_String);
        }

        //型式名を取得する
        public string GetRenderModelName(uint idx)
        {
            return GetPropertyStringWhenConnected(idx, ETrackedDeviceProperty.Prop_RenderModelName_String);
        }

        //型式名を取得する
        public string GetRegisteredDeviceType(uint idx)
        {
            return GetPropertyStringWhenConnected(idx, ETrackedDeviceProperty.Prop_RegisteredDeviceType_String);
        }

        //電池残量を取得する
        public float GetDeviceBatteryPercentage(uint idx)
        {
            return GetPropertyFloatWhenConnected(idx, ETrackedDeviceProperty.Prop_DeviceBatteryPercentage_Float) * 100.0f;
        }

        public bool IsCharging(uint idx, out bool result)
        {
            return GetPropertyBool(idx, ETrackedDeviceProperty.Prop_DeviceIsCharging_Bool, out result);
        }

        public bool TakeScreenShot(string path, string pathVR)
        {
            CVRScreenshots screenshot = OpenVR.Screenshots;
            if (screenshot == null)
            {
                return false;
            }
            string previewfile = path;
            string vrfile = pathVR;

            EVRScreenshotError error = EVRScreenshotError.None;
            uint pOutScreenshotHandle = 0;
            error = screenshot.TakeStereoScreenshot(ref pOutScreenshotHandle, previewfile, vrfile);
            return (error == EVRScreenshotError.None);
        }

        public string GetDeviceDebugInfo(uint idx)
        {
            string s = "Device ID:" + idx + " ";
            if (!IsDeviceConnected(idx))
            {
                s += "is not connected.";
                return s;
            }
            string result;
            result = GetSerialNumber(idx);
            if (result != null)
            {
                s += "Serial:" + result + " ";
            }
            result = GetRenderModelName(idx);
            if (result != null)
            {
                s += "Model:" + result + " ";
            }
            result = GetRegisteredDeviceType(idx);
            if (result != null)
            {
                s += "DeviceType:" + result + " ";
            }
            float batt = GetDeviceBatteryPercentage(idx);
            if (!float.IsNaN(batt))
            {
                s += "DeviceBattery:" + batt + "% ";
            }
            bool r = false;
            bool b = IsCharging(idx, out r);
            if (b)
            {
                s += "Charging:" + r + " ";
            }

            return s;
        }

        public int ConnectedDevices()
        {
            //接続されているdeviceの数をカウントする
            int ConnectedDevices = 0;
            for (uint i = 0; i < OpenVR.k_unMaxTrackedDeviceCount; i++)
            {
                if (IsDeviceConnected(i))
                {
                    ConnectedDevices++;
                }
            }
            return ConnectedDevices;
        }

        public string PutDeviceInfoListString()
        {
            string s = "";
            int connectedDeviceNum = ConnectedDevices();
            //deviceの詳細情報を1つづつ読み出す
            uint connectedDeviceCount = 0;
            for (uint i = 0; i < OpenVR.k_unMaxTrackedDeviceCount; i++)
            {
                //接続中だったら、読み取り完了数を1増やす
                if (IsDeviceConnected(i))
                {
                    s += GetDeviceDebugInfo(i) + "\n";
                    connectedDeviceCount++;
                }
                //接続されている数だけ読み取り終わったら終了する
                if (connectedDeviceCount >= connectedDeviceNum)
                {
                    break;
                }
            }
            return s;
        }

        public string PutDeviceInfoListStringFromDeviceIndexList(List<uint> devices)
        {
            string s = "";

            foreach (uint i in devices)
            {
                if (IsDeviceConnected(i))
                {
                    s += GetDeviceDebugInfo(i) + "\n";
                }
            }
            return s;
        }


        public uint GetDeviceIndexBySerialNumber(string serial)
        {
            if (!IsReady()) { return InvalidDeviceIndex; }

            int connectedDeviceNum = ConnectedDevices();
            //deviceの詳細情報を1つづつ読み出す
            uint connectedDeviceCount = 0;
            for (uint i = 0; i < OpenVR.k_unMaxTrackedDeviceCount; i++)
            {
                //接続中だったら、読み取り完了数を1増やす
                if (IsDeviceConnected(i))
                {
                    //一致するか調べる
                    if (serial.Equals(GetSerialNumber(i)))
                    {
                        return i;
                    }
                    connectedDeviceCount++;
                }
                //接続されている数だけ読み取り終わったら終了する
                if (connectedDeviceCount >= connectedDeviceNum)
                {
                    break;
                }
            }
            return InvalidDeviceIndex; //見つからなかった
        }

        public List<uint> GetDeviceIndexListByRenderModelName(string name)
        {
            List<uint> devices = new List<uint>();
            if (!IsReady()) { return devices; }

            int connectedDeviceNum = ConnectedDevices();
            //deviceの詳細情報を1つづつ読み出す
            uint connectedDeviceCount = 0;
            for (uint i = 0; i < OpenVR.k_unMaxTrackedDeviceCount; i++)
            {
                //接続中だったら、読み取り完了数を1増やす
                if (IsDeviceConnected(i))
                {
                    string res = GetRenderModelName(i);
                    if (res != null)
                    {
                        //含んでいるか調べる
                        if (res.Contains(name))
                        {
                            devices.Add(i);
                        }
                    }
                    connectedDeviceCount++;
                }
                //接続されている数だけ読み取り終わったら終了する
                if (connectedDeviceCount >= connectedDeviceNum)
                {
                    break;
                }
            }
            return devices;
        }

        public List<uint> GetDeviceIndexListByRegisteredDeviceType(string name)
        {
            List<uint> devices = new List<uint>();
            if (!IsReady()) { return devices; }

            int connectedDeviceNum = ConnectedDevices();
            //deviceの詳細情報を1つづつ読み出す
            uint connectedDeviceCount = 0;
            for (uint i = 0; i < OpenVR.k_unMaxTrackedDeviceCount; i++)
            {
                //接続中だったら、読み取り完了数を1増やす
                if (IsDeviceConnected(i))
                {
                    string res = GetRegisteredDeviceType(i);
                    if (res != null)
                    {
                        //含んでいるか調べる
                        if (res.Contains(name))
                        {
                            devices.Add(i);
                        }
                    }
                    connectedDeviceCount++;
                }
                //接続されている数だけ読み取り終わったら終了する
                if (connectedDeviceCount >= connectedDeviceNum)
                {
                    break;
                }
            }
            return devices;
        }

        public List<uint> GetViveTrackerIndexList()
        {
            return GetDeviceIndexListByRegisteredDeviceType("htc/vive_tracker");
        }

        public List<uint> GetViveControllerIndexList()
        {
            return GetDeviceIndexListByRegisteredDeviceType("htc/vive_controller");
        }

        public List<uint> GetBaseStationIndexList()
        {
            return GetDeviceIndexListByRenderModelName("lh_basestation_vive");
        }



        //----------------光子遅延時間----------------------------

        //予測遅延時間(動作-光子遅延時間)を設定
        public void UpdatePredictedTime()
        {
            PredictedTime = GetPredictedTime();
        }

        //予測遅延時間(動作-光子遅延時間)を無効化
        public void ClearPredictedTime()
        {
            PredictedTime = 0;
        }

        //現在の予測遅延時間(動作-光子遅延時間)を取得
        public float GetPredictedTime()
        {
            //最後のVsyncからの経過時間(フレーム経過時間)を取得
            float FrameTime = 0;
            ulong FrameCount = 0;

            if (!IsReady()) { return 0; }

            if (!openvr.GetTimeSinceLastVsync(ref FrameTime, ref FrameCount))
            {
                return 0; //有効な値を取得できなかった
            }

            //たまにすごい勢いで増えることがある
            if (FrameTime > 1.0f)
            {
                return 0; //有効な値を取得できなかった
            }

            //1フレームあたりの時間取得
            float DisplayFrequency = 0;
            if (!GetPropertyFloat(GetHMDIndex(), ETrackedDeviceProperty.Prop_DisplayFrequency_Float, out DisplayFrequency))
            {
                return 0; //有効な値を取得できなかった
            }
            float DisplayCycle = 1f / DisplayFrequency;

            //光子遅延時間(出力からHMD投影までにかかる時間)取得
            float PhotonDelay = 0;
            if (!GetPropertyFloat(GetHMDIndex(), ETrackedDeviceProperty.Prop_SecondsFromVsyncToPhotons_Float, out PhotonDelay))
            {
                return 0; //有効な値を取得できなかった
            }

            //予測遅延時間(1フレームあたりの時間 - 現在フレーム経過時間 + 光子遅延時間)
            var PredictedTimeNow = DisplayCycle - FrameTime + PhotonDelay;

            //負の値は過去になる。
            if (PredictedTimeNow < 0)
            {
                return 0;
            }

            return PredictedTimeNow;
        }


    }
}