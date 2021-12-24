#pragma once

#include <stdint.h>

namespace W
{
	namespace Logger
	{
		void Print(const char* text);
		void PrintFormat(const char* format, ...);

		void AssertFailure(const char* filePath, int lineNumber, const char* condition, const char* message, ...);
	} // namespace Logger
} // namespace W