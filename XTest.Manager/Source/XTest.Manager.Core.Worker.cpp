#include <XLib.Debug.h>

#include "XTest.Manager.Core.Worker.h"
#include "XTest.Manager.h"
#include "XTest.Protocols.WorkerManager.MessageLayer.h"

using namespace XLib;
using namespace XTest::Manager::_Core;
using namespace XTest::Protocols::WorkerManager::MessageLayer;

void Worker::onPacketReceived(uint8 length, const void* data)
{
	PacketType packetType = *to<PacketType*>(data);

	if (state == State::InitRequestSent)
	{
		if (packetType != PacketType::WorkerInitResponse)
		{
			Debug::Warning(DbgMsgFmt("worker initialization error. Invalid response"));
			state = State::None;
			connection.drop();
			return;
		}

		const Packet::WorkerInitResponse &packet =
			*to<const Packet::WorkerInitResponse*>(data);

		if (packet.slotCount > slotsLimit)
		{
			Debug::Warning(DbgMsgFmt("worker initialization error. Invalid slot count"));
			state = State::None;
			connection.drop();
			return;
		}

		slotCount = packet.slotCount;
		state = State::Active;
		return;
	}

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

void Worker::initialize(XTMCore *core, uint8 id)
{

}

void Worker::setConnected(XLib::TCPSocket& socket)
{
	connection.setConnected(socket);

	Packet::WorkerInitRequest packet;
	packet.protocolVersion = ProtocolConstants::Version;
	connection.sendPacket(packet);

	state = State::InitRequestSent;
}