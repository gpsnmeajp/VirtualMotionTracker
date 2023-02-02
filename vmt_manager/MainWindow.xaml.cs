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
using System;
using System.IO;
using System.Text;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Navigation;
using System.Windows.Threading;
using System.Collections;
using System.Collections.Generic;
using Valve.VR;
using EasyLazyLibrary;
using Rug.Osc;

namespace vmt_manager
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        const string Version = "VMT_015";
        private DispatcherTimer dispatcherTimer;
        Random rnd;
        string title = "";
        string installPath = "";

        int aliveCnt = 0;
        bool ReceiveError = false;
        EasyOpenVRUtil util;
        OSC osc;

        bool loadTest = false;
        Dictionary<string, string> subscribedDevicePoseDictionary = new Dictionary<string, string>();
        int subscribeReceiveCnt = 0;
        bool autosetup = false;

        Action FixItAction = null;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void FixItButton(object sender, RoutedEventArgs e) {
            TryFixItButton.Visibility = Visibility.Collapsed;
            FixItAction?.Invoke();
        }

        private void TopErrorMessage(string message, bool permitInstall = false, Action Fixit = null)
        {
            TopErrorDockPanel.Visibility = Visibility.Visible;
            TopErrorTextBlock.Text = message;
            TopErrorDockPanel.Background = new SolidColorBrush(Color.FromRgb(255, 0, 0));
            TopErrorTextBlock.Foreground = new SolidColorBrush(Color.FromRgb(255, 255, 255));
            TopNotInstalledTextBlock.Visibility = Visibility.Collapsed;

            TryFixItButton.Visibility = Fixit != null?Visibility.Visible:Visibility.Collapsed;
            FixItAction = Fixit;

            if (autosetup) {
                MessageBox.Show(message, title, MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
        private void TopWarningMessage(string message, bool enable = false)
        {
            TopErrorDockPanel.Visibility = Visibility.Visible;
            TopErrorTextBlock.Text = message;
            TopErrorDockPanel.Background = new SolidColorBrush(Color.FromRgb(255, 255, 0));
            TopErrorTextBlock.Foreground = new SolidColorBrush(Color.FromRgb(0, 0, 0));
            TopNotInstalledTextBlock.Visibility = Visibility.Collapsed;
            TryFixItButton.Visibility = Visibility.Collapsed;

            if (autosetup)
            {
                MessageBox.Show(message, title, MessageBoxButton.OK, MessageBoxImage.Warning);
            }
        }

        private void Close(Exception ex) {
            Console.WriteLine(ex.ToString());
            Close();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                Console.WriteLine("Loaded");
                title = this.Title;
                ManagerVersion.Text = Version;
                DriverVersion.Text = " - ";

                InputPositionXTextBox.IsEnabled = false;
                InputPositionYTextBox.IsEnabled = false;
                InputPositionZTextBox.IsEnabled = false;
                InputPositionSerialTextBox.IsEnabled = false;
                InputPositionModeTextBox.IsEnabled = false;
                InputPositionQXTextBox.IsEnabled = false;
                InputPositionQYTextBox.IsEnabled = false;
                InputPositionQZTextBox.IsEnabled = false;
                InputPositionQWTextBox.IsEnabled = false;
                InputPositionAixisLinkTextBox.IsEnabled = false;
                InputPositionRXTextBox.IsEnabled = false;
                InputPositionRYTextBox.IsEnabled = false;
                InputPositionRZTextBox.IsEnabled = false;
                InputPositionBoneTextBox.IsEnabled = false;
                InputPositionValueTextBox.IsEnabled = false;

                //ProtocolTextBlock.Text = File.ReadAllText(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\PROTOCOL.md");

                rnd = new Random();
                osc = new OSC("127.0.0.1", 39571, 39570, OnBundle, OnMessage);
                osc.Send(new OscMessage("/VMT/Set/Destination", "127.0.0.1", 39571));

                util = new EasyOpenVRUtil();
                if (!util.StartOpenVR())
                {
                    TopErrorMessage("Steam VR not ready. Maybe not ready for HMD or Tracking system.\nStream VRが利用できません。HMDやトラッキングシステムが利用できない状態の可能性があります。\n\nPlease enable Null driver If you want to use without HMD. \nHMDなしで利用したい場合は、Null driverを有効にして再起動してください。", true, () => {
                        EnableNullHMDDriverButton(null, null);
                    });
                    //Close();
                    //タイマー起動してはいけない
                    return;
                }

                if (Environment.Is64BitOperatingSystem == false)
                {
                    TopErrorMessage("VMT works 64bit OS only.\n VMTは64bit OSでのみ動作します。");
                    InstallButtonName.IsEnabled = false;
                    //タイマー起動してはいけない
                    return;
                }

                //タイマー起動
                dispatcherTimer = new DispatcherTimer();
                dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
                dispatcherTimer.Tick += new EventHandler(GenericTimer);
                dispatcherTimer.Start();

                //セーフモードチェック
                EVRSettingsError eVRSettingsError = EVRSettingsError.None;
                bool safemode = OpenVR.Settings.GetBool("driver_vmt", OpenVR.k_pch_Driver_BlockedBySafemode_Bool, ref eVRSettingsError);
                if (eVRSettingsError == EVRSettingsError.None && safemode)
                {
                    TopErrorMessage("SteamVR running on safe mode and VMT has blocked.\nPlease unblock, and restart SteamVR.\n\nSteamVRがセーフモードで動作し、VMTがブロックされています。ブロックを解除し、SteamVRを再起動してください。", true, () => {
                        OpenVR.Settings.SetBool("driver_vmt", OpenVR.k_pch_Driver_BlockedBySafemode_Bool, false, ref eVRSettingsError);
                        RequestRestart();
                    });
                }
                //Enableチェック
                eVRSettingsError = EVRSettingsError.None;
                bool enable = OpenVR.Settings.GetBool("driver_vmt", OpenVR.k_pch_Driver_Enable_Bool, ref eVRSettingsError);
                if (eVRSettingsError == EVRSettingsError.None && !enable)
                {
                    TopErrorMessage("VMT has disabled in Steam VR setting.\nPlease enable, and restart SteamVR.\n\nVMTはSteamVR上で無効に設定されています。有効にし、SteamVRを再起動してください。", true, () => {
                        OpenVR.Settings.SetBool("driver_vmt", OpenVR.k_pch_Driver_Enable_Bool, true, ref eVRSettingsError);
                        RequestRestart();
                    });
                }
                //requireHmdチェック
                eVRSettingsError = EVRSettingsError.None;
                bool requireHmd = OpenVR.Settings.GetBool("steamvr", OpenVR.k_pch_SteamVR_RequireHmd_String, ref eVRSettingsError);
                if (eVRSettingsError == EVRSettingsError.None && !requireHmd)
                {
                    TopWarningMessage("Manager detected requireHmd is false on SteamVR. Some functions will not work probably.\nSteamVRにてrequireHmdがfalseに設定されています。いくつかの機能は正常に動かない可能性があります。", true);
                }

                //デバッグのためにセーフモードを有効化
                //OpenVR.Settings.SetBool(OpenVR.k_pch_SteamVR_Section, OpenVR.k_pch_SteamVR_EnableSafeMode, true,ref eVRSettingsError);

                //コマンドライン引数処理
                string[] args = System.Environment.GetCommandLineArgs();
                if (args.Length > 1)
                {
                    this.Dispatcher.Invoke(async () =>
                    {
                        if (args[1] == "install")
                        {
                            this.Hide();
                            autosetup = true;
                            await Install();
                            Close();
                            return;
                        }
                        if (args[1] == "uninstall")
                        {
                            this.Hide();
                            autosetup = true;
                            await Uninstall();
                            Close();
                            return;
                        }
                        if (args[1] == "setroommatrix")
                        {
                            this.Hide();
                            autosetup = true;

                            //最大30秒待つ
                            for (int i = 0; i < 30; i++)
                            {
                                await Task.Delay(1000);
                                if (SetRoomMatrixButtonName.IsEnabled == true) { break; }
                            }

                            SetRoomMatrixButton(null, null);
                            await Task.Delay(3000);
                            Close();
                            return;
                        }
                    });
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
                Close(ex);
                return;
            }
        }

        private void OnMessage(OscMessage message)
        {
            //エラー時はそれ以上受信しない
            if (ReceiveError)
            {
                return;
            }
            try
            {
                if (message.Address == "/VMT/Out/Log")
                {
                    int stat = (int)message[0];
                    string msg = (string)message[1];

                    switch (stat)
                    {
                        case 0:
                            MessageBox.Show(msg, title, MessageBoxButton.OK, MessageBoxImage.Information);
                            break;
                        case 1:
                            MessageBox.Show(msg, title, MessageBoxButton.OK, MessageBoxImage.Warning);
                            break;
                        case 2:
                            MessageBox.Show(msg, title, MessageBoxButton.OK, MessageBoxImage.Error);
                            break;
                        default:
                            MessageBox.Show(msg, title, MessageBoxButton.OK);
                            break;
                    }
                }
                else if (message.Address == "/VMT/Out/Alive")
                {
                    //Keep Alive
                    string versionFromOSC = (string)message[0];
                    this.Dispatcher.Invoke(() =>
                    {
                        DriverVersion.Text = versionFromOSC;
                        if (message.Count > 1 && message[1] is string)
                        {
                            installPath = (string)message[1];
                            string mypath = Path.GetFullPath(Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt");
                            if (installPath != "" && mypath.StartsWith(installPath) == false) {
                                TopWarningMessage("Different directory VMT detected. You can not use VMT Manager for setting. but you can use this to uninstall VMT driver.\n違うディレクトリにVMTがインストールされています。このVMT Managerでは設定はできません。VMTをアンインストールすることは出来ます。\n" + installPath + "\n" + mypath);
                            }
                        }

                        ControlDock.IsEnabled = true;

                        if (versionFromOSC != Version)
                        {
                            DriverVersion.Foreground = new SolidColorBrush(Color.FromRgb(255, 100, 100));
                            TopWarningMessage("Different version VMT detected. You can not use VMT Manager for setting. but you can use this to uninstall VMT driver.\n\n違うバージョンのVMTがインストールされています。このVMT Managerでは設定はできません。VMTをアンインストールすることは出来ます。");
                        }
                        else
                        {
                            DriverVersion.Foreground = new SolidColorBrush(Color.FromRgb(0, 255, 0));
                        }
                        InstallButtonName.IsEnabled = false;
                        TopNotInstalledTextBlock.Visibility = Visibility.Collapsed;

                        aliveCnt = 0;
                    });
                }
                else if (message.Address == "/VMT/Out/Haptic")
                {
                    int index = (int)message[0];
                    float frequency = (float)message[1];
                    float amplitude = (float)message[2];
                    float duration = (float)message[3];
                    //振動
                    this.Dispatcher.Invoke(() =>
                    {
                        InputVMTHapticTextBox.Text = string.Format("VMT_{0,0} f:{1:0.0}Hz A:{2:0.0} d:{3:0.0}s", index, frequency, amplitude, duration);
                        InputVMTHapticTextBox.Background = new SolidColorBrush(Color.FromRgb(0, 255, 0));
                    });
                }
                else if (message.Address == "/VMT/Out/Unavailable")
                {
                    //利用不可ステータス
                    int code = (int)message[0];
                    string reason = (string)message[1];

                    this.Dispatcher.Invoke(() =>
                    {
                        StatusBarTextBlock.Text = code.ToString() + " : " + reason;
                        if (code != 0)
                        {
                            StatusBar.Background = new SolidColorBrush(Color.FromRgb(255, 255, 0));
                        }
                        else
                        {
                            StatusBar.Background = new SolidColorBrush(Color.FromRgb(255, 255, 255));
                        }
                    });
                }
                else if (message.Address == "/VMT/Out/Devices/List")
                {
                    this.Dispatcher.Invoke(() =>
                    {
                        //デバイス一覧
                        string[] list = ((string)message[0]).Split('\n');
                        for (uint i = 0; i < list.Length; i++)
                        {
                            DeviceListView.Items.Add(list[i]);
                        }
                    });
                }
                else if (message.Address == "/VMT/Out/SubscribedDevice")
                {
                    //購読済みデバイス情報

                    //毎フレーム送られるため間引く
                    subscribeReceiveCnt++;
                    if (subscribeReceiveCnt > 30)
                    {
                        subscribeReceiveCnt = 0;
                        string serial = (string)message[0];
                        float x = (float)message[1];
                        float y = (float)message[2];
                        float z = (float)message[3];
                        float qx = (float)message[4];
                        float qy = (float)message[5];
                        float qz = (float)message[6];
                        float qw = (float)message[7];
                        this.Dispatcher.Invoke(() =>
                        {
                            string s = x + "," + y + "," + z + "," + qx + "," + qy + "," + qz + "," + qw;
                            subscribedDevicePoseDictionary[serial] = s;

                            SubscribedDeviceListView.Items.Clear();
                            foreach (var d in subscribedDevicePoseDictionary)
                            {
                                SubscribedDeviceListView.Items.Add(d.Key + " : " + d.Value);
                            }
                        });
                    }
                }
                else
                {
                    //Do noting
                }

            }
            catch (Exception ex)
            {
                ReceiveError = true;
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
                this.Dispatcher.Invoke(() =>
                {
                    Close(ex);
                });
                return;
            }
        }

        private void OnBundle(OscBundle bundle)
        {
            for (int i = 0; i < bundle.Count; i++)
            {
                switch (bundle[i])
                {
                    //Messageを受信した
                    case OscMessage msg:
                        OnMessage(msg);
                        break;
                    default:
                        //Do noting
                        break;
                }
            }
        }

        bool vibrationFlag = false;
        private void GenericTimer(object sender, EventArgs e)
        {
            try
            {
                HmdMatrix34_t m = new HmdMatrix34_t();
                OpenVR.ChaperoneSetup.GetWorkingStandingZeroPoseToRawTrackingPose(ref m);

                StatusBarDashboardAlertTextBlock.Visibility = OpenVR.Overlay.IsDashboardVisible() ? Visibility.Visible : Visibility.Collapsed;

                RoomMatrixTextBox.Text =
                    String.Format("{0:0.00}, {1:0.00}, {2:0.00}, {3:0.00}\n{4:0.00}, {5:0.00}, {6:0.00}, {7:0.00}\n{8:0.00}, {9:0.00}, {10:0.00}, {11:0.00}\n", m.m0, m.m1, m.m2, m.m3, m.m4, m.m5, m.m6, m.m7, m.m8, m.m9, m.m10, m.m11);
                if (
                    m.m0 == 0 &&
                    m.m1 == 0 &&
                    m.m2 == 0 &&
                    m.m3 == 0 &&
                    m.m4 == 0 &&
                    m.m5 == 0 &&
                    m.m6 == 0 &&
                    m.m7 == 0 &&
                    m.m8 == 0 &&
                    m.m9 == 0 &&
                    m.m10 == 0 &&
                    m.m11 == 0
                    )
                {
                    RoomMatrixTextBox.Background = new SolidColorBrush(Color.FromRgb(255, 100, 100));
                    SetRoomMatrixButtonName.IsEnabled = false;
                }
                else
                {
                    RoomMatrixTextBox.Background = new SolidColorBrush(Color.FromRgb(0, 255, 0));
                    SetRoomMatrixButtonName.IsEnabled = true;
                }


                var t1 = util.GetTransformBySerialNumber("VMT_0");
                if (t1 != null)
                {
                    string roomPos = string.Format("{0:0.00}, {1:0.00}, {2:0.00}", t1.position.X, t1.position.Y, t1.position.Z);

                    //Unity座標系ではZが反転する
                    if (CoordinateCombo.SelectedIndex == 1)
                    {
                        roomPos = string.Format("{0:0.00}, {1:0.00}, {2:0.00}", t1.position.X, t1.position.Y, -t1.position.Z);
                    }

                    CheckPositionTextBox.Text = roomPos;
                    if (roomPos == "1.00, 1.00, 1.00")
                    {
                        CheckPositionTextBox.Background = new SolidColorBrush(Color.FromRgb(0, 255, 0));
                    }
                    else
                    {
                        CheckPositionTextBox.Background = new SolidColorBrush(Color.FromRgb(255, 100, 100));
                    }

                    if (VMT_0_PositionVibrationCheckBox.IsChecked.Value) {
                        float vibration = vibrationFlag ? 0.005f : -0.005f;
                        vibrationFlag = !vibrationFlag;
                        osc.Send(new OscMessage("/VMT/Room/Driver",
                            0, 1, 0f,
                            (float)t1.position.X, (float)t1.position.Y, (float)(t1.position.Z + vibration),
                            (float)t1.rotation.X, (float)t1.rotation.Y, (float)t1.rotation.Z, (float)t1.rotation.W));
                    }
                }
                var t2 = util.GetTransformBySerialNumberRaw("VMT_0");
                if (t2 != null)
                {
                    CheckPositionRawTextBox.Text = string.Format("{0:0.00}, {1:0.00}, {2:0.00}", t2.position.X, t2.position.Y, t2.position.Z);
                    CheckPositionRawSpeedTextBox.Text = string.Format("{0:0.00}, {1:0.00}, {2:0.00}", t2.velocity.X, t2.velocity.Y, t2.velocity.Z);
                }

                if (aliveCnt > 90)
                {
                    installPath = "";
                    DriverVersion.Text = " - ";
                    DriverVersion.Foreground = new SolidColorBrush(Color.FromRgb(255, 255, 255));
                    ControlDock.IsEnabled = false;
                }
                else
                {
                    aliveCnt++;
                }

                InputVMTHapticTextBox.Background = new SolidColorBrush(Color.FromRgb(255, 255, 255));

                if (loadTest)
                {
                    ShowAllButton(null, null);
                }
            }
            catch (Exception ex)
            {
                dispatcherTimer.Stop(); //タイマー停止
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
                Close(ex);
                return;
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            //クローズ
            Console.WriteLine("Closed");
            if (osc != null)
            {
                osc.Dispose();
            }
        }

        private void ResetButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Reset"));
        }

        private void ReloadJsonButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/LoadSetting"));
        }

        private void SetRoomMatrixButton(object sender, RoutedEventArgs e)
        {
            HmdMatrix34_t m = new HmdMatrix34_t();
            OpenVR.ChaperoneSetup.GetWorkingStandingZeroPoseToRawTrackingPose(ref m);

            if (SetRoomMatrixTemporaryCheckBox.IsChecked.Value)
            {
                osc.Send(new OscMessage("/VMT/Set/RoomMatrix/Temporary",
                    m.m0, m.m1, m.m2, m.m3,
                    m.m4, m.m5, m.m6, m.m7,
                    m.m8, m.m9, m.m10, m.m11));
            }
            else
            {
                osc.Send(new OscMessage("/VMT/Set/RoomMatrix",
                    m.m0, m.m1, m.m2, m.m3,
                    m.m4, m.m5, m.m6, m.m7,
                    m.m8, m.m9, m.m10, m.m11));
            }
        }

        private void ResetRoomMatrixButton(object sender, RoutedEventArgs e)
        {
            if (SetRoomMatrixTemporaryCheckBox.IsChecked.Value)
            {
                osc.Send(new OscMessage("/VMT/Set/RoomMatrix/Temporary",
                    1f, 0f, 0f, 0f,
                    0f, 1f, 0f, 0f,
                    0f, 0f, 1f, 0f));
            }
            else
            {
                osc.Send(new OscMessage("/VMT/Set/RoomMatrix",
                    1f, 0f, 0f, 0f,
                    0f, 1f, 0f, 0f,
                    0f, 0f, 1f, 0f));
            }
        }
        private void CheckPositionButton(object sender, RoutedEventArgs e)
        {
            if (CoordinateCombo.SelectedIndex == 1)
            {
                osc.Send(new OscMessage("/VMT/Room/Unity",
                0, 1, 0f,
                1f, 1f, 1f,
                0f, 0f, 0f, 1f));
            }
            else
            {
                osc.Send(new OscMessage("/VMT/Room/Driver",
                0, 1, 0f,
                1f, 1f, 1f,
                0f, 0f, 0f, 1f));
            }
        }
        private void CheckPositionRawButton(object sender, RoutedEventArgs e)
        {
            if (CoordinateCombo.SelectedIndex == 1)
            {
                osc.Send(new OscMessage("/VMT/Raw/Unity",
                0, 1, 0f,
                1f, 1f, 1f,
                0f, 0f, 0f, 1f));
            }
            else
            {
                osc.Send(new OscMessage("/VMT/Raw/Driver",
                0, 1, 0f,
                1f, 1f, 1f,
                0f, 0f, 0f, 1f));
            }
        }
        private void CheckJointPositionButton(object sender, RoutedEventArgs e)
        {
            if (CoordinateCombo.SelectedIndex == 1)
            {
                osc.Send(new OscMessage("/VMT/Joint/Unity",
                    0, 1, 0f,
                    0.1f, 0f, -0.5f,
                    0f, 0f, 0f, 1f,
                    JointSerialNoTextBox.Text));
            }
            else
            {
                osc.Send(new OscMessage("/VMT/Joint/Driver",
                    0, 1, 0f,
                    0.1f, 0f, -0.5f,
                    0f, 0f, 0f, 1f,
                    JointSerialNoTextBox.Text));
            }
        }
        private void CheckJointPosition2Button(object sender, RoutedEventArgs e)
        {
            if (CoordinateCombo.SelectedIndex == 1)
            {
                osc.Send(new OscMessage("/VMT/Follow/Unity",
                    0, 1, 0f,
                    -0.1f, 0f, -0.5f,
                    0f, 0f, 0f, 1f,
                    JointSerialNoTextBox.Text));
            }
            else
            {
                osc.Send(new OscMessage("/VMT/Follow/Driver",
                    0, 1, 0f,
                    -0.1f, 0f, -0.5f,
                    0f, 0f, 0f, 1f,
                    JointSerialNoTextBox.Text));
            }
        }

        private void CheckTemplatePosition1Button(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Room/UEuler",
                1, 2, 0f,
                -0.15f, 0f, 0.26f,
                -60f, 60f, 0f));
            osc.Send(new OscMessage("/VMT/Room/UEuler",
                2, 3, 0f,
                0.15f, 0f, 0.26f,
                -60f, -60f, 0f));
        }
        private void CheckTemplatePosition1JointButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Joint/UEuler",
                1, 2, 0f,
                -0.15f, 0f, 0.26f,
                -60f, 60f, 0f,
                JointSerialNoTextBox.Text));
            osc.Send(new OscMessage("/VMT/Joint/UEuler",
                2, 3, 0f,
                0.15f, 0f, 0.26f,
                -60f, -60f, 0f,
                JointSerialNoTextBox.Text));
        }
        private void CheckTemplatePosition1FollowButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Follow/UEuler",
                1, 2, 0f,
                -0.15f, 0f, 0.26f,
                -60f, 60f, 0f,
                JointSerialNoTextBox.Text));
            osc.Send(new OscMessage("/VMT/Follow/UEuler",
                2, 3, 0f,
                0.15f, 0f, 0.26f,
                -60f, -60f, 0f,
                JointSerialNoTextBox.Text));
        }

        private async void InstallButton(object sender, RoutedEventArgs e)
        {
            await Install();
        }
        private async Task Install()
        {
            AllDockPanel.IsEnabled = false;
            try
            {
                if (installPath == "")
                {
                    //インストールパスが受信できていない場合少し待つ
                    TopErrorDockPanel.Visibility = Visibility.Visible;
                    TopErrorTextBlock.Text = "Please wait";
                    TopErrorDockPanel.Background = new SolidColorBrush(Color.FromRgb(255, 255, 0));
                    TopErrorTextBlock.Foreground = new SolidColorBrush(Color.FromRgb(0, 0, 0));
                    TopNotInstalledTextBlock.Visibility = Visibility.Collapsed;
                    for (int i = 0; i < 10; i++)
                    {
                        await Task.Delay(500);
                        TopErrorTextBlock.Text = TopErrorTextBlock.Text + ".";
                        if(DriverVersion.Text != " - ") { break; }
                    }
                    TopErrorDockPanel.Visibility = Visibility.Collapsed;
                }

                if (DriverVersion.Text != " - ")
                {
                    //MessageBox.Show("Please uninstall VMT before install.\nインストールを続ける前に、VMTをアンインストールしてください", title, MessageBoxButton.OK, MessageBoxImage.Error);
                    TopWarningMessage("Please uninstall VMT before install.\nインストールを続ける前に、VMTをアンインストールしてください", true);
                    AllDockPanel.IsEnabled = true;
                    return;
                }

                string driverPath_rel = System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt";
                string driverPath = System.IO.Path.GetFullPath(driverPath_rel);
                Console.WriteLine(OpenVR.RuntimePath() + @"\bin\win64");
                Console.WriteLine(driverPath);

                if (driverPath.Length > 100)
                {
                    MessageBox.Show("Path length over 100. it couldn't install.\nパスの長さが100を超えています。インストールできません。\n\n" + driverPath, title, MessageBoxButton.OK, MessageBoxImage.Error);
                    return;
                }

                if (driverPath.Contains("Program Files"))
                {
                    MessageBox.Show("Path contains \"Program Files\". it couldn't install.\nパスに\"Program Files\"が入っています。インストールできません。\n\n" + driverPath, title, MessageBoxButton.OK, MessageBoxImage.Error);
                    return;
                }

                if (driverPath.Contains(" "))
                {
                    MessageBox.Show("Path contains space. it couldn't install.\nパスにスペースが入っています。インストールできません。\n\n" + driverPath, title, MessageBoxButton.OK, MessageBoxImage.Error);
                    return;
                }

                if (!File.Exists(driverPath + @"\bin\win64\driver_vmt.dll"))
                {
                    MessageBox.Show("driver_vmt.dll not found. Do not break apart files. it couldn't install.\ndriver_vmt.dllが見つかりませんでした。ファイル構成をバラバラにしないでください。。インストールできません。\n\n" + driverPath, title, MessageBoxButton.OK, MessageBoxImage.Error);
                    return;
                }

                Encoding enc = Encoding.GetEncoding("us-ascii", new EncoderExceptionFallback(), new DecoderExceptionFallback());
                try
                {
                    enc.GetBytes(driverPath);
                }
                catch (EncoderFallbackException)
                {
                    MessageBox.Show("Path contains non-ascii (Japanese, Chinese, emoji, or other). it couldn't install.\nパスに非ASCII文字(日本語、中国語、絵文字、その他)が入っています。インストールできません。\n\n" + driverPath, title, MessageBoxButton.OK, MessageBoxImage.Error);
                    return;
                }

                System.Diagnostics.Process process = new System.Diagnostics.Process();
                process.StartInfo.WorkingDirectory = OpenVR.RuntimePath() + @"\bin\win64";
                process.StartInfo.FileName = OpenVR.RuntimePath() + @"\bin\win64\vrpathreg.exe";
                process.StartInfo.Arguments = "adddriver \"" + driverPath + "\"";
                process.StartInfo.UseShellExecute = false;
                process.Start();
                process.WaitForExit();

                MessageBox.Show("OK (ExitCode=" + process.ExitCode + ")\nPlease restart SteamVR.\nSteamVRを再起動してください。", title);
                Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
                Close(ex);
                return;
            }
        }
        private async void UninstallButton(object sender, RoutedEventArgs e) {
            await Uninstall();
        }

        private async Task Uninstall()
        {
            AllDockPanel.IsEnabled = false;
            try
            {
                string driverPath_rel = System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt";
                string driverPath = System.IO.Path.GetFullPath(driverPath_rel);

                if (installPath == "")
                {
                    //インストールパスが受信できていない場合少し待つ
                    TopErrorDockPanel.Visibility = Visibility.Visible;
                    TopErrorTextBlock.Text = "Please wait";
                    TopErrorDockPanel.Background = new SolidColorBrush(Color.FromRgb(255, 255, 0));
                    TopErrorTextBlock.Foreground = new SolidColorBrush(Color.FromRgb(0, 0, 0));
                    TopNotInstalledTextBlock.Visibility = Visibility.Collapsed;
                    for (int i = 0; i < 10; i++) {
                        await Task.Delay(500);
                        TopErrorTextBlock.Text = TopErrorTextBlock.Text + ".";
                        if (DriverVersion.Text != " - ") { break; }
                    }
                    TopErrorDockPanel.Visibility = Visibility.Collapsed;
                }

                if (installPath != "")
                {
                    //場所がわかっている
                    var res = MessageBox.Show("Uninstall VMT Driver?\nVMTドライバをアンインストールしますか?\n\n" + installPath, title, MessageBoxButton.OKCancel, MessageBoxImage.Question);
                    if (res != MessageBoxResult.OK)
                    {
                        AllDockPanel.IsEnabled = true;
                        return;
                    }
                    //現在のフォルダパスの代わりに、受信したパスでアンインストールを試す
                    driverPath = installPath;
                }
                else
                {
                    //場所不明だがとりあえず現在の場所としてアンインストールしようとするか確認する
                    var res = MessageBox.Show("Manager couldn't communication to VMT Driver.\nMaybe VMT is not installed\n Or other VMT Driver version maybe installed.\nIn many cases, this driver located on different path.\nmanager can not uninstallation these.\nIf you want to remove currently installed VMT driver, press cancel and please use manager of drivers itself.\n\nIf you want to try uninstallation anyway, press OK. (it will fail in many cases.)\n\nVMTドライバと通信できませんでした。\nVMTがまだインストールされていない\nもしくは違うバージョンのVMTドライバが入っている可能性があります。\n多くの場合これは別のパスにあります。\n本Managerではアンインストールできません。\nアンインストールしたい場合は、キャンセルを押し、そのVMTドライバに付属のManagerを使用してください\n\nOKを押すと、とにかくアンインストールを試します。(多くの場合失敗します。)", title, MessageBoxButton.OKCancel, MessageBoxImage.Error);
                    if (res != MessageBoxResult.OK)
                    {
                        AllDockPanel.IsEnabled = true;
                        return;
                    }
                }

                System.Diagnostics.Process process = new System.Diagnostics.Process();
                process.StartInfo.WorkingDirectory = OpenVR.RuntimePath() + @"\bin\win64";
                process.StartInfo.FileName = OpenVR.RuntimePath() + @"\bin\win64\vrpathreg.exe";
                process.StartInfo.Arguments = "removedriver \"" + driverPath + "\"";
                process.StartInfo.UseShellExecute = false;
                process.Start();
                process.WaitForExit();

                //Null Driverなし設定で上書き
                string fromPath = System.IO.Path.GetFullPath(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt\resources\settings\default.vrsettings_nohmd");
                string toPath = System.IO.Path.GetFullPath(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt\resources\settings\default.vrsettings");

                try
                {
                    File.WriteAllText(toPath, File.ReadAllText(fromPath, new UTF8Encoding(false)), new UTF8Encoding(false));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
                }


                if (File.Exists(driverPath + @"\setting.json"))
                {
                    var res = MessageBox.Show("Do you want to remove setting.json?(Recommended: Yes)\nSetting.jsonを消去しますか?(推奨: はい)\n\n" + installPath + @"\setting.json", title, MessageBoxButton.YesNo, MessageBoxImage.Question);
                    if (res != MessageBoxResult.Yes)
                    {
                        RequestRestart();
                        return;
                    }
                    File.Delete(driverPath + @"\setting.json");
                    RequestRestart();
                    Close();
                    return;
                }
                else
                {
                    RequestRestart();
                    Close();
                    return;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
                Close(ex);
                return;
            }
        }
        private void EnableNullHMDDriverButton(object sender, RoutedEventArgs e)
        {
            string fromPath = System.IO.Path.GetFullPath(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt\resources\settings\default.vrsettings_nullhmd");
            string toPath = System.IO.Path.GetFullPath(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt\resources\settings\default.vrsettings");

            try
            {
                File.WriteAllText(toPath, File.ReadAllText(fromPath, new UTF8Encoding(false)), new UTF8Encoding(false));

                if (DriverVersion.Text == " - ")
                {
                    var yesno = MessageBox.Show("Need VMT to apply null driver settings. Do you want to Install VMT?\nNull Driverの設定の反映にはVMTが必要です。インストールしますか?", title, MessageBoxButton.YesNo, MessageBoxImage.Information);
                    if (yesno == MessageBoxResult.Yes)
                    {
                        InstallButton(null, null);
                    }
                }
                else {
                    RequestRestart();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
            }

        }
        private void DisableNullHMDDriverButton(object sender, RoutedEventArgs e)
        {
            string fromPath = System.IO.Path.GetFullPath(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt\resources\settings\default.vrsettings_nohmd");
            string toPath = System.IO.Path.GetFullPath(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt\resources\settings\default.vrsettings");

            try
            {
                File.WriteAllText(toPath, File.ReadAllText(fromPath, new UTF8Encoding(false)), new UTF8Encoding(false));
                RequestRestart();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
            }
        }
        private void ShowAllButton(object sender, RoutedEventArgs e)
        {
            for (int i = 0; i < 58; i++)
            {
                osc.Send(new OscMessage("/VMT/Room/Driver",
                    i, 1, 0f,
                    (float)rnd.NextDouble() - 0.5f, (float)rnd.NextDouble() - 0.5f, (float)rnd.NextDouble() - 0.5f,
                    0f, 0f, 0f, 1f));
            }
        }

        private void TargetHMDButton(object sender, RoutedEventArgs e)
        {
            InputVMTNoTextBox.Text = "0";
        }
        private void TargetLeftButton(object sender, RoutedEventArgs e)
        {
            InputVMTNoTextBox.Text = "1";
        }
        private void TargetRightButton(object sender, RoutedEventArgs e)
        {
            InputVMTNoTextBox.Text = "2";
        }
        private (bool ok, int i) GetInputIndex()
        {
            int index = 0;
            if (InputVMTNoTextBox != null && int.TryParse(InputVMTNoTextBox?.Text, out index))
            {
                return (ok: true, i: index);
            }
            return (ok: true, i: index);
        }
        private void ButtonSend(int buttonIndex, bool press)
        {
            var index = GetInputIndex();
            int value = press ? 1 : 0;
            float timeoffset = 0;
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Input/Button",
                    index.i, buttonIndex, timeoffset, value));
            }
        }

        private void ButtonTouchSend(int buttonIndex, bool press)
        {
            var index = GetInputIndex();
            int value = press ? 1 : 0;
            float timeoffset = 0;
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Input/Button/Touch",
                    index.i, buttonIndex, timeoffset, value));
            }
        }
        private void TriggerClickSend(int buttonIndex, bool press)
        {
            var index = GetInputIndex();
            int value = press ? 1 : 0;
            float timeoffset = 0;
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger/Click",
                    index.i, buttonIndex, timeoffset, value));
            }
        }

        private void TriggerTouchSend(int buttonIndex, bool press)
        {
            var index = GetInputIndex();
            int value = press ? 1 : 0;
            float timeoffset = 0;
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger/Touch",
                    index.i, buttonIndex, timeoffset, value));
            }
        }
        private void JoystickClickSend(int buttonIndex, bool press)
        {
            var index = GetInputIndex();
            int value = press ? 1 : 0;
            float timeoffset = 0;
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Input/Joystick/Click",
                    index.i, buttonIndex, timeoffset, value));
            }
        }

        private void JoystickTouchSend(int buttonIndex, bool press)
        {
            var index = GetInputIndex();
            int value = press ? 1 : 0;
            float timeoffset = 0;
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Input/Joystick/Touch",
                    index.i, buttonIndex, timeoffset, value));
            }
        }

        private void Button0_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(0, true);
        }
        private void Button0_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(0, false);
        }
        private void Button1_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(1, true);
        }
        private void Button1_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(1, false);
        }
        private void Button2_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(2, true);
        }
        private void Button2_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(2, false);
        }
        private void Button3_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(3, true);
        }
        private void Button3_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(3, false);
        }
        private void Button4_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(4, true);
        }
        private void Button4_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(4, false);
        }
        private void Button5_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(5, true);
        }
        private void Button5_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(5, false);
        }
        private void Button6_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(6, true);
        }
        private void Button6_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(6, false);
        }
        private void Button7_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(7, true);
        }
        private void Button7_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(7, false);
        }
        private void Button8_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(8, true);
        }
        private void Button8_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(8, false);
        }
        private void Button9_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(9, true);
        }
        private void Button9_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(9, false);
        }
        private void Button10_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(10, true);
        }
        private void Button10_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(10, false);
        }
        private void Button11_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(11, true);
        }
        private void Button11_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(11, false);
        }
        private void Button12_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(12, true);
        }
        private void Button12_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(12, false);
        }
        private void Button13_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(13, true);
        }
        private void Button13_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(13, false);
        }
        private void Button14_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(14, true);
        }
        private void Button14_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(14, false);
        }
        private void Button15_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(15, true);
        }
        private void Button15_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(15, false);
        }
        private void Button16_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(16, true);
        }
        private void Button16_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(16, false);
        }
        private void Button17_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(17, true);
        }
        private void Button17_MouseUp(object sender, MouseButtonEventArgs e)
        {
            ButtonSend(17, false);
        }

        private void Slider0_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 0;
            float timeoffset = 0;
            if (index.ok && Slider0 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider0.Value));
            }
        }
        private void Slider1_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 1;
            float timeoffset = 0;
            if (index.ok && Slider1 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider1.Value));
            }
        }

        private void Slider2_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 2;
            float timeoffset = 0;
            if (index.ok && Slider2 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider2.Value));
            }
        }
        private void Slider3_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 3;
            float timeoffset = 0;
            if (index.ok && Slider3 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider3.Value));
            }
        }
        private void Slider4_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 4;
            float timeoffset = 0;
            if (index.ok && Slider4 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider4.Value));
            }
        }
        private void Slider5_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 5;
            float timeoffset = 0;
            if (index.ok && Slider5 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider5.Value));
            }
        }
        private void Slider6_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 6;
            float timeoffset = 0;
            if (index.ok && Slider6 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider6.Value));
            }
        }
        private void Slider7_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 7;
            float timeoffset = 0;
            if (index.ok && Slider7 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider7.Value));
            }
        }
        private void Slider8_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int triggerIndex = 8;
            float timeoffset = 0;
            if (index.ok && Slider8 != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Trigger",
                    index.i, triggerIndex, timeoffset, (float)Slider8.Value));
            }
        }


        private void SliderXY_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int joystickIndex = 0;
            float timeoffset = 0;
            if (index.ok && SliderX != null && SliderY != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Joystick",
                    index.i, joystickIndex, timeoffset, (float)SliderX.Value, (float)SliderY.Value));
            }
        }
        private void SliderUV_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int joystickIndex = 1;
            float timeoffset = 0;
            if (index.ok && SliderU != null && SliderV != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Joystick",
                    index.i, joystickIndex, timeoffset, (float)SliderU.Value, (float)SliderV.Value));
            }
        }
        private void SliderAB_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int joystickIndex = 2;
            float timeoffset = 0;
            if (index.ok && SliderA != null && SliderB != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Joystick",
                    index.i, joystickIndex, timeoffset, (float)SliderA.Value, (float)SliderB.Value));
            }
        }
        private void SliderCD_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int joystickIndex = 3;
            float timeoffset = 0;
            if (index.ok && SliderC != null && SliderD != null)
            {
                osc.Send(new OscMessage("/VMT/Input/Joystick",
                    index.i, joystickIndex, timeoffset, (float)SliderC.Value, (float)SliderD.Value));
            }
        }
        private void InputResetButton(object sender, RoutedEventArgs e)
        {
            Slider0.Value = 0f;
            Slider1.Value = 0f;
            Slider2.Value = 0f;
            Slider3.Value = 0f;
            Slider4.Value = 0f;
            Slider5.Value = 0f;
            Slider6.Value = 0f;
            Slider7.Value = 0f;
            Slider8.Value = 0f;
            SliderX.Value = 0f;
            SliderY.Value = 0f;
            SliderU.Value = 0f;
            SliderV.Value = 0f;
            SliderA.Value = 0f;
            SliderB.Value = 0f;
            SliderC.Value = 0f;
            SliderD.Value = 0f;
        }

        private void HapticTestButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                var deviceIndex = util.GetDeviceIndexBySerialNumber("VMT_" + index.i);
                if (deviceIndex != EasyOpenVRUtil.InvalidDeviceIndex)
                {
                    util.TriggerHapticPulse(deviceIndex);
                }
            }
        }
        private void DebugCommandButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Debug",
                    index.i, InputVMTDebugTextBox.Text));
                InputVMTDebugTextBox.Text = "";
            }
        }
        private void InputVMTDebugTextBox_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            //Enterキーを押したら送信
            if (e.Key == Key.Enter)
            {
                DebugCommandButton(null, null);
            }
        }
        private void RootAndWristSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int fingerIndex = 0;
            int blendMode = 0;
            float timeoffset = 0;
            if (index.ok && RootAndWristSlider != null)
            {
                osc.Send(new OscMessage("/VMT/Skeleton/Scalar",
                    index.i, fingerIndex, (float)RootAndWristSlider.Value, (int)blendMode, (int)(AxisLinkingEnableCheckBox.IsChecked.Value ? 1 : 0)));
                osc.Send(new OscMessage("/VMT/Skeleton/Apply",
                    index.i, timeoffset));
            }
        }
        private void ThumbSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int fingerIndex = 1;
            int blendMode = 0;
            float timeoffset = 0;
            if (index.ok && ThumbSlider != null)
            {
                osc.Send(new OscMessage("/VMT/Skeleton/Scalar",
                    index.i, fingerIndex, (float)ThumbSlider.Value, (int)blendMode, (int)(AxisLinkingEnableCheckBox.IsChecked.Value ? 1 : 0)));
                osc.Send(new OscMessage("/VMT/Skeleton/Apply",
                    index.i, timeoffset));
            }
        }
        private void IndexSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int fingerIndex = 2;
            int blendMode = 0;
            float timeoffset = 0;
            if (index.ok && IndexSlider != null)
            {
                osc.Send(new OscMessage("/VMT/Skeleton/Scalar",
                    index.i, fingerIndex, (float)IndexSlider.Value, (int)blendMode, (int)(AxisLinkingEnableCheckBox.IsChecked.Value ? 1 : 0)));
                osc.Send(new OscMessage("/VMT/Skeleton/Apply",
                    index.i, timeoffset));
            }
        }
        private void MiddleSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int fingerIndex = 3;
            int blendMode = 0;
            float timeoffset = 0;
            if (index.ok && MiddleSlider != null)
            {
                osc.Send(new OscMessage("/VMT/Skeleton/Scalar",
                    index.i, fingerIndex, (float)MiddleSlider.Value, (int)blendMode, (int)(AxisLinkingEnableCheckBox.IsChecked.Value ? 1 : 0)));
                osc.Send(new OscMessage("/VMT/Skeleton/Apply",
                    index.i, timeoffset));
            }
        }
        private void RingSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int fingerIndex = 4;
            int blendMode = 0;
            float timeoffset = 0;
            if (index.ok && RingSlider != null)
            {
                osc.Send(new OscMessage("/VMT/Skeleton/Scalar",
                    index.i, fingerIndex, (float)RingSlider.Value, (int)blendMode, (int)(AxisLinkingEnableCheckBox.IsChecked.Value ? 1 : 0)));
                osc.Send(new OscMessage("/VMT/Skeleton/Apply",
                    index.i, timeoffset));
            }
        }
        private void PinkyLittleSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var index = GetInputIndex();

            int fingerIndex = 5;
            int blendMode = 0;
            float timeoffset = 0;
            if (index.ok && PinkyLittleSlider != null)
            {
                osc.Send(new OscMessage("/VMT/Skeleton/Scalar",
                    index.i, fingerIndex, (float)PinkyLittleSlider.Value, (int)blendMode, (int)(AxisLinkingEnableCheckBox.IsChecked.Value ? 1 : 0)));
                osc.Send(new OscMessage("/VMT/Skeleton/Apply",
                    index.i, timeoffset));
            }
        }

        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            System.Diagnostics.Process.Start(e.Uri.AbsoluteUri);
        }
        private void TrackingReferenceButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Room/Unity",
                index.i, 4, 0f,
                0f, 0f, 0f,
                0f, 0f, 0f, 1f));
            }
        }

        private void RightHandButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Joint/UEuler",
                    index.i, 3, 0f,
                    0.15f, 0f, 0.26f,
                    -60f, -60f, 0f, "HMD"));
            }
        }

        private void RightHandCompatibleButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Joint/UEuler",
                    index.i, 6, 0f,
                    0.15f, 0f, 0.26f,
                    -60f, -60f, 0f, "HMD"));

            }
        }
        private void TrackerCompatibleButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Room/Unity",
                index.i, 7, 0f,
                0f, 0f, 0f,
                0f, 0f, 0f, 1f));
            }
        }

        private void LeftHandButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Joint/UEuler",
                    index.i, 2, 0f,
                    -0.15f, 0f, 0.26f,
                    -60f, 60f, 0f, "HMD"));
            }
        }

        private void LeftHandCompatibleButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Joint/UEuler",
                    index.i, 5, 0f,
                    -0.15f, 0f, 0.26f,
                    -60f, 60f, 0f, "HMD"));
            }
        }

        private void DisableButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Room/Unity",
                index.i, 0, 0f,
                0f, 0f, 0f,
                0f, 0f, 0f, 1f));
            }
        }

        private void TrackerButton(object sender, RoutedEventArgs e)
        {
            var index = GetInputIndex();
            if (index.ok)
            {
                osc.Send(new OscMessage("/VMT/Room/Unity",
                index.i, 1, 0f,
                0f, 0f, 0f,
                0f, 0f, 0f, 1f));
            }
        }

        private void EnableAutoPoseUdateButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Set/AutoPoseUpdate", 1));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableAutoPoseUdateButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Set/AutoPoseUpdate", 0));
            System.Media.SystemSounds.Beep.Play();
        }

        private void EnableDiaglogButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Set/DiagLog", 1));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableDiaglogButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Set/DiagLog", 0));
            System.Media.SystemSounds.Beep.Play();
        }

        private void EnableOptoutTrackingRoleButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "OptoutTrackingRole", "true"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableOptoutTrackingRoleButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "OptoutTrackingRole", "false"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void EnableRejectWhenCannotTrackingButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "RejectWhenCannotTracking", "true"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableRejectWhenCannotTrackingButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "RejectWhenCannotTracking", "false"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void EnableSkeletonInputButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "SkeletonInput", "true"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableSkeletonInputButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "SkeletonInput", "false"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void EnableAutoPoseUpdateOnStartupButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "AutoPoseUpdateOnStartup", "true"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableAutoPoseUpdateOnStartupButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "AutoPoseUpdateOnStartup", "false"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void EnableAddControllerOnStartupButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "AddControllerOnStartup", "true"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableAddControllerOnStartupButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "AddControllerOnStartup", "false"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void EnableAddCompatibleControllerOnStartupButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "AddCompatibleControllerOnStartup", "true"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableAddCompatibleControllerOnStartupButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "AddCompatibleControllerOnStartup", "false"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void EnableDiagLogOnStartupButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "DiagLogOnStartup", "true"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableDiagLogOnStartupButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "DiagLogOnStartup", "false"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void EnableAlwaysCompatibleButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "AlwaysCompatible", "true"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DisableAlwaysCompatibleButton(object sender, RoutedEventArgs e)
        {
            osc.Send(new OscMessage("/VMT/Config", "AlwaysCompatible", "false"));
            System.Media.SystemSounds.Beep.Play();
        }

        private void DeviceReloadButton(object sender, RoutedEventArgs e)
        {
            DeviceListView.Items.Clear();
            osc.Send(new OscMessage("/VMT/Get/Devices/List"));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DeviceCopyButton(object sender, RoutedEventArgs e)
        {
            if (DeviceListView.SelectedItem != null)
            {
                string s = (string)DeviceListView.SelectedItem;
                s = s.Split(':')[1].Trim();

                System.Windows.Clipboard.SetText(s);
            }
        }
        private void DeviceSubscribeButton(object sender, RoutedEventArgs e)
        {
            subscribedDevicePoseDictionary.Clear();
            osc.Send(new OscMessage("/VMT/Subscribe/Device", SubscribeDeviceSerialTextBox.Text));
            System.Media.SystemSounds.Beep.Play();
        }
        private void DeviceUnsubscribeButton(object sender, RoutedEventArgs e)
        {
            subscribedDevicePoseDictionary.Clear();
            osc.Send(new OscMessage("/VMT/Unsubscribe/Device", SubscribeDeviceSerialTextBox.Text));
            System.Media.SystemSounds.Beep.Play();
        }

        private void JsonLoadButton(object sender, RoutedEventArgs e)
        {
            string jsonPath_rel = System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt\setting.json";
            string jsonPath = System.IO.Path.GetFullPath(jsonPath_rel);

            try
            {
                JsonTextBox.Text = File.ReadAllText(jsonPath, new UTF8Encoding(false));
                System.Media.SystemSounds.Beep.Play();
                //MessageBox.Show("Loaded.", title, MessageBoxButton.OK, MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
            }

        }
        private void JsonSaveButton(object sender, RoutedEventArgs e)
        {
            string jsonPath_rel = System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location) + @"\..\vmt\setting.json";
            string jsonPath = System.IO.Path.GetFullPath(jsonPath_rel);
            try
            {
                File.WriteAllText(jsonPath, JsonTextBox.Text, new UTF8Encoding(false));
                System.Media.SystemSounds.Beep.Play();
                //MessageBox.Show("Saved.", title, MessageBoxButton.OK, MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\n" + ex.StackTrace, title);
            }
        }

        private void LoadTestCheckBoxEvent(object sender, RoutedEventArgs e)
        {
            loadTest = LoadTestCheckBox.IsChecked.Value;
        }

        private void InputPositionComboBox_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            if (InputPositionXTextBox != null)
            {
                InputPositionXTextBox.IsEnabled = false;
                InputPositionYTextBox.IsEnabled = false;
                InputPositionZTextBox.IsEnabled = false;
                InputPositionSerialTextBox.IsEnabled = false;
                InputPositionModeTextBox.IsEnabled = false;
                InputPositionQXTextBox.IsEnabled = false;
                InputPositionQYTextBox.IsEnabled = false;
                InputPositionQZTextBox.IsEnabled = false;
                InputPositionQWTextBox.IsEnabled = false;
                InputPositionAixisLinkTextBox.IsEnabled = false;
                InputPositionRXTextBox.IsEnabled = false;
                InputPositionRYTextBox.IsEnabled = false;
                InputPositionRZTextBox.IsEnabled = false;
                InputPositionBoneTextBox.IsEnabled = false;
                InputPositionValueTextBox.IsEnabled = false;

                var s = InputPositionComboBox.SelectedItem.ToString();
                if (s.Contains("Skeleton"))
                {
                    InputPositionBoneTextBox.IsEnabled = true;

                    if (s.Contains("Unity") || s.Contains("Driver"))
                    {
                        InputPositionXTextBox.IsEnabled = true;
                        InputPositionYTextBox.IsEnabled = true;
                        InputPositionZTextBox.IsEnabled = true;
                        InputPositionQXTextBox.IsEnabled = true;
                        InputPositionQYTextBox.IsEnabled = true;
                        InputPositionQZTextBox.IsEnabled = true;
                        InputPositionQWTextBox.IsEnabled = true;
                    }
                    if (s.Contains("UEuler"))
                    {
                        InputPositionXTextBox.IsEnabled = true;
                        InputPositionYTextBox.IsEnabled = true;
                        InputPositionZTextBox.IsEnabled = true;
                        InputPositionRXTextBox.IsEnabled = true;
                        InputPositionRYTextBox.IsEnabled = true;
                        InputPositionRZTextBox.IsEnabled = true;
                    }
                    if (s.Contains("Scalar"))
                    {
                        InputPositionModeTextBox.IsEnabled = true;
                        InputPositionAixisLinkTextBox.IsEnabled = true;
                    }
                }
                else
                {
                    if (s.Contains("Unity") || s.Contains("Driver"))
                    {
                        InputPositionXTextBox.IsEnabled = true;
                        InputPositionYTextBox.IsEnabled = true;
                        InputPositionZTextBox.IsEnabled = true;
                        InputPositionQXTextBox.IsEnabled = true;
                        InputPositionQYTextBox.IsEnabled = true;
                        InputPositionQZTextBox.IsEnabled = true;
                        InputPositionQWTextBox.IsEnabled = true;
                    }
                    if (s.Contains("UEuler"))
                    {
                        InputPositionXTextBox.IsEnabled = true;
                        InputPositionYTextBox.IsEnabled = true;
                        InputPositionZTextBox.IsEnabled = true;
                        InputPositionRXTextBox.IsEnabled = true;
                        InputPositionRYTextBox.IsEnabled = true;
                        InputPositionRZTextBox.IsEnabled = true;
                    }
                    if (s.Contains("Joint") || s.Contains("Follow"))
                    {
                        InputPositionSerialTextBox.IsEnabled = true;
                    }
                }
            }
        }

        private void InputPositionSendButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var index = GetInputIndex();
                if (!index.ok)
                {
                    MessageBox.Show("No target", title, MessageBoxButton.OK, MessageBoxImage.Error);
                    return;
                }

                int enable = int.Parse(InputPositionEnableTextBox.Text);
                float x = float.Parse(InputPositionXTextBox.Text);
                float y = float.Parse(InputPositionYTextBox.Text);
                float z = float.Parse(InputPositionZTextBox.Text);
                string serial = InputPositionSerialTextBox.Text;
                int mode = int.Parse(InputPositionModeTextBox.Text);
                float qx = float.Parse(InputPositionQXTextBox.Text);
                float qy = float.Parse(InputPositionQYTextBox.Text);
                float qz = float.Parse(InputPositionQZTextBox.Text);
                float qw = float.Parse(InputPositionQWTextBox.Text);
                int axisLink = int.Parse(InputPositionAixisLinkTextBox.Text);
                float rx = float.Parse(InputPositionRXTextBox.Text);
                float ry = float.Parse(InputPositionRYTextBox.Text);
                float rz = float.Parse(InputPositionRZTextBox.Text);
                float v = float.Parse(InputPositionValueTextBox.Text);
                int bone = int.Parse(InputPositionBoneTextBox.Text);

                var s = InputPositionComboBox.Text;
                switch (s)
                {
                    case "/VMT/Room/Unity":
                        osc.Send(new OscMessage("/VMT/Room/Unity",
                            index.i, enable, 0f,
                            x, y, z,
                            qx, qy, qz, qw));
                        break;
                    case "/VMT/Room/UEuler":
                        osc.Send(new OscMessage("/VMT/Room/UEuler",
                            index.i, enable, 0f,
                            x, y, z,
                            rx, ry, rz));
                        break;
                    case "/VMT/Room/Driver":
                        osc.Send(new OscMessage("/VMT/Room/Driver",
                            index.i, enable, 0f,
                            x, y, z,
                            qx, qy, qz, qw));
                        break;
                    case "/VMT/Raw/Unity":
                        osc.Send(new OscMessage("/VMT/Raw/Unity",
                            index.i, enable, 0f,
                            x, y, z,
                            qx, qy, qz, qw));
                        break;
                    case "/VMT/Raw/UEuler":
                        osc.Send(new OscMessage("/VMT/Raw/UEuler",
                            index.i, enable, 0f,
                            x, y, z,
                            rx, ry, rz));
                        break;
                    case "/VMT/Raw/Driver":
                        osc.Send(new OscMessage("/VMT/Raw/Driver",
                            index.i, enable, 0f,
                            x, y, z,
                            qx, qy, qz, qw));
                        break;
                    case "/VMT/Joint/Unity":
                        osc.Send(new OscMessage("/VMT/Joint/Unity",
                            index.i, enable, 0f,
                            x, y, z,
                            qx, qy, qz, qw, serial));
                        break;
                    case "/VMT/Joint/UEuler":
                        osc.Send(new OscMessage("/VMT/Joint/UEuler",
                            index.i, enable, 0f,
                            x, y, z,
                            rx, ry, rz, serial));
                        break;
                    case "/VMT/Joint/Driver":
                        osc.Send(new OscMessage("/VMT/Joint/Driver",
                            index.i, enable, 0f,
                            x, y, z,
                            qx, qy, qz, qw, serial));
                        break;
                    case "/VMT/Follow/Unity":
                        osc.Send(new OscMessage("/VMT/Follow/Unity",
                            index.i, enable, 0f,
                            x, y, z,
                            qx, qy, qz, qw, serial));
                        break;
                    case "/VMT/Follow/UEuler":
                        osc.Send(new OscMessage("/VMT/Follow/UEuler",
                            index.i, enable, 0f,
                            x, y, z,
                            rx, ry, rz, serial));
                        break;
                    case "/VMT/Follow/Driver":
                        osc.Send(new OscMessage("/VMT/Follow/Driver",
                            index.i, enable, 0f,
                            x, y, z,
                            qx, qy, qz, qw, serial));
                        break;
                    case "/VMT/Skeleton/Unity":
                        osc.Send(new OscMessage("/VMT/Skeleton/Unity",
                            index.i, bone,
                            x, y, z,
                            qx, qy, qz, qw));
                        break;
                    case "/VMT/Skeleton/UEuler":
                        osc.Send(new OscMessage("/VMT/Skeleton/UEuler",
                            index.i, bone,
                            x, y, z,
                            rx, ry, rz));
                        break;
                    case "/VMT/Skeleton/Driver":
                        osc.Send(new OscMessage("/VMT/Skeleton/Driver",
                            index.i, bone,
                            x, y, z,
                            qx, qy, qz, qw));
                        break;
                    case "/VMT/Skeleton/Scalar":
                        osc.Send(new OscMessage("/VMT/Skeleton/Scalar",
                            index.i, bone, v,
                            mode, axisLink));
                        break;
                    case "/VMT/Skeleton/Apply":
                        osc.Send(new OscMessage("/VMT/Skeleton/Apply",
                            index.i, 0f));
                        break;
                    default:
                        MessageBox.Show("Not found", title, MessageBoxButton.OK, MessageBoxImage.Error);
                        break;
                }
                System.Media.SystemSounds.Beep.Play();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, title, MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void Button0TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(0, Button0TouchCheckBox.IsChecked.Value);
        }
        private void Button1TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(1, Button1TouchCheckBox.IsChecked.Value);
        }
        private void Button2TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(2, Button2TouchCheckBox.IsChecked.Value);
        }
        private void Button3TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(3, Button3TouchCheckBox.IsChecked.Value);
        }
        private void Button4TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(4, Button4TouchCheckBox.IsChecked.Value);
        }
        private void Button5TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(5, Button5TouchCheckBox.IsChecked.Value);
        }
        private void Button6TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(6, Button6TouchCheckBox.IsChecked.Value);
        }
        private void Button7TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(7, Button7TouchCheckBox.IsChecked.Value);
        }
        private void Button8TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(8, Button8TouchCheckBox.IsChecked.Value);
        }
        private void Button9TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(9, Button9TouchCheckBox.IsChecked.Value);
        }
        private void Button10TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(10, Button10TouchCheckBox.IsChecked.Value);
        }
        private void Button11TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(11, Button11TouchCheckBox.IsChecked.Value);
        }
        private void Button12TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(12, Button12TouchCheckBox.IsChecked.Value);
        }
        private void Button13TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(13, Button13TouchCheckBox.IsChecked.Value);
        }
        private void Button14TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(14, Button14TouchCheckBox.IsChecked.Value);
        }
        private void Button15TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(15, Button15TouchCheckBox.IsChecked.Value);
        }
        private void Button16TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(16, Button16TouchCheckBox.IsChecked.Value);
        }
        private void Button17TouchChecked(object sender, RoutedEventArgs e)
        {
            ButtonTouchSend(17, Button17TouchCheckBox.IsChecked.Value);
        }

        private void Trigger0TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(0, Trigger0TouchCheckBox.IsChecked.Value);
        }
        private void Trigger1TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(1, Trigger1TouchCheckBox.IsChecked.Value);
        }
        private void Trigger2TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(2, Trigger2TouchCheckBox.IsChecked.Value);
        }
        private void Trigger3TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(3, Trigger3TouchCheckBox.IsChecked.Value);
        }
        private void Trigger4TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(4, Trigger4TouchCheckBox.IsChecked.Value);
        }
        private void Trigger5TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(5, Trigger5TouchCheckBox.IsChecked.Value);
        }
        private void Trigger6TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(6, Trigger6TouchCheckBox.IsChecked.Value);
        }
        private void Trigger7TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(7, Trigger7TouchCheckBox.IsChecked.Value);
        }
        private void Trigger8TouchChecked(object sender, RoutedEventArgs e)
        {
            TriggerTouchSend(8, Trigger8TouchCheckBox.IsChecked.Value);
        }

        private void Trigger0ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(0, Trigger0ClickCheckBox.IsChecked.Value);
        }
        private void Trigger1ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(1, Trigger1ClickCheckBox.IsChecked.Value);
        }
        private void Trigger2ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(2, Trigger2ClickCheckBox.IsChecked.Value);
        }
        private void Trigger3ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(3, Trigger3ClickCheckBox.IsChecked.Value);
        }
        private void Trigger4ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(4, Trigger4ClickCheckBox.IsChecked.Value);
        }
        private void Trigger5ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(5, Trigger5ClickCheckBox.IsChecked.Value);
        }
        private void Trigger6ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(6, Trigger6ClickCheckBox.IsChecked.Value);
        }
        private void Trigger7ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(7, Trigger7ClickCheckBox.IsChecked.Value);
        }
        private void Trigger8ClickChecked(object sender, RoutedEventArgs e)
        {
            TriggerClickSend(8, Trigger8ClickCheckBox.IsChecked.Value);
        }


        private void Joystick0TouchChecked(object sender, RoutedEventArgs e)
        {
            JoystickTouchSend(0, Joystick0TouchCheckBox.IsChecked.Value);
        }
        private void Joystick1TouchChecked(object sender, RoutedEventArgs e)
        {
            JoystickTouchSend(1, Joystick1TouchCheckBox.IsChecked.Value);
        }
        private void Joystick2TouchChecked(object sender, RoutedEventArgs e)
        {
            JoystickTouchSend(2, Joystick2TouchCheckBox.IsChecked.Value);
        }
        private void Joystick3TouchChecked(object sender, RoutedEventArgs e)
        {
            JoystickTouchSend(3, Joystick3TouchCheckBox.IsChecked.Value);
        }


        private void Joystick0ClickChecked(object sender, RoutedEventArgs e)
        {
            JoystickClickSend(0, Joystick0ClickCheckBox.IsChecked.Value);
        }
        private void Joystick1ClickChecked(object sender, RoutedEventArgs e)
        {
            JoystickClickSend(1, Joystick1ClickCheckBox.IsChecked.Value);
        }
        private void Joystick2ClickChecked(object sender, RoutedEventArgs e)
        {
            JoystickClickSend(2, Joystick2ClickCheckBox.IsChecked.Value);
        }
        private void Joystick3ClickChecked(object sender, RoutedEventArgs e)
        {
            JoystickClickSend(3, Joystick3ClickCheckBox.IsChecked.Value);
        }

        private void TrackingOverridesAddButton_Click(object sender, RoutedEventArgs e)
        {
            if (TrackingOverridesFromTextBox.Text == "") { return; }
            if (TrackingOverridesToTextBox.Text == "") { return; }

            EVRSettingsError eVRSettingsError = EVRSettingsError.None;
            OpenVR.Settings.SetString(OpenVR.k_pch_TrackingOverride_Section, TrackingOverridesFromTextBox.Text, TrackingOverridesToTextBox.Text, ref eVRSettingsError);
            if (eVRSettingsError != EVRSettingsError.None)
            {
                MessageBox.Show("Error", title, MessageBoxButton.OK, MessageBoxImage.Error);
            }
            else
            {
                RequestRestart();
            }
        }
        private void TrackingOverridesRemoveAllButton_Click(object sender, RoutedEventArgs e)
        {
            var yesno = MessageBox.Show("Are you sure? (Remove All)", title, MessageBoxButton.YesNo, MessageBoxImage.Question);
            if (yesno == MessageBoxResult.Yes)
            {
                EVRSettingsError eVRSettingsError = EVRSettingsError.None;
                OpenVR.Settings.RemoveSection(OpenVR.k_pch_TrackingOverride_Section, ref eVRSettingsError);
                if (eVRSettingsError != EVRSettingsError.None)
                {
                    MessageBox.Show("Error", title, MessageBoxButton.OK, MessageBoxImage.Error);
                }
                else
                {
                    RequestRestart();
                }
            }
        }

        private void IPAddressChangeButton(object sender, RoutedEventArgs e)
        {
            try { 
                osc.Dispose();

                osc = new OSC(DriverIPAddressTextBox.Text, int.Parse(ManagerPortTextBox.Text), int.Parse(DriverPortTextBox.Text), OnBundle, OnMessage);
                osc.Send(new OscMessage("/VMT/Set/Destination", ManagerIPAddressTextBox.Text, int.Parse(ManagerPortTextBox.Text)));
                System.Media.SystemSounds.Beep.Play();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, title, MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void RequestRestartButton(object sender, RoutedEventArgs e)
        {
            RequestRestart();
        }
        

        private void RequestRestart()
        {
            var yesno = MessageBox.Show("SteamVR Restart required to apply. Restart now?\n適用にはSteamVRの再起動が必要です。今再起動しますか?", title, MessageBoxButton.YesNo, MessageBoxImage.Information);
            if (yesno == MessageBoxResult.Yes)
            {
                osc.Send(new OscMessage("/VMT/RequestRestart"));
                Close();
            }
        }
    }
}
