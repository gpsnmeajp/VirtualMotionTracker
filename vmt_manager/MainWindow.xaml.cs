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
using Rug.Osc;
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
        OSC osc;

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

            osc = new OSC(39530,OnBundle,OnMessage);

            HmdMatrix34_t m3 = new HmdMatrix34_t();
            OpenVR.ChaperoneSetup.GetWorkingStandingZeroPoseToRawTrackingPose(ref m3);
            Console.WriteLine(util.HmdMatrix34ToMatrix4x4(m3));
        }

        private void OnMessage(OscMessage message)
        {
            /*
             * ルーム座標系からドライバ座標系に変換して送る
            Console.WriteLine("## " + message);
            if (message.Address == "/tra") {
                int idx = (int)message[0];
                bool enable = (int)message[1]!=0;
                Vector3 position = new Vector3((float)message[2], (float)message[3], (float)message[4]);
                position.Z = -position.Z; //Unity座標系からドライバ座標系へ
                Quaternion rotation = new Quaternion((float)message[5], (float)message[6], (float)message[7], (float)message[8]);
                rotation.Z = -rotation.Z;
                rotation.W = -rotation.W;

                HmdMatrix34_t m3 = new HmdMatrix34_t();
                OpenVR.ChaperoneSetup.GetWorkingStandingZeroPoseToRawTrackingPose(ref m3);
                Console.WriteLine(m3);

                Matrix4x4 pos = Matrix4x4.CreateTranslation(position);
                Matrix4x4 rot = Matrix4x4.CreateFromQuaternion(rotation);
                Matrix4x4 posrot = rot * pos;
                Matrix4x4 m = posrot * util.HmdMatrix34ToMatrix4x4(m3);
                EasyOpenVRUtil.Transform t2 = util.Matrix4x4ToTransform(m);

                Console.WriteLine(position);
                Console.WriteLine(rotation);

                sharedMemory.Write(JsonSerializer.Serialize(new Communication.Base
                {
                    type = "Pos",
                    json = JsonSerializer.Serialize(new Communication.Pos
                    {
                        idx = idx,
                        en = enable,
                        x = t2.position.X,
                        y = t2.position.Y,
                        z = t2.position.Z,
                        qx = t2.rotation.X,
                        qy = t2.rotation.Y,
                        qz = t2.rotation.Z,
                        qw = t2.rotation.W,
                    })
                })); ;
            }
            */
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

        private void GenericTimer(object sender, EventArgs e)
        {
            HmdMatrix34_t m3 = new HmdMatrix34_t();
            OpenVR.ChaperoneSetup.GetWorkingStandingZeroPoseToRawTrackingPose(ref m3);
            Console.WriteLine("###");
            Console.WriteLine(util.HmdMatrix34ToMatrix4x4(m3));
            Console.WriteLine("###");

            do
            {
                string read = sharedMemory.Read();
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

            //ルーム座標をそのまま送る(ドライバ側で変換されるはず...)
            EasyOpenVRUtil.Transform t = util.GetLeftControllerTransform();
            if (t != null)
            {
                Console.WriteLine(t);

                sharedMemory.Write(JsonSerializer.Serialize(new Communication.Base
                {
                    type = "Pos",
                    json = JsonSerializer.Serialize(new Communication.Pos
                    {
                        idx = 0,
                        en = true,
                        x = t.position.X,
                        y = t.position.Y,
                        z = t.position.Z,
                        qx = t.rotation.X,
                        qy = t.rotation.Y,
                        qz = t.rotation.Z,
                        qw = t.rotation.W,
                    })
                })); ;

            }


            /*
            EasyOpenVRUtil.Transform t = util.GetLeftControllerTransform();
            if (t != null)
            {
                Console.WriteLine(t);

                HmdMatrix34_t m3 = new HmdMatrix34_t();
                OpenVR.ChaperoneSetup.GetWorkingStandingZeroPoseToRawTrackingPose(ref m3);

                Matrix4x4 pos = Matrix4x4.CreateTranslation(t.position);

                Quaternion q = t.rotation;
                Matrix4x4 rot = Matrix4x4.CreateFromQuaternion(q);
                Matrix4x4 posrot = rot* pos;
                Console.WriteLine(pos);
                Console.WriteLine(rot);
                Console.WriteLine(posrot);
                Console.WriteLine(t.matrix4X4);

                Matrix4x4 m = posrot*util.HmdMatrix34ToMatrix4x4(m3);

                EasyOpenVRUtil.Transform t2 = util.Matrix4x4ToTransform(m);
                Console.WriteLine(t2);

                sharedMemory.Write(JsonSerializer.Serialize(new Communication.Base
                {
                    type = "Pos",
                    json = JsonSerializer.Serialize(new Communication.Pos
                    {
                        idx = 0,
                        en = true,
                        x = t2.position.X,
                        y = t2.position.Y,
                        z = t2.position.Z,
                        qx = t2.rotation.X,
                        qy = t2.rotation.Y,
                        qz = t2.rotation.Z,
                        qw = t2.rotation.W,
                    })
                })); ;

            }
            else {
                sharedMemory.Write(JsonSerializer.Serialize(new Communication.Base
                {
                    type = "Pos",
                    json = JsonSerializer.Serialize(new Communication.Pos
                    {
                        idx = 0,
                        en = true,
                        x = rnd.NextDouble(),
                        y = rnd.NextDouble(),
                        z = rnd.NextDouble(),
                        qx = 0,
                        qy = 0,
                        qz = 0,
                        qw = 1,
                    })
                }));
            }
            sharedMemory.Write(JsonSerializer.Serialize(new Communication.Base
            {
                type = "Pos",
                json = JsonSerializer.Serialize(new Communication.Pos
                {
                    idx = 1,
                    en = true,
                    x = rnd.NextDouble(),
                    y = rnd.NextDouble(),
                    z = rnd.NextDouble(),
                    qx = 0,
                    qy = 0,
                    qz = 0,
                    qw = 1,
                })
            }));
            sharedMemory.Write(JsonSerializer.Serialize(new Communication.Base
            {
                type = "Pos",
                json = JsonSerializer.Serialize(new Communication.Pos
                {
                    idx = 2,
                    en = false,
                    x = rnd.NextDouble(),
                    y = rnd.NextDouble(),
                    z = rnd.NextDouble(),
                    qx = 0,
                    qy = 0,
                    qz = 0,
                    qw = 1,
                })
            }));
            */
        }


        private void Window_Closed(object sender, EventArgs e)
        {
            //クローズ
            Console.WriteLine("Closed");

            for(int i = 0; i < 16; i++) {
                sharedMemory.Write(JsonSerializer.Serialize(new Communication.Base
                {
                    type = "Pos",
                    json = JsonSerializer.Serialize(new Communication.Pos
                    {
                        idx = i,
                        en = false,
                        x = 0,
                        y = 0,
                        z = 0,
                        qx = 0,
                        qy = 0,
                        qz = 0,
                        qw = 1,
                    })
                }));
            }
            osc.Dispose();
            Thread.Sleep(500);
        }
    }
}
