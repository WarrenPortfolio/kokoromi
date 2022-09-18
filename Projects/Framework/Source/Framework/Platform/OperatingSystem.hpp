#include <Framework/Text/Text.hpp>
#include <Framework/Debug/Debug.hpp>

#include <memory>
#include <string>

namespace W
{
    namespace OS
    {
        const char* GetEnvironmentVariable(const char* variableName);

        void CreateDirectory(const char* path);
    } // namespace OS
} // namespace W