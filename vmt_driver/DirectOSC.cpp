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
#include "DirectOSC.h"

namespace DirectOSC {
	std::mutex Mutex;
	std::unique_ptr<std::thread> Thread;

	//受信スレッド
	void ThreadWorker()
	{
		OSC::GetInstance().GetSocketRx().RunUntilSigInt();
	}

	OSC::OSC()
	{
	}
	OSC::~OSC()
	{
	}

	//シングルトンインスタンスの取得
	OSC& OSC::GetInstance()
	{
		static OSC osc;
		return osc;
	}

	//受信ソケットの取得
	UdpListeningReceiveSocket& OSC::GetSocketRx()
	{
		return *socketRx;
	}

	//送信ソケットの取得
	UdpTransmitSocket& OSC::GetSocketTx()
	{
		return *socketTx;
	}

	//oscpackを初期化し、受信処理を登録する。受信スレッドを立てる
	void OSC::Open(osc::OscPacketListener* listen, int portRx,int portTx)
	{
		if (m_opened) {
			return;
		}
		this->listener = listen;
		socketRx = std::make_unique<UdpListeningReceiveSocket>(IpEndpointName(IpEndpointName::ANY_ADDRESS, portRx), listener);
		socketTx = std::make_unique<UdpTransmitSocket>(IpEndpointName("127.0.0.1", portTx));

		Thread = std::make_unique<std::thread>(ThreadWorker);
		m_opened = true;
	}

	//送信先を切り替える
	void OSC::ReOpen(std::string address, int portTx)
	{
		socketTx = std::make_unique<UdpTransmitSocket>(IpEndpointName(address.c_str(), portTx));
	}

	//受信スレッドを停止し、処理を終える
	void OSC::Close()
	{
		if (!m_opened) {
			return;
		}
		socketRx->AsynchronousBreak();
		Thread->join();

		listener = nullptr;
		m_opened = false;
	}
}