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
        int cnt = 0;
        Random rnd;

        EasyOpenVRUtil util;

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
        }

        private void GenericTimer(object sender, EventArgs e)
        {
            var read = sharedMemory.ReadStringD2M();
            if (read.Length > 0) {
                var baseObject = JsonSerializer.Deserialize<Communication.Base>(read);
                if (baseObject.type == "Hello")
                {
                    var hello = JsonSerializer.Deserialize<Communication.Hello>(baseObject.json);
                    Console.WriteLine("[" + baseObject.type + "]" + hello.msg);
                }
            }

            EasyOpenVRUtil.Transform t = util.GetLeftControllerTransform();
            if (t != null) {
                Console.WriteLine(t.ToString());

                Vector3 rot = Vector3.Transform(Vector3.One, t.rotation);
                Console.WriteLine(rot.ToString());


                sharedMemory.WriteStringM2D(JsonSerializer.Serialize(new Communication.Base
                {
                    type = "Pos",
                    json = JsonSerializer.Serialize(new Communication.Pos
                    {
                        x = t.position.X,
                        y = t.position.Y,
                        z = t.position.Z,
                        qx = t.rotation.X,
                        qy = t.rotation.Y,
                        qz = t.rotation.Z,
                        qw = t.rotation.W,
                    })
                }));;

            }
        }


        private void Window_Closed(object sender, EventArgs e)
        {
            //クローズ
            Console.WriteLine("Closed");
        }
    }
}
