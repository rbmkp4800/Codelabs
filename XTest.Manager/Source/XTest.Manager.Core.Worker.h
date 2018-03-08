#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.System.Network.Socket.h>

#include "XTest.Manager.Core.Worker.Connection.h"

namespace XTest::Manager { class ManagerCore; }
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
			InitRequestSent,
			Active,
			SendingSolution,
		};

		enum class SlotState : uint8
		{
			None = 0,
		};

	private: // data
		Internal::Solution* solutionsSlots[slotsLimit];
		SlotState slotStates[slotsLimit];

		_Worker::Connection connection;

		ManagerCore *core = nullptr;
		uint8 id = 0;
		uint8 slotCount = 0, freeSlotCount = 0;

		State state = State::None;

	private: // code
		void onPacketReceived(uint8 length, const void* data,
			uint32 extensionLength, void** extensionReceiveBuffer);
		void onPacketExtensionReceived();
		void onDisconnected();

		inline void revokePendingSolutions();

	public:
		Worker() = default;
		~Worker() = default;

		void initialize(ManagerCore* core, uint8 id);

		void setConnected(XLib::TCPSocket& socket);
		void pushSolution(Internal::Solution* solution);

		inline uint8 getSlotCount() { return slotCount; }
		inline uint8 getFreeSlotCount() { return freeSlotCount; }
		inline bool canAcceptSolution() { return state == State::Active; }
	};
}