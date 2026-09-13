module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanDebugMessenger::VulkanDebugMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	std::optional<VkAllocationCallbacks> allocator) : VulkanResource(allocator), _instance(instance)
{
	auto createDebugFn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT"));

	if (!createDebugFn) throw std::runtime_error("Failed to retrieve instance debug messenger creator");

	VkResult result{ createDebugFn(_instance, pCreateInfo, GetAllocator(), &_debugMessenger) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan debug messenger");
}

VulkanDebugMessenger::~VulkanDebugMessenger()
{
	if (_debugMessenger)
	{
		auto destroyDebugFn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (destroyDebugFn) destroyDebugFn(_instance, _debugMessenger, GetAllocator());
		_debugMessenger = VK_NULL_HANDLE;
	}
}

VkDebugUtilsMessengerEXT VulkanDebugMessenger::GetDebugMessenger() const
{
	return _debugMessenger;
}