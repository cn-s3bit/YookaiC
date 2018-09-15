#include "SDLExVulkan.h"

void create_frame_buffers(SDLExVulkanSwapChain * pSwapChain, SDLExVulkanGraphicsPipeline * pPipeline) {
	pPipeline->FrameBuffers = malloc(sizeof(VkFramebuffer) * pSwapChain->ImageCount);
	for (unsigned i = 0; i < pSwapChain->ImageCount; i++) {
		VkImageView attachments[1];
		attachments[0] = pSwapChain->ImageViews[i];

		VkFramebufferCreateInfo framebufferInfo = { .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = pPipeline->RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = pSwapChain->SwapChainInfo.imageExtent.width;
		framebufferInfo.height = pSwapChain->SwapChainInfo.imageExtent.height;
		framebufferInfo.layers = 1;

		int ret = vkCreateFramebuffer(get_vk_device(), &framebufferInfo, NULL, &pPipeline->FrameBuffers[i]);
		if (ret != VK_SUCCESS) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to Create Frame Buffer #%d: vkCreateFramebuffer returns %d\n", i, ret);
		}
	}
}

void cleanup_frame_buffers(SDLExVulkanSwapChain * pSwapChain, SDLExVulkanGraphicsPipeline * pPipeline) {
	VkDevice device = get_vk_device();
	for (unsigned i = 0; i < pSwapChain->ImageCount; i++) {
		vkDestroyFramebuffer(device, pPipeline->FrameBuffers[i], NULL);
	}
	free(pPipeline->FrameBuffers);
}
