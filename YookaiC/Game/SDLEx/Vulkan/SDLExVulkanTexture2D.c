#include "SDLExVulkan.h"

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

void load_texture2d(const char * filename) {
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

	void * data;
	vkMapMemory(get_vk_device(), *textureImageMemory, 0, imageSize, 0, &data);
	SDL_memcpy(data, raw->pixels, (size_t)(imageSize));
	vkUnmapMemory(get_vk_device(), *textureImageMemory);

	SDL_FreeSurface(raw);
}
