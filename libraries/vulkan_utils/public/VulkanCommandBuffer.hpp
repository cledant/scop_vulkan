#ifndef SCOP_VULKAN_VULKANCOMMANDBUFFER_HPP
#define SCOP_VULKAN_VULKANCOMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

VkCommandBuffer beginSingleTimeCommands(VkDevice device,
                                        VkCommandPool command_pool);
void endSingleTimeCommands(VkDevice device,
                           VkCommandPool command_pool,
                           VkCommandBuffer command_buffer,
                           VkQueue gfx_queue);
VkCommandPool createCommandPool(VkDevice device,
                                uint32_t queueIndex,
                                VkCommandPoolCreateFlags flags);

#endif // SCOP_VULKAN_VULKANCOMMANDBUFFER_HPP
