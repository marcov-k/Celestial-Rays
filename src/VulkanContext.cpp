module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>
#include <print>
#include <stdexcept>
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
}

VulkanContext::~VulkanContext()
{
	if (_renderImageView) vkDestroyImageView(_device, _renderImageView, nullptr);

	if (_renderImage) vkDestroyImage(_device, _renderImage, nullptr);

	if (_renderImageMemory) vkFreeMemory(_device, _renderImageMemory, nullptr);

	if (_swapchain) vkDestroySwapchainKHR(_device, _swapchain, nullptr);

	if (_device) vkDestroyDevice(_device, nullptr);

	if (_surface) vkDestroySurfaceKHR(_instance, _surface, nullptr);

	if (_debugMessenger)
	{
		auto destroyDebugFn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (destroyDebugFn) destroyDebugFn(_instance, _debugMessenger, nullptr);
	}

	if (_instance) vkDestroyInstance(_instance, nullptr);
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

	VkResult result{ vkCreateInstance(&instanceInfo, nullptr, &_instance) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan instance");
}

void VulkanContext::CreateDebugMessenger()
{
	auto createDebugFn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT"));

	if (!createDebugFn) throw std::runtime_error("Failed to retrieve instance debug messenger creator");

	VkResult result{ createDebugFn(_instance, &MessengerInfo, nullptr, &_debugMessenger) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan debug messenger");
}

void VulkanContext::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR surfaceInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = _window.Instance(),
		.hwnd = _window.Handle()
	};

	VkResult result{ vkCreateWin32SurfaceKHR(_instance, &surfaceInfo, nullptr, &_surface) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan surface");
}

void VulkanContext::PickPhysicalDevice()
{
	std::uint32_t deviceCount{};
	VkResult result{ vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to enumerate physical devices");
	if (deviceCount == 0) throw std::runtime_error("No Vulkan-capable physical devices found");

	std::vector<VkPhysicalDevice> availableDevices(deviceCount);
	result = vkEnumeratePhysicalDevices(_instance, &deviceCount, availableDevices.data());

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to retrieve physical devices");

	for (auto device : availableDevices)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);

		std::println("Found GPU: {}", properties.deviceName);
		
		QueueFamilyIndices indices{ FindQueueFamilies(device, _surface) };

		if (!indices.Complete()) continue;

		if (!CheckDeviceExtensionSupport(device)) continue;

		_physicalDevice = device;
		_queueFamily = indices.graphics;

		std::println("Selected GPU: {}", properties.deviceName);

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

	VkDeviceCreateInfo deviceInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueInfo,
		.enabledExtensionCount = DeviceExtensionCount,
		.ppEnabledExtensionNames = DeviceExtensions
	};

	VkResult result{ vkCreateDevice(_physicalDevice, &deviceInfo, nullptr, &_device) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create logical device");

	vkGetDeviceQueue(_device, _queueFamily, 0, &_queue);
}

void VulkanContext::CreateSwapchain()
{

	SwapchainSupportDetails support{ QuerySwapchainSupport(_physicalDevice, _surface) };

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
		.surface = _surface,
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

	VkResult result{ vkCreateSwapchainKHR(_device, &swapchainInfo, nullptr, &_swapchain) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create swapchain");

	_swapchainFormat = surfaceFormat.format;
	_swapchainExtent = extent;

	std::uint32_t actualImageCount{};
	result = vkGetSwapchainImagesKHR(_device, _swapchain, &actualImageCount, nullptr);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to query swapchain image count");

	_swapchainImages.resize(actualImageCount);
	result = vkGetSwapchainImagesKHR(_device, _swapchain, &actualImageCount, _swapchainImages.data());

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to query swapchain images");

	std::println("\nSwapchain: {}x{}, {} images",
		_swapchainExtent.width,
		_swapchainExtent.height,
		_swapchainImages.size());
}

void VulkanContext::CreateRenderImage()
{
	constexpr VkImageUsageFlags RenderImageUsage{
		VK_IMAGE_USAGE_STORAGE_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT
	};

	_renderImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	_renderImageExtent = _swapchainExtent;

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

	VkResult result{ vkCreateImage(_device, &imageInfo, nullptr, &_renderImage) };

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create render image");

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(_device, _renderImage, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = FindMemoryType(
			_physicalDevice,
			memoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		)
	};

	result = vkAllocateMemory(_device, &allocateInfo, nullptr, &_renderImageMemory);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate render image memory");

	result = vkBindImageMemory(_device, _renderImage, _renderImageMemory, 0);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to bind render image memory");

	VkImageViewCreateInfo viewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = _renderImage,
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

	result = vkCreateImageView(_device, &viewInfo, nullptr, &_renderImageView);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create render image view");

	std::println("\nRender image: {}x{}, format {}",
		_renderImageExtent.width,
		_renderImageExtent.height,
		static_cast<int>(_renderImageFormat));
}