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
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Collections;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using Valve.VR;
using EasyLazyLibrary;
using System.Numerics;
namespace vmt_manager
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>

    /*
     * メモリマップ
     * 0x00: 送信カウンタ(→)
     * 0x01: ハンドシェイクカウンタ(←)
     * 0x10-0x13: 32bitデータサイズ(→)
     * 0x20～: 文字列領域(→)
     */
public partial class MainWindow : Window
    {
        private DispatcherTimer dispatcherTimer;
        SharedMemory sharedMemory;
        Random rnd;

        Vector3 offsetPos;
        Quaternion offsetRot;

        EasyOpenVRUtil util;

        Task CommunicationThread;
        bool CommunicationThreadExit = false;
        Queue<string> ReadQueue = new Queue<string>();
        Queue<string> WriteQueue = new Queue<string>();
        Object CommunicationThreadLockObject = new Object();

        void Write(string s)
        {
            lock (CommunicationThreadLockObject)
            {
                if (WriteQueue.Count < 1024) //1024件以上は捨てる(異常時、通信不良時)
                {
                    WriteQueue.Enqueue(s);
                }
            }
        }
        string Read()
        {
            string read = "";
            lock (CommunicationThreadLockObject)
            {
                if (ReadQueue.Count > 0)
                {
                    read = ReadQueue.Dequeue();
                }
            }
            return read;
        }

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            sharedMemory = new SharedMemory();
            Console.WriteLine("Loaded");

            dispatcherTimer = new DispatcherTimer();
            dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            dispatcherTimer.Tick += new EventHandler(GenericTimer);
            dispatcherTimer.Start();

            rnd = new Random();

            util = new EasyOpenVRUtil();
            util.StartOpenVR();

            EasyOpenVRUtil.Transform t = util.GetTransformBySerialNumber("Hello Device0");
            if (t != null)
            {
                offsetPos = t.position;
                offsetRot = t.rotation;
                Console.WriteLine(t);
            }

            CommunicationThread = Task.Run(() =>
            {
                while (!CommunicationThreadExit) {
                    lock (CommunicationThreadLockObject)
                    {
                        try
                        {
                            //送信キューにデータが有る場合
                            if (WriteQueue.Count > 0)
                            {
                                if (sharedMemory.WriteStringM2D(WriteQueue.Peek()))
                                {
                                    //送信成功したら引き抜き、そのまま続ける
                                    WriteQueue.Dequeue();
                                    continue;
                                }
                                //書き込み失敗時は次の周期まで待つ(相手が起動していないか、相手がいっぱいいっぱい)
                            }

                            //受信データが有る場合(かつリミッター以下の場合)
                            string r = sharedMemory.ReadStringD2M();
                            if (r.Length > 0 && ReadQueue.Count<1024)
                            {
                                ReadQueue.Enqueue(r);
                                continue;
                            }
                        }
                        catch (Exception ex) {
                            Console.WriteLine(ex.ToString());
                        }
                    }
                    Thread.Sleep(4);
                }
            });
        }

        private void GenericTimer(object sender, EventArgs e)
        {

            do
            {
                string read = Read();
                if (read.Length < 1)
                {
                    break;
                }
                var baseObject = JsonSerializer.Deserialize<Communication.Base>(read);
                if (baseObject.type == "Hello")
                {
                    var hello = JsonSerializer.Deserialize<Communication.Hello>(baseObject.json);
                    Console.WriteLine("[" + baseObject.type + "]" + hello.msg);
                }
            } while (true);

            EasyOpenVRUtil.Transform t = util.GetLeftControllerTransform();
            if (t != null) {
                Console.WriteLine(t);

                HmdMatrix34_t m3 = new HmdMatrix34_t();
                OpenVR.ChaperoneSetup.GetWorkingStandingZeroPoseToRawTrackingPose(ref m3);

                Matrix4x4 m = util.HmdMatrix34ToMatrix4x4(m3)* t.matrix4X4;

                EasyOpenVRUtil.Transform t2 = util.Matrix4x4ToTransform(m);
                Console.WriteLine(t2);

                Write(JsonSerializer.Serialize(new Communication.Base
                {
                    type = "Pos",
                    json = JsonSerializer.Serialize(new Communication.Pos
                    {
                        x = t2.position.X,
                        y = t2.position.Y,
                        z = t2.position.Z,
                        qx = t2.rotation.X,
                        qy = t2.rotation.Y,
                        qz = t2.rotation.Z,
                        qw = t2.rotation.W,
                    })
                }));;

            }
        }


        private void Window_Closed(object sender, EventArgs e)
        {
            //クローズ
            Console.WriteLine("Closed");

            Write(JsonSerializer.Serialize(new Communication.Base
            {
                type = "Pos",
                json = JsonSerializer.Serialize(new Communication.Pos
                {
                    x = 0,
                    y = 0,
                    z = 0,
                    qx = 0,
                    qy = 0,
                    qz = 0,
                    qw = 1,
                })
            })); ;
            Thread.Sleep(10);
            CommunicationThreadExit = true;
            CommunicationThread.Wait();
        }
    }
}
