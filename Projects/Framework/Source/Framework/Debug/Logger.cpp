#include "Logger.hpp"
#include <Framework/Text/Text.hpp>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace W
{
	void Logger::PrintFormat(const char* format, ...)
	{
		// stack buffer to format the string into
		char buffer[8192];

		va_list args;
		va_start(args, format);
		Text::Format(buffer, format, args);
		va_end(args);

		// assign the new value
		Print(buffer);
	}

	void Logger::AssertFailure(const char* filePath, int lineNumber, const char* condition, const char* message, ...)
	{
		// stack buffer to format the string into
		char buffer[2048];
		if (message != nullptr)
		{
			va_list args;
			va_start(args, message);
			Text::Format(buffer, message, args);
			va_end(args);
		}
		else
		{
			strcpy_s(buffer, "[ASSERT]");
		}

		PrintFormat(
			"\n"
			"+---------------------------------------+\n"
			"|             ASSERT FAILED             |\n"
			"+---------------------------------------+\n"
			"%s(%d):\n"
			"Condition: %s\n"
			"Message: %s\n"
			"\n",
			filePath, lineNumber,
			condition,
			buffer);
	}
} // namespace W