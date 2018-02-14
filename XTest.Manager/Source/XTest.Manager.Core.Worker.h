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

	private: // meta
		static constexpr uint32 slotsLimit = 16;

		enum class State : uint8
		{
			None = 0,

			InitRequestSent = 1,
			Active = 2,
		};

		enum class SlotState : uint8
		{
			None = 0,
		};

	private: // data
		Internal::Solution* solutionsSlots[slotsLimit];
		SlotState slotStates[slotsLimit];

		_Worker::Connection connection;

		XTMCore *core = nullptr;
		uint8 id = 0;
		uint8 slotCount = 0;

		State state = State::None;

	private: // code
		void onPacketReceived(uint8 length, const void* data);
		void onDisconnected();

	public:
		Worker() = default;
		~Worker() = default;

		void initialize(XTMCore *core, uint8 id);
		void destroy();

		void setConnected(XLib::TCPSocket& socket);
	};
}