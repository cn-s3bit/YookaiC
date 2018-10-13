#include <stdlib.h>
#include "SDLExVulkan.h"

VkInstance VulkanInstance;
VkSurfaceKHR VulkanSurface;
VkPhysicalDevice VulkanPhysicalDevice;
VkDevice VulkanVirualDevice;
VkQueue VulkanGraphicsQueue;
float QueuePriority = 1.0f;

SDLExVulkanSwapChain VulkanSwapChain;


VkInstance get_vk_instance(void) {
	return VulkanInstance;
}

VkSurfaceKHR get_vk_surface(void) {
	return VulkanSurface;
}

VkPhysicalDevice get_vk_physical_device(void) {
	return VulkanPhysicalDevice;
}

VkDevice get_vk_device(void) {
	return VulkanVirualDevice;
}

VkQueue get_vk_queue(void) {
	return VulkanGraphicsQueue;
}

SDLExVulkanSwapChain * get_vk_swap_chain(void) {
	return &VulkanSwapChain;
}

int find_queue_families(VkPhysicalDevice device, int required_flag_bit) {
	unsigned queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties * queueFamilies = (VkQueueFamilyProperties *) malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for (unsigned i = 0; i < queueFamilyCount; i++) {
		if (queueFamilies[i].queueCount > 0 && (queueFamilies[i].queueFlags & required_flag_bit)) {
			free(queueFamilies);
			return i;
		}
	}

	free(queueFamilies);
	return -1;
}

static SwapChainSupportDetails _sdlex_scs_details;
static void _sdlex_fetch_scs_details(VkPhysicalDevice device);

static SDL_bool _sdlex_vulkan_check_device_extension_support(VkPhysicalDevice device) {
	unsigned extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

	VkExtensionProperties * availableExtensions = (VkExtensionProperties *)malloc(extensionCount * sizeof(VkExtensionProperties));
	
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);
	int got = 0;

	for (unsigned i = 0; i < extensionCount; i++) {
		if (SDL_strcmp(availableExtensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			++got;
	}

	free(availableExtensions);

	_sdlex_fetch_scs_details(device);

	if (_sdlex_scs_details.formats == NULL || _sdlex_scs_details.presentModes == NULL)
		return SDL_FALSE;

	VkBool32 sup;
	vkGetPhysicalDeviceSurfaceSupportKHR(device,
		find_queue_families(device, VK_QUEUE_GRAPHICS_BIT),
		VulkanSurface, &sup);

	if (!sup)
		return SDL_FALSE;

	return got == 1 ? SDL_TRUE : SDL_FALSE;
}

static void _sdlex_vulkan_pick_physical_device(void) {
	unsigned deviceCount = 0;
	vkEnumeratePhysicalDevices(VulkanInstance, &deviceCount, NULL);
	if (deviceCount == 0) {
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to find any GPU that supports Vulkan!\n"
		);
		return;
	}

	VkPhysicalDevice * devices = (VkPhysicalDevice *) malloc(deviceCount * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(VulkanInstance, &deviceCount, devices);
	VkPhysicalDevice result = VK_NULL_HANDLE;
	for (unsigned i = 0; i < deviceCount; i++) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		// VkPhysicalDeviceFeatures deviceFeatures;
		// vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
			&& _sdlex_vulkan_check_device_extension_support(devices[i])
			&& find_queue_families(devices[i], VK_QUEUE_GRAPHICS_BIT) >= 0) {
			result = devices[i];
			SDL_Log("GPU Device: %s at %d", deviceProperties.deviceName, (unsigned)result);
			break;
		}
	}
	if (result == VK_NULL_HANDLE) {
		VkPhysicalDeviceProperties deviceProperties;
		for (unsigned i = 0; i < deviceCount; i++) {
			vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
			if (find_queue_families(devices[i], VK_QUEUE_GRAPHICS_BIT) >= 0
				&& _sdlex_vulkan_check_device_extension_support(devices[i])) {
				result = devices[i];
				SDL_Log("No Discrete GPU Supported, Using Device: %s at %d",
					deviceProperties.deviceName, (unsigned)result
				);
				break;
			}
		}
	}
	if (result == VK_NULL_HANDLE) {
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"No Device Supported!\n"
		);
		free(devices);
		return;
	}
	VulkanPhysicalDevice = result;
	free(devices);
}

static void _sdlex_vulkan_create_virtual_device(void) {
	VkDeviceQueueCreateInfo queueCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfo.queueFamilyIndex = find_queue_families(VulkanPhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &QueuePriority;

	VkDeviceCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.enabledExtensionCount = 1;
	char * extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledLayerCount = 0;

	int ret = vkCreateDevice(VulkanPhysicalDevice, &createInfo, NULL, &VulkanVirualDevice);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to create virtual device: vkCreateDevice returns %d\n",
			ret
		);
		return;
	}
	vkGetDeviceQueue(VulkanVirualDevice, queueCreateInfo.queueFamilyIndex, 0, &VulkanGraphicsQueue);
	SDL_Log("Vulkan Virtual Device Created: %d with queue at %d", (unsigned)VulkanVirualDevice, (unsigned)VulkanGraphicsQueue);

}

static void _sdlex_fetch_scs_details(VkPhysicalDevice device) {
	free(_sdlex_scs_details.formats);
	_sdlex_scs_details.formats = NULL;
	free(_sdlex_scs_details.presentModes);
	_sdlex_scs_details.presentModes = NULL;

	unsigned formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanSurface, &formatCount, NULL);
	_sdlex_scs_details.formatCount = formatCount;
	if (formatCount != 0) {
		_sdlex_scs_details.formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanSurface, &formatCount, _sdlex_scs_details.formats);
	}

	unsigned presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanSurface, &presentModeCount, NULL);
	_sdlex_scs_details.presentModeCount = presentModeCount;
	if (presentModeCount > 0) {
		_sdlex_scs_details.presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanSurface, &presentModeCount, _sdlex_scs_details.presentModes);
	}
}

static void _sdlex_vulkan_create_swap_chain(SDL_Window * window) {
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanPhysicalDevice, VulkanSurface, &capabilities);

	_sdlex_fetch_scs_details(VulkanPhysicalDevice);
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	for (unsigned i = 0; i < _sdlex_scs_details.presentModeCount; i++) {
		if (_sdlex_scs_details.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			SDL_Log("The Device Supports VK_PRESENT_MODE_MAILBOX_KHR.");
			break;
		} else if (_sdlex_scs_details.presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}
	if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
		presentMode = _sdlex_scs_details.presentModes[0];

	VkSurfaceFormatKHR surfaceFormat = {
		.format = VK_FORMAT_UNDEFINED,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	};
	if (!(_sdlex_scs_details.formatCount == 1 && _sdlex_scs_details.formats[0].format == VK_FORMAT_UNDEFINED)) {
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
	for (unsigned i = 0; i < _sdlex_scs_details.formatCount; i++) {
		VkSurfaceFormatKHR availableFormat = _sdlex_scs_details.formats[i];
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surfaceFormat = availableFormat;
		}
	}
	if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
		surfaceFormat = _sdlex_scs_details.formats[0];

	VkExtent2D extent;
	if (capabilities.currentExtent.width != SDL_MAX_UINT32) {
		extent = capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent;
		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		actualExtent.width = (unsigned)w;
		actualExtent.height = (unsigned)h;
		actualExtent.width = SDL_max(capabilities.minImageExtent.width, SDL_min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = SDL_max(capabilities.minImageExtent.height, SDL_min(capabilities.maxImageExtent.height, actualExtent.height));

		extent = actualExtent;
	}

	unsigned imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = { .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = VulkanSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	int ret = vkCreateSwapchainKHR(VulkanVirualDevice, &createInfo, NULL, &VulkanSwapChain.SwapChain);
	if (ret != VK_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to create swap chain: vkCreateSwapchainKHR returns %d\n",
			ret
		);
		return;
	}
	VulkanSwapChain.SwapChainInfo = createInfo;
	vkGetSwapchainImagesKHR(VulkanVirualDevice, VulkanSwapChain.SwapChain, &VulkanSwapChain.ImageCount, NULL);
	VulkanSwapChain.Images = (VkImage *) malloc(VulkanSwapChain.ImageCount * sizeof(VkImage));
	vkGetSwapchainImagesKHR(VulkanVirualDevice, VulkanSwapChain.SwapChain, &VulkanSwapChain.ImageCount, VulkanSwapChain.Images);
	VulkanSwapChain.ImageViews = (VkImageView *) malloc(VulkanSwapChain.ImageCount * sizeof(VkImageView));
	for (unsigned i = 0; i < VulkanSwapChain.ImageCount; i++) {
		VkImageViewCreateInfo createInfo2 = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo2.image = VulkanSwapChain.Images[i];
		createInfo2.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo2.format = surfaceFormat.format;
		createInfo2.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo2.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo2.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo2.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo2.subresourceRange.baseMipLevel = 0;
		createInfo2.subresourceRange.levelCount = 1;
		createInfo2.subresourceRange.baseArrayLayer = 0;
		createInfo2.subresourceRange.layerCount = 1;
		ret = vkCreateImageView(VulkanVirualDevice, &createInfo2, NULL, &VulkanSwapChain.ImageViews[i]);
		if (ret != VK_SUCCESS) {
			SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
				"Failed to create image view: vkCreateImageView returns %d\n",
				ret
			);
			return;
		}
	}
	SDL_Log("Created Swap Chain at %d", (unsigned)VulkanSwapChain.SwapChain);
}

VkInstance initialize_vulkan(SDL_Window * window, unsigned appVer) {
	unsigned count;
	if (!SDL_Vulkan_GetInstanceExtensions(window, &count, NULL))
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to get vulkan extension count for SDL: %s\n",
			SDL_GetError()
		);

	char ** extensions = (char **) malloc(count * sizeof(char *));

	if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions))
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to get vulkan extensions for SDL: %s\n",
			SDL_GetError()
		);

	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_0,
		.engineVersion = VK_MAKE_VERSION(0, 1, 0)
	};
	appInfo.applicationVersion = appVer;
	appInfo.pApplicationName = SDL_GetWindowTitle(window);
	appInfo.pEngineName = "SDLExVulkan";

	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.flags = 0
	};
	createInfo.enabledExtensionCount = count;
	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.pApplicationInfo = &appInfo;

#ifdef SDLEX_VK_VALIDATION
	const char* validationLayers[1];
	validationLayers[0] = "VK_LAYER_LUNARG_standard_validation";
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = validationLayers;
#endif

	int createResult = vkCreateInstance(&createInfo, NULL, &VulkanInstance);
	if (createResult != VK_SUCCESS)
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to initialize vulkan: vkCreateInstance returns %d\n",
			createResult
		);

	free(extensions);
	
	if (!SDL_Vulkan_CreateSurface(window, VulkanInstance, &VulkanSurface))
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to get vulkan extensions for SDL: %s\n",
			SDL_GetError()
		);

	_sdlex_vulkan_pick_physical_device();
	_sdlex_vulkan_create_virtual_device();

	_sdlex_vulkan_create_swap_chain(window);
	create_command_buffer(&VulkanSwapChain);
	
	return VulkanInstance;
}

void cleanup_vulkan(void) {
	cleanup_command_buffer(&VulkanSwapChain);
	for (unsigned i = 0; i < VulkanSwapChain.ImageCount; i++) {
		vkDestroyImageView(VulkanVirualDevice, VulkanSwapChain.ImageViews[i], NULL);
	}
	free(VulkanSwapChain.Images);
	free(VulkanSwapChain.ImageViews);
	vkDestroySwapchainKHR(VulkanVirualDevice, VulkanSwapChain.SwapChain, NULL);
	vkDestroyDevice(VulkanVirualDevice, NULL);
	vkDestroySurfaceKHR(VulkanInstance, VulkanSurface, NULL);
	vkDestroyInstance(VulkanInstance, NULL);
}
