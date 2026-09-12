module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <vector>

export module Celestial.Vulkan;

import Celestial.Vulkan.Wrappers;
import Celestial.Window;

export class VulkanContext
{
public:
	explicit VulkanContext(const Window& window);
	~VulkanContext();

	VulkanContext(const VulkanContext&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;

private:
	const Window& _window;

	std::optional<VulkanInstance> _instance;
	std::optional<VulkanDebugMessenger> _debugMessenger;
	std::optional<VulkanSurface> _surface;
	VkPhysicalDevice _physicalDevice{};
	std::optional<VulkanDevice> _device;
	std::uint32_t _queueFamily{ UINT32_MAX };
	VkQueue _queue{};

	std::optional<VulkanSwapchain> _swapchain;
	VkFormat _swapchainFormat{};
	VkExtent2D _swapchainExtent{};
	std::vector<VkImage> _swapchainImages;

	std::optional<VulkanDeviceMemory> _renderImageMemory;
	std::optional<VulkanImage> _renderImage;
	std::optional<VulkanImageView> _renderImageView;
	VkFormat _renderImageFormat{};
	VkExtent2D _renderImageExtent{};

	std::optional<VulkanCommandPool> _commandPool;
	VkCommandBuffer _commandBuffer{};

	void CreateInstance();
	void CreateDebugMessenger();
	void CreateSurface();

	void PickPhysicalDevice();
	void CreateLogicalDevice();

	void CreateSwapchain();
	void CreateRenderImage();

	void CreateCommandPool();
	void AllocateCommandBuffer();
};