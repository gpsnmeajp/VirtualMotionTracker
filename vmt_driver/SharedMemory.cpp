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
#include "SharedMemory.h"

namespace SharedMemory {



	SharedMemory::SharedMemory()
	{
		open();
	}

	SharedMemory::~SharedMemory() {
		close();
	}

	bool SharedMemory::open()
	{
		if (m_opened) {
			return true;
		}
		DWORD size_high = 0;
		DWORD size_low = SHARED_MEMORY_SIZE & 0xFFFFFFFF;

		//ポインタをnull初期化
		close();

		m_sharedMemoryHandleM2D = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, size_high, size_low, SHARED_MEMORY_M2D.c_str());
		if (m_sharedMemoryHandleM2D == nullptr) {
			close();
			return false;
		}
		m_sharedMemoryHandleD2M = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, size_high, size_low, SHARED_MEMORY_D2M.c_str());
		if (m_sharedMemoryHandleD2M == nullptr) {
			close();
			return false;
		}

		m_sharedMemoryViewPointerM2D = MapViewOfFile(m_sharedMemoryHandleM2D, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (m_sharedMemoryViewPointerM2D == nullptr) {
			close();
			return false;
		}
		m_sharedMemoryViewPointerD2M = MapViewOfFile(m_sharedMemoryHandleD2M, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (m_sharedMemoryViewPointerD2M == nullptr) {
			close();
			return false;
		}

		uint8_t* pM2D = reinterpret_cast<uint8_t*>(m_sharedMemoryViewPointerM2D); //ポインタをキャスト
		uint8_t* pD2M = reinterpret_cast<uint8_t*>(m_sharedMemoryViewPointerD2M); //ポインタをキャスト
		pM2D[SHARED_MEMORY_ADDRESS_COUNTER_SEND] = 0;
		pM2D[SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE] = 0;
		pD2M[SHARED_MEMORY_ADDRESS_COUNTER_SEND] = 0;
		pD2M[SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE] = 0;

		m_opened = true;
		return true;
	}

	void SharedMemory::close()
	{
		if (m_sharedMemoryViewPointerD2M != nullptr) {
			UnmapViewOfFile(m_sharedMemoryViewPointerD2M);
			m_sharedMemoryViewPointerD2M = nullptr;
		}
		if (m_sharedMemoryViewPointerM2D != nullptr) {
			UnmapViewOfFile(m_sharedMemoryViewPointerM2D);
			m_sharedMemoryViewPointerM2D = nullptr;
		}
		if (m_sharedMemoryHandleM2D != nullptr) {
			CloseHandle(m_sharedMemoryHandleM2D);
			m_sharedMemoryHandleM2D = nullptr;
		}
		if (m_sharedMemoryHandleD2M != nullptr) {
			CloseHandle(m_sharedMemoryHandleD2M);
			m_sharedMemoryHandleD2M = nullptr;
		}
		m_opened = false;
		return;
	}

	string SharedMemory::readM2D()
	{
		return read(m_sharedMemoryViewPointerM2D);
	}

	bool SharedMemory::writeD2M(string s)
	{
		return write(m_sharedMemoryViewPointerD2M,s);
	}
/*
	int SharedMemory::log(const char* fmt, ...)
	{
		//ログ出力(状況に合わせて切り替えられるように)
		va_list arg;
		va_start(arg, fmt);
		int ret = vprintf(fmt, arg);
		va_end(arg);
		return ret;
	}
*/
	int SharedMemory::log(const char* fmt, ...)
	{
		//通常のログは多すぎるので吐かない
		/*
		const int size = 8192;
		char output[size] = { 0 };

		va_list arg;
		va_start(arg, fmt);
		int ret = vsnprintf(output, size, fmt, arg);
		va_end(arg);

		output[size - 1] = '\0';
		VMTDriver::Log::Output(output);

		return ret;
		*/
		return 0;
	}
	int SharedMemory::logError(const char* fmt, ...)
	{
		const int size = 8192;
		char output[size] = { 0 };

		va_list arg;
		va_start(arg, fmt);
		int ret = vsnprintf(output, size, fmt, arg);
		va_end(arg);

		output[size - 1] = '\0';
		VMTDriver::Log::Output(output);

		return ret;
	}


	std::string SharedMemory::read(LPVOID smvp)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(smvp); //ポインタをキャスト
		log(">ReadString\n");

		//ハンドシェイクカウンタチェック
		uint8_t sendcnt = p[SHARED_MEMORY_ADDRESS_COUNTER_SEND];
		uint8_t handshakecnt = p[SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE];

		log("sendcnt: %d\n", sendcnt);
		log("handshakecnt: %d\n", handshakecnt);

		//ハンドシェイクが送信と一致している = 前回受信した値のまま
		if (sendcnt == handshakecnt)
		{
			//受信中止
			log("Receive abort: no update\n");
			return "";
		}

		//データサイズ取得
		uint8_t len[4];
		for (int i = 0; i < 4; i++)
		{
			len[i] = p[SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + i];
		}
		uint32_t length = (((uint32_t)len[0]) & 0x000000FF) | (((uint32_t)len[1] << 8) & 0x0000FF00) | (((uint32_t)len[2] << 16) & 0x00FF0000) | (((uint32_t)len[3] << 24) & 0xFF000000);

		if (length > SHARED_MEMORY_STRING_AREA_SIZE)
		{
			logError("ERROR: Size is corrupted.\n");
			return "";
		}

		//データ受信
		char* buf = new char[length + 1];
		memcpy(buf, p + SHARED_MEMORY_ADDRESS_STRING_AREA, length);
		buf[length] = '\0';
		string msg = string(buf);
		log("Receive OK: %s\n", buf);
		delete[] buf;

		//ハンドシェイクカウンタ更新
		p[SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE] = sendcnt;
		log("+handshakecnt: %d\n", sendcnt);

		//受信成功
		log("<ReadString\n");
		return msg;
	}

	bool SharedMemory::write(LPVOID smvp, std::string msg)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(smvp); //ポインタをキャスト
		log(">WriteString\n");

		//ハンドシェイクカウンタチェック
		uint8_t sendcnt = p[SHARED_MEMORY_ADDRESS_COUNTER_SEND];
		uint8_t handshakecnt = p[SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE];

		log("sendcnt: %d\n", sendcnt);
		log("handshakecnt: %d\n", handshakecnt);

		//ハンドシェイクと送信が一致していない = 相手が受け取れていない
		if (sendcnt != handshakecnt)
		{
			//送信中止
			log("Send abort: waiting for read\n");
			return false;
		}

		//送信データ更新
		size_t len = strlen(msg.c_str());
		if (len > SHARED_MEMORY_STRING_AREA_SIZE)
		{
			logError("ERROR: Size is too large.");
			return false;
		}

		p[SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 0] = (uint8_t)((len >> 0) & 0xFF);
		p[SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 1] = (uint8_t)((len >> 8) & 0xFF);
		p[SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 2] = (uint8_t)((len >> 16) & 0xFF);
		p[SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA + 3] = (uint8_t)((len >> 24) & 0xFF);

		memcpy(p + SHARED_MEMORY_ADDRESS_STRING_AREA, msg.c_str(), len);

		//送信データカウンタ更新
		sendcnt = (uint8_t)((((int)sendcnt) + 1) & 0xFF);
		p[SHARED_MEMORY_ADDRESS_COUNTER_SEND] = (uint8_t)sendcnt;

		log("+sendcnt: %d\n", sendcnt);

		//送信成功
		log("Send OK: %s\n",msg.c_str());
		log("<WriteString\n");
		return true;
	}
}