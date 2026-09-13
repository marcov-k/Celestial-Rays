module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateDescriptorPool(_device, pCreateInfo, GetAllocator(), &_descriptorPool) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan descriptor pool");
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	if (_descriptorPool)
	{
		vkDestroyDescriptorPool(_device, _descriptorPool, GetAllocator());
		_descriptorPool = VK_NULL_HANDLE;
	}
}

VkDescriptorPool VulkanDescriptorPool::GetDescriptorPool() const
{
	return _descriptorPool;
}