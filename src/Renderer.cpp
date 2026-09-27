module;

#include <vulkan/vulkan.h>

#include <cstdint>
#include <print>
#include <vector>

module Celestial.Rendering;

Renderer::Renderer(VulkanContext& context, const std::vector<MaterialGPUData>& materials, float exposure,
	bool correctGamma)
	: _context(context), _materialCount(materials.size()), _exposure(exposure), _correctGamma(correctGamma)
{
	CreateShaderModules();

	AllocateBuffers();

	CreateDescriptorSetLayouts();
	CreateDescriptorPools();
	AllocateDescriptorSets();
	UpdateDescriptorSet();

	CreatePipelineLayouts();
	CreatePipelines();

	PushMaterials(materials);
}

Renderer::~Renderer()
{
	vkDeviceWaitIdle(_context.GetDevice().GetDevice());
}

void Renderer::Render(const SimulationGPUState& simulationState, const CameraGPUData& cameraData, std::uint32_t frameIndex)
{
	if (!_context.BeginFrame()) return;

	// Raytracer

	_cameraBuffer->Write(&cameraData, sizeof(CameraGPUData), 0);

	std::uint32_t sphereCount{ static_cast<std::uint32_t>(simulationState.sphereData.size()) };
	GrowSphereBuffer(sphereCount);
	_sphereBuffer->Write(simulationState.sphereData.data(), sphereCount * sizeof(SphereGPUData), 0);

	std::uint32_t emitterCount{ static_cast<std::uint32_t>(simulationState.emitterData.size()) };
	GrowEmitterBuffer(emitterCount);
	_emitterBuffer->Write(simulationState.emitterData.data(), emitterCount * sizeof(EmitterGPUData), 0);

	VkCommandBuffer commandBuffer{ _context.GetCommandBuffer() };
	const VkExtent2D& renderImageExtent{ _context.GetRenderImageExtent() };

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _raytracerPipeline->GetPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _raytracerPipelineLayout->GetPipelineLayout(),
		0, 1, &_raytracerDescriptorSet, 0, nullptr);

	float emitterWeightSum{};
	for (auto& emitter : simulationState.emitterData)
	{
		emitterWeightSum += emitter.selectionWeight;
	}

	RaytracerPushConstants raytracerPushConstants{ sphereCount, emitterCount, emitterWeightSum, frameIndex };
	vkCmdPushConstants(commandBuffer, _raytracerPipelineLayout->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RaytracerPushConstants), &raytracerPushConstants);

	std::uint32_t workGroupsX{ (renderImageExtent.width + 7) / 8 };
	std::uint32_t workGroupsY{ (renderImageExtent.height + 7) / 8 };
	vkCmdDispatch(commandBuffer, workGroupsX, workGroupsY, 1);

	// Tone mapper

	_context.PrepareRenderImageForRead();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _toneMapperPipeline->GetPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _toneMapperPipelineLayout->GetPipelineLayout(),
		0, 1, &_toneMapperDescriptorSet, 0, nullptr);

	ToneMapperPushConstants toneMapperPushConstants{ _exposure, _correctGamma };
	vkCmdPushConstants(commandBuffer, _toneMapperPipelineLayout->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ToneMapperPushConstants), &toneMapperPushConstants);

	vkCmdDispatch(commandBuffer, workGroupsX, workGroupsY, 1);

	if (!_context.EndFrame()) UpdateDescriptorSet();
}

float Renderer::GetExposure() const
{
	return _exposure;
}

void Renderer::SetExposure(float exposure)
{
	_exposure = exposure;
}

bool Renderer::GetCorrectGamma() const
{
	return _correctGamma;
}

void Renderer::SetCorrectGamma(bool correctGamma)
{
	_correctGamma = correctGamma;
}

void Renderer::CreateShaderModules()
{
	_raytracerShaderModule = _context.CreateShaderModule("raytracer");
	_toneMapperShaderModule = _context.CreateShaderModule("tone_mapper");
}

void Renderer::AllocateBuffers()
{
	_cameraBuffer = _context.CreateBuffer(sizeof(CameraGPUData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	_sphereBuffer = _context.CreateBuffer(sizeof(SphereGPUData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	_sphereBufferCapacity = 1;

	_emitterBuffer = _context.CreateBuffer(sizeof(EmitterGPUData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	_emitterBufferCapacity = 1;

	_materialBuffer = _context.CreateBuffer(_materialCount * sizeof(MaterialGPUData),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void Renderer::GrowSphereBuffer(std::uint64_t sphereCount)
{
	if (sphereCount <= _sphereBufferCapacity) return;

	while (_sphereBufferCapacity < sphereCount) _sphereBufferCapacity *= 2;

	_sphereBuffer = _context.CreateBuffer(_sphereBufferCapacity * sizeof(SphereGPUData),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkDescriptorBufferInfo sphereBufferInfo{
		.buffer = _sphereBuffer->GetBuffer(),
		.offset = 0,
		.range = _sphereBufferCapacity * sizeof(SphereGPUData)
	};

	VkWriteDescriptorSet writeSphereBuffer{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _raytracerDescriptorSet,
		.dstBinding = 2,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &sphereBufferInfo
	};

	_context.UpdateDescriptorSet(writeSphereBuffer);
}

void Renderer::GrowEmitterBuffer(std::uint64_t emitterCount)
{
	if (emitterCount <= _emitterBufferCapacity) return;

	while (_emitterBufferCapacity < emitterCount) _emitterBufferCapacity *= 2;

	_emitterBuffer = _context.CreateBuffer(_emitterBufferCapacity * sizeof(EmitterGPUData),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkDescriptorBufferInfo emitterBufferInfo{
		.buffer = _emitterBuffer->GetBuffer(),
		.offset = 0,
		.range = _emitterBufferCapacity * sizeof(EmitterGPUData)
	};

	VkWriteDescriptorSet writeEmitterBuffer{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _raytracerDescriptorSet,
		.dstBinding = 3,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &emitterBufferInfo
	};

	_context.UpdateDescriptorSet(writeEmitterBuffer);
}

void Renderer::CreateDescriptorSetLayouts()
{
	VkDescriptorSetLayoutBinding renderImageBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	VkDescriptorSetLayoutBinding displayImageBinding{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	VkDescriptorSetLayoutBinding cameraBufferBinding{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	VkDescriptorSetLayoutBinding sphereBufferBinding{
		.binding = 2,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	VkDescriptorSetLayoutBinding emitterBufferBinding{
		.binding = 3,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	VkDescriptorSetLayoutBinding materialBufferBinding{
		.binding = 4,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	_raytracerDescriptorLayout = _context.CreateDescriptorSetLayout({ renderImageBinding, cameraBufferBinding,
		sphereBufferBinding, emitterBufferBinding, materialBufferBinding });
	_toneMapperDescriptorLayout = _context.CreateDescriptorSetLayout({ renderImageBinding, displayImageBinding });
}

void Renderer::CreateDescriptorPools()
{
	VkDescriptorPoolSize storageImagePool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 3
	};

	VkDescriptorPoolSize bufferPool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 4
	};

	_descriptorPool = _context.CreateDescriptorPool({ storageImagePool, bufferPool }, 2);
}

void Renderer::AllocateDescriptorSets()
{
	_raytracerDescriptorSet = _context.AllocateDescriptorSet(*_descriptorPool, *_raytracerDescriptorLayout);
	_toneMapperDescriptorSet = _context.AllocateDescriptorSet(*_descriptorPool, *_toneMapperDescriptorLayout);
}

void Renderer::CreatePipelineLayouts()
{
	VkPushConstantRange raytracerConstantRange{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = sizeof(RaytracerPushConstants)
	};

	VkPushConstantRange toneMapperConstantRange{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = sizeof(ToneMapperPushConstants)
	};

	_raytracerPipelineLayout = _context.CreatePipelineLayout({ _raytracerDescriptorLayout->GetDescriptorSetLayout() }, { raytracerConstantRange });
	_toneMapperPipelineLayout = _context.CreatePipelineLayout({ _toneMapperDescriptorLayout->GetDescriptorSetLayout() }, { toneMapperConstantRange });
}

void Renderer::CreatePipelines()
{
	_raytracerPipeline = _context.CreateComputePipeline(*_raytracerShaderModule, *_raytracerPipelineLayout);
	_toneMapperPipeline = _context.CreateComputePipeline(*_toneMapperShaderModule, *_toneMapperPipelineLayout);
}

void Renderer::PushMaterials(const std::vector<MaterialGPUData>& materials) const
{
	_materialBuffer->Write(materials.data(), _materialCount * sizeof(MaterialGPUData), 0);
}

void Renderer::UpdateDescriptorSet() const
{
	VkDescriptorImageInfo renderImageInfo{
		.imageView = _context.GetRenderImageView().GetImageView(),
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL
	};

	VkWriteDescriptorSet writeRenderImage{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _raytracerDescriptorSet,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.pImageInfo = &renderImageInfo
	};

	VkWriteDescriptorSet writeToneMapperRenderImage{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _toneMapperDescriptorSet,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.pImageInfo = &renderImageInfo
	};

	VkDescriptorImageInfo displayImageInfo{
		.imageView = _context.GetDisplayImageView().GetImageView(),
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL
	};

	VkWriteDescriptorSet writeDisplayImage{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _toneMapperDescriptorSet,
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.pImageInfo = &displayImageInfo
	};

	VkDescriptorBufferInfo cameraBufferInfo{
		.buffer = _cameraBuffer->GetBuffer(),
		.offset = 0,
		.range = sizeof(CameraGPUData)
	};

	VkWriteDescriptorSet writeCameraBuffer{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _raytracerDescriptorSet,
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &cameraBufferInfo
	};

	VkDescriptorBufferInfo sphereBufferInfo{
		.buffer = _sphereBuffer->GetBuffer(),
		.offset = 0,
		.range = _sphereBufferCapacity * sizeof(SphereGPUData)
	};

	VkWriteDescriptorSet writeSphereBuffer{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _raytracerDescriptorSet,
		.dstBinding = 2,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &sphereBufferInfo
	};

	VkDescriptorBufferInfo emitterBufferInfo{
		.buffer = _emitterBuffer->GetBuffer(),
		.offset = 0,
		.range = _emitterBufferCapacity * sizeof(EmitterGPUData)
	};

	VkWriteDescriptorSet writeEmitterBuffer{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _raytracerDescriptorSet,
		.dstBinding = 3,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &emitterBufferInfo
	};

	VkDescriptorBufferInfo materialBufferInfo{
		.buffer = _materialBuffer->GetBuffer(),
		.offset = 0,
		.range = _materialCount * sizeof(MaterialGPUData)
	};

	VkWriteDescriptorSet writeMaterialBuffer{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _raytracerDescriptorSet,
		.dstBinding = 4,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &materialBufferInfo
	};

	_context.UpdateDescriptorSets({ writeRenderImage, writeToneMapperRenderImage, writeDisplayImage,
		writeCameraBuffer, writeSphereBuffer, writeEmitterBuffer, writeMaterialBuffer });
}