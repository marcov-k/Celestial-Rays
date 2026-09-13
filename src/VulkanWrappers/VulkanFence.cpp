module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanFence::VulkanFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateFence(_device, pCreateInfo, GetAllocator(), &_fence) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan fence");
}

VulkanFence::~VulkanFence()
{
	if (_fence)
	{
		vkDestroyFence(_device, _fence, GetAllocator());
		_fence = VK_NULL_HANDLE;
	}
}

VkFence VulkanFence::GetFence() const
{
	return _fence;
}