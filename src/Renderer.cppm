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
	explicit Renderer(VulkanContext& vulkanContext, const std::vector<MaterialGPUData>& materials, float exposure = 0.0f,
		bool correctGamma = false);
	~Renderer();

	void Render(const SimulationGPUState& simulationState, const CameraGPUData& cameraData, std::uint32_t frameIndex);

	float GetExposure() const;
	void SetExposure(float exposure);

	bool GetCorrectGamma() const;
	void SetCorrectGamma(bool correctGamma);

private:
	VulkanContext& _context;

	std::unique_ptr<VulkanShaderModule> _raytracerShaderModule;
	std::unique_ptr<VulkanShaderModule> _toneMapperShaderModule;

	std::unique_ptr<VulkanBuffer> _cameraBuffer;
	std::unique_ptr<VulkanBuffer> _sphereBuffer;
	std::uint64_t _sphereBufferCapacity{};
	std::unique_ptr<VulkanBuffer> _emitterBuffer;
	std::uint64_t _emitterBufferCapacity{};
	std::unique_ptr<VulkanBuffer> _materialBuffer;
	std::uint64_t _materialCount{};

	std::unique_ptr<VulkanDescriptorSetLayout> _raytracerDescriptorLayout;
	std::unique_ptr<VulkanDescriptorSetLayout> _toneMapperDescriptorLayout;
	std::unique_ptr<VulkanDescriptorPool> _descriptorPool;
	VkDescriptorSet _raytracerDescriptorSet{};
	VkDescriptorSet _toneMapperDescriptorSet{};

	std::unique_ptr<VulkanPipelineLayout> _raytracerPipelineLayout;
	std::unique_ptr<VulkanPipelineLayout> _toneMapperPipelineLayout;
	std::unique_ptr<VulkanComputePipeline> _raytracerPipeline;
	std::unique_ptr<VulkanComputePipeline> _toneMapperPipeline;

	float _exposure{};
	bool _correctGamma{};

	void CreateShaderModules();

	void AllocateBuffers();
	void GrowSphereBuffer(std::uint64_t sphereCount);
	void GrowEmitterBuffer(std::uint64_t emitterCount);

	void CreateDescriptorSetLayouts();
	void CreateDescriptorPools();
	void AllocateDescriptorSets();

	void CreatePipelineLayouts();
	void CreatePipelines();

	void PushMaterials(const std::vector<MaterialGPUData>& materials) const;

	void UpdateDescriptorSet() const;
};