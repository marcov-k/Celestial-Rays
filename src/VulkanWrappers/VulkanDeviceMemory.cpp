module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanDeviceMemory::VulkanDeviceMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkAllocateMemory(_device, pAllocateInfo, GetAllocator(), &_memory) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate Vulkan device memory");
}

VulkanDeviceMemory::~VulkanDeviceMemory()
{
	if (_memory)
	{
		vkFreeMemory(_device, _memory, GetAllocator());
		_memory = VK_NULL_HANDLE;
	}
}

VkDeviceMemory VulkanDeviceMemory::GetMemory() const
{
	return _memory;
}