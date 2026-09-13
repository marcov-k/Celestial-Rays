module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreatePipelineLayout(_device, pCreateInfo, GetAllocator(), &_pipelineLayout) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan pipeline layout");
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
	if (_pipelineLayout)
	{
		vkDestroyPipelineLayout(_device, _pipelineLayout, GetAllocator());
		_pipelineLayout = VK_NULL_HANDLE;
	}
}

VkPipelineLayout VulkanPipelineLayout::GetPipelineLayout() const
{
	return _pipelineLayout;
}