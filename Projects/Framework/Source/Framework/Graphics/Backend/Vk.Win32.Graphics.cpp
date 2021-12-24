#define VK_USE_PLATFORM_WIN32_KHR
#include "Vk.Graphics.hpp"

namespace W
{
	std::vector<const char*> VK::GetRequiredExtensions()
	{
		std::vector<const char*> extensions =
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		};

		if (s_enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	void VK::CreateWindowSurface(uintptr_t hwnd, VkInstance instance, VkSurfaceKHR* surface)
	{
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)hwnd;
		createInfo.hinstance = GetModuleHandle(nullptr);

		VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, surface));
	}
} // namespace W