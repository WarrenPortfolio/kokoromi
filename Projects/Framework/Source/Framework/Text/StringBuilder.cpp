#include "StringBuilder.hpp"
#include <Framework/Text/Text.hpp>

#include <stdio.h>
#include <stdarg.h>

namespace W
{
    StringBuilder::StringBuilder()
    {
        mData.reserve(256);
        mData.emplace_back('\0');
    }

    const char* StringBuilder::Text()
    {
        return &mData[0];
    }

    void StringBuilder::Clear()
    {
        mData.clear();
        mData.emplace_back('\0');
    }

    void StringBuilder::Append(const char* text)
    {
        if (Text::IsNullOrEmpty(text))
            return;

        size_t text_length = strlen(text);
        size_t required_capacity = mData.size() + text_length;

        if (mData.capacity() < required_capacity)
        {
            size_t reserve_size = required_capacity + required_capacity;
            mData.reserve(reserve_size);
        }
        
        mData.insert(mData.end() - 1, text, text + text_length);
    }

	void StringBuilder::AppendFormat(const char* format, ...)
	{
        va_list args;
        va_start(args, format);

        char buffer[256] = {};
        Text::Format(buffer, format, args);

        va_end(args);

        // append formatted string
        Append(buffer);
	}
} // namespace W