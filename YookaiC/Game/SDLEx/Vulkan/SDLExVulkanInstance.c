#include <stdlib.h>
#include "SDLExVulkan.h"

VkInstance VulkanInstance;

VkInstance get_vk_instance(void) {
	return VulkanInstance;
}

VkInstance initialize_vulkan(SDL_Window * window, unsigned int appVer) {
	unsigned int count;
	if (!SDL_Vulkan_GetInstanceExtensions(window, &count, NULL))
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to get vulkan extension count for SDL: %s\n",
			SDL_GetError()
		);

	char ** extensions = (char **) malloc(count * sizeof(char *));

	SDL_Log("Extension Malloc: %d\n", (int)extensions);

	if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions))
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to get vulkan extensions for SDL: %s\n",
			SDL_GetError()
		);

	SDL_Log("Extension Get: %d\n", (int)extensions);
	SDL_Log("Extension Count: %d\n", count);

	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_1,
		.engineVersion = VK_MAKE_VERSION(1, 0, 0)
	};
	appInfo.applicationVersion = appVer;
	appInfo.pApplicationName = SDL_GetWindowTitle(window);
	appInfo.pEngineName = "SDLExVulkan";

	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
	};
	createInfo.enabledExtensionCount = count;
	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.pApplicationInfo = &appInfo;

	int createResult = vkCreateInstance(&createInfo, NULL, &VulkanInstance);
	if (createResult != VK_SUCCESS)
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to initialize vulkan: vkCreateInstance returns %d\n",
			createResult
		);
	
	return VulkanInstance;
}
