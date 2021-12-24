#pragma once
#include <Framework/Debug/Debug.hpp>

#include <vulkan/vulkan.h>

#include <vector>

namespace W
{
	namespace VK
	{
		// settings
		extern bool s_enableValidationLayers;

		// debug
		const char* TraslateResult(VkResult result);

		// device setup
		void CreateInstance(VkInstance* instance);
		void CreateDebugReport(VkInstance instance, VkDebugReportCallbackEXT* callback);
		void PickPhysicalDevice(VkInstance instance, VkPhysicalDevice* physicalDevice);
		void CreateDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool);

		// window setup
		void CreateWindowSurface(uintptr_t hwnd, VkInstance instance, VkSurfaceKHR* surface);


		std::vector<const char*> GetRequiredExtensions();
		std::vector<const char*> GetValidationLayers();

		bool CheckValidationLayerSupport();

		uint32_t CalcDeviceScore(VkPhysicalDevice device);
		VkResult GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat& outDepthFormat);
	} // namespace VK
} // namespace W

#define VK_CHECK(result) Debug_AssertMsg(result == VK_SUCCESS, "%s(%d)", ::W::VK::TraslateResult(result), (int)result)
