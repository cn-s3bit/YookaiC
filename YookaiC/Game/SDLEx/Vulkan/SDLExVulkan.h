#ifndef SDLEX_VULKAN_H
#define SDLEX_VULKAN_H
#include <vulkan/vulkan.h>
#include "../SDLWithPlugins.h"
typedef struct SDLExVulkanSwapChain {
	unsigned ImageCount;
	VkImage * Images;
	VkImageView * ImageViews;
	VkCommandBuffer * CommandBuffers;
	VkSwapchainKHR SwapChain;
	VkSwapchainCreateInfoKHR SwapChainInfo;
} SDLExVulkanSwapChain;

typedef struct SDLExVulkanGraphicsPipeline {
	VkPipelineLayout PipelineLayout;
	VkRenderPass RenderPass;
	VkPipeline GraphicsPipeline;
	VkFramebuffer * FrameBuffers;
} SDLExVulkanGraphicsPipeline;

VkInstance get_vk_instance(void);
VkSurfaceKHR get_vk_surface(void);
VkInstance initialize_vulkan(SDL_Window * window, unsigned appVer);
void cleanup_vulkan(void);

int find_queue_families(VkPhysicalDevice device, int required_flag_bit);
VkDevice get_vk_device(void);
VkQueue get_vk_queue(void);
VkPhysicalDevice get_vk_physical_device(void);
SDLExVulkanSwapChain * get_vk_swap_chain(void);

SDLExVulkanGraphicsPipeline * get_vk_pipeline(void);
VkShaderModule create_shader_module(char * code, size_t codeSize);
VkPipeline create_graphics_pipeline_f(const char * vertShaderFilename, const char * fragShaderFilename);
VkPipeline create_graphics_pipeline(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule);
void cleanup_vulkan_pipeline(void);

void create_frame_buffers(SDLExVulkanSwapChain * pSwapChain, SDLExVulkanGraphicsPipeline * pPipeline);
void cleanup_frame_buffers(SDLExVulkanSwapChain * pSwapChain, SDLExVulkanGraphicsPipeline * pPipeline);

VkCommandPool create_command_buffer(SDLExVulkanSwapChain * pSwapChain);
void cleanup_command_buffer(SDLExVulkanSwapChain * pSwapChain);
#endif
