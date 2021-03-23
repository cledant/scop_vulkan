#include "VulkanCommandBuffer.hpp"

#include <stdexcept>

VkCommandBuffer
beginSingleTimeCommands(VkDevice device, VkCommandPool command_pool)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return (command_buffer);
}

void
endSingleTimeCommands(VkDevice device,
                      VkCommandPool command_pool,
                      VkCommandBuffer command_buffer,
                      VkQueue gfx_queue)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(gfx_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(gfx_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

VkCommandPool
createCommandPool(VkDevice device,
                  uint32_t queueIndex,
                  VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo command_pool_info{};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = queueIndex;
    command_pool_info.flags = flags;

    VkCommandPool cmd_pool{};
    if (vkCreateCommandPool(device, &command_pool_info, nullptr, &cmd_pool) !=
        VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Failed to create command pool");
    }
    return (cmd_pool);
}
