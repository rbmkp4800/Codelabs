#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.Delegate.h>
#include <XLib.Heap.h>
#include <XLib.System.Network.Socket.h>

namespace XTest::Manager::_Core { class Worker; }

namespace XTest::Manager::_Core::_Worker
{
	using PacketExtensionSentHandler = XLib::Delegate<void>;
	using PacketExtensionReceivedHandler = XLib::Delegate<void>;

	class Connection : public XLib::NonCopyable
	{
	private:
		static constexpr uint32 receiveBufferSize = 128;
		static constexpr uint32 sendBufferSize = 64;

		XLib::TCPSocket socket;
		XLib::DispatchedAsyncTask receiveTask, sendTask;

		byte receiveBuffer[receiveBufferSize];
		byte sendBuffer[sendBufferSize];
		uint16 receiveBufferBytesUsed, receiveBufferBytesConsumed;

		void onReceiveCompleted(bool result, uint32 receivedSize, uintptr);
		void onSendCompleted(bool result, uint32 sentSize, uintptr);

		inline Worker& getWorker();

	public:
		void setConnected(XLib::TCPSocket& socket);
		void drop();

		void sendPacket(uint8 legnth, const void* data);
		void sendPacket(uint8 length, const void* data, uint32 extensionLength,
			const void* extensionData, PacketExtensionSentHandler handler);

		template <typename Type>
		inline void sendPacket(const Type& data)
		{
			static_assert(sizeof(Type) < 256, "invalid packet size");
			sendPacket(sizeof(Type), data);
		}
	};
}