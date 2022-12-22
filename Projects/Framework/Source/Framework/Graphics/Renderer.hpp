#pragma once

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <memory>

namespace W
{
	struct QueueFamilyIndices
	{
		int GraphicsFamily = -1;
		int PresentFamily = -1;

		bool IsComplete()
		{
			return GraphicsFamily >= 0 && PresentFamily >= 0;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void Startup();
		void Shutdown();

		void FrameUpdate(float deltaTime);
		void FrameRender();
		void FramePresent();

	private:

		VkDebugReportCallbackEXT mCallbackExt = VK_NULL_HANDLE;

		VkInstance mInstance = VK_NULL_HANDLE;
		VkSurfaceKHR mSurface = VK_NULL_HANDLE;

		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;

		VkQueue mGraphicsQueue = VK_NULL_HANDLE;
		VkQueue mPresentQueue = VK_NULL_HANDLE;

		VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> mSwapChainImages;
		VkFormat mSwapChainImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D mSwapChainExtent = {};
		std::vector<VkImageView> mSwapChainImageViews;
		std::vector<VkFramebuffer> mSwapChainFramebuffers;

		VkRenderPass mRenderPass = VK_NULL_HANDLE;
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout mDescriptorSetLayout2 = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;

		VkCommandPool mCommandPool = VK_NULL_HANDLE;

		VkImage mDepthImage = VK_NULL_HANDLE;
		VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
		VkImageView mDepthImageView = VK_NULL_HANDLE;

		VkBuffer mUniformBuffers = VK_NULL_HANDLE;
		VkDeviceMemory mUniformBuffersMemory = VK_NULL_HANDLE;

		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

		uint32_t mCurrentFrame = 0;
		uint32_t imageIndex = 0;

		struct FrameData
		{
			VkCommandBuffer     CommandBuffer;
			VkFence             Fence;
			VkSemaphore         ImageAcquiredSemaphore;
			VkSemaphore         RenderCompleteSemaphore;
		};

		std::vector<FrameData> mFrameData;

		VkAllocationCallbacks mAllocationCallbacks = {};

		bool mFrameBufferResized = false;

	private:
		void InitRenderDoc();
		void InitVulkan();
		void InitImGui();

		void CleanupSwapChain();
		void RecreateSwapChain();

		void CreateLogicalDevice();

		void CreateSwapChain();
		void CreateFrameData();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateDepthResources();

		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void CreateDescriptorPool();
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		std::vector<const char*> GetRequiredExtensions();
		bool CheckValidationLayerSupport();
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};

} // namespace W
