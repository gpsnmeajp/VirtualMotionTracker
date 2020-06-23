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
#include "CommunicationManager.h"
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"
#include "DirectOSC.h"

namespace VMTDriver {
	void OSCReceiver::SetPose(bool roomToDriver,int idx, int enable, double x, double y, double z, double qx, double qy, double qz, double qw)
	{
		DriverPose_t pose{ 0 };
		pose.qRotation = VMTDriver::HmdQuaternion_Identity;
		pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
		pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;

		if (enable) {
			pose.deviceIsConnected = true;
			pose.poseIsValid = true;
			pose.result = TrackingResult_Running_OK;
		}
		else {
			pose.deviceIsConnected = false;
			pose.poseIsValid = false;
			pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
		}
		Eigen::Translation3d pos(x, y, z);
		Eigen::Quaterniond rot(qw, qx, qy, qz);
		Eigen::Affine3d affin(pos * rot);
		//{ {M11:-0.9998478 M12:0 M13:-0.01745246 M14:0} 
		//  {M21:0 M22:1 M23:0 M24:0}
		//  {M31:0.01745246 M32:0 M33:-0.9998478 M34:0}
		//  {M41:0.5270745 M42:-2.244383 M43:-0.778713 M44:1} }

		Eigen::Matrix4d RoomToDriver;
		if (roomToDriver)
		{
			RoomToDriver << -0.9998478, 0, -0.01745246, 0,
				0, 1, 0, 0,
				0.01745246, 0, -0.9998478, 0,
				0.5270745, -2.244383, -0.778713, 1;
		}
		else {
			RoomToDriver = Eigen::Matrix4d::Identity();
		}
		Eigen::Affine3d RoomToDriverAffin;
		RoomToDriverAffin = RoomToDriver.transpose();

		Eigen::Affine3d outputAffin(RoomToDriverAffin * affin);
		Eigen::Translation3d outpos(outputAffin.translation());
		Eigen::Quaterniond outrot(outputAffin.rotation());

		/*
		Log::printf("Origin: %10lf,%10lf,%10lf\n", pos.x(), pos.y(), pos.z());
		Log::printf("Trans : %10lf,%10lf,%10lf\n", outpos.x(), outpos.y(), outpos.z());
		Log::printf("Origin: %10lf,%10lf,%10lf,%10lf\n", rot.x(), rot.y(), rot.z(), rot.w());
		Log::printf("Trans : %10lf,%10lf,%10lf,%10lf\n", outrot.x(), outrot.y(), outrot.z(), outrot.w());
		*/

		pose.vecPosition[0] = outpos.x();
		pose.vecPosition[1] = outpos.y();
		pose.vecPosition[2] = outpos.z();
		pose.qRotation.x = outrot.x();
		pose.qRotation.y = outrot.y();
		pose.qRotation.z = outrot.z();
		pose.qRotation.w = outrot.w();

		ServerTrackedDeviceProvider* sever = CommunicationManager::GetInstance()->GetServer();
		if (idx >= 0 && idx <= sever->GetDevices().size())
		{
			sever->GetDevices()[idx].RegisterToVRSystem();
			sever->GetDevices()[idx].SetPose(pose);
		}
	}
	void OSCReceiver::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint)
	{
		try {
			string adr = m.AddressPattern();
			if (adr == "/TrackerPoseRoomUnity")
			{
				int idx, enable;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;

				SetPose(true, idx, enable, x, y, -z, qx, qy, -qz, -qw);
			}
			else if (adr == "/TrackerPoseRoomDriver")
			{
				int idx, enable;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;

				SetPose(true, idx, enable, x, y, z, qx, qy, qz, qw);
			}
			else if (adr == "/TrackerPoseRawUnity")
			{
				int idx, enable;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;

				SetPose(false, idx, enable, x, y, -z, qx, qy, -qz, -qw);
			}
			else if (adr == "/TrackerPoseRawDriver")
			{
				int idx, enable;
				float x, y, z, qx, qy, qz, qw;
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				args >> idx >> enable >> x >> y >> z >> qx >> qy >> qz >> qw >> osc::EndMessage;

				SetPose(false, idx, enable, x, y, z, qx, qy, qz, qw);
			}
			else {
				Log::printf("Unkown: %s", adr.c_str());
			}
		}
		catch (osc::Exception& e)
		{
			Log::printf("Exp: %s\n", e.what());
		}
	}
	/*
	void sendXYZ(DirectOSC::OSC* osc, float x, float y, float z) {
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
	*/
}
namespace VMTDriver {

	CommunicationManager* CommunicationManager::GetInstance()
	{
		static CommunicationManager cm;
		return &cm;
	}

	ServerTrackedDeviceProvider* CommunicationManager::GetServer()
	{
		return m_server;
	}

	void CommunicationManager::Open(ServerTrackedDeviceProvider* server)
	{
		if (m_opened) {
			return;
		}
		m_server = server;
		DirectOSC::OSC::GetInstance()->Open(&m_rcv, 39570, 39571);
		m_opened = true;
	}
	void CommunicationManager::Close()
	{
		DirectOSC::OSC::GetInstance()->Close();
		m_opened = false;
	}
	void CommunicationManager::Process()
	{
		//ì¡Ç…èàóùÇ™Ç»Ç¢
	}
}