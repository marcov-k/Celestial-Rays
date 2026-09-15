module;

#include <vulkan/vulkan.h>

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
	explicit Renderer(VulkanContext& vulkanContext);
	~Renderer();

	void Render(const SimulationGPUState& simulationState);

private:
	VulkanContext& _context;

	std::unique_ptr<VulkanShaderModule> _shaderModule;

	std::unique_ptr<VulkanBuffer> _cameraBuffer;

	std::unique_ptr<VulkanDescriptorSetLayout> _descriptorLayout;
	std::unique_ptr<VulkanDescriptorPool> _descriptorPool;
	VkDescriptorSet _descriptorSet{};

	std::unique_ptr<VulkanPipelineLayout> _pipelineLayout;
	std::unique_ptr<VulkanComputePipeline> _pipeline;

	void CreateShaderModule();

	void AllocateBuffers();

	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void AllocateDescriptorSet();

	void CreatePipelineLayout();
	void CreatePipeline();

	void UpdateDescriptorSet() const;
};