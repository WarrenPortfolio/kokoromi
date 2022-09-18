#include "Process.hpp"

#include <Framework/Text/Text.hpp>

#include <string>

#include <windows.h>

namespace W
{
	struct Process::PlatformImpl
	{
		HANDLE mStdInRead = NULL;
		HANDLE mStdInWrite = NULL;
		HANDLE mStdOutRead = NULL;
		HANDLE mStdOutWrite = NULL;
		PROCESS_INFORMATION mProcessInfo = {};

		PlatformImpl() = default;
		~PlatformImpl() { Close(); }

		void Close()
		{
			// Close process and thread handles. 
			CloseHandle(mProcessInfo.hProcess);
			CloseHandle(mProcessInfo.hThread);

			// Close stdin and stdout pipe handles.
			CloseHandle(mStdInRead);
			CloseHandle(mStdInWrite);
			CloseHandle(mStdOutRead);
			CloseHandle(mStdOutWrite);
		}
	};

	Process::Process()
	{
		mImpl = std::make_unique<PlatformImpl>();
	}

	Process::~Process() = default;

	void Process::Start(const char* commandLine)
	{
		Close();

		SECURITY_ATTRIBUTES security_attributes = {};
		security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		security_attributes.bInheritHandle = TRUE;
		security_attributes.lpSecurityDescriptor = NULL;

		BOOL error = 0;

		// Create a pipe for the child process's STDOUT. 

		error = CreatePipe(&mImpl->mStdOutRead, &mImpl->mStdOutWrite, &security_attributes, 0);
		Debug_Assert(error != 0);

		// Ensure the read handle to the pipe for STDOUT is not inherited.
		error = SetHandleInformation(mImpl->mStdOutRead, HANDLE_FLAG_INHERIT, 0);
		Debug_Assert(error != 0);

		STARTUPINFO startupInfo = {};
		startupInfo.cb = sizeof(startupInfo);
		startupInfo.hStdError = mImpl->mStdOutWrite;
		startupInfo.hStdOutput = mImpl->mStdOutWrite;
		startupInfo.hStdInput = mImpl->mStdOutRead;
		startupInfo.wShowWindow = FALSE;
		startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

		// convert to wide character command line
		wchar_t wchar_command_line[4096];
		Text::UTF8::Decode(commandLine, wchar_command_line);

		// Start the child process. If the function succeeds, the return value is nonzero.
		error = CreateProcessW(
			NULL,                   // No module name (use command line)
			wchar_command_line,     // Command line
			NULL,                   // Process handle not inheritable
			NULL,                   // Thread handle not inheritable
			TRUE,					// Set handle inheritance
			0,                      // No creation flags
			NULL,                   // Use parent's environment block
			NULL,                   // Use parent's starting directory
			&startupInfo,           // Pointer to STARTUPINFO structure
			&mImpl->mProcessInfo    // Pointer to PROCESS_INFORMATION structure
		);
		Debug_Assert(error != 0, "CreateProcess - ErrorCode=%d\n", GetLastError());
	}

	bool Process::IsRunning() const
	{
		if (mImpl && mImpl->mProcessInfo.hProcess == nullptr)
			return false;

		DWORD result = WaitForSingleObject(mImpl->mProcessInfo.hProcess, 0);
		return (result == WAIT_TIMEOUT);
	}

	void Process::WaitForExit() const
	{
		if (mImpl && mImpl->mProcessInfo.hProcess == nullptr)
			return;

		WaitForSingleObject(mImpl->mProcessInfo.hProcess, INFINITE);
	}

	uint32_t Process::GetExitCode() const
	{
		DWORD exit_code = 0;

		// GetExitCodeProcess - If the function succeeds, the return value is nonzero.
		BOOL GetExitCodeProcess_Result = GetExitCodeProcess(mImpl->mProcessInfo.hProcess, &exit_code);
		Debug_AssertMsg(GetExitCodeProcess_Result != 0, "GetExitCodeProcess");
		return exit_code;
	}

	const char* Process::GetOutputText() const
	{
		return mOutput.c_str();
	}

	void Process::ReadOutput()
	{
		if (mImpl)
		{
			DWORD bytes_available = 0;
			PeekNamedPipe(mImpl->mStdOutRead, NULL, NULL, NULL, &bytes_available, NULL);

			if (bytes_available > 0)
			{
				size_t current_length = mOutput.length();
				mOutput.resize(current_length + bytes_available);

				DWORD bytes_read = 0;
				BOOL ReadFile_Result = ReadFile(mImpl->mStdOutRead, (LPVOID)&mOutput[current_length], bytes_available, &bytes_read, NULL);
				Debug_AssertMsg(ReadFile_Result != 0, "ReadFile");

				mOutput.resize(current_length + bytes_read);
			}
		}
	}

	void Process::Close()
	{
		mImpl->Close();
		mOutput.clear();
	}
} // namespace W