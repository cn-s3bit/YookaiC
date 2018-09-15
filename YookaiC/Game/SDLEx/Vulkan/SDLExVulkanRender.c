#include "SDLExVulkan.h"

void sdlex_test_render(SDLExVulkanSwapChain * swapchain, SDLExVulkanGraphicsPipeline * pipeline) {
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

		vkCmdBindPipeline(pipeline->FrameBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GraphicsPipeline);
		vkCmdDraw(pipeline->FrameBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(swapchain->CommandBuffers[i]);
		if ((ret = vkEndCommandBuffer(swapchain->CommandBuffers[i])) != VK_SUCCESS) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to Record Command Buffer: vkEndCommandBuffer returns %d\n", ret);
		}
	}
}
