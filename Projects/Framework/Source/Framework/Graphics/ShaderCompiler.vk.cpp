#include "ShaderCompiler.hpp"

#include <Framework/Debug/Debug.hpp>
#include <Framework/Platform/Process.hpp>
#include <Framework/Platform/OperatingSystem.hpp>
#include <Framework/Text/StringBuilder.hpp>

namespace W
{
	void ShaderCompiler::Compile_SPIRV_GLSLC()
	{
		OS::CreateDirectory("build");
		OS::CreateDirectory("build\\Data");
		OS::CreateDirectory("build\\Data\\Shaders");

		const char* vulkan_sdk_path = OS::GetEnvironmentVariable("VULKAN_SDK");
		Debug_AssertMsg(vulkan_sdk_path != nullptr, "missing VULKAN_SDK environment variable");

		StringBuilder command_line;
		Process process;

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

	enum class ShaderStage
	{
		Vertex,
		Pixel,
		Geom,
		Hull,
		Domain,
		Compute,
	};

	static const char* GetShaderTargetProfile(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:	return "vs_5_0";
		case ShaderStage::Pixel:	return "ps_5_0";
		case ShaderStage::Geom:		return "gs_5_0";
		case ShaderStage::Hull:		return "hs_5_0";
		case ShaderStage::Domain:	return "ds_5_0";
		case ShaderStage::Compute:	return "cs_5_0";
		default: Debug_AssertMsg(false, "Missing Shader Target Profile"); break;
		}

		return nullptr;
	}

	void ShaderCompiler::Compile_SPIRV_DXC()
	{
		OS::CreateDirectory("build");
		OS::CreateDirectory("build\\Data");
		OS::CreateDirectory("build\\Data\\Shaders");

		const char* vulkan_sdk_path = OS::GetEnvironmentVariable("VULKAN_SDK");
		Debug_AssertMsg(vulkan_sdk_path != nullptr, "missing VULKAN_SDK environment variable");

		StringBuilder command_line;
		Process process;

		const char* shader_build_list[] = {
			"Data\\Shaders\\Debug.hlsl",
		};

		for (const char* shader_path : shader_build_list)
		{
			ShaderStage shader_stage = ShaderStage::Vertex;
			const char* entry_point = "PS";

			command_line.Clear();
			command_line.AppendFormat("%s\\Bin\\dxc.exe", vulkan_sdk_path);

			command_line.Append(" -Zpr"); // Pack matrices in row-major order.
			command_line.Append(" -Ges"); // Enable strict mode.
			command_line.Append(" -WX"); // Treat warnings as errors.
			// command_line.Append(" -no-warnings"); // Suppresses all warnings

			command_line.Append(" -spirv"); // Generates SPIR-V code.
			command_line.Append(" -fspv-reflect"); // Emits additional SPIR-V instructions to aid reflection.

			// Shader profile
			const char* profile_name = GetShaderTargetProfile(shader_stage);

			// Command line
			command_line.AppendFormat(" -T %s", profile_name);
			command_line.AppendFormat(" -E %s", entry_point);
			command_line.AppendFormat(" -Fo build\\%s.spv", shader_path);

			command_line.AppendFormat(" %s", shader_path);

			process.Start(command_line.Text());
			process.WaitForExit();
			if (process.GetExitCode() != 0)
			{
				process.ReadOutput();
				Debug_AssertMsg(false, "Shader Build Failed - %s\n%s", shader_path, process.GetOutputText());
			}
		}
	}

} // namespace W
