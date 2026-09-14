module;

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>

module Celestial.Vulkan.Wrappers;

VulkanBuffer::VulkanBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags memoryProperties, std::optional<VkAllocationCallbacks> bufferAllocator,
	std::optional<VkAllocationCallbacks> memoryAllocator) : VulkanResource(bufferAllocator), _device(device), _size(size)
{
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = _size,
		.usage = usage
	};

	VkResult result{ vkCreateBuffer(_device, &bufferInfo, GetAllocator(), &_buffer)};

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan buffer");

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(_device, _buffer, &memoryRequirements);

	VkPhysicalDeviceMemoryProperties physicalMemoryProperties{};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalMemoryProperties);

	std::uint32_t memoryTypeIndex{ FindMemoryTypeIndex(physicalMemoryProperties, memoryRequirements, memoryProperties) };
	if (memoryTypeIndex == UINT32_MAX) throw std::runtime_error("Failed to find memory type index");

	VkMemoryAllocateInfo memoryInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = memoryTypeIndex
	};

	_memory.emplace(_device, &memoryInfo, memoryAllocator);

	result = vkBindBufferMemory(_device, _buffer, _memory->GetMemory(), 0);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to bind Vulkan buffer memory");
}

VulkanBuffer::~VulkanBuffer()
{
	if (_buffer)
	{
		vkDestroyBuffer(_device, _buffer, GetAllocator());
		_buffer = VK_NULL_HANDLE;
	}
}

VkBuffer VulkanBuffer::GetBuffer() const
{
	return _buffer;
}

VkDeviceSize VulkanBuffer::GetSize() const
{
	return _size;
}

void VulkanBuffer::Write(const void* data, VkDeviceSize size, VkDeviceSize offset) const
{
	if (offset > _size || size > _size - offset) throw std::runtime_error("Attempted buffer write beyond Vulkan buffer size");

	void* mapPtr{};
	VkResult result{ vkMapMemory(_device, _memory->GetMemory(), offset, size, 0, &mapPtr) };
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to map buffer memory");

	std::memcpy(mapPtr, data, size);
	vkUnmapMemory(_device, _memory->GetMemory());
}

std::uint32_t VulkanBuffer::FindMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties physicalMemoryProps,
	const VkMemoryRequirements memoryReqs, const VkMemoryPropertyFlags memoryProps)
{
	for (std::uint32_t i{}; i < physicalMemoryProps.memoryTypeCount; ++i)
	{
		const bool compatible{ (memoryReqs.memoryTypeBits & (1 << i)) != 0 };
		const bool propertiesMatch{ (physicalMemoryProps.memoryTypes[i].propertyFlags & memoryProps) == memoryProps };

		if (compatible && propertiesMatch) return i;
	}

	return UINT32_MAX;
}