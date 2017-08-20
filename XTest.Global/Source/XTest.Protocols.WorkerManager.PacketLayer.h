#pragma once

#include <XLib.Types.h>

namespace XTest::Protocols::WorkerManager::PacketLayer
{
	struct PacketHeader
	{
		uint8 length_extensionFlag;

		inline uint8 getLength() { return length_extensionFlag & 0b0111'1111; }
		inline bool hasExtension() { return (length_extensionFlag & 0b1000'0000) != 0; }
	};
}