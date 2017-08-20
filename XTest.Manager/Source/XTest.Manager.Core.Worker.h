#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.System.Network.Socket.h>

#include "XTest.Manager.Core.Worker.Connection.h"

namespace XTest::Manager { class XTMCore; }
namespace XTest::Manager::Internal { class Solution; }

namespace XTest::Manager::_Core
{
	class Worker : public XLib::NonCopyable
	{
		friend _Worker::Connection;

	private:
		enum class SlotState : uint8
		{
			None = 0,
		};

		struct Slot
		{
			Internal::Solution *solution;
			SlotState state;
		};

		static constexpr uint32 slotsLimit = 16;

		//--------------------------------------------------------

		XTMCore *core;
		uint8 id;

		uint8 slotCount;

		_Worker::Connection connection;
		Slot slots[slotsLimit];

		void onPacketReceived(uint8 length, const void* data);
		void onDisconnected();

	public:
		Worker(XTMCore *core, uint8 id, XLib::TCPSocket& socket);
	};
}