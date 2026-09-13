module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanInstance::VulkanInstance(const VkInstanceCreateInfo* pCreateInfo, std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator)
{
	VkResult result{ vkCreateInstance(pCreateInfo, GetAllocator(), &_instance)};

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan instance");
}

VulkanInstance::~VulkanInstance()
{
	if (_instance)
	{
		vkDestroyInstance(_instance, GetAllocator());
		_instance = VK_NULL_HANDLE;
	}
}

VkInstance VulkanInstance::GetInstance() const
{
	return _instance;
}