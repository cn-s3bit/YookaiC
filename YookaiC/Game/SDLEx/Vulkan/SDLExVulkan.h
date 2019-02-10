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
	VkDescriptorSet * DescriptorSets;
} SDLExVulkanGraphicsPipeline;

typedef struct Vertex {
	Vector2 Pos;
	Vector3 Color;
	Vector2 TexCoord;
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
VkCommandBuffer begin_single_time_commands();
void end_single_time_commands(VkCommandBuffer commandBuffer);

void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * out_buffer, VkDeviceMemory * out_memory);
void create_vertex_buffer(unsigned nVertices);
void recreate_vertex_buffer(unsigned nVertices);
void cleanup_vertex_buffer(void);
VkBuffer get_vk_vertex_buffer(void);
void * request_vertex_buffer_memory(void);
void flush_vertex_buffer_memory(void);

SDL_Rect texture_frame(SDL_Texture * texture);
VkImageCreateInfo create_image(unsigned width, unsigned height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
VkImageView create_image_view(VkImage image, VkFormat format);
void transition_image_layout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
void copy_buffer_to_image(VkBuffer buffer, VkImage image, unsigned width, unsigned height);
long load_texture2d(const char * filename);
void dispose_texture2d(long texture_id);
void bind_texture2d(unsigned imageIndex, long texture_id);

void create_descriptor_pool();
void create_descriptor_sets();
void cleanup_descriptor_pool();
void bind_texture(unsigned imageIndex, VkImageView textureImageView, VkSampler textureSampler, VkImageCreateInfo textureInfo);
VkImageCreateInfo * sdlex_get_current_texture_info(unsigned imageIndex);

unsigned sdlex_begin_frame();
void sdlex_render_init(SDLExVulkanSwapChain * swapchain, SDLExVulkanGraphicsPipeline * pipeline, int clear);
void sdlex_render_flush(unsigned imageIndex);
void sdlex_render_texture(unsigned imageIndex, SDL_Rect target);
void sdlex_render_texture_ex(unsigned imageIndex, Vector2 position, Vector2 origin, float rotation, Vector2 scale);
void sdlex_render_texture_region_ex(unsigned imageIndex, Vector2 position, Vector2 origin, float rotation, Vector2 scale, SDL_Rect sourceRegion);
void sdlex_end_frame(unsigned imageIndex);


inline VkVertexInputBindingDescription _sdlex_get_binding_description(void) {
	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

inline VkVertexInputAttributeDescription * _sdlex_get_attribute_descriptions(void) {
	VkVertexInputAttributeDescription * attributeDescriptions;
	attributeDescriptions = malloc(3 * sizeof(VkVertexInputAttributeDescription));
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, Pos);
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, Color);
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, TexCoord);
	return attributeDescriptions;
}
#endif
