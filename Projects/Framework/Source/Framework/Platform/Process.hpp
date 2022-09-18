#include <Framework/Text/Text.hpp>
#include <Framework/Debug/Debug.hpp>

#include <memory>
#include <string>

namespace W
{
    class Process
    {
    private:
        struct PlatformImpl;
        std::unique_ptr<PlatformImpl> mImpl;

        std::string mOutput;

    public:
        Process();
        ~Process();

    public:
        void Start(const char* commandLine);

        bool IsRunning() const;
        void WaitForExit() const;

        uint32_t GetExitCode() const;
        const char* GetOutputText() const;

        void ReadOutput();

        void Close();
    };
} // namespace W