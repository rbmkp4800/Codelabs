#include "XTest.Manager.Core.Worker.h"
#include "XTest.Manager.h"
#include "XTest.Protocols.WorkerManager.MessageLayer.h"

using namespace XTest::Manager::_Core;
using namespace XTest::Protocols::WorkerManager::MessageLayer;

void Worker::onPacketReceived(uint8 length, const void* data)
{
	PacketType packetType = *to<PacketType*>(data);

	switch (packetType)
	{
		case PacketType::WorkerInitResponse:
		{
			core->onWorkerInitComplete(id);
			break;
		}

		default:
			break;
	}
}

void Worker::onDisconnected()
{
	core->onWorkerDisconnected(id);
}