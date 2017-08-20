#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.System.Network.Socket.h>
#include <XLib.System.Threading.Event.h>

namespace XTest::Worker
{
	class ManagerConnectionDispatcher : public XLib::NonCopyable
	{
	private:
		static constexpr uint32 receiveBufferSize = 128;
		static constexpr uint32 sendBufferSize = 256;

		byte receiveBuffer[receiveBufferSize];
		byte sendBuffer[sendBufferSize];
		uint16 receiveBufferBytesUsed;

		XLib::TCPSocket socket;
		XLib::Event sendEvent; // send complete / send enqueued
		XLib::Event receiveCompleteEvent;

	public:
		void initialize();
		void connect();
		void dispatch();
		void sendPacket(const void* data, uint8 length);
	};
}