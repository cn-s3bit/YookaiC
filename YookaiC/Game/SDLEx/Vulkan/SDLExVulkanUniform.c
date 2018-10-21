#include "SDLExVulkan.h"
VkDescriptorPool VulkanDescriptorPool;
extern VkDescriptorSetLayout VulkanDescriptorSetLayout;

VkImageCreateInfo VulkanCurrentTextureInfo[1024];

VkImageCreateInfo * sdlex_get_current_texture_info(unsigned imageIndex) {
	return &VulkanCurrentTextureInfo[imageIndex];
}

void create_descriptor_pool() {
	VkDescriptorPoolSize poolSize = { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	poolSize.descriptorCount = get_vk_swap_chain()->ImageCount;

	VkDescriptorPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = poolSize.descriptorCount;

	if (vkCreateDescriptorPool(get_vk_device(), &poolInfo, NULL, &VulkanDescriptorPool) != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Create Descriptor Pool!");
	}
	create_descriptor_sets();
}

void cleanup_descriptor_pool() {
	vkDestroyDescriptorPool(get_vk_device(), VulkanDescriptorPool, NULL);
}

void bind_texture(unsigned imageIndex, VkImageView textureImageView, VkSampler textureSampler, VkImageCreateInfo textureInfo) {
	unsigned i = imageIndex; {
		VulkanCurrentTextureInfo[imageIndex] = textureInfo;
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		VkWriteDescriptorSet descriptorWrites;
		descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites.dstSet = get_vk_pipeline()->DescriptorSets[i];
		descriptorWrites.dstBinding = 0;
		descriptorWrites.dstArrayElement = 0;
		descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites.descriptorCount = 1;
		descriptorWrites.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(get_vk_device(), 1, &descriptorWrites, 0, NULL);
	}
}

void create_descriptor_sets() {
	VkDescriptorSetLayout * layouts = malloc(sizeof(VkDescriptorSetLayout) * get_vk_swap_chain()->ImageCount);
	for (unsigned i = 0; i < get_vk_swap_chain()->ImageCount; i++)
		layouts[i] = VulkanDescriptorSetLayout;
	VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = VulkanDescriptorPool;
	allocInfo.descriptorSetCount = get_vk_swap_chain()->ImageCount;
	allocInfo.pSetLayouts = layouts;

	get_vk_pipeline()->DescriptorSets = malloc(sizeof(VkDescriptorSet) * allocInfo.descriptorSetCount);

	if (vkAllocateDescriptorSets(get_vk_device(), &allocInfo, get_vk_pipeline()->DescriptorSets) != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Allocate Descriptor Sets!");
	}
}
