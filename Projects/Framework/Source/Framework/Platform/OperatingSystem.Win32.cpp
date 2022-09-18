#define _CRT_SECURE_NO_WARNINGS

#include "OperatingSystem.hpp"

#include <Framework/Debug/Debug.hpp>

#include <string>

#include <windows.h>
#undef GetEnvironmentVariable
#undef CreateDirectory

namespace W
{
	const char* OS::GetEnvironmentVariable(const char* variableName)
	{
		return getenv(variableName);
	}

	void OS::CreateDirectory(const char* path)
	{
		BOOL result = CreateDirectoryA(path, NULL);
		Debug_AssertMsg(result != ERROR_PATH_NOT_FOUND, "CreateDirectory - One or more intermediate directories do not exist; this function will only create the final directory in the path.");
	}
} // namespace W