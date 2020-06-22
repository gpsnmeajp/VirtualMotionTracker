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
using System.Collections;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

public class SharedMemory : IDisposable
{
    const string SHARED_MEMORY_M2D = "VirtualMotionTracker_M2D";
    const string SHARED_MEMORY_D2M = "VirtualMotionTracker_D2M";

    const int SHARED_MEMORY_SIZE = 1 * 1024 * 1024; //1MB
    const int SHARED_MEMORY_STRING_AREA_SIZE = SHARED_MEMORY_SIZE - SHARED_MEMORY_ADDRESS_STRING_AREA -1; //1MB

    const int SHARED_MEMORY_ADDRESS_COUNTER_SEND = 0x00;
    const int SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE = 0x01;

    const int SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA = 0x10; //0x10-0x13
    const int SHARED_MEMORY_ADDRESS_STRING_AREA = 0x20;

    MemoryMappedFile memoryMappedFileM2D;
    MemoryMappedViewAccessor memoryMappedViewAccessorM2D;

    MemoryMappedFile memoryMappedFileD2M;
    MemoryMappedViewAccessor memoryMappedViewAccessorD2M;

    Task CommunicationThread;
    bool CommunicationThreadExit = false;
    Queue<string> ReadQueue = new Queue<string>();
    Queue<string> WriteQueue = new Queue<string>();
    Object CommunicationThreadLockObject = new Object();

    public SharedMemory()
	{
        //オープン
        memoryMappedFileM2D = MemoryMappedFile.CreateOrOpen(SHARED_MEMORY_M2D, SHARED_MEMORY_SIZE);
        memoryMappedViewAccessorM2D = memoryMappedFileM2D.CreateViewAccessor();

        memoryMappedFileD2M = MemoryMappedFile.CreateOrOpen(SHARED_MEMORY_D2M, SHARED_MEMORY_SIZE);
        memoryMappedViewAccessorD2M = memoryMappedFileD2M.CreateViewAccessor();

        //カウンタの初期化
        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_COUNTER_SEND, (byte)0);
        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, (byte)0);
        memoryMappedViewAccessorD2M.Write(SHARED_MEMORY_ADDRESS_COUNTER_SEND, (byte)0);
        memoryMappedViewAccessorD2M.Write(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, (byte)0);

        CommunicationThread = Task.Run(() =>
        {
            while (!CommunicationThreadExit)
            {
                lock (CommunicationThreadLockObject)
                {
                    try
                    {
                        //送信キューにデータが有る場合
                        if (WriteQueue.Count > 0)
                        {
                            if (WriteStringM2D(WriteQueue.Peek()))
                            {
                                //送信成功したら引き抜き、そのまま続ける
                                WriteQueue.Dequeue();
                                continue;
                            }
                            //書き込み失敗時は次の周期まで待つ(相手が起動していないか、相手がいっぱいいっぱい)
                        }

                        //受信データが有る場合(かつリミッター以下の場合)
                        string r = ReadStringD2M();
                        if (r.Length > 0 && ReadQueue.Count < 1024)
                        {
                            ReadQueue.Enqueue(r);
                            continue;
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                    }
                }
                Thread.Sleep(4);
            }
        });

    }

    public void Dispose() {
        CommunicationThreadExit = true;
        CommunicationThread.Wait();

        memoryMappedViewAccessorM2D.Dispose();
        memoryMappedViewAccessorD2M.Dispose();
        memoryMappedFileM2D.Dispose();
        memoryMappedFileD2M.Dispose();
    }

    public void Write(string s)
    {
        lock (CommunicationThreadLockObject)
        {
            if (WriteQueue.Count < 1024) //1024件以上は捨てる(異常時、通信不良時)
            {
                WriteQueue.Enqueue(s);
            }
        }
    }
    public string Read()
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

    public bool WriteStringM2D(string msg)
    {
        return WriteString(msg, memoryMappedViewAccessorM2D);
    }
    public bool WriteStringD2M(string msg)
    {
        return WriteString(msg, memoryMappedViewAccessorD2M);
    }

    public bool WriteString(string msg, MemoryMappedViewAccessor memoryMappedViewAccessor)
    {
        //Console.WriteLine(">WriteString");

        //ハンドシェイクカウンタチェック
        memoryMappedViewAccessor.Read(SHARED_MEMORY_ADDRESS_COUNTER_SEND, out byte sendcnt);
        memoryMappedViewAccessor.Read(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, out byte handshakecnt);

        //Console.WriteLine("sendcnt:" + sendcnt);
        //Console.WriteLine("handshakecnt:" + handshakecnt);

        //ハンドシェイクと送信が一致していない = 相手が受け取れていない
        if (sendcnt != handshakecnt)
        {
            //送信中止
            //Console.WriteLine("送信中止: 相手待機中");
            return false;
        }

        //送信データ更新
        byte[] stringArray = ASCIIEncoding.ASCII.GetBytes(msg);
        if (stringArray.Length > SHARED_MEMORY_STRING_AREA_SIZE)
        {
            Console.WriteLine("例外: 文字列が大きすぎる");
            throw new ArgumentOutOfRangeException("string too large");
        }

        memoryMappedViewAccessor.Write(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 0, (byte)((stringArray.Length >> 0) & 0xFF));
        memoryMappedViewAccessor.Write(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 1, (byte)((stringArray.Length >> 8) & 0xFF));
        memoryMappedViewAccessor.Write(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 2, (byte)((stringArray.Length >> 16) & 0xFF));
        memoryMappedViewAccessor.Write(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 3, (byte)((stringArray.Length >> 24) & 0xFF));
        memoryMappedViewAccessor.WriteArray(SHARED_MEMORY_ADDRESS_STRING_AREA, stringArray, 0, stringArray.Length);

        //送信データカウンタ更新
        sendcnt = (byte)((((int)sendcnt) + 1) & 0xFF);
        memoryMappedViewAccessor.Write(SHARED_MEMORY_ADDRESS_COUNTER_SEND, (byte)sendcnt);

        //Console.WriteLine("+sendcnt:" + sendcnt);

        //送信成功
        //Console.WriteLine("送信完了: " + msg);
        //Console.WriteLine("<WriteString\n");
        return true;
    }

    public string ReadStringD2M()
    {
        return ReadString(memoryMappedViewAccessorD2M);
    }
    public string ReadStringM2D()
    {
        return ReadString(memoryMappedViewAccessorM2D);
    }
    public string ReadString(MemoryMappedViewAccessor memoryMappedViewAccessor)
    {
        //Console.WriteLine(">ReadString");

        //ハンドシェイクカウンタチェック
        memoryMappedViewAccessor.Read(SHARED_MEMORY_ADDRESS_COUNTER_SEND, out byte sendcnt);
        memoryMappedViewAccessor.Read(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, out byte handshakecnt);

        //Console.WriteLine("sendcnt:" + sendcnt);
        //Console.WriteLine("handshakecnt:" + handshakecnt);

        //ハンドシェイクが送信と一致している = 前回受信した値のまま
        if (sendcnt == handshakecnt)
        {
            //受信中止
            //Console.WriteLine("受信中止: 更新なし");
            return "";
        }

        //データサイズ取得
        byte[] len = new byte[4];
        for (int i = 0; i < 4; i++)
        {
            memoryMappedViewAccessor.Read(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + i, out len[i]);
        }
        uint ulen = (((uint)len[0]) & 0x000000FF) | (((uint)len[1] << 8) & 0x0000FF00) | (((uint)len[2] << 16) & 0x00FF0000) | (((uint)len[3] << 24) & 0xFF000000);
        int length = (int)(ulen & 0x7FFFFFFF);

        if (length > SHARED_MEMORY_STRING_AREA_SIZE)
        {
            Console.WriteLine("例外: サイズが破損している");
            throw new InvalidDataException("size is corrupted");
        }

        //データ受信
        byte[] stringArray = new byte[length];
        memoryMappedViewAccessor.ReadArray(SHARED_MEMORY_ADDRESS_STRING_AREA, stringArray, 0, length);
        string msg = ASCIIEncoding.ASCII.GetString(stringArray);

        //ハンドシェイクカウンタ更新
        memoryMappedViewAccessor.Write(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, (byte)sendcnt);
        //Console.WriteLine("+handshakecnt:" + sendcnt);

        //受信成功
        //Console.WriteLine("受信成功: " + msg);
        //Console.WriteLine("<ReadString\n");
        return msg;
    }
}
