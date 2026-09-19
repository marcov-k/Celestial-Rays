module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanSemaphore::VulkanSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, std::optional<VkAllocationCallbacks> allocator)
	: VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateSemaphore(_device, pCreateInfo, GetAllocator(), &_semaphore) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan semaphore");
}

VulkanSemaphore::~VulkanSemaphore()
{
	if (_semaphore)
	{
		vkDestroySemaphore(_device, _semaphore, GetAllocator());
		_semaphore = VK_NULL_HANDLE;
	}
}

VkSemaphore VulkanSemaphore::GetSemaphore() const
{
	return _semaphore;
}