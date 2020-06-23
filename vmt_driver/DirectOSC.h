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

//oscpackÇÃèàóùÇÇ∑ÇÈ
#include <windows.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <deque>

#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"

namespace DirectOSC {
	class OSC {
	private:
		bool m_opened = false;
		osc::OscPacketListener* listener = nullptr;
		UdpListeningReceiveSocket* socketRx = nullptr;
		UdpTransmitSocket* socketTx = nullptr;

		OSC();
		~OSC();
	public:
		static OSC* GetInstance();
		UdpListeningReceiveSocket* GetSocketRx();
		UdpTransmitSocket* GetSocketTx();

		void Open(osc::OscPacketListener* listen, int portRx, int portTx);
		void Close();
	};

}