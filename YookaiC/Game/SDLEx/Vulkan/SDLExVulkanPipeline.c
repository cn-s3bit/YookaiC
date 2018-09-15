#include "SDLExVulkan.h"
#include "../FileUtils.h"

VkShaderModule create_shader_module(char * code, size_t codeSize) {
	VkShaderModuleCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = codeSize;
	createInfo.pCode = (unsigned *) code;

	VkShaderModule shaderModule;
	int ret;
	if ((ret = vkCreateShaderModule(get_vk_device(), &createInfo, NULL, &shaderModule)) != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Shader Module: vkCreateShaderModule returns %d", ret);
		return VK_NULL_HANDLE;
	}

	return shaderModule;
}

void create_graphics_pipeline_f(const char * vertShaderFilename, const char * fragShaderFilename) {
	size_t size;
	char * vert = read_file_to_char_array(vertShaderFilename, &size);
	VkShaderModule vertShader = create_shader_module(vert, size);
	free(vert);
	char * frag = read_file_to_char_array(fragShaderFilename, &size);
	VkShaderModule fragShader = create_shader_module(frag, size);
	free(frag);
	return create_graphics_pipeline(vertShader, fragShader);
}

void create_graphics_pipeline(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule) {
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT
	};
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT
	};
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0] = vertShaderStageInfo;
	shaderStages[1] = fragShaderStageInfo;

	vkDestroyShaderModule(get_vk_device(), fragShaderModule, NULL);
	vkDestroyShaderModule(get_vk_device(), vertShaderModule, NULL);
}
