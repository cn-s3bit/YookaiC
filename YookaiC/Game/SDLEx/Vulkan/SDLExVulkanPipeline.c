#include "SDLExVulkan.h"
#include "../Utils/FileUtils.h"

SDLExVulkanGraphicsPipeline VulkanPipeline;
VkDescriptorSetLayout VulkanDescriptorSetLayout;
SDLExVulkanGraphicsPipeline * get_vk_pipeline(void) {
	return &VulkanPipeline;
}

VkShaderModule create_shader_module(char * code, size_t codeSize) {
	VkShaderModuleCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = codeSize;
	createInfo.pCode = (unsigned *) code;

	VkShaderModule shaderModule;
	int ret;
	if ((ret = vkCreateShaderModule(get_vk_device(), &createInfo, NULL, &shaderModule)) != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Shader Module: vkCreateShaderModule returns %d\n", ret);
		return VK_NULL_HANDLE;
	}

	return shaderModule;
}

VkPipeline create_graphics_pipeline_f(const char * vertShaderFilename, const char * fragShaderFilename) {
	size_t size;
	char * vert = read_file_to_char_array(vertShaderFilename, &size);
	VkShaderModule vertShader = create_shader_module(vert, size);
	free(vert);
	char * frag = read_file_to_char_array(fragShaderFilename, &size);
	VkShaderModule fragShader = create_shader_module(frag, size);
	free(frag);
	return create_graphics_pipeline(vertShader, fragShader);
}

static void _sdlex_prepare_pipeline(void) {
	VkDescriptorSetLayoutBinding samplerLayoutBinding;
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = NULL;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerLayoutBinding;

	vkCreateDescriptorSetLayout(get_vk_device(), &layoutInfo, NULL, &VulkanDescriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &VulkanDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = NULL;
	int ret;
	if ((ret = vkCreatePipelineLayout(get_vk_device(),
		&pipelineLayoutInfo, NULL,
		&VulkanPipeline.PipelineLayout)) != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Pipeline Layout: vkCreatePipelineLayout returns %d\n", ret);
		return;
	}

	VkAttachmentDescription colorAttachment = {
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE
	};
	colorAttachment.format = get_vk_swap_chain()->SwapChainInfo.imageFormat;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS };
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if ((ret = vkCreateRenderPass(get_vk_device(), &renderPassInfo, NULL,
		&VulkanPipeline.RenderPass)) != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Render Pass: vkCreateRenderPass returns %d\n", ret);
		return;
	}
}

VkPipeline create_graphics_pipeline(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule) {
	_sdlex_prepare_pipeline();
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

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	VkVertexInputBindingDescription dex = _sdlex_get_binding_description();
	vertexInputInfo.pVertexBindingDescriptions = &dex;
	vertexInputInfo.vertexAttributeDescriptionCount = 3;
	VkVertexInputAttributeDescription * att = _sdlex_get_attribute_descriptions();
	vertexInputInfo.pVertexAttributeDescriptions = att;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)get_vk_swap_chain()->SwapChainInfo.imageExtent.width;
	viewport.height = (float)get_vk_swap_chain()->SwapChainInfo.imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = get_vk_swap_chain()->SwapChainInfo.imageExtent;

	VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT ^ VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = NULL;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = NULL;
	pipelineInfo.layout = VulkanPipeline.PipelineLayout;
	pipelineInfo.renderPass = VulkanPipeline.RenderPass;
	pipelineInfo.subpass = 0;

	int ret = vkCreateGraphicsPipelines(get_vk_device(), VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &VulkanPipeline.GraphicsPipeline);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to Create Graphics Pipeline: vkCreateGraphicsPipelines returns %d\n", ret);
	}
	else {
		SDL_Log("Created Pipeline at %u\n", (unsigned)VulkanPipeline.GraphicsPipeline);
		create_frame_buffers(get_vk_swap_chain(), &VulkanPipeline);
		create_vertex_buffer(6);
		create_descriptor_pool();
	}
	free(att);
	vkDestroyShaderModule(get_vk_device(), fragShaderModule, NULL);
	vkDestroyShaderModule(get_vk_device(), vertShaderModule, NULL);
	return VulkanPipeline.GraphicsPipeline;
}

void cleanup_vulkan_pipeline(void) {
	VkDevice device = get_vk_device();
	cleanup_frame_buffers(get_vk_swap_chain(), &VulkanPipeline);
	cleanup_vertex_buffer();
	vkDestroyPipeline(device, VulkanPipeline.GraphicsPipeline, NULL);
	vkDestroyPipelineLayout(device, VulkanPipeline.PipelineLayout, NULL);
	vkDestroyRenderPass(device, VulkanPipeline.RenderPass, NULL);
	cleanup_descriptor_pool();
	vkDestroyDescriptorSetLayout(device, VulkanDescriptorSetLayout, NULL);
}
