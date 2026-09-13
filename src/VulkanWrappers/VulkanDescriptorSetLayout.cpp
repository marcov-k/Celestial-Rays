module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateDescriptorSetLayout(_device, pCreateInfo, GetAllocator(), &_setLayout) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan descriptor set layout");
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	if (_setLayout)
	{
		vkDestroyDescriptorSetLayout(_device, _setLayout, GetAllocator());
		_setLayout = VK_NULL_HANDLE;
	}
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::GetDescriptorSetLayout() const
{
	return _setLayout;
}