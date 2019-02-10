#include "SDLExVulkan.h"
#include "../Utils/MathUtils.h"
#include "../MathEx/MathEx.h"
#include "../Utils/ArrayList.h"

Vertex Vertices[6] = {
	{ { 0.25f, -1.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
	{ { 0.25f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
	{ { 0.9f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
	{ { 0.25f, -1.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
	{ { 0.9f, -1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
	{ { 0.9f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
};

VkSemaphore imageAvailableSemaphore;
VkFence the_fence = VK_NULL_HANDLE;
ArrayList * batch_buffer;

#define VKRENDER_GLOBALS_INIT \
	if (batch_buffer == NULL) {\
		batch_buffer = create_array_list(sizeof(Vertices), 16u);\
	}

unsigned sdlex_begin_frame() {
	VKRENDER_GLOBALS_INIT
	VkDevice device = get_vk_device();
	SDLExVulkanSwapChain * swapchain = get_vk_swap_chain();
	VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphore);
	unsigned imageIndex;
	vkAcquireNextImageKHR(device, swapchain->SwapChain, SDL_MAX_SINT32,
		imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	sdlex_render_init(swapchain, get_vk_pipeline(), 1);
	VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &get_vk_swap_chain()->CommandBuffers[imageIndex];
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.waitSemaphoreCount = 1;
	vkQueueSubmit(get_vk_queue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(get_vk_queue());
	vkDestroySemaphore(device, imageAvailableSemaphore, NULL);

	VkFenceCreateInfo ci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(device, &ci, NULL, &the_fence);
	return imageIndex;
}

void sdlex_render_texture(unsigned imageIndex, SDL_Rect target) {
	// TODO: Image Specificated Batching
	VKRENDER_GLOBALS_INIT
	VkExtent2D screenSize = get_vk_swap_chain()->SwapChainInfo.imageExtent;
#define MAP_POS_TO_VIEWPORT_X(val) sdlex_map_float((float)val, 0.0f, (float)screenSize.width, -1.0f, 1.0f)
#define MAP_POS_TO_VIEWPORT_Y(val) sdlex_map_float((float)val, 0.0f, (float)screenSize.height, -1.0f, 1.0f)
	Vertices[0].Pos.X = Vertices[1].Pos.X = Vertices[3].Pos.X = MAP_POS_TO_VIEWPORT_X(target.x);
	Vertices[2].Pos.X = Vertices[4].Pos.X = Vertices[5].Pos.X = MAP_POS_TO_VIEWPORT_X(target.x + target.w);
	Vertices[0].Pos.Y = Vertices[3].Pos.Y = Vertices[4].Pos.Y = MAP_POS_TO_VIEWPORT_Y(target.y);
	Vertices[1].Pos.Y = Vertices[2].Pos.Y = Vertices[5].Pos.Y = MAP_POS_TO_VIEWPORT_Y(target.y + target.h);
#undef MAP_POS_TO_VIEWPORT_X
#undef MAP_POS_TO_VIEWPORT_Y
	append_array_list(batch_buffer, Vertices);
}

void sdlex_render_texture_ex(unsigned imageIndex, Vector2 position, Vector2 origin, float rotation, Vector2 scale) {
	VKRENDER_GLOBALS_INIT
		VkExtent2D screenSize = get_vk_swap_chain()->SwapChainInfo.imageExtent;
	VkImageCreateInfo * currentTextureInfo = sdlex_get_current_texture_info(imageIndex);
	Vector2 leftBottom = vector2_sub(position, origin);
	Vector2 rightUpper = vector2_adds(leftBottom, (float)currentTextureInfo->extent.width, (float)currentTextureInfo->extent.height);
	leftBottom = vector2_sub(leftBottom, position);
	rightUpper = vector2_sub(rightUpper, position);
	leftBottom.X *= scale.X;
	leftBottom.Y *= scale.Y;
	rightUpper.X *= scale.X;
	rightUpper.Y *= scale.Y;
	leftBottom = vector2_add(leftBottom, position);
	rightUpper = vector2_add(rightUpper, position);
#define MAP_POS_TO_VIEWPORT_X(val) sdlex_map_float((float)val, 0.0f, (float)screenSize.width, -1.0f, 1.0f)
#define MAP_POS_TO_VIEWPORT_Y(val) sdlex_map_float((float)val, 0.0f, (float)screenSize.height, -1.0f, 1.0f)
	Vertices[0].Pos.X = Vertices[1].Pos.X = Vertices[3].Pos.X = (leftBottom.X);
	Vertices[2].Pos.X = Vertices[4].Pos.X = Vertices[5].Pos.X = (rightUpper.X);
	Vertices[0].Pos.Y = Vertices[3].Pos.Y = Vertices[4].Pos.Y = (leftBottom.Y);
	Vertices[1].Pos.Y = Vertices[2].Pos.Y = Vertices[5].Pos.Y = (rightUpper.Y);
	Vertices[0].Pos = vector2_rotate_around(Vertices[0].Pos, position, rotation);
	Vertices[1].Pos = vector2_rotate_around(Vertices[1].Pos, position, rotation);
	Vertices[2].Pos = vector2_rotate_around(Vertices[2].Pos, position, rotation);
	Vertices[3].Pos = vector2_rotate_around(Vertices[3].Pos, position, rotation);
	Vertices[4].Pos = vector2_rotate_around(Vertices[4].Pos, position, rotation);
	Vertices[5].Pos = vector2_rotate_around(Vertices[5].Pos, position, rotation);

	Vertices[0].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[0].Pos.X);
	Vertices[1].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[1].Pos.X);
	Vertices[2].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[2].Pos.X);
	Vertices[3].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[3].Pos.X);
	Vertices[4].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[4].Pos.X);
	Vertices[5].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[5].Pos.X);

	Vertices[0].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[0].Pos.Y);
	Vertices[1].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[1].Pos.Y);
	Vertices[2].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[2].Pos.Y);
	Vertices[3].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[3].Pos.Y);
	Vertices[4].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[4].Pos.Y);
	Vertices[5].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[5].Pos.Y);
#undef MAP_POS_TO_VIEWPORT_X
#undef MAP_POS_TO_VIEWPORT_Y
	append_array_list(batch_buffer, Vertices);
}

void sdlex_render_texture_region_ex(unsigned imageIndex, Vector2 position, Vector2 origin, float rotation, Vector2 scale, SDL_Rect sourceRegion) {
	VKRENDER_GLOBALS_INIT
		VkExtent2D screenSize = get_vk_swap_chain()->SwapChainInfo.imageExtent;
	Vector2 leftBottom = vector2_sub(position, origin);
	Vector2 rightUpper = vector2_adds(leftBottom, (float)sourceRegion.w, (float)sourceRegion.h);
	VkImageCreateInfo * textureInfo = sdlex_get_current_texture_info(imageIndex);
	Vertices[0].TexCoord.X = Vertices[1].TexCoord.X = Vertices[3].TexCoord.X = (float)sourceRegion.x / textureInfo->extent.width;
	Vertices[2].TexCoord.X = Vertices[4].TexCoord.X = Vertices[5].TexCoord.X = (float)(sourceRegion.x + sourceRegion.w) / textureInfo->extent.width;
	Vertices[0].TexCoord.Y = Vertices[3].TexCoord.Y = Vertices[4].TexCoord.Y = (float)sourceRegion.y / textureInfo->extent.height;
	Vertices[1].TexCoord.Y = Vertices[2].TexCoord.Y = Vertices[5].TexCoord.Y = (float)(sourceRegion.y + sourceRegion.h) / textureInfo->extent.height;
	leftBottom = vector2_sub(leftBottom, position);
	rightUpper = vector2_sub(rightUpper, position);
	leftBottom.X *= scale.X;
	leftBottom.Y *= scale.Y;
	rightUpper.X *= scale.X;
	rightUpper.Y *= scale.Y;
	leftBottom = vector2_add(leftBottom, position);
	rightUpper = vector2_add(rightUpper, position);
#define MAP_POS_TO_VIEWPORT_X(val) sdlex_map_float((float)val, 0.0f, (float)screenSize.width, -1.0f, 1.0f)
#define MAP_POS_TO_VIEWPORT_Y(val) sdlex_map_float((float)val, 0.0f, (float)screenSize.height, -1.0f, 1.0f)
	Vertices[0].Pos.X = Vertices[1].Pos.X = Vertices[3].Pos.X = (leftBottom.X);
	Vertices[2].Pos.X = Vertices[4].Pos.X = Vertices[5].Pos.X = (rightUpper.X);
	Vertices[0].Pos.Y = Vertices[3].Pos.Y = Vertices[4].Pos.Y = (leftBottom.Y);
	Vertices[1].Pos.Y = Vertices[2].Pos.Y = Vertices[5].Pos.Y = (rightUpper.Y);
	Vertices[0].Pos = vector2_rotate_around(Vertices[0].Pos, position, rotation);
	Vertices[1].Pos = vector2_rotate_around(Vertices[1].Pos, position, rotation);
	Vertices[2].Pos = vector2_rotate_around(Vertices[2].Pos, position, rotation);
	Vertices[3].Pos = vector2_rotate_around(Vertices[3].Pos, position, rotation);
	Vertices[4].Pos = vector2_rotate_around(Vertices[4].Pos, position, rotation);
	Vertices[5].Pos = vector2_rotate_around(Vertices[5].Pos, position, rotation);

	Vertices[0].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[0].Pos.X);
	Vertices[1].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[1].Pos.X);
	Vertices[2].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[2].Pos.X);
	Vertices[3].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[3].Pos.X);
	Vertices[4].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[4].Pos.X);
	Vertices[5].Pos.X = MAP_POS_TO_VIEWPORT_X(Vertices[5].Pos.X);

	Vertices[0].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[0].Pos.Y);
	Vertices[1].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[1].Pos.Y);
	Vertices[2].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[2].Pos.Y);
	Vertices[3].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[3].Pos.Y);
	Vertices[4].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[4].Pos.Y);
	Vertices[5].Pos.Y = MAP_POS_TO_VIEWPORT_Y(Vertices[5].Pos.Y);
#undef MAP_POS_TO_VIEWPORT_X
#undef MAP_POS_TO_VIEWPORT_Y
	append_array_list(batch_buffer, Vertices);
}

void sdlex_end_frame(unsigned imageIndex) {
	sdlex_render_flush(imageIndex);
	VkPresentInfoKHR presentInfo = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	VkSwapchainKHR * swapChains = &get_vk_swap_chain()->SwapChain;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = NULL;
	vkQueuePresentKHR(get_vk_queue(), &presentInfo);
	vkQueueWaitIdle(get_vk_queue());
	vkDestroyFence(get_vk_device(), the_fence, NULL);
}

void sdlex_render_flush(unsigned imageIndex) {
	VKRENDER_GLOBALS_INIT
	if (batch_buffer->Size == 0)
		return;
	unsigned i = imageIndex;
	SDLExVulkanSwapChain * swapchain = get_vk_swap_chain();
	SDLExVulkanGraphicsPipeline * pipeline = get_vk_pipeline();
	recreate_vertex_buffer(batch_buffer->Size);
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
	vkCmdBindDescriptorSets(swapchain->CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->PipelineLayout, 0, 1, &pipeline->DescriptorSets[i], 0, NULL);
	vkCmdDraw(swapchain->CommandBuffers[i], batch_buffer->Size * 6, 1, 0, 0);
	vkCmdEndRenderPass(swapchain->CommandBuffers[i]);
	if ((ret = vkEndCommandBuffer(swapchain->CommandBuffers[i])) != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Record Command Buffer: vkEndCommandBuffer returns %d\n", ret);
	}
	void * addr = request_vertex_buffer_memory();
	SDL_memcpy4(addr, batch_buffer->_data, batch_buffer->ElementSize * batch_buffer->Size / 4);
	flush_vertex_buffer_memory();

	VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &get_vk_swap_chain()->CommandBuffers[imageIndex];
	vkQueueSubmit(get_vk_queue(), 1, &submitInfo, the_fence);
	vkWaitForFences(get_vk_device(), 1, &the_fence, VK_TRUE, SDL_MAX_SINT64);
	vkResetFences(get_vk_device(), 1, &the_fence);
	batch_buffer->Size = 0;
}

void sdlex_render_init(SDLExVulkanSwapChain * swapchain, SDLExVulkanGraphicsPipeline * pipeline, int clear) {
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

		if (clear) {
			VkClearAttachment clearatt;
			clearatt.clearValue = clearColor;
			clearatt.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			clearatt.colorAttachment = 0;
			VkClearRect clearrect;
			clearrect.baseArrayLayer = 0;
			clearrect.layerCount = 1;
			clearrect.rect.offset.x = clearrect.rect.offset.y = 0;
			clearrect.rect.extent = swapchain->SwapChainInfo.imageExtent;
			vkCmdClearAttachments(swapchain->CommandBuffers[i], 1, &clearatt, 1, &clearrect);
		}
		else {
			VkBuffer buffer = get_vk_vertex_buffer();
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(swapchain->CommandBuffers[i], 0, 1, &buffer, &offset);
			vkCmdBindDescriptorSets(swapchain->CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->PipelineLayout, 0, 1, &pipeline->DescriptorSets[i], 0, NULL);
			vkCmdDraw(swapchain->CommandBuffers[i], 6, 1, 0, 0);
		}
		vkCmdEndRenderPass(swapchain->CommandBuffers[i]);
		if ((ret = vkEndCommandBuffer(swapchain->CommandBuffers[i])) != VK_SUCCESS) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to Record Command Buffer: vkEndCommandBuffer returns %d\n", ret);
		}
	}
}
