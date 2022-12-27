#include "Vk.Graphics.hpp"

#include <Framework/Debug/Debug.hpp>

namespace W
{
	bool VK::s_enableValidationLayers = true;

	//////////////////////////////////////////////////////////////////////////
	//                         Vulkan Debug Layer                           //
	//////////////////////////////////////////////////////////////////////////

	static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static VkResult DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
		{
			func(instance, callback, pAllocator);
			return VK_SUCCESS;
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
	{
		Logger::Print(msg);
		return VK_FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	//                                Vulkan                                //
	//////////////////////////////////////////////////////////////////////////

	const char* VK::TraslateResult(VkResult result)
	{
		switch (result)
		{
#define RESULT_STR(v) case VK_ ##v: return #v
			RESULT_STR(SUCCESS);
			RESULT_STR(NOT_READY);
			RESULT_STR(TIMEOUT);
			RESULT_STR(EVENT_SET);
			RESULT_STR(EVENT_RESET);
			RESULT_STR(INCOMPLETE);
			RESULT_STR(ERROR_OUT_OF_HOST_MEMORY);
			RESULT_STR(ERROR_OUT_OF_DEVICE_MEMORY);
			RESULT_STR(ERROR_INITIALIZATION_FAILED);
			RESULT_STR(ERROR_DEVICE_LOST);
			RESULT_STR(ERROR_MEMORY_MAP_FAILED);
			RESULT_STR(ERROR_LAYER_NOT_PRESENT);
			RESULT_STR(ERROR_EXTENSION_NOT_PRESENT);
			RESULT_STR(ERROR_FEATURE_NOT_PRESENT);
			RESULT_STR(ERROR_INCOMPATIBLE_DRIVER);
			RESULT_STR(ERROR_TOO_MANY_OBJECTS);
			RESULT_STR(ERROR_FORMAT_NOT_SUPPORTED);
			RESULT_STR(ERROR_FRAGMENTED_POOL);
			RESULT_STR(ERROR_UNKNOWN);
			RESULT_STR(ERROR_OUT_OF_POOL_MEMORY);
			RESULT_STR(ERROR_INVALID_EXTERNAL_HANDLE);
			RESULT_STR(ERROR_FRAGMENTATION);
			RESULT_STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
			RESULT_STR(ERROR_SURFACE_LOST_KHR);
			RESULT_STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
			RESULT_STR(SUBOPTIMAL_KHR);
			RESULT_STR(ERROR_OUT_OF_DATE_KHR);
			RESULT_STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
			RESULT_STR(ERROR_VALIDATION_FAILED_EXT);
			RESULT_STR(ERROR_INVALID_SHADER_NV);
			RESULT_STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
			RESULT_STR(ERROR_NOT_PERMITTED_EXT);
			RESULT_STR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
			RESULT_STR(THREAD_IDLE_KHR);
			RESULT_STR(THREAD_DONE_KHR);
			RESULT_STR(OPERATION_DEFERRED_KHR);
			RESULT_STR(OPERATION_NOT_DEFERRED_KHR);
			RESULT_STR(PIPELINE_COMPILE_REQUIRED_EXT);
#undef RESULT_STR
		default:
			Debug_AssertMsg(false, "Unknown VkResult: %d", result);
			return "UNKNOWN_RESULT_CODE";
		}
	};

	std::vector<const char*> VK::GetValidationLayers()
	{
		const std::vector<const char*> validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		return validationLayers;
	}

	bool VK::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		std::vector<const char*> validationLayers = GetValidationLayers();
		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void VK::CreateInstance(VkInstance* instance)
	{
		if (s_enableValidationLayers)
		{
			Debug_AssertMsg(CheckValidationLayerSupport(), "validation layers requested, but not available!");
		}

		std::vector<const char*> extensions = GetRequiredExtensions();
		std::vector<const char*> validationLayers = GetValidationLayers();

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "unknown";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "kokoromi";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (s_enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		VK_CHECK(vkCreateInstance(&createInfo, nullptr, instance));
	}

	void VK::CreateDebugReport(VkInstance instance, VkDebugReportCallbackEXT* callback)
	{
		if (!s_enableValidationLayers)
			return;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		createInfo.pfnCallback = DebugReportCallback;

		VK_CHECK(CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, callback));
	}

	void VK::DestroyDebugReport(VkInstance instance, VkDebugReportCallbackEXT callback)
	{
		if (!s_enableValidationLayers)
			return;

		VK_CHECK(DestroyDebugReportCallbackEXT(instance, callback, nullptr));
	}


	void VK::PickPhysicalDevice(VkInstance instance, VkPhysicalDevice* physicalDevice)
	{
		uint32_t deviceCount = 0;
		VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
		Debug_AssertMsg(deviceCount > 0, "failed to find a GPU with Vulkan support!");

		std::vector<VkPhysicalDevice> devices;
		devices.resize(deviceCount, VK_NULL_HANDLE);

		VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

		constexpr uint32_t InvalidIndex = ~0;

		uint32_t deviceIndex = InvalidIndex;
		uint32_t deviceScore = 0;

		for (uint32_t i = 0; i < devices.size(); ++i)
		{
			uint32_t score = CalcDeviceScore(devices[i]);
			if (score > deviceScore)
			{
				deviceScore = score;
				deviceIndex = i;
			}
		}

		Debug_AssertMsg(deviceIndex != InvalidIndex, "failed to find a suitable GPU!");
		if (deviceIndex != InvalidIndex)
		{
			(*physicalDevice) = devices[deviceIndex];
		}
	}

	void VK::CreateDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool)
	{
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		pool_info.pPoolSizes = poolSizes.data();
		VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, descriptorPool));

	}

	uint32_t VK::CalcDeviceScore(VkPhysicalDevice device)
	{
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		// required features
		if (!supportedFeatures.samplerAnisotropy)
		{
			return 0;
		}

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		int score = 1;

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 5;
		}
		else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			score += 10; // favor integrated GPU
		}

		return score;
	}


	VkResult VK::GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat& outDepthFormat)
	{
		// Since all depth formats may be optional, we need to find a suitable depth format to use ordered by priority
		VkFormat depthFormats[] =
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (VkFormat& format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

			// Format must support depth stencil attachment for optimal tiling
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				outDepthFormat = format;
				return VK_SUCCESS;
			}
		}

		outDepthFormat = VK_FORMAT_UNDEFINED;
		return VK_ERROR_FORMAT_NOT_SUPPORTED;
	}
} // namespace W