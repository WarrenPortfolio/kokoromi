#include "pch.h"

#include <Framework/Text/Text.hpp>

namespace W
{
	TEST(Framework, Text)
	{
		char	buffer[256];
		wchar_t	bufferWide[256];

		Text::Format(buffer, "%s%s!", "Hello", "World");
		EXPECT_STREQ(buffer, "HelloWorld!");
		EXPECT_TRUE(Text::IsAscii(buffer));

		Text::Format(buffer, "%s-%s", u8"こんにちは", u8"안녕하세요");
		EXPECT_STREQ(buffer, u8"こんにちは-안녕하세요");
		EXPECT_FALSE(Text::IsAscii(buffer));

		Text::UTF8::Encode(L"Hello-こんにちは-안녕하세요", buffer);
		EXPECT_STREQ(buffer, u8"Hello-こんにちは-안녕하세요");

		Text::UTF8::Decode(u8"Hello-こんにちは-안녕하세요", bufferWide);
		EXPECT_STREQ(bufferWide, L"Hello-こんにちは-안녕하세요");
	}
}