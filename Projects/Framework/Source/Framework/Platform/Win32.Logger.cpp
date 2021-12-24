#include <Framework/Debug/Logger.hpp>
#include <Framework/Text/Text.hpp>

#include <stdio.h>
#include <windows.h>

namespace W
{
	void Logger::Print(const char* text)
	{
		if (Text::IsAscii(text))
		{
			fputs(text, stdout);
			OutputDebugStringA(text);
		}
		else
		{
			wchar_t textBuffer[4096];
			Text::UTF8::Decode(text, textBuffer);

			fputws(textBuffer, stdout);
			OutputDebugStringW(textBuffer);
		}
	}
} // namespace W