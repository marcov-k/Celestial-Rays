module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

export module Celestial.Vulkan;

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

	VkInstance _instance{};
	VkDebugUtilsMessengerEXT _debugMessenger{};
	VkSurfaceKHR _surface{};
	VkPhysicalDevice _physicalDevice{};
	VkDevice _device{};
	std::uint32_t _queueFamily{ UINT32_MAX };
	VkQueue _queue{};

	VkSwapchainKHR _swapchain{};
	VkFormat _swapchainFormat{};
	VkExtent2D _swapchainExtent{};
	std::vector<VkImage> _swapchainImages;

	VkImage _renderImage{};
	VkDeviceMemory _renderImageMemory{};
	VkImageView _renderImageView{};
	VkFormat _renderImageFormat{};
	VkExtent2D _renderImageExtent{};

	void CreateInstance();
	void CreateDebugMessenger();
	void CreateSurface();

	void PickPhysicalDevice();
	void CreateLogicalDevice();

	void CreateSwapchain();
	void CreateRenderImage();
};