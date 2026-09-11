module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <optional>

module Celestial.Vulkan.Wrappers;

VulkanResource::VulkanResource(std::optional<VkAllocationCallbacks> allocator) : _allocator(allocator) { }

const VkAllocationCallbacks* VulkanResource::GetAllocator() const
{
	return _allocator.has_value() ? &_allocator.value() : nullptr;
}