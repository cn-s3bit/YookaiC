#include <stdlib.h>
#include "SDLExVulkan.h"

VkInstance VulkanInstance;
VkSurfaceKHR VulkanSurface;
VkPhysicalDevice VulkanPhysicalDevice;
VkDevice VulkanVirualDevice;
VkQueue VulkanGraphicsQueue;
float QueuePriority = 1.0f;

VkInstance get_vk_instance(void) {
	return VulkanInstance;
}

VkSurfaceKHR get_vk_surface(void) {
	return VulkanSurface;
}

VkDevice get_vk_device(void) {
	return VulkanVirualDevice;
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
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && find_queue_families(devices[i], VK_QUEUE_GRAPHICS_BIT) >= 0) {
			result = devices[i];
			SDL_Log("GPU Device: %s at %d", deviceProperties.deviceName, (unsigned)result);
			break;
		}
	}
	if (result == VK_NULL_HANDLE) {
		VkPhysicalDeviceProperties deviceProperties;
		for (unsigned i = 0; i < deviceCount; i++) {
			vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
			if (find_queue_families(devices[i], VK_QUEUE_GRAPHICS_BIT) >= 0) {
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
	createInfo.enabledExtensionCount = 0;
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

	free(extensions);

	_sdlex_vulkan_pick_physical_device();
	_sdlex_vulkan_create_virtual_device();

	if (!SDL_Vulkan_CreateSurface(window, VulkanInstance, &VulkanSurface))
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
			"Failed to get vulkan extensions for SDL: %s\n",
			SDL_GetError()
		);
	
	return VulkanInstance;
}

void cleanup_vulkan(void) {
	vkDestroyDevice(VulkanVirualDevice, NULL);
	vkDestroySurfaceKHR(VulkanInstance, VulkanSurface, NULL);
	vkDestroyInstance(VulkanInstance, NULL);
}
