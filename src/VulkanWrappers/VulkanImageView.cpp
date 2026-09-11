module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanImageView::VulkanImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateImageView(_device, pCreateInfo, GetAllocator(), &_imageView) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan image view");
}

VulkanImageView::~VulkanImageView()
{
	if (_imageView)
	{
		vkDestroyImageView(_device, _imageView, GetAllocator());
		_imageView = VK_NULL_HANDLE;
	}
}

VkImageView VulkanImageView::GetImageView() const
{
	return _imageView;
}