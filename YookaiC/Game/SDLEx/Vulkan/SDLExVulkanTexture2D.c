#include "SDLExVulkan.h"
#include "../Utils/HashMap.h"

void sdlex_free_image(void * pt) {
	vkDestroyImage(get_vk_device(), *(VkImage *)pt, NULL);
	free(pt);
}
void sdlex_free_memory(void * pt) {
	vkFreeMemory(get_vk_device(), *(VkDeviceMemory *)pt, NULL);
	free(pt);
}
void sdlex_free_imageview(void * pt) {
	vkDestroyImageView(get_vk_device(), *(VkImageView *)pt, NULL);
	free(pt);
}
void sdlex_free_sampler(void * pt) {
	vkDestroySampler(get_vk_device(), *(VkSampler *)pt, NULL);
	free(pt);
}

CODEGEN_CUCKOO_HASHMAP(teximagemap, long, VkImage, sdlex_hash_int, sdlex_equal_int, memorypool_free_4bytes, sdlex_free_image)
CODEGEN_CUCKOO_HASHMAP(texmemorymap, long, VkDeviceMemory, sdlex_hash_int, sdlex_equal_int, memorypool_free_4bytes, sdlex_free_memory)
CODEGEN_CUCKOO_HASHMAP(texviewmap, long, VkImageView, sdlex_hash_int, sdlex_equal_int, memorypool_free_4bytes, sdlex_free_imageview)
CODEGEN_CUCKOO_HASHMAP(texsamplermap, long, VkSampler, sdlex_hash_int, sdlex_equal_int, memorypool_free_4bytes, sdlex_free_sampler)
CODEGEN_CUCKOO_HASHMAP(texinfomap, long, VkImageCreateInfo, sdlex_hash_int, sdlex_equal_int, memorypool_free_4bytes, free)

CuckooHashMap * texture_images, * texture_memories, * texture_views, * texture_samplers, * texture_infos;
long next_image_id = 0;

SDL_Rect texture_frame(SDL_Texture * texture) {
	SDL_Rect result = { .x = 0,.y = 0 };
	SDL_QueryTexture(texture, NULL, NULL, &result.w, &result.h);
	return result;
}

VkImageCreateInfo create_image(unsigned width, unsigned height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory) {
	VkImageCreateInfo imageInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	int ret = vkCreateImage(get_vk_device(), &imageInfo, NULL, image);
	if (ret != VK_SUCCESS) {
		imageInfo.imageType = VK_IMAGE_TYPE_MAX_ENUM;
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Create Image for Texture: vkCreateImage returns %d\n", ret);
		return imageInfo;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(get_vk_device(), *image, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(get_vk_physical_device(), &memProperties);

	unsigned typeFilter = memRequirements.memoryTypeBits;
	VkMemoryAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	unsigned found = SDL_MAX_UINT32;
	for (unsigned i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			found = i;
			break;
		}
	}

	if (found == SDL_MAX_UINT32) {
		imageInfo.imageType = VK_IMAGE_TYPE_MAX_ENUM;
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Find Suitable Memory Type!");
		return imageInfo;
	}
	allocInfo.memoryTypeIndex = found;

	ret = vkAllocateMemory(get_vk_device(), &allocInfo, NULL, imageMemory);
	if (ret != VK_SUCCESS) {
		imageInfo.imageType = VK_IMAGE_TYPE_MAX_ENUM;
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Allocate Image Memory: vkAllocateMemory returns %d\n", ret);
		return imageInfo;
	}
	vkBindImageMemory(get_vk_device(), *image, *imageMemory, 0);
	return imageInfo;
}

VkImageView create_image_view(VkImage image, VkFormat format) {
	VkImageViewCreateInfo viewInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	int ret = vkCreateImageView(get_vk_device(), &viewInfo, NULL, &imageView);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Create Image View for Texture: vkCreateImageView returns %d\n", ret);
	}

	return imageView;
}

VkSampler create_sampler() {
	VkSamplerCreateInfo samplerInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 1.0f,
		.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.mipLodBias = 0.0f,
		.minLod = 0.0f,
		.maxLod = 0.0f
	};
	VkSampler textureSampler;
	int ret = vkCreateSampler(get_vk_device(), &samplerInfo, NULL, &textureSampler);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Create Sampler for Texture: vkCreateSampler returns %d\n", ret);
	}
	return textureSampler;
}

void transition_image_layout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = begin_single_time_commands();

	VkImageMemoryBarrier barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported Layout Transition from %d to %d!", oldLayout, newLayout);
		return;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, NULL,
		0, NULL,
		1, &barrier
	);
	end_single_time_commands(commandBuffer);
}

void copy_buffer_to_image(VkBuffer buffer, VkImage image, unsigned width, unsigned height) {
	VkCommandBuffer commandBuffer = begin_single_time_commands();

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = region.imageOffset.y = region.imageOffset.z = 0;
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	end_single_time_commands(commandBuffer);
}

long load_texture2d(const char * filename) {
	if (!texture_images) {
		// Initialize
		texture_images = create_teximagemap();
		texture_memories = create_texmemorymap();
		texture_views = create_texviewmap();
		texture_samplers = create_texsamplermap();
		texture_infos = create_texinfomap();
	}
	SDL_Surface * raw = IMG_Load(filename);
	if (!raw) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Load Texture2D: %s", SDL_GetError());
		return -1;
	}

	VkDeviceSize imageSize = raw->w * raw->h * raw->format->BytesPerPixel;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
	void* data;
	vkMapMemory(get_vk_device(), stagingBufferMemory, 0, imageSize, 0, &data);
	SDL_memcpy(data, raw->pixels, (size_t)(imageSize));
	vkUnmapMemory(get_vk_device(), stagingBufferMemory);
	
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageCreateInfo imageInfo = create_image(raw->w, raw->h, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &textureImage, &textureImageMemory);
	transition_image_layout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copy_buffer_to_image(stagingBuffer, textureImage, (unsigned)(raw->w), (unsigned)(raw->h));
	transition_image_layout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	VkImageView textureImageView = create_image_view(textureImage, VK_FORMAT_R8G8B8A8_UNORM);
	VkSampler textureSampler = create_sampler();

	++next_image_id;
	put_teximagemap(texture_images, next_image_id, textureImage);
	put_texmemorymap(texture_memories, next_image_id, textureImageMemory);
	put_texviewmap(texture_views, next_image_id, textureImageView);
	put_texsamplermap(texture_samplers, next_image_id, textureSampler);
	put_texinfomap(texture_infos, next_image_id, imageInfo);
	// TODO: Disposal

	SDL_FreeSurface(raw);
	vkDestroyBuffer(get_vk_device(), stagingBuffer, NULL);
	vkFreeMemory(get_vk_device(), stagingBufferMemory, NULL);
	return next_image_id;
}

void bind_texture2d(unsigned imageIndex, long texture_id) {
	if (texture_id < 0 || texture_id > next_image_id) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid texture_id: %d", texture_id);
		return;
	}
	sdlex_render_flush(imageIndex);
	bind_texture(imageIndex, get_texviewmap(texture_views, texture_id), get_texsamplermap(texture_samplers, texture_id), get_texinfomap(texture_infos, texture_id));
	sdlex_render_init(get_vk_swap_chain(), get_vk_pipeline(), 0);
}

void dispose_texture2d(long texture_id) {
	remove_from_texviewmap(texture_views, texture_id);
	remove_from_texsamplermap(texture_samplers, texture_id);
	remove_from_texinfomap(texture_infos, texture_id);
	remove_from_teximagemap(texture_images, texture_id);
	remove_from_texmemorymap(texture_memories, texture_id);
}
