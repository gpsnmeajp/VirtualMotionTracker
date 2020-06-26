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
using System.Threading;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using Rug.Osc;

class OSC : IDisposable
{
    //イベント打ち上げ用
    public Action<OscBundle> OnBundle = null;
    public Action<OscMessage> OnMessage = null;

    public long PacketCounter = 0;

    OscReceiver oscReceiver = null;
    OscSender oscSender = null;
    Thread thread = null;
    public int Port;

    //受信待受開始
    public OSC(string adr, int portRx, int portTx, Action<OscBundle> OnBundle, Action<OscMessage> OnMessage)
    {
        this.Port = portRx;
        this.OnBundle = OnBundle;
        this.OnMessage = OnMessage;

        //送信
        IPAddress ip = IPAddress.Parse(adr);
        oscSender = new OscSender(ip, 0, portTx);
        oscSender.Connect();

        //受信待受
        oscReceiver = new OscReceiver(portRx);
        oscReceiver.Connect();

        //受信処理スレッド
        thread = new Thread(new ThreadStart(ReceiveThread));
        thread.Start();

        //例外は上位に打ち上げる
    }

    //受信待受停止
    public void Dispose()
    {
        //待受停止
        try
        {
            oscReceiver?.Close();
            oscSender?.Close();
        }
        finally
        {
            //Thread終了を1500msの間待機
            thread?.Join(1500);
        }
    }

    public void Send(OscPacket packet)
    {
        oscSender.Send(packet);
    }

    //受信Thread
    private void ReceiveThread()
    {
        try
        {
            //ソケットが閉じていない間受信を続ける
            while (oscReceiver.State != OscSocketState.Closed)
            {
                //接続状態なら処理を行う
                if (oscReceiver.State == OscSocketState.Connected)
                {
                    //受信を行う
                    var packet = oscReceiver.Receive();
                    ProcessPacket(packet);
                }
                else
                {
                    //接続されていない場合は待つ
                    Thread.Sleep(4);
                }
            }

        }
        catch (Exception e)
        {
            //ソケットが閉じられた例外のときは無視する
            if (e.Message != "The receiver socket has been disconnected")
            {
                Console.WriteLine("# ReceiveThread : " + e);
            }
        }
    }
    //パケットを処理して、bundleとMessageに振り分け
    private void ProcessPacket(OscPacket packet)
    {
        PacketCounter++;
        switch (packet)
        {
            //bundleを受信した
            case OscBundle bundle:
                try
                {
                    OnBundle?.Invoke(bundle);
                }
                catch (Exception e)
                {
                    Console.WriteLine("# Callback : " + e);
                }
                break;
            //Messageを受信した
            case OscMessage msg:
                try
                {
                    OnMessage?.Invoke(msg);
                }
                catch (Exception e)
                {
                    Console.WriteLine("# Callback : " + e);
                }
                break;
            default:
                //Do noting
                break;
        }
    }
}