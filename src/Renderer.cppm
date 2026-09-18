module;

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <vector>

export module Celestial.Rendering;

import Celestial.GPUDatatypes;
import Celestial.Simulation;
import Celestial.Simulation.Camera;
import Celestial.Vulkan;
import Celestial.Vulkan.Wrappers;

export class Renderer
{
public:
	explicit Renderer(VulkanContext& vulkanContext, const std::vector<MaterialGPUData>& materials);
	~Renderer();

	void Render(const SimulationGPUState& simulationState, std::uint32_t frameIndex);

private:
	VulkanContext& _context;

	std::unique_ptr<VulkanShaderModule> _shaderModule;

	std::unique_ptr<VulkanBuffer> _cameraBuffer;
	std::unique_ptr<VulkanBuffer> _sphereBuffer;
	std::uint64_t _sphereBufferCapacity{};
	std::unique_ptr<VulkanBuffer> _materialBuffer;
	std::uint64_t _materialCount{};

	std::unique_ptr<VulkanDescriptorSetLayout> _descriptorLayout;
	std::unique_ptr<VulkanDescriptorPool> _descriptorPool;
	VkDescriptorSet _descriptorSet{};

	std::unique_ptr<VulkanPipelineLayout> _pipelineLayout;
	std::unique_ptr<VulkanComputePipeline> _pipeline;

	void CreateShaderModule();

	void AllocateBuffers();
	void GrowSphereBuffer(std::uint64_t sphereCount);

	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void AllocateDescriptorSet();

	void CreatePipelineLayout();
	void CreatePipeline();

	void PushMaterials(const std::vector<MaterialGPUData>& materials) const;

	void UpdateDescriptorSet() const;
};