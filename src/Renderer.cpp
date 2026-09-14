module;

#include <vulkan/vulkan.h>

#include <cstdint>

module Celestial.Rendering;

Renderer::Renderer(VulkanContext& context) : _context(context)
{
	CreateShaderModule();

	AllocateBuffers();

	CreateDescriptorSetLayout();
	CreateDescriptorPool();
	AllocateDescriptorSet();
	UpdateDescriptorSet();

	CreatePipelineLayout();
	CreatePipeline();
}

Renderer::~Renderer() { }

void Renderer::Render(float time, const TestData& testData)
{
	if (!_context.BeginFrame()) return;

	_testDataBuffer->Write(&testData, sizeof(TestData), 0);

	VkCommandBuffer commandBuffer{ _context.GetCommandBuffer() };
	const VkExtent2D& renderImageExtent{ _context.GetRenderImageExtent() };

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline->GetPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout->GetPipelineLayout(),
		0, 1, &_descriptorSet, 0, nullptr);

	vkCmdPushConstants(commandBuffer, _pipelineLayout->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float), &time);

	std::uint32_t workGroupsX{ (renderImageExtent.width + 7) / 8};
	std::uint32_t workGroupsY{ (renderImageExtent.height + 7) / 8 };
	vkCmdDispatch(commandBuffer, workGroupsX, workGroupsY, 1);

	if (!_context.EndFrame()) UpdateDescriptorSet();
}

void Renderer::CreateShaderModule()
{
	_shaderModule = _context.CreateShaderModule("gradient");
}

void Renderer::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding renderImageBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	VkDescriptorSetLayoutBinding testDataBufferBinding{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	_descriptorLayout = _context.CreateDescriptorSetLayout({ renderImageBinding, testDataBufferBinding });
}

void Renderer::CreateDescriptorPool()
{
	VkDescriptorPoolSize renderImagePool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 1
	};

	VkDescriptorPoolSize storageBufferPool{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1
	};

	_descriptorPool = _context.CreateDescriptorPool({ renderImagePool, storageBufferPool }, 1);
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
		.size = sizeof(float)
	};

	_pipelineLayout = _context.CreatePipelineLayout({ _descriptorLayout->GetDescriptorSetLayout() }, { constantRange });
}

void Renderer::CreatePipeline()
{
	_pipeline = _context.CreateComputePipeline(*_shaderModule, *_pipelineLayout);
}

void Renderer::AllocateBuffers()
{
	_testDataBuffer = _context.CreateBuffer(sizeof(TestData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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

	VkDescriptorBufferInfo testDataBufferInfo{
		.buffer = _testDataBuffer->GetBuffer(),
		.offset = 0,
		.range = sizeof(TestData)
	};

	VkWriteDescriptorSet writeTestDataBuffer{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descriptorSet,
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &testDataBufferInfo
	};

	_context.UpdateDescriptorSets({ writeRenderImage, writeTestDataBuffer });
}