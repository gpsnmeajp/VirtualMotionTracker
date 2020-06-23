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
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "SharedMemory.h"
#include "DirectOSC.h"
#include "..\..\json.hpp"

#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"

using std::string;
using json = nlohmann::json;

#include <iostream>
#include <cstring>

class OSCReceiver : public osc::OscPacketListener {
	virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint) override
	{
		try {
			string adr = m.AddressPattern();
			if (adr == "/test1")
			{
				float x;
				float y;
				float z;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> x >> y >> z >> osc::EndMessage;

				printf("X:%lf, Y:%lf, Z:%lf\n",x,y,z);
			}
		}
		catch (osc::Exception& e)
		{
			printf("Exp: %s\n", e.what());		
		}
	}
};

void sendXYZ(DirectOSC::OSC* osc,float x, float y, float z) {
	const size_t bufsize = 1024;
	char buf[bufsize];
	osc::OutboundPacketStream packet(buf, bufsize);
	packet << osc::BeginMessage("/test1")
		<< x
		<< y
		<< z
		<< osc::EndMessage;
	osc->GetSocketTx()->Send(packet.Data(), packet.Size());
}

int main(void)
{
    DirectOSC::OSC *osc = DirectOSC::OSC::GetInstance();
    
	OSCReceiver rcv;
    osc->Open(&rcv, 39500, 39500);

    printf("Start\n");
	sendXYZ(osc, 1.2, 3.4, 5.66);
    getch();
    printf("Stop\n");

    osc->Close();
    printf("End\n");
    /*
	SharedMemory::SharedMemory sm;

	sm.open();
	while (!_kbhit()) {
		string r = sm.readM2D();
		if (!r.empty()) {
			json j = json::parse(r);
			string type = j["type"];

			printf("%s\n", type.c_str());
		}

		json jw;

		jw["type"] = "Hello";
		jw["json"] = json{ {"msg","Hello from cpp"} }.dump();
		sm.writeD2M(jw.dump());
		Sleep(800);
	}
	sm.close();

	return 0;
	*/
}

