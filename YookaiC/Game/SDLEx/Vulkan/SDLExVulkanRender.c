#include "SDLExVulkan.h"
#include "../MathUtils.h"
#include "../MathEx/MathEx.h"

Vertex Vertices[3] = {
	{ { 0.0f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, 0.7f },{ 0.0f, 1.0f, 0.0f } },
	{ { -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } }
};

void sdlex_test_render(SDLExVulkanSwapChain * swapchain) {
	// First Update Logic!
	void * addr = request_vertex_buffer_memory();
	Vertices[0].Pos.X += 0.002f;
	Vertices[0].Pos.X = SDLEx_pong(Vertices[0].Pos.X, -1.0f, 1.0f);
	Vertices[1].Pos.X += 0.004f;
	Vertices[1].Pos.X = SDLEx_pong(Vertices[1].Pos.X, -1.0f, 1.0f);
	Vertices[2].Pos.X += 0.006f;
	Vertices[2].Pos.X = SDLEx_pong(Vertices[2].Pos.X, -1.0f, 1.0f);
	SDL_memcpy(addr, Vertices, sizeof(Vertices));
	Vertices[0].Pos.Y *= -1.0f;
	Vertices[1].Pos.Y *= -1.0f;
	Vertices[2].Pos.Y *= -1.0f;
	SDL_memcpy((char *)addr + sizeof(Vertices), Vertices, sizeof(Vertices));
	Vertices[0].Pos.Y *= -1.0f;
	Vertices[1].Pos.Y *= -1.0f;
	Vertices[2].Pos.Y *= -1.0f;
	flush_vertex_buffer_memory();

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

		VkBuffer buffer = get_vk_vertex_buffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(swapchain->CommandBuffers[i], 0, 1, &buffer, &offset);

		vkCmdDraw(swapchain->CommandBuffers[i], 6, 1, 0, 0);

		vkCmdEndRenderPass(swapchain->CommandBuffers[i]);
		if ((ret = vkEndCommandBuffer(swapchain->CommandBuffers[i])) != VK_SUCCESS) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to Record Command Buffer: vkEndCommandBuffer returns %d\n", ret);
		}
	}
}
