module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanImage::VulkanImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateImage(_device, pCreateInfo, GetAllocator(), &_image) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan image");
}

VulkanImage::~VulkanImage()
{
	if (_image)
	{
		vkDestroyImage(_device, _image, GetAllocator());
		_image = VK_NULL_HANDLE;
	}
}

VkImage VulkanImage::GetImage() const
{
	return _image;
}