#ifndef SDLEX_VULKAN_H
#define SDLEX_VULKAN_H
#include <vulkan/vulkan.h>
#include "../SDLWithPlugins.h"

VkInstance get_vk_instance(void);
VkInstance initialize_vulkan(SDL_Window * window, unsigned int appVer);
#endif