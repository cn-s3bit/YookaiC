#ifndef SDLEX_VULKAN_H
#define SDLEX_VULKAN_H
#include <vulkan/vulkan.h>
#include "../SDLWithPlugins.h"
#include "../MathEx/MathEx.h"
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

typedef struct Vertex {
	Vector2 Pos;
	Vector3 Color;
} Vertex;

typedef struct SwapChainSupportDetails {
	unsigned formatCount, presentModeCount;
	VkSurfaceFormatKHR * formats;
	VkPresentModeKHR * presentModes;
} SwapChainSupportDetails;

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

void create_vertex_buffer(void);
void cleanup_vertex_buffer(void);
VkBuffer get_vk_vertex_buffer(void);
void * request_vertex_buffer_memory(void);
void flush_vertex_buffer_memory(void);
inline VkVertexInputBindingDescription _sdlex_get_binding_description(void) {
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

inline VkVertexInputAttributeDescription * _sdlex_get_attribute_descriptions(void) {
	VkVertexInputAttributeDescription * attributeDescriptions;
	attributeDescriptions = malloc(2 * sizeof(VkVertexInputAttributeDescription));
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, Pos);
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, Color);
	return attributeDescriptions;
}
#endif
