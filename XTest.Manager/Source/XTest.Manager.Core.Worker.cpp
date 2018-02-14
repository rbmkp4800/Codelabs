#include <XLib.Debug.h>

#include "XTest.Manager.Core.Worker.h"
#include "XTest.Manager.h"
#include "XTest.Protocols.WorkerManager.MessageLayer.h"

#define InitErrorMsgFmt(message) DbgMsgFmt("worker initialization error (" message ")")
#define CommErrorMsgFmt(message) DbgMsgFmt("worker communication error (" message ")")

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
			Debug::Warning(InitErrorMsgFmt("invalid response"));
			state = State::None;
			connection.drop();
			return;
		}

		const Packet::WorkerInitResponse &packet =
			*to<const Packet::WorkerInitResponse*>(data);

		if (packet.slotCount > slotsLimit)
		{
			Debug::Warning(InitErrorMsgFmt("invalid slot count"));
			state = State::None;
			connection.drop();
			return;
		}

		slotCount = packet.slotCount;
		freeSlotCount = slotCount;
		state = State::Active;

		core->onWorkerSlotReady(id);

		return;
	}

	switch (packetType)
	{
		default:
		{
			Debug::Warning(CommErrorMsgFmt("invalid packet code"));
			core->onWorkerDisconnected(id);
			revokePendingSolutions();
			connection.drop();
			state = State::None;
			return;
		}
	}
}

void Worker::onDisconnected()
{
	core->onWorkerDisconnected(id);
	revokePendingSolutions();
}

inline void Worker::revokePendingSolutions()
{

}

void Worker::initialize(ManagerCore* core, uint8 id)
{
	this->core = core;
	this->id = id;
}

void Worker::setConnected(XLib::TCPSocket& socket)
{
	connection.setConnected(socket);

	Packet::WorkerInitRequest packet;
	packet.protocolVersion = ProtocolConstants::Version;
	connection.sendPacket(packet);

	state = State::InitRequestSent;
}