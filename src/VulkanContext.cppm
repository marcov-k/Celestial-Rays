module;

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

export module Celestial.Vulkan;

import Celestial.Vulkan.Wrappers;
import Celestial.Window;

export class VulkanContext
{
public:
	explicit VulkanContext(const Window& window);
	~VulkanContext();

	VulkanContext(const VulkanContext&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;

	void DrawFrame();

private:
	const Window& _window;

	std::optional<VulkanInstance> _instance;
	std::optional<VulkanDebugMessenger> _debugMessenger;
	std::optional<VulkanSurface> _surface;
	VkPhysicalDevice _physicalDevice{};
	std::optional<VulkanDevice> _device;
	std::uint32_t _queueFamily{ UINT32_MAX };
	VkQueue _queue{};

	std::optional<VulkanSwapchain> _swapchain;
	VkFormat _swapchainFormat{};
	VkExtent2D _swapchainExtent{};
	std::vector<VkImage> _swapchainImages;
	std::uint32_t _swapchainImageIndex{ UINT32_MAX };

	std::optional<VulkanDeviceMemory> _renderImageMemory;
	std::optional<VulkanImage> _renderImage;
	std::optional<VulkanImageView> _renderImageView;
	VkFormat _renderImageFormat{};
	VkExtent2D _renderImageExtent{};

	std::optional<VulkanCommandPool> _commandPool;
	VkCommandBuffer _commandBuffer{};

	std::optional<VulkanShaderModule> _shaderModule;

	std::optional<VulkanDescriptorSetLayout> _descriptorSetLayout;
	std::optional<VulkanPipelineLayout> _pipelineLayout;
	std::optional<VulkanComputePipeline> _pipeline;

	std::optional<VulkanDescriptorPool> _descriptorPool;
	VkDescriptorSet _descriptorSet{};

	std::optional<VulkanSemaphore> _imageAvailableSemaphore;
	std::vector<std::unique_ptr<VulkanSemaphore>> _renderFinishedSemaphores;
	std::optional<VulkanFence> _inFlightFence;

	void CreateInstance();
	void CreateDebugMessenger();
	void CreateSurface();

	void PickPhysicalDevice();
	void CreateLogicalDevice();

	void CreateSwapchain();
	void CreateRenderImage();
	void RecreateSwapchain();

	void CreateCommandPool();
	void AllocateCommandBuffer();

	void CreateShaderModule();

	void CreateDescriptorSetLayout();
	void CreatePipelineLayout();
	void CreatePipeline();

	void CreateDescriptorPool();
	void AllocateDescriptorSet();
	void UpdateDescriptorSet() const;

	void CreateSemaphores();
	void CreateRenderSemaphores();
	void CreateFence();

	void BeginCommandBuffer() const;
	void TransitionRenderImage() const;
	void BindAndDispatchShader();

	bool GetSwapchainImageIndex();
	
	void PrepareRenderImageForCopy() const;
	void PrepareSwapchainImageForCopy() const;
	void CopyRenderImageToSwapchain() const;
	void PrepareSwapchainImageForPresent() const;

	void SubmitCommandBuffer() const;
	bool Present() const;
};