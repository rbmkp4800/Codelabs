#pragma once

#include <XLib.Types.h>

#pragma pack(push, 1)

struct uint24
{
	uint16 lo16;
	uint8 hi8;

	uint24() = default;
	constexpr inline uint24(uint32 value) : lo16(uint16(value)), hi8(uint8(value >> 16)) {}
	constexpr inline operator uint32() { return uint32(lo16) | uint32(hi8 << 16); }
};

#pragma pack(pop)