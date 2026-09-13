module;

#include <vulkan/vulkan.h>

#include <optional>

export module Celestial.Vulkan.Wrappers;

class VulkanResource
{
public:
	VulkanResource(const VulkanResource&) = delete;
	VulkanResource& operator=(const VulkanResource&) = delete;
	VulkanResource(const VulkanResource&&) = delete;
	VulkanResource& operator=(VulkanResource&&) = delete;

protected:
	explicit VulkanResource(std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanResource() { }

	const VkAllocationCallbacks* GetAllocator() const;

private:
	std::optional<VkAllocationCallbacks> _allocator;
};

export class VulkanInstance : VulkanResource
{
public:
	explicit VulkanInstance(const VkInstanceCreateInfo* pCreateInfo, std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanInstance();

	VkInstance GetInstance() const;

private:
	VkInstance _instance{};
};

export class VulkanDebugMessenger : VulkanResource
{
public:
	explicit VulkanDebugMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanDebugMessenger();

	VkDebugUtilsMessengerEXT GetDebugMessenger() const;

private:
	VkInstance _instance{};
	VkDebugUtilsMessengerEXT _debugMessenger{};
};

export class VulkanSurface : VulkanResource
{
public:
	explicit VulkanSurface(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanSurface();

	VkSurfaceKHR GetSurface() const;

private:
	VkInstance _instance{};
	VkSurfaceKHR _surface{};
};

export class VulkanDevice : VulkanResource
{
public:
	explicit VulkanDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanDevice();

	VkDevice GetDevice() const;

private:
	VkDevice _device{};
};

export class VulkanSwapchain : VulkanResource
{
public:
	explicit VulkanSwapchain(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanSwapchain();

	VkSwapchainKHR GetSwapchain() const;

private:
	VkDevice _device{};
	VkSwapchainKHR _swapchain{};
};

export class VulkanImage : VulkanResource
{
public:
	explicit VulkanImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanImage();

	VkImage GetImage() const;

private:
	VkDevice _device{};
	VkImage _image{};
};

export class VulkanImageView : VulkanResource
{
public:
	explicit VulkanImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanImageView();

	VkImageView GetImageView() const;

private:
	VkDevice _device{};
	VkImageView _imageView{};
};

export class VulkanDeviceMemory : VulkanResource
{
public:
	explicit VulkanDeviceMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanDeviceMemory();

	VkDeviceMemory GetMemory() const;

private:
	VkDevice _device{};
	VkDeviceMemory _memory{};
};

export class VulkanCommandPool : VulkanResource
{
public:
	explicit VulkanCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanCommandPool();

	VkCommandPool GetCommandPool() const;

private:
	VkDevice _device{};
	VkCommandPool _commandPool{};
};

export class VulkanShaderModule : VulkanResource
{
public:
	explicit VulkanShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanShaderModule();

	VkShaderModule GetShaderModule() const;

private:
	VkDevice _device{};
	VkShaderModule _shaderModule{};
};

export class VulkanDescriptorSetLayout : VulkanResource
{
public:
	explicit VulkanDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanDescriptorSetLayout();

	VkDescriptorSetLayout GetDescriptorSetLayout() const;

private:
	VkDevice _device{};
	VkDescriptorSetLayout _setLayout{};
};

export class VulkanPipelineLayout : VulkanResource
{
public:
	explicit VulkanPipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanPipelineLayout();

	VkPipelineLayout GetPipelineLayout() const;

private:
	VkDevice _device{};
	VkPipelineLayout _pipelineLayout{};
};

export class VulkanComputePipeline : VulkanResource
{
public:
	explicit VulkanComputePipeline(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanComputePipeline();

	VkPipeline GetPipeline() const;

private:
	VkDevice _device{};
	VkPipeline _pipeline{};
};

export class VulkanDescriptorPool : VulkanResource
{
public:
	explicit VulkanDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt);
	~VulkanDescriptorPool();

	VkDescriptorPool GetDescriptorPool() const;

private:
	VkDevice _device{};
	VkDescriptorPool _descriptorPool{};
};