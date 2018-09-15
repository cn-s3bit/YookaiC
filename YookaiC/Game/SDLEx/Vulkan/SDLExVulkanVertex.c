#include "SDLExVulkan.h"
VkBuffer VulkanVertexBuffer;
VkDeviceMemory VulkanVertexBufferMemory;

void * SDLExVertexBufferMemory;

VkBuffer get_vk_vertex_buffer(void) {
	return VulkanVertexBuffer;
}

void create_vertex_buffer(void) {
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = sizeof(Vertex) * 3;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	int ret;
	ret = vkCreateBuffer(get_vk_device(), &bufferInfo, NULL, &VulkanVertexBuffer);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Vertex Buffer: vkCreateBuffer returns %d", ret);
		return;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(get_vk_device(), VulkanVertexBuffer, &memRequirements);
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(get_vk_physical_device(), &memProperties);

	unsigned typeFilter = memRequirements.memoryTypeBits;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

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

	ret = vkAllocateMemory(get_vk_device(), &allocInfo, NULL, &VulkanVertexBufferMemory);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Allocate Vertex Buffer Memory: vkAllocateMemory returns %d", ret);
		return;
	}

	vkBindBufferMemory(get_vk_device(), VulkanVertexBuffer, VulkanVertexBufferMemory, 0);

	SDL_Log("Created Vertex Buffer at %u with VRAM at %u",
		(unsigned)VulkanVertexBuffer,
		(unsigned)VulkanVertexBufferMemory);
}

void * request_vertex_buffer_memory(void) {
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
