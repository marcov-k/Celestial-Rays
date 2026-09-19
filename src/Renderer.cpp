module;

#include <vulkan/vulkan.h>

#include <cstdint>
#include <print>
#include <vector>

module Celestial.Rendering;

Renderer::Renderer(VulkanContext& context, const std::vector<MaterialGPUData>& materials)
	: _context(context), _materialCount(materials.size())
{
	CreateShaderModule();

	AllocateBuffers();

	CreateDescriptorSetLayout();
	CreateDescriptorPool();
	AllocateDescriptorSet();
	UpdateDescriptorSet();

	CreatePipelineLayout();
	CreatePipeline();

	PushMaterials(materials);
}

Renderer::~Renderer() { }

void Renderer::Render(const SimulationGPUState& simulationState, std::uint32_t frameIndex)
{
	if (!_context.BeginFrame()) return;

	_cameraBuffer->Write(&simulationState.cameraData, sizeof(CameraGPUData), 0);

	std::uint32_t sphereCount{ static_cast<std::uint32_t>(simulationState.sphereData.size()) };
	GrowSphereBuffer(sphereCount);
	_sphereBuffer->Write(simulationState.sphereData.data(), sphereCount * sizeof(SphereGPUData), 0);

	std::uint32_t emitterCount{ static_cast<std::uint32_t>(simulationState.emitterData.size()) };
	GrowEmitterBuffer(emitterCount);
	_emitterBuffer->Write(simulationState.emitterData.data(), emitterCount * sizeof(EmitterGPUData), 0);

	VkCommandBuffer commandBuffer{ _context.GetCommandBuffer() };
	const VkExtent2D& renderImageExtent{ _context.GetRenderImageExtent() };

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline->GetPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout->GetPipelineLayout(),
		0, 1, &_descriptorSet, 0, nullptr);

	float emitterWeightSum{};
	for (auto& emitter : simulationState.emitterData)
	{
		emitterWeightSum += emitter.selectionWeight;
	}

	PushConstants pushConstants{ sphereCount, emitterCount, emitterWeightSum, frameIndex };

	vkCmdPushConstants(commandBuffer, _pipelineLayout->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pushConstants);

	std::uint32_t workGroupsX{ (renderImageExtent.width + 7) / 8 };
	std::uint32_t workGroupsY{ (renderImageExtent.height + 7) / 8 };
	vkCmdDispatch(commandBuffer, workGroupsX, workGroupsY, 1);

	if (!_context.EndFrame()) UpdateDescriptorSet();
}

void Renderer::CreateShaderModule()
{
	_shaderModule = _context.CreateShaderModule("raytracer");
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
		.dstSet = _descriptorSet,
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
		.dstSet = _descriptorSet,
		.dstBinding = 3,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &emitterBufferInfo
	};

	_context.UpdateDescriptorSet(writeEmitterBuffer);
}

void Renderer::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding renderImageBinding{
		.binding = 0,
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

	_descriptorLayout = _context.CreateDescriptorSetLayout({ renderImageBinding, cameraBufferBinding,
		sphereBufferBinding, emitterBufferBinding, materialBufferBinding });
}

void Renderer::CreateDescriptorPool()
{
	VkDescriptorPoolSize renderImagePool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 1
	};

	VkDescriptorPoolSize cameraBufferPool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1
	};

	VkDescriptorPoolSize sphereBufferPool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1
	};

	VkDescriptorPoolSize emitterBufferPool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1
	};

	VkDescriptorPoolSize materialBufferPool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1
	};

	_descriptorPool = _context.CreateDescriptorPool({ renderImagePool, cameraBufferPool, sphereBufferPool,
		emitterBufferPool, materialBufferPool }, 1);
}

void Renderer::AllocateDescriptorSet()
{
	_descriptorSet = _context.AllocateDescriptorSet(*_descriptorPool, *_descriptorLayout);
}

void Renderer::CreatePipelineLayout()
{
	VkPushConstantRange constantRange{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = sizeof(PushConstants)
	};

	_pipelineLayout = _context.CreatePipelineLayout({ _descriptorLayout->GetDescriptorSetLayout() }, { constantRange });
}

void Renderer::CreatePipeline()
{
	_pipeline = _context.CreateComputePipeline(*_shaderModule, *_pipelineLayout);
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
		.dstSet = _descriptorSet,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.pImageInfo = &renderImageInfo
	};

	VkDescriptorBufferInfo cameraBufferInfo{
		.buffer = _cameraBuffer->GetBuffer(),
		.offset = 0,
		.range = sizeof(CameraGPUData)
	};

	VkWriteDescriptorSet writeCameraBuffer{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descriptorSet,
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
		.dstSet = _descriptorSet,
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
		.dstSet = _descriptorSet,
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
		.dstSet = _descriptorSet,
		.dstBinding = 4,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &materialBufferInfo
	};

	_context.UpdateDescriptorSets({ writeRenderImage, writeCameraBuffer, writeSphereBuffer,
		writeEmitterBuffer, writeMaterialBuffer });
}