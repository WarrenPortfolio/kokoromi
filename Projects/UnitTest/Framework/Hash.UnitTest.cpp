#include "pch.h"

#include <Framework/Cryptography/Hash.hpp>

namespace W
{
	TEST(Framework, Hash32)
	{
		uint32_t emptyHash32 = Hash::StringHash32("");
		EXPECT_EQ(emptyHash32, Hash::EmptyHash32);

		uint32_t helloHash32 = Hash::StringHash32("Hello");
		uint32_t worldHash32 = Hash::StringHash32("World");
		EXPECT_NE(helloHash32, worldHash32);

		uint32_t helloWorldHash32 = Hash::StringHash32("HelloWorld");
		uint32_t helloWorldHash32Combined = Hash::StringHash32("World", helloHash32);
		EXPECT_EQ(helloWorldHash32, helloWorldHash32Combined);
	}

	TEST(Framework, Hash64)
	{
		uint64_t emptyHash64 = Hash::StringHash64("");
		EXPECT_EQ(emptyHash64, Hash::EmptyHash64);

		uint64_t helloHash64 = Hash::StringHash64("Hello");
		uint64_t worldHash64 = Hash::StringHash64("World");
		EXPECT_NE(helloHash64, worldHash64);

		uint64_t helloWorldHash64 = Hash::StringHash64("HelloWorld");
		uint64_t helloWorldHash64Combined = Hash::StringHash64("World", helloHash64);
		EXPECT_EQ(helloWorldHash64, helloWorldHash64Combined);
	}
}