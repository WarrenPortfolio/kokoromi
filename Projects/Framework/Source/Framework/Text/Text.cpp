#include "Text.hpp"

#include <stdio.h>

namespace W
{
	bool Text::IsAscii(const char* text)
	{
		while (*text != '\0')
		{
			if (!('\x00' <= *text && *text <= '\x7f'))
				return false;
			++text;
		}
		return true;
	}

	void Text::Format(char* outBuffer, int outBufferSize, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Format(outBuffer, outBufferSize, format, args);
		va_end(args);
	}

	void Text::Format(char* outBuffer, int outBufferSize, const char* format, va_list args)
	{
		vsprintf_s(outBuffer, outBufferSize, format, args);
	}
} // namespace W