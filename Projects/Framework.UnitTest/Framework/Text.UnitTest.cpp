#include "pch.h"

#include <Framework/Text/Text.hpp>

namespace W
{
	TEST(Framework, Text)
	{
		char	buffer[256];
		wchar_t	bufferWide[256];

		EXPECT_TRUE(Text::IsAscii(u8"Hello"));
		EXPECT_FALSE(Text::IsAscii(u8"こんにちは"));

		EXPECT_TRUE(Text::IsNullOrEmpty(nullptr));
		EXPECT_TRUE(Text::IsNullOrEmpty(""));
		EXPECT_FALSE(Text::IsNullOrEmpty(u8"Hello"));
		EXPECT_FALSE(Text::IsNullOrEmpty(u8"こんにちは"));

		Text::Format(buffer, "%s%s!", "Hello", "World");
		EXPECT_STREQ(buffer, "HelloWorld!");

		Text::Format(buffer, "%s-%s", u8"Hello", u8"こんにちは");
		EXPECT_STREQ(buffer, u8"Hello-こんにちは");

		Text::UTF8::Encode(L"Hello-こんにちは", buffer);
		EXPECT_STREQ(buffer, u8"Hello-こんにちは");

		Text::UTF8::Decode(u8"Hello-こんにちは", bufferWide);
		EXPECT_STREQ(bufferWide, L"Hello-こんにちは");
	}
}