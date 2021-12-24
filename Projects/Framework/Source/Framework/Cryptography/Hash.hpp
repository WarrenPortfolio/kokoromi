#pragma once

#include <stdint.h>

namespace W
{
	namespace Hash
	{
		constexpr uint32_t EmptyHash32 = 0;
		constexpr uint64_t EmptyHash64 = 0;

		uint32_t StringHash32(const char* text, uint32_t previousHash = EmptyHash32);
		uint64_t StringHash64(const char* text, uint64_t previousHash = EmptyHash64);
	} // namespace Hash
} // namespace W