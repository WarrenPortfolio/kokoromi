#include "Build.h"

#include <Framework/Debug/Debug.hpp>
#include <Framework/Platform/Process.hpp>
#include <Framework/Platform/OperatingSystem.hpp>
#include <Framework/Text/StringBuilder.hpp>

//////////////////////////////////////////////////////////////////////////
//                          CompileShaders                              //
//////////////////////////////////////////////////////////////////////////
void Build::CompileShaders()
{
	W::OS::CreateDirectory("build");
	W::OS::CreateDirectory("build\\Data");
	W::OS::CreateDirectory("build\\Data\\Shaders");

	const char* vulkan_sdk_path = W::OS::GetEnvironmentVariable("VULKAN_SDK");
	Debug_AssertMsg(vulkan_sdk_path != nullptr, "missing VULKAN_SDK environment variable");

	W::StringBuilder command_line;
	W::Process process;

	const char* shader_build_list[] = {
		"Data\\Shaders\\shader.vert",
		"Data\\Shaders\\shader.frag",
	};

	for (const char* shader_path : shader_build_list)
	{
		command_line.Clear();
		command_line.AppendFormat("%s\\Bin\\glslc.exe", vulkan_sdk_path);
		command_line.AppendFormat(" %s", shader_path);
		command_line.AppendFormat(" -o build\\%s.spv", shader_path);

		process.Start(command_line.Text());
		process.WaitForExit();
		if (process.GetExitCode() != 0)
		{
			process.ReadOutput();
			Debug_AssertMsg(false, "Shader Build Failed - %s\n%s", shader_path, process.GetOutputText());
		}
	}
}