#pragma once
#include <stdarg.h>

namespace W
{
	namespace Text
	{
		bool IsAscii(const char* text);

		void Format(char* outBuffer, int outBufferSize, const char* format, ...);
		void Format(char* outBuffer, int outBufferSize, const char* format, va_list args);

		template <int N, typename ...ARGS>
		void Format(char(&outBuffer)[N], const char* format, ARGS... args)
		{
			Format(outBuffer, N, format, args...);
		}

		template <int N>
		void Format(char(&outBuffer)[N], const char* format, va_list args)
		{
			Format(outBuffer, N, format, args);
		}

		namespace UTF8
		{
			void Encode(const wchar_t* str, char* outBuffer, int outBufferSize);
			void Decode(const char* str, wchar_t* outBuffer, int outBufferSize);

			template <int N>
			void Encode(const wchar_t* str, char(&outBuffer)[N])
			{
				Encode(str, outBuffer, N);
			}

			template <int N>
			void Decode(const char* str, wchar_t(&outBuffer)[N])
			{
				Decode(str, outBuffer, N);
			}
		} // namespace UTF8
	} // namespace Text
} // namespace W