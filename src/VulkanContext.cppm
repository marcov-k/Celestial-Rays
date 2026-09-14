module;

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
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

	[[nodiscard]]
	bool BeginFrame();
	[[nodiscard]]
	bool EndFrame();

	const VulkanImageView& GetRenderImageView() const;
	const VkExtent2D& GetRenderImageExtent() const;
	VkCommandBuffer GetCommandBuffer() const;

	std::unique_ptr<VulkanBuffer> CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags memoryProperties, std::optional<VkAllocationCallbacks> bufferAllocator = std::nullopt,
		std::optional<VkAllocationCallbacks> memoryAllocator = std::nullopt) const;

	std::unique_ptr<VulkanDescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt) const;

	std::unique_ptr<VulkanShaderModule> CreateShaderModule(const std::string_view shaderName,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt) const;

	std::unique_ptr<VulkanPipelineLayout> CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
		const std::vector<VkPushConstantRange>& pushConstantRanges,
		std::optional<VkAllocationCallbacks> allocator = std::nullopt) const;

	std::unique_ptr<VulkanComputePipeline> CreateComputePipeline(const VulkanShaderModule& shaderModule,
		const VulkanPipelineLayout& pipelineLayout, std::optional<VkAllocationCallbacks> allocator = std::nullopt) const;

	std::unique_ptr<VulkanDescriptorPool> CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& descriptorPoolSizes,
		std::uint32_t maxSets, std::optional<VkAllocationCallbacks> allocator = std::nullopt) const;

	VkDescriptorSet AllocateDescriptorSet(const VulkanDescriptorPool& descriptorPool, const VulkanDescriptorSetLayout& descriptorSetLayout) const;

	std::vector<VkDescriptorSet> AllocateDescriptorSets(const VulkanDescriptorPool& descriptorPool,
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) const;

	void UpdateDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet) const;
	void UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writeDescriptorSets) const;

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

	void CreateSemaphores();
	void CreateRenderSemaphores();
	void CreateFence();

	void BeginCommandBuffer() const;
	void TransitionRenderImage() const;

	bool GetSwapchainImageIndex();
	
	void PrepareRenderImageForCopy() const;
	void PrepareSwapchainImageForCopy() const;
	void CopyRenderImageToSwapchain() const;
	void PrepareSwapchainImageForPresent() const;

	void SubmitCommandBuffer() const;
	bool Present() const;
};