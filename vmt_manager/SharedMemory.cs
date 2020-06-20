using System;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Text;

public class SharedMemory : IDisposable
{
    const string SHARED_MEMORY_M2D = "VirtualMotionTracker_M2D";
    const string SHARED_MEMORY_D2M = "VirtualMotionTracker_D2M";//デバッグ用に同じにしている。

    const int SHARED_MEMORY_ADDRESS_SIZE = 1 * 1024 * 1024; //1MB

    const int SHARED_MEMORY_ADDRESS_COUNTER_SEND = 0x00;
    const int SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE = 0x01;

    const int SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA = 0x10; //0x10-0x13
    const int SHARED_MEMORY_ADDRESS_STRING_AREA = 0x20;

    MemoryMappedFile memoryMappedFileM2D;
    MemoryMappedViewAccessor memoryMappedViewAccessorM2D;

    MemoryMappedFile memoryMappedFileD2M;
    MemoryMappedViewAccessor memoryMappedViewAccessorD2M;

    public SharedMemory()
	{
        //オープン
        memoryMappedFileM2D = MemoryMappedFile.CreateOrOpen(SHARED_MEMORY_M2D, SHARED_MEMORY_ADDRESS_SIZE);
        memoryMappedViewAccessorM2D = memoryMappedFileM2D.CreateViewAccessor();
        memoryMappedFileD2M = MemoryMappedFile.CreateOrOpen(SHARED_MEMORY_D2M, SHARED_MEMORY_ADDRESS_SIZE);
        memoryMappedViewAccessorD2M = memoryMappedFileM2D.CreateViewAccessor();

        //カウンタの初期化
        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_COUNTER_SEND, (byte)0);
        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, (byte)0);
        memoryMappedViewAccessorD2M.Write(SHARED_MEMORY_ADDRESS_COUNTER_SEND, (byte)0);
        memoryMappedViewAccessorD2M.Write(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, (byte)0);
    }

    public void Dispose() {
        memoryMappedViewAccessorM2D.Dispose();
        memoryMappedViewAccessorD2M.Dispose();
        memoryMappedFileM2D.Dispose();
        memoryMappedFileD2M.Dispose();
    }

    public bool WriteStringM2D(string msg)
    {
        Console.WriteLine(">WriteString");

        //ハンドシェイクカウンタチェック
        memoryMappedViewAccessorM2D.Read(SHARED_MEMORY_ADDRESS_COUNTER_SEND, out byte sendcnt);
        memoryMappedViewAccessorM2D.Read(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, out byte handshakecnt);

        Console.WriteLine("sendcnt:" + sendcnt);
        Console.WriteLine("handshakecnt:" + handshakecnt);

        //ハンドシェイクと送信が一致していない = 相手が受け取れていない
        if (sendcnt != handshakecnt)
        {
            //送信中止
            Console.WriteLine("送信中止: 相手待機中");
            return false;
        }

        //送信データ更新
        byte[] stringArray = ASCIIEncoding.ASCII.GetBytes(msg);
        if (stringArray.Length > SHARED_MEMORY_ADDRESS_SIZE)
        {
            Console.WriteLine("例外: 文字列が大きすぎる");
            throw new ArgumentOutOfRangeException("string too large");
        }

        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 0, (byte)((stringArray.Length >> 0) & 0xFF));
        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 1, (byte)((stringArray.Length >> 8) & 0xFF));
        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 2, (byte)((stringArray.Length >> 16) & 0xFF));
        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 3, (byte)((stringArray.Length >> 24) & 0xFF));
        memoryMappedViewAccessorM2D.WriteArray(SHARED_MEMORY_ADDRESS_STRING_AREA, stringArray, 0, stringArray.Length);

        //送信データカウンタ更新
        sendcnt = (byte)((((int)sendcnt) + 1) & 0xFF);
        memoryMappedViewAccessorM2D.Write(SHARED_MEMORY_ADDRESS_COUNTER_SEND, (byte)sendcnt);

        Console.WriteLine("+sendcnt:" + sendcnt);

        //送信成功
        Console.WriteLine("送信完了: " + msg);
        Console.WriteLine("<WriteString\n");
        return true;
    }

    public string ReadStringD2M()
    {
        Console.WriteLine(">ReadString");

        //ハンドシェイクカウンタチェック
        memoryMappedViewAccessorD2M.Read(SHARED_MEMORY_ADDRESS_COUNTER_SEND, out byte sendcnt);
        memoryMappedViewAccessorD2M.Read(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, out byte handshakecnt);

        Console.WriteLine("sendcnt:" + sendcnt);
        Console.WriteLine("handshakecnt:" + handshakecnt);

        //ハンドシェイクが送信と一致している = 前回受信した値のまま
        if (sendcnt == handshakecnt)
        {
            //受信中止
            Console.WriteLine("受信中止: 更新なし");
            return null;
        }

        //データサイズ取得
        byte[] len = new byte[4];
        for (int i = 0; i < 4; i++)
        {
            memoryMappedViewAccessorD2M.Read(SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + i, out len[i]);
        }
        uint ulen = (((uint)len[0]) & 0x000000FF) | (((uint)len[1] << 8) & 0x0000FF00) | (((uint)len[2] << 16) & 0x00FF0000) | (((uint)len[3] << 24) & 0xFF000000);
        int length = (int)(ulen & 0x7FFFFFFF);

        if (length > SHARED_MEMORY_ADDRESS_SIZE)
        {
            Console.WriteLine("例外: サイズが破損している");
            throw new InvalidDataException("size is corrupted");
        }

        //データ受信
        byte[] stringArray = new byte[length];
        memoryMappedViewAccessorD2M.ReadArray(SHARED_MEMORY_ADDRESS_STRING_AREA, stringArray, 0, length);
        string msg = ASCIIEncoding.ASCII.GetString(stringArray);

        //ハンドシェイクカウンタ更新
        handshakecnt = (byte)((((int)sendcnt) + 1) & 0xFF);
        memoryMappedViewAccessorD2M.Write(SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE, (byte)sendcnt);
        Console.WriteLine("+handshakecnt:" + sendcnt);

        //受信成功
        Console.WriteLine("受信成功: " + msg);
        Console.WriteLine("<ReadString\n");
        return msg;
    }
}
