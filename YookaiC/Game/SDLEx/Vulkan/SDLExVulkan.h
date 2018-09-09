#ifndef SDLEX_VULKAN_H
#define SDLEX_VULKAN_H
#include <vulkan/vulkan.h>
#include "../SDLWithPlugins.h"

VkInstance get_vk_instance(void);
VkSurfaceKHR get_vk_surface(void);
VkInstance initialize_vulkan(SDL_Window * window, unsigned appVer);
void cleanup_vulkan(void);
int find_queue_families(VkPhysicalDevice device, int required_flag_bit);
VkDevice get_vk_device(void);
#endif
