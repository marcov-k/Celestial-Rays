module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanShaderModule::VulkanShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _device(device)
{
	VkResult result{ vkCreateShaderModule(_device, pCreateInfo, GetAllocator(), &_shaderModule) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan shader module");
}

VulkanShaderModule::~VulkanShaderModule()
{
	if (_shaderModule)
	{
		vkDestroyShaderModule(_device, _shaderModule, GetAllocator());
		_shaderModule = VK_NULL_HANDLE;
	}
}

VkShaderModule VulkanShaderModule::GetShaderModule() const
{
	return _shaderModule;
}