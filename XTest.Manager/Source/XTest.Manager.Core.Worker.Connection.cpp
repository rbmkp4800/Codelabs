#include <XLib.Debug.h>

#include "XTest.Manager.Core.Worker.Connection.h"
#include "XTest.Manager.Core.Worker.h"
#include "XTest.Protocols.WorkerManager.PacketLayer.h"

#define worker getWorker()

using namespace XLib;
using namespace XTest::Manager::_Core;
using namespace XTest::Manager::_Core::_Worker;
using namespace XTest::Protocols::WorkerManager::PacketLayer;

inline Worker& Connection::getWorker() { return *to<Worker*>(to<byte*>(this) - offsetof(Worker, connection)); }

void Connection::onReceiveCompleted(bool result, uint32 receivedSize, uintptr)
{
	if (!result)
	{
		Debug::Warning(DbgMsgFmt("receive error"));
		//disconnect();
		return;
	}

	receiveBufferBytesUsed += uint16(receivedSize);
	for (;;)
	{
		if (receiveBufferBytesUsed - receiveBufferBytesConsumed >= sizeof(PacketHeader))
		{
			PacketHeader *header = to<PacketHeader*>(receiveBuffer + receiveBufferBytesConsumed);
			uint16 packetLength = header->getLength();
			if (header->hasExtension())
			{
				uint16 bytesToConsume = sizeof(PacketHeader) + sizeof(uint32) + packetLength;
				if (receiveBufferBytesConsumed + bytesToConsume <= receiveBufferBytesUsed)
				{
					uint32 extensionLength = *to<uint32*>(receiveBuffer +
						receiveBufferBytesConsumed + sizeof(PacketHeader));
					void* extensionReceiveBuffer = nullptr;

					const void *packetData = receiveBuffer + receiveBufferBytesConsumed +
						sizeof(PacketHeader) + sizeof(uint32);

					worker.onPacketReceived(packetLength, packetData,
						extensionLength, &extensionReceiveBuffer);

					socket.asyncReceive(extensionReceiveBuffer, )

					if (extensionReceiveBuffer)
					{

					}
				}
			}
			else
			{
				uint16 bytesToConsume = sizeof(PacketHeader) + packetLength;
				if (receiveBufferBytesConsumed + bytesToConsume <= receiveBufferBytesUsed)
				{
					worker.onPacketReceived(packetLength, receiveBuffer +
						receiveBufferBytesConsumed + sizeof(PacketHeader), nullptr);
				}
			}

			

			if (receiveBufferBytesConsumed + sizeof(PacketHeader) + packetLength <= receiveBufferBytesUsed)
			{
				worker.onPacketReceived(packetLength, )
			}
		}
	}
}

void Connection::onSendCompleted(bool result, uint32 sentSize, uintptr)
{

}

void Connection::setConnected(XLib::TCPSocket& socket)
{
	this->socket = move(socket);
}

void Connection::drop()
{
	//socket.disconect();
	socket.destroy();
}