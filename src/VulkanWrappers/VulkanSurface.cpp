module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanSurface::VulkanSurface(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _instance(instance)
{
	VkResult result{ vkCreateWin32SurfaceKHR(_instance, pCreateInfo, GetAllocator(), &_surface) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan surface");
}

VulkanSurface::~VulkanSurface()
{
	if (_surface)
	{
		vkDestroySurfaceKHR(_instance, _surface, GetAllocator());
		_surface = VK_NULL_HANDLE;
	}
}

VkSurfaceKHR VulkanSurface::GetSurface() const
{
	return _surface;
}