module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanComputePipeline::VulkanComputePipeline(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, pCreateInfo, GetAllocator(), &_pipeline) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan compute pipeline");
}

VulkanComputePipeline::~VulkanComputePipeline()
{
	if (_pipeline)
	{
		vkDestroyPipeline(_device, _pipeline, GetAllocator());
		_pipeline = VK_NULL_HANDLE;
	}
}

VkPipeline VulkanComputePipeline::GetPipeline() const
{
	return _pipeline;
}