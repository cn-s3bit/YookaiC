#include "SDLExVulkan.h"
VkBuffer VulkanVertexBuffer;
VkDeviceMemory VulkanVertexBufferMemory;

void * SDLExVertexBufferMemory;

VkBuffer get_vk_vertex_buffer(void) {
	return VulkanVertexBuffer;
}

void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * out_buffer, VkDeviceMemory * out_memory) {
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	int ret;
	ret = vkCreateBuffer(get_vk_device(), &bufferInfo, NULL, out_buffer);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Buffer: vkCreateBuffer returns %d", ret);
		return;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(get_vk_device(), *out_buffer, &memRequirements);
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(get_vk_physical_device(), &memProperties);

	unsigned typeFilter = memRequirements.memoryTypeBits;

	unsigned found = SDL_MAX_UINT32;
	for (unsigned i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			found = i;
			break;
		}
	}

	if (found == SDL_MAX_UINT32) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Find Suitable Memory Type!");
		return;
	}

	VkMemoryAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = found;

	ret = vkAllocateMemory(get_vk_device(), &allocInfo, NULL, out_memory);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Allocate Buffer Memory: vkAllocateMemory returns %d", ret);
		return;
	}

	vkBindBufferMemory(get_vk_device(), *out_buffer, *out_memory, 0);

	SDL_Log("Created Buffer at %u with VRAM at %u",
		(unsigned)*out_buffer,
		(unsigned)*out_memory);
}

void create_vertex_buffer(void) {
	create_buffer(
		sizeof(Vertex) * 6,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&VulkanVertexBuffer,
		&VulkanVertexBufferMemory
	);
}

void * request_vertex_buffer_memory(void) {
	// TODO: Implement Staging Buffer
	vkMapMemory(get_vk_device(), VulkanVertexBufferMemory, 0, sizeof(Vertex) * 3, 0, &SDLExVertexBufferMemory);
	return SDLExVertexBufferMemory;
}

void flush_vertex_buffer_memory(void) {
	vkUnmapMemory(get_vk_device(), VulkanVertexBufferMemory);
}

void cleanup_vertex_buffer(void) {
	vkDestroyBuffer(get_vk_device(), VulkanVertexBuffer, NULL);
	vkFreeMemory(get_vk_device(), VulkanVertexBufferMemory, NULL);
}
