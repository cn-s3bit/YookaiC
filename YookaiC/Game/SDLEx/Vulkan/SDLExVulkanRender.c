#include "SDLExVulkan.h"

void sdlex_test_render(SDLExVulkanSwapChain * swapchain) {
	VkDevice device = get_vk_device();
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphore) != VK_SUCCESS) {
		return;
	}

	unsigned imageIndex;
	vkAcquireNextImageKHR(device, swapchain->SwapChain, SDL_MAX_SINT32,
		imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };

	VkSemaphore * waitSemaphores = &imageAvailableSemaphore;
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &swapchain->CommandBuffers[imageIndex];
	VkSemaphore * signalSemaphores = &renderFinishedSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	int ret = vkQueueSubmit(get_vk_queue(), 1, &submitInfo, VK_NULL_HANDLE);
	if (ret != VK_SUCCESS) {
		return;
	}

	VkPresentInfoKHR presentInfo = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR * swapChains = &swapchain->SwapChain;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = NULL;
	vkQueuePresentKHR(get_vk_queue(), &presentInfo);
	vkQueueWaitIdle(get_vk_queue());

	vkDestroySemaphore(device, renderFinishedSemaphore, NULL);
	vkDestroySemaphore(device, imageAvailableSemaphore, NULL);
}

void sdlex_test_render_init(SDLExVulkanSwapChain * swapchain, SDLExVulkanGraphicsPipeline * pipeline) {
	for (size_t i = 0; i < swapchain->ImageCount; i++) {
		VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		int ret = vkBeginCommandBuffer(swapchain->CommandBuffers[i], &beginInfo);
		if (ret != VK_SUCCESS) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to Begin Recording Command Buffer: vkBeginCommandBuffer returns %d\n", ret);
		}

		VkRenderPassBeginInfo renderPassInfo = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassInfo.renderPass = pipeline->RenderPass;
		renderPassInfo.framebuffer = pipeline->FrameBuffers[i];
		renderPassInfo.renderArea.offset.x = renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = swapchain->SwapChainInfo.imageExtent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(swapchain->CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(swapchain->CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GraphicsPipeline);
		vkCmdDraw(swapchain->CommandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(swapchain->CommandBuffers[i]);
		if ((ret = vkEndCommandBuffer(swapchain->CommandBuffers[i])) != VK_SUCCESS) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to Record Command Buffer: vkEndCommandBuffer returns %d\n", ret);
		}
	}
}
