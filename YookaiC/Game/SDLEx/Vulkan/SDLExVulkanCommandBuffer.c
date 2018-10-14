#include "SDLExVulkan.h"

VkCommandPool VulkanCommandPool;

VkCommandPool create_command_buffer(SDLExVulkanSwapChain * pSwapChain) {
	int gqf = find_queue_families(get_vk_physical_device(), VK_QUEUE_GRAPHICS_BIT);

	VkCommandPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.queueFamilyIndex = gqf;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

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

VkCommandBuffer begin_single_time_commands() {
	VkCommandBufferAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = VulkanCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(get_vk_device(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void end_single_time_commands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(get_vk_queue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(get_vk_queue());

	vkFreeCommandBuffers(get_vk_device(), VulkanCommandPool, 1, &commandBuffer);
}
