#pragma once
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

#include <Windows.h>
#include <stdio.h>
#include <string>
#include "dllmain.h"

using std::string;

namespace SharedMemory {
	const string SHARED_MEMORY_M2D = "VirtualMotionTracker_M2D";
	const string SHARED_MEMORY_D2M = "VirtualMotionTracker_D2M";

	const int SHARED_MEMORY_ADDRESS_COUNTER_SEND = 0x00;
	const int SHARED_MEMORY_ADDRESS_COUNTER_HANDSHAKE = 0x01;

	const int SHARED_MEMORY_ADDRESS_SIZE_OF_STRING_AREA = 0x10; //0x10-0x13
	const int SHARED_MEMORY_ADDRESS_STRING_AREA = 0x20;

	const int SHARED_MEMORY_SIZE = 1 * 1024 * 1024; //1MB
	const int SHARED_MEMORY_STRING_AREA_SIZE = (SHARED_MEMORY_SIZE - SHARED_MEMORY_ADDRESS_STRING_AREA - 1); //1MB

	class SharedMemory {
	public:
		SharedMemory();
		~SharedMemory();

		bool open();
		void close();
		string read(LPVOID);
		bool write(LPVOID, string);

		string readM2D();
		bool writeD2M(string);

		int log(const char* fmt, ...);
		int logError(const char* fmt, ...);

	private:
		HANDLE m_sharedMemoryHandleM2D = nullptr;
		LPVOID m_sharedMemoryViewPointerM2D = nullptr;
		HANDLE m_sharedMemoryHandleD2M = nullptr;
		LPVOID m_sharedMemoryViewPointerD2M = nullptr;
	};
}