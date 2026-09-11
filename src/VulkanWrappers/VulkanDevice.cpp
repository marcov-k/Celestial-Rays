module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator)
{
	VkResult result{ vkCreateDevice(physicalDevice, pCreateInfo, GetAllocator(), &_device) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan logical device");
}

VulkanDevice::~VulkanDevice()
{
	if (_device)
	{
		vkDestroyDevice(_device, GetAllocator());
		_device = VK_NULL_HANDLE;
	}
}

VkDevice VulkanDevice::GetDevice() const
{
	return _device;
}