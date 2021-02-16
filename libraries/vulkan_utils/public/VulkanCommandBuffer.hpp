#ifndef SCOP_VULKAN_VULKANCOMMANDBUFFER_HPP
#define SCOP_VULKAN_VULKANCOMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

VkCommandBuffer beginSingleTimeCommands(VkDevice device,
                                        VkCommandPool command_pool);
VkCommandBuffer beginNTimesCommands(VkDevice device,
                                    VkCommandPool command_pool,
                                    uint32_t n);
void endSingleTimeCommands(VkDevice device,
                           VkCommandPool command_pool,
                           VkCommandBuffer command_buffer,
                           VkQueue gfx_queue);
void endNTimesCommands(VkDevice device,
                       VkCommandPool command_pool,
                       VkCommandBuffer command_buffer,
                       VkQueue gfx_queue,
                       uint32_t n);

#endif // SCOP_VULKAN_VULKANCOMMANDBUFFER_HPP
