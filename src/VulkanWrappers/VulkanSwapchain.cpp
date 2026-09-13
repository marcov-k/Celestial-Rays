module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanSwapchain::VulkanSwapchain(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateSwapchainKHR(_device, pCreateInfo, GetAllocator(), &_swapchain) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan swapchain");
}

VulkanSwapchain::~VulkanSwapchain()
{
	if (_swapchain)
	{
		vkDestroySwapchainKHR(_device, _swapchain, GetAllocator());
		_swapchain = VK_NULL_HANDLE;
	}
}

VkSwapchainKHR VulkanSwapchain::GetSwapchain() const
{
	return _swapchain;
}