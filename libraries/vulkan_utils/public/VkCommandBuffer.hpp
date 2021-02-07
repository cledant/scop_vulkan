#ifndef SCOP_VULKAN_VKCOMMANDBUFFER_HPP
#define SCOP_VULKAN_VKCOMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

VkCommandBuffer beginSingleTimeCommands(VkDevice device,
                                        VkCommandPool command_pool);
void endSingleTimeCommands(VkDevice device,
                           VkCommandPool command_pool,
                           VkCommandBuffer command_buffer,
                           VkQueue gfx_queue);

#endif // SCOP_VULKAN_VKCOMMANDBUFFER_HPP
