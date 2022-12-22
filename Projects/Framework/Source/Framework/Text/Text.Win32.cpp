#include <Framework/Text/Text.hpp>
#include <Framework/Debug/Debug.hpp>

#include <windows.h>

namespace W
{
    // Convert a wide Unicode string to an UTF8 string
    void Text::UTF8::Encode(const wchar_t* str, char* outBuffer, int outBufferSize)
    {       
        int strLength = lstrlenW(str);
        if (str <= 0)
        {
            outBuffer[0] = '\0';
            return;
        }

        int requiredSize = WideCharToMultiByte(CP_UTF8, 0, str, strLength, NULL, 0, NULL, NULL);
        Debug_Assert(requiredSize < outBufferSize);
        
        WideCharToMultiByte(CP_UTF8, 0, str, strLength + 1, outBuffer, outBufferSize, NULL, NULL);
    }

    // Convert an UTF8 string to a wide Unicode string
    void Text::UTF8::Decode(const char* str, wchar_t* outBuffer, int outBufferSize)
    {
        int strLength = lstrlenA(str);
        if (str <= 0)
        {
            outBuffer[0] = L'\0';
            return;
        }

        int requiredSize = MultiByteToWideChar(CP_UTF8, 0, str, strLength, NULL, 0);
        Debug_Assert(requiredSize < outBufferSize);

        MultiByteToWideChar(CP_UTF8, 0, str, strLength + 1, outBuffer, outBufferSize);
    }
} // namespace W