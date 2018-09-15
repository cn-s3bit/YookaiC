#ifndef SDLEX_VULKAN_H
#define SDLEX_VULKAN_H
#include <vulkan/vulkan.h>
#include "../SDLWithPlugins.h"
struct SDLExVulkanSwapChain {
	unsigned ImageCount;
	VkImage * Images;
	VkImageView * ImageViews;
	VkSwapchainKHR SwapChain;
	VkSwapchainCreateInfoKHR SwapChainInfo;
};

VkInstance get_vk_instance(void);
VkSurfaceKHR get_vk_surface(void);
VkInstance initialize_vulkan(SDL_Window * window, unsigned appVer);
void cleanup_vulkan(void);
int find_queue_families(VkPhysicalDevice device, int required_flag_bit);
VkDevice get_vk_device(void);
VkQueue get_vk_queue(void);
VkPhysicalDevice get_vk_physical_device(void);
struct SDLExVulkanSwapChain get_vk_swap_chain(void);
VkShaderModule create_shader_module(char * code, size_t codeSize);
void create_graphics_pipeline_f(const char * vertShaderFilename, const char * fragShaderFilename);
void create_graphics_pipeline(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule);
#endif
