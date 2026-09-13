module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanCommandPool::VulkanCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateCommandPool(_device, pCreateInfo, GetAllocator(), &_commandPool) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan command pool");
}

VulkanCommandPool::~VulkanCommandPool()
{
	if (_commandPool)
	{
		vkDestroyCommandPool(_device, _commandPool, GetAllocator());
		_commandPool = VK_NULL_HANDLE;
	}
}

VkCommandPool VulkanCommandPool::GetCommandPool() const
{
	return _commandPool;
}