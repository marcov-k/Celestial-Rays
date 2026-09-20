module;

#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>
#include <fstream>
#include <optional>
#include <print>
#include <stdexcept>
#include <string_view>
#include <vector>

module Celestial.Vulkan;

namespace
{
	constexpr const char* ValidationLayer = "VK_LAYER_KHRONOS_validation";

	constexpr std::uint32_t LayerCount = 1;
	const char* ValidationLayers[] = {
		ValidationLayer
	};

	constexpr std::uint32_t ExtensionCount = 3;
	const char* Extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	constexpr std::uint32_t DeviceExtensionCount = 1;
	const char* DeviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severityFlag,
		VkDebugUtilsMessageTypeFlagsEXT typeFlag,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* pUserData);

	constexpr VkDebugUtilsMessengerCreateInfoEXT MessengerInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = nullptr,
		.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugMessengerCallback
	};

	struct QueueFamilyIndices
	{
		std::uint32_t graphics{ UINT32_MAX };
		std::uint32_t compute{ UINT32_MAX };
		std::uint32_t present{ UINT32_MAX };

		bool Complete() const noexcept
		{
			return graphics != UINT32_MAX && graphics == compute && graphics == present;
		}
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	bool CheckValidationLayerSupport()
	{
		std::uint32_t layerCount{};
		VkResult result{ vkEnumerateInstanceLayerProperties(&layerCount, nullptr) };

		if (result != VK_SUCCESS) return false;

		std::vector<VkLayerProperties> availableLayers(layerCount);
		result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		if (result != VK_SUCCESS) return false;

		for (const auto& layer : availableLayers)
		{
			if (std::strcmp(layer.layerName, ValidationLayer) == 0) return true;
		}

		return false;
	}

	bool CheckInstanceExtensionSupport()
	{
		std::uint32_t extensionCount{};
		VkResult result{ vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) };

		if (result != VK_SUCCESS) return false;

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		if (result != VK_SUCCESS) return false;

		for (const char* required : Extensions)
		{
			bool found{};

			for (const auto& extension : availableExtensions)
			{
				if (std::strcmp(extension.extensionName, required) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found) return false;
		}

		return true;
	}

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		std::uint32_t extensionCount{};
		VkResult result{ vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) };

		if (result != VK_SUCCESS) return false;

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		if (result != VK_SUCCESS) return false;

		for (const char* required : DeviceExtensions)
		{
			bool found{};

			for (const auto& extension : availableExtensions)
			{
				if (std::strcmp(extension.extensionName, required) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found) return false;
		}

		return true;
	}

	bool CheckImageFormatSupport(VkPhysicalDevice device, VkFormat format, VkImageUsageFlags usage)
	{
		VkImageFormatProperties properties{};
		VkResult result{ vkGetPhysicalDeviceImageFormatProperties(device, format,
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0, &properties) };

		return result == VK_SUCCESS;
	}

	std::uint32_t GetSupportedApiVersion()
	{
		std::uint32_t version{};

		VkResult result{ vkEnumerateInstanceVersion(&version) };

		if (result == VK_ERROR_INCOMPATIBLE_DRIVER) return VK_API_VERSION_1_0;

		if (result != VK_SUCCESS) throw std::runtime_error("Failed to determine supported Vulkan API version");

		return version;
	}

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices{};

		std::uint32_t queueFamilyCount{};
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		for (std::uint32_t i{}; i < queueFamilyCount; ++i)
		{
			const auto& queueFamily = queueFamilies[i];

			const bool graphics{ (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 };
			const bool compute{ (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 };

			VkBool32 presentSupport{ VK_FALSE };
			VkResult result{ vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport) };
			
			if (result != VK_SUCCESS) throw std::runtime_error("Failed to query queue family presentation structure");

			if (graphics && compute && presentSupport)
			{
				indices.graphics = i;
				indices.compute = i;
				indices.present = i;
				break;
			}

			if (graphics) indices.graphics = i;
			if (compute) indices.compute = i;
			if (presentSupport) indices.present = i;
		}

		return indices;
	}

	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapchainSupportDetails details{};

		VkResult result{ vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities) };

		if (result != VK_SUCCESS) throw std::runtime_error("Failed to query surface capabilities");

		std::uint32_t formatCount{};
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (result != VK_SUCCESS) throw std::runtime_error("Failed to query device surface format count");

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());

			if (result != VK_SUCCESS) throw std::runtime_error("Failed to query device surface formats");
		}

		std::uint32_t presentModeCount{};
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (result != VK_SUCCESS) throw std::runtime_error("Failed to query device present mode count");

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());

			if (result != VK_SUCCESS) throw std::runtime_error("Failed to query device present modes");
		}

		return details;
	}

	VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		return formats[0];
	}

	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
	{
		for (const auto& mode : presentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}

		VkExtent2D extent{
			.width = window.Width(),
			.height = window.Height()
		};

		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return extent;
	}

	std::uint32_t FindMemoryType(VkPhysicalDevice device, std::uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

		for (std::uint32_t i{}; i < memoryProperties.memoryTypeCount; ++i)
		{
			const bool typeSupported{ ((typeFilter & (1u << i)) != 0) };
			const bool propertiesSupported{ (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties };

			if (typeSupported && propertiesSupported) return i;
		}

		throw std::runtime_error("Failed to find suitable Vulkan memory type");
	}

	std::vector<std::uint32_t> GetShaderBinary(const std::string_view shaderName)
	{
		std::string path{ std::format("{}/{}.comp.spv", CELESTIAL_SHADER_DIR, shaderName) };

		std::ifstream file{ path, std::ios::binary | std::ios::ate };
		if (!file) throw std::runtime_error("Failed to open shader binary file");

		const std::streamsize fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		if (fileSize <= 0) throw std::runtime_error("Shader binary file empty");
		if (fileSize % sizeof(std::uint32_t) != 0) throw std::runtime_error("Invalid SPIR-V file size");

		std::vector<std::uint32_t> code(static_cast<std::size_t>(fileSize) / sizeof(std::uint32_t));

		file.read(reinterpret_cast<char*>(code.data()), fileSize);

		return code;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severityFlag,
		VkDebugUtilsMessageTypeFlagsEXT typeFlag,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* pUserData)
	{
		std::println();

		if (severityFlag & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			std::println("[Vulkan Error]");
		}
		else if (severityFlag & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			std::println("[Vulkan Warning]");
		}
		else if (severityFlag & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			std::println("[Vulkan Info]");
		}
		else
		{
			std::println("[Vulkan Verbose]");
		}

		std::println("  ID:      {}", callbackData->pMessageIdName);
		std::println("  Message: {}", callbackData->pMessage);

		return VK_FALSE;
	}
}

VulkanContext::VulkanContext(const Window& window) : _window(window)
{
	CreateInstance();
	CreateDebugMessenger();
	CreateSurface();

	PickPhysicalDevice();
	CreateLogicalDevice();

	CreateSwapchain();
	CreateRenderImage();

	CreateCommandPool();
	AllocateCommandBuffer();

	CreateSemaphores();
	CreateFence();
}

VulkanContext::~VulkanContext()
{
	if (_device)
	{
		vkDeviceWaitIdle(_device->GetDevice());
	}
}

bool VulkanContext::BeginFrame()
{
	VkDevice device{ _device->GetDevice() };
	VkFence fence{ _inFlightFence->GetFence() };

	VkResult result{ vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX) };
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to wait for in-flight fence");

	if (!GetSwapchainImageIndex()) return false;

	result = vkResetFences(device, 1, &fence);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to reset in-flight fence");

	result = vkResetCommandBuffer(_commandBuffer, 0);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to reset command buffer");

	BeginCommandBuffer();
	TransitionRenderImage();
	
	return true;
}

bool VulkanContext::EndFrame()
{
	PrepareRenderImageForCopy();
	PrepareSwapchainImageForCopy();
	CopyRenderImageToSwapchain();
	PrepareSwapchainImageForPresent();

	VkResult result{ vkEndCommandBuffer(_commandBuffer) };
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to end command buffer");

	SubmitCommandBuffer();
	if (!Present())
	{
		RecreateSwapchain();
		return false;
	}

	return true;
}

const VulkanDevice& VulkanContext::GetDevice() const
{
	return _device.value();
}

const VulkanImageView& VulkanContext::GetRenderImageView() const
{
	return _renderImageView.value();
}

const VkExtent2D& VulkanContext::GetRenderImageExtent() const
{
	return _renderImageExtent;
}

VkCommandBuffer VulkanContext::GetCommandBuffer() const
{
	return _commandBuffer;
}

std::unique_ptr<VulkanBuffer> VulkanContext::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags memoryProperties, std::optional<VkAllocationCallbacks> bufferAllocator,
	std::optional<VkAllocationCallbacks> memoryAllocator) const
{
	return std::make_unique<VulkanBuffer>(_physicalDevice, _device->GetDevice(), size, usage,
		memoryProperties, bufferAllocator, memoryAllocator);
}

std::unique_ptr<VulkanDescriptorSetLayout> VulkanContext::CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings,
	std::optional<VkAllocationCallbacks> allocator) const
{
	VkDescriptorSetLayoutCreateInfo layoutInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = (std::uint32_t)bindings.size(),
		.pBindings = bindings.data()
	};

	return std::make_unique<VulkanDescriptorSetLayout>(_device->GetDevice(), &layoutInfo, allocator);
}

std::unique_ptr<VulkanPipelineLayout> VulkanContext::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
	const std::vector<VkPushConstantRange>& pushConstantRanges, std::optional<VkAllocationCallbacks> allocator) const
{
	VkPipelineLayoutCreateInfo layoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = (std::uint32_t)descriptorSetLayouts.size(),
		.pSetLayouts = descriptorSetLayouts.data(),
		.pushConstantRangeCount = (std::uint32_t)pushConstantRanges.size(),
		.pPushConstantRanges = pushConstantRanges.data()
	};

	return std::make_unique<VulkanPipelineLayout>(_device->GetDevice(), &layoutInfo, allocator);
}

std::unique_ptr<VulkanShaderModule> VulkanContext::CreateShaderModule(const std::string_view shaderName,
	std::optional<VkAllocationCallbacks> allocator) const
{
	auto code = GetShaderBinary(shaderName);

	VkShaderModuleCreateInfo moduleInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size() * sizeof(std::uint32_t),
		.pCode = code.data()
	};

	return std::make_unique<VulkanShaderModule>(_device->GetDevice(), &moduleInfo, allocator);
}

std::unique_ptr<VulkanComputePipeline> VulkanContext::CreateComputePipeline(const VulkanShaderModule& shaderModule,
	const VulkanPipelineLayout& pipelineLayout, std::optional<VkAllocationCallbacks> allocator) const
{
	VkPipelineShaderStageCreateInfo shaderStageInfo{
	.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	.stage = VK_SHADER_STAGE_COMPUTE_BIT,
	.module = shaderModule.GetShaderModule(),
	.pName = "main"
	};

	VkComputePipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = shaderStageInfo,
		.layout = pipelineLayout.GetPipelineLayout(),
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	return std::make_unique<VulkanComputePipeline>(_device->GetDevice(), &pipelineInfo, allocator);
}

std::unique_ptr<VulkanDescriptorPool> VulkanContext::CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& descriptorPoolSizes,
	std::uint32_t maxSets, std::optional<VkAllocationCallbacks> allocator) const
{
	VkDescriptorPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = maxSets,
		.poolSizeCount = (std::uint32_t)descriptorPoolSizes.size(),
		.pPoolSizes = descriptorPoolSizes.data()
	};

	return std::make_unique<VulkanDescriptorPool>(_device->GetDevice(), &poolInfo, allocator);
}

VkDescriptorSet VulkanContext::AllocateDescriptorSet(const VulkanDescriptorPool& descriptorPool, const VulkanDescriptorSetLayout& descriptorSetLayout) const
{
	VkDescriptorSetLayout setLayout{ descriptorSetLayout.GetDescriptorSetLayout() };

	VkDescriptorSetAllocateInfo setInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool.GetDescriptorPool(),
		.descriptorSetCount = 1,
		.pSetLayouts = &setLayout
	};

	VkDescriptorSet descriptorSet{};
	VkResult result{ vkAllocateDescriptorSets(_device->GetDevice(), &setInfo, &descriptorSet) };
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate descriptor set");

	return descriptorSet;
}

std::vector<VkDescriptorSet> VulkanContext::AllocateDescriptorSets(const VulkanDescriptorPool& descriptorPool,
	const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) const
{
	VkDescriptorSetAllocateInfo setInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool.GetDescriptorPool(),
		.descriptorSetCount = (std::uint32_t)descriptorSetLayouts.size(),
		.pSetLayouts = descriptorSetLayouts.data()
	};

	std::vector<VkDescriptorSet> descriptorSets(descriptorSetLayouts.size());
	VkResult result{ vkAllocateDescriptorSets(_device->GetDevice(), &setInfo, descriptorSets.data()) };
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate descriptor sets");

	return descriptorSets;
}

void VulkanContext::UpdateDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet) const
{
	vkUpdateDescriptorSets(_device->GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanContext::UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writeDescriptorSets) const
{
	vkUpdateDescriptorSets(_device->GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanContext::CreateInstance()
{
	if (!CheckValidationLayerSupport()) throw std::runtime_error(std::format("{} is not available", ValidationLayer));

	if (!CheckInstanceExtensionSupport()) throw std::runtime_error("Required extensions are not available");

	std::uint32_t apiVersion{ GetSupportedApiVersion() };

	VkApplicationInfo applicationInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Celestial Rays",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Celestial Rays Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = apiVersion
	};

	VkInstanceCreateInfo instanceInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = &MessengerInfo,
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = LayerCount,
		.ppEnabledLayerNames = ValidationLayers,
		.enabledExtensionCount = ExtensionCount,
		.ppEnabledExtensionNames = Extensions
	};

	_instance.emplace(&instanceInfo);
}

void VulkanContext::CreateDebugMessenger()
{
	_debugMessenger.emplace(_instance->GetInstance(), &MessengerInfo);
}

void VulkanContext::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR surfaceInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = _window.Instance(),
		.hwnd = _window.Handle()
	};

	_surface.emplace(_instance->GetInstance(), &surfaceInfo);
}

void VulkanContext::PickPhysicalDevice()
{
	VkInstance instance{ _instance->GetInstance() };

	std::uint32_t deviceCount{};
	VkResult result{ vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to enumerate physical devices");
	if (deviceCount == 0) throw std::runtime_error("No Vulkan-capable physical devices found");

	std::vector<VkPhysicalDevice> availableDevices(deviceCount);
	result = vkEnumeratePhysicalDevices(instance, &deviceCount, availableDevices.data());

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to retrieve physical devices");

	for (auto device : availableDevices)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);
		
		QueueFamilyIndices indices{ FindQueueFamilies(device, _surface->GetSurface()) };

		if (!indices.Complete()) continue;

		if (!CheckDeviceExtensionSupport(device)) continue;

		_physicalDevice = device;
		_queueFamily = indices.graphics;

		break;
	}

	if (_physicalDevice == VK_NULL_HANDLE) throw std::runtime_error("Failed to find a suitable physical device");
}

void VulkanContext::CreateLogicalDevice()
{
	constexpr float QueuePriority{ 1.0f };

	VkDeviceQueueCreateInfo queueInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = _queueFamily,
		.queueCount = 1,
		.pQueuePriorities = &QueuePriority
	};

	VkPhysicalDeviceVulkan13Features sync2Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.synchronization2 = VK_TRUE
	};

	VkDeviceCreateInfo deviceInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &sync2Features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueInfo,
		.enabledExtensionCount = DeviceExtensionCount,
		.ppEnabledExtensionNames = DeviceExtensions
	};

	_device.emplace(_physicalDevice, &deviceInfo);

	vkGetDeviceQueue(_device->GetDevice(), _queueFamily, 0, &_queue);
}

void VulkanContext::CreateSwapchain()
{
	VkSurfaceKHR surface{ _surface->GetSurface() };
	VkDevice device{ _device->GetDevice() };

	SwapchainSupportDetails support{ QuerySwapchainSupport(_physicalDevice, surface) };

	if (support.formats.empty()) throw std::runtime_error("Surface has no supported formats");
	if (support.presentModes.empty()) throw std::runtime_error("Surface has no supported present modes");

	VkSurfaceFormatKHR surfaceFormat{ ChooseSurfaceFormat(support.formats) };
	VkPresentModeKHR presentMode{ ChoosePresentMode(support.presentModes) };
	VkExtent2D extent{ ChooseExtent(support.capabilities, _window) };

	std::uint32_t imageCount{ support.capabilities.minImageCount + 1 };

	if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount)
	{
		imageCount = support.capabilities.maxImageCount;
	}

	const bool transferSupported{ (support.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0 };
	if (!transferSupported) throw std::runtime_error("Swapchain image does not support transfer destination usage");

	VkSwapchainCreateInfoKHR swapchainInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = support.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE
	};

	_swapchain.emplace(device, &swapchainInfo);

	VkSwapchainKHR swapchain{ _swapchain->GetSwapchain() };

	_swapchainFormat = surfaceFormat.format;
	_swapchainExtent = extent;

	std::uint32_t actualImageCount{};
	VkResult result{ vkGetSwapchainImagesKHR(device, swapchain, &actualImageCount, nullptr) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to query swapchain image count");

	_swapchainImages.resize(actualImageCount);
	result = vkGetSwapchainImagesKHR(device, swapchain, &actualImageCount, _swapchainImages.data());

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to query swapchain images");
}

void VulkanContext::CreateRenderImage()
{
	constexpr VkImageUsageFlags RenderImageUsage{
		VK_IMAGE_USAGE_STORAGE_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT
	};

	_renderImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	_renderImageExtent = _swapchainExtent;

	VkDevice device{ _device->GetDevice() };

	if (!CheckImageFormatSupport(_physicalDevice, _renderImageFormat, RenderImageUsage))
	{
		throw std::runtime_error("Render image format does not support required usage");
	}

	VkImageCreateInfo imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = _renderImageFormat,
		.extent = {
			.width = _renderImageExtent.width,
			.height = _renderImageExtent.height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = RenderImageUsage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	_renderImage.emplace(device, &imageInfo);

	VkImage renderImage{ _renderImage->GetImage() };

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(device, renderImage, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = FindMemoryType(
			_physicalDevice,
			memoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		)
	};

	_renderImageMemory.emplace(device, &allocateInfo);

	VkResult result{ vkBindImageMemory(device, renderImage, _renderImageMemory->GetMemory(), 0)};

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to bind render image memory");

	VkImageViewCreateInfo viewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = renderImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = _renderImageFormat,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	_renderImageView.emplace(device, &viewInfo);
}

void VulkanContext::RecreateSwapchain()
{
	if (_window.Width() == 0 || _window.Height() == 0) return;

	VkDevice device{ _device->GetDevice() };
	vkDeviceWaitIdle(device);

	_renderImageView.reset();
	_renderImage.reset();
	_renderImageMemory.reset();
	_swapchain.reset();

	CreateSwapchain();
	CreateRenderImage();

	if (_renderFinishedSemaphores.size() != _swapchainImages.size())
	{
		_renderFinishedSemaphores.clear();
		CreateRenderSemaphores();
	}
}

void VulkanContext::CreateCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = _queueFamily
	};

	_commandPool.emplace(_device->GetDevice(), &poolInfo);
}

void VulkanContext::AllocateCommandBuffer()
{
	VkCommandBufferAllocateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = _commandPool->GetCommandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkResult result{ vkAllocateCommandBuffers(_device->GetDevice(), &bufferInfo, &_commandBuffer) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate command buffer");
}

void VulkanContext::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	_imageAvailableSemaphore.emplace(_device->GetDevice(), &semaphoreInfo);
	
	CreateRenderSemaphores();
}

void VulkanContext::CreateRenderSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	VkDevice device{ _device->GetDevice() };

	_renderFinishedSemaphores.resize(_swapchainImages.size());
	for (auto& semaphore : _renderFinishedSemaphores)
	{
		semaphore = std::make_unique<VulkanSemaphore>(device, &semaphoreInfo);
	}
}

void VulkanContext::CreateFence()
{
	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	_inFlightFence.emplace(_device->GetDevice(), &fenceInfo);
}

void VulkanContext::BeginCommandBuffer() const
{
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
	};

	VkResult result{ vkBeginCommandBuffer(_commandBuffer, &beginInfo) };
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to begin Vulkan command buffer");
}

void VulkanContext::TransitionRenderImage() const
{
	VkImageMemoryBarrier2 memoryBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
		.srcAccessMask = VK_ACCESS_2_NONE,
		.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_GENERAL,
		.image = _renderImage->GetImage(),
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
	};

	VkDependencyInfo dependencyInfo{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &memoryBarrier
	};

	vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
}

bool VulkanContext::GetSwapchainImageIndex()
{
	VkResult result{ vkAcquireNextImageKHR(_device->GetDevice(), _swapchain->GetSwapchain(), UINT64_MAX,
		_imageAvailableSemaphore->GetSemaphore(), VK_NULL_HANDLE, &_swapchainImageIndex) };

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		return false;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire next swapchain image index");
	}

	return true;
}

void VulkanContext::PrepareRenderImageForCopy() const
{
	VkImageMemoryBarrier2 memoryBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.image = _renderImage->GetImage(),
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
	};

	VkDependencyInfo dependencyInfo{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &memoryBarrier
	};

	vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
}

void VulkanContext::PrepareSwapchainImageForCopy() const
{
	VkImageMemoryBarrier2 memoryBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
		.srcAccessMask = VK_ACCESS_2_NONE,
		.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.image = _swapchainImages[_swapchainImageIndex],
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
	};

	VkDependencyInfo dependencyInfo{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &memoryBarrier
	};

	vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
}

void VulkanContext::CopyRenderImageToSwapchain() const
{
	VkImageBlit region{
		.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		.srcOffsets = { { 0, 0, 0 }, {
			static_cast<std::int32_t>(_renderImageExtent.width),
			static_cast<std::int32_t>(_renderImageExtent.height), 1 } },
		.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		.dstOffsets = { { 0, 0, 0 }, {
			static_cast<std::int32_t>(_swapchainExtent.width),
			static_cast<std::int32_t>(_swapchainExtent.height), 1 } }
	};

	vkCmdBlitImage(_commandBuffer, _renderImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		_swapchainImages[_swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region,
		VK_FILTER_NEAREST);
}

void VulkanContext::PrepareSwapchainImageForPresent() const
{
	VkImageMemoryBarrier2 memoryBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
		.dstAccessMask = VK_ACCESS_2_NONE,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.image = _swapchainImages[_swapchainImageIndex],
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
	};

	VkDependencyInfo dependencyInfo{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &memoryBarrier
	};

	vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
}

void VulkanContext::SubmitCommandBuffer() const
{
	VkSemaphoreSubmitInfo waitInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = _imageAvailableSemaphore->GetSemaphore(),
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	};

	VkSemaphoreSubmitInfo signalInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = _renderFinishedSemaphores[_swapchainImageIndex]->GetSemaphore(),
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	};

	VkCommandBufferSubmitInfo cmdInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = _commandBuffer
	};

	VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &waitInfo,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalInfo
	};

	VkResult result{ vkQueueSubmit2(_queue, 1, &submitInfo, _inFlightFence->GetFence()) };
	
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to submit command buffer");
}

bool VulkanContext::Present() const
{
	VkSwapchainKHR swapchain{ _swapchain->GetSwapchain() };
	VkSemaphore renderFinished{ _renderFinishedSemaphores[_swapchainImageIndex]->GetSemaphore() };

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderFinished,
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &_swapchainImageIndex
	};

	VkResult result{ vkQueuePresentKHR(_queue, &presentInfo) };

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) return false;
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to present swapchain image");

	return true;
}