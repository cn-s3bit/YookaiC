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

CODEGEN_CUCKOO_HASHMAP(teximagemap, int, VkImage, sdlex_hash_int, sdlex_equal_int, free, sdlex_free_image)
CODEGEN_CUCKOO_HASHMAP(texmemorymap, int, VkDeviceMemory, sdlex_hash_int, sdlex_equal_int, free, sdlex_free_memory)
CODEGEN_CUCKOO_HASHMAP(texviewmap, int, VkImageView, sdlex_hash_int, sdlex_equal_int, free, sdlex_free_imageview)
CODEGEN_CUCKOO_HASHMAP(texsamplermap, int, VkSampler, sdlex_hash_int, sdlex_equal_int, free, sdlex_free_sampler)

CuckooHashMap * texture_images, * texture_memories, * texture_views, * texture_samplers;
unsigned next_image_id = 0;

SDL_Rect texture_frame(SDL_Texture * texture) {
	SDL_Rect result = { .x = 0,.y = 0 };
	SDL_QueryTexture(texture, NULL, NULL, &result.w, &result.h);
	return result;
}

void create_image(unsigned width, unsigned height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory) {
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
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Create Image for Texture: vkCreateImage returns %d\n", ret);
		return;
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
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Find Suitable Memory Type!");
		return;
	}
	allocInfo.memoryTypeIndex = found;

	ret = vkAllocateMemory(get_vk_device(), &allocInfo, NULL, imageMemory);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Allocate Image Memory: vkAllocateMemory returns %d\n", ret);
		return;
	}

	vkBindImageMemory(get_vk_device(), *image, *imageMemory, 0);
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
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = 1.0f,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
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

int load_texture2d(const char * filename) {
	if (!texture_images) {
		// Initialize
		texture_images = create_teximagemap();
		texture_memories = create_texmemorymap();
		texture_views = create_texviewmap();
		texture_samplers = create_texsamplermap();
	}
	SDL_Surface * raw = IMG_Load(filename);
	VkDeviceSize imageSize = raw->w * raw->h * raw->format->BytesPerPixel;

	if (!raw) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Load Texture2D: %s", SDL_GetError());
	}

	// TODO: Implement Staging Buffer
	// VkBuffer stagingBuffer;
	// VkDeviceMemory stagingBufferMemory;
	// createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	VkImage * textureImage = NULL;
	VkDeviceMemory * textureImageMemory = NULL;
	create_image(raw->w, raw->h, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
	VkImageView textureImageView = create_image_view(*textureImage, VK_FORMAT_R8G8B8A8_UNORM);
	VkSampler textureSampler = create_sampler();
	void * data;
	vkMapMemory(get_vk_device(), *textureImageMemory, 0, imageSize, 0, &data);
	SDL_memcpy(data, raw->pixels, (size_t)(imageSize));
	vkUnmapMemory(get_vk_device(), *textureImageMemory);

	++next_image_id;
	put_teximagemap(texture_images, next_image_id, *textureImage);
	put_texmemorymap(texture_memories, next_image_id, *textureImageMemory);
	put_texviewmap(texture_views, next_image_id, textureImageView);
	put_texsamplermap(texture_samplers, next_image_id, textureSampler);

	SDL_FreeSurface(raw);
	return next_image_id;
}
