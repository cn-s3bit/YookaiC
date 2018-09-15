#include "SDLExVulkan.h"

VkCommandPool VulkanCommandPool;

VkCommandPool create_command_buffer(SDLExVulkanSwapChain * pSwapChain) {
	int gqf = find_queue_families(get_vk_physical_device(), VK_QUEUE_GRAPHICS_BIT);

	VkCommandPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.queueFamilyIndex = gqf;
	poolInfo.flags = 0;

	int ret = vkCreateCommandPool(get_vk_device(), &poolInfo, NULL, &VulkanCommandPool);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Command Pool: vkCreateCommandPool returns %d\n", ret);
		return VK_NULL_HANDLE;
	}

	pSwapChain->CommandBuffers = malloc(sizeof(VkCommandBuffer) * pSwapChain->ImageCount);
	VkCommandBufferAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = VulkanCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = pSwapChain->ImageCount;

	ret = vkAllocateCommandBuffers(get_vk_device(), &allocInfo, pSwapChain->CommandBuffers);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Command Buffer: vkAllocateCommandBuffers returns %d\n", ret);
		return VK_NULL_HANDLE;
	}
	return VulkanCommandPool;
}

void cleanup_command_buffer(SDLExVulkanSwapChain * pSwapChain) {
	free(pSwapChain->CommandBuffers);
	vkDestroyCommandPool(get_vk_device(), VulkanCommandPool, NULL);
}
