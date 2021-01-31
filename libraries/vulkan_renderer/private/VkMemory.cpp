#include "VkMemory.hpp"

#include <stdexcept>

uint32_t
findMemoryType(VkPhysicalDevice physical_device,
               uint32_t type_filter,
               VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_prop;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_prop);

    for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) {
        if (type_filter & (1 << i) && ((mem_prop.memoryTypes[i].propertyFlags &
                                        properties) == properties)) {
            return i;
        }
    }

    throw std::runtime_error("VkMemory: Failed to get memory type");
}

void
createBuffer(VkDevice device,
             VkBuffer &buffer,
             VkDeviceSize size,
             VkBufferUsageFlags usage)
{
    VkBufferCreateInfo buffer_info{};

    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("VkMemory: Failed to create buffer");
    }
}

void
allocateBuffer(VkPhysicalDevice physical_device,
               VkDevice device,
               VkBuffer &buffer,
               VkDeviceMemory &buffer_memory,
               VkMemoryPropertyFlags properties)
{
    VkMemoryRequirements mem_requirement;
    vkGetBufferMemoryRequirements(device, buffer, &mem_requirement);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirement.size;
    alloc_info.memoryTypeIndex = findMemoryType(
      physical_device, mem_requirement.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory) !=
        VK_SUCCESS) {
        throw std::runtime_error("VkMemory: Failed to allocate memory");
    }
    vkBindBufferMemory(device, buffer, buffer_memory, 0);
}

void
copyBuffer(VkDevice device,
           VkCommandPool command_pool,
           VkQueue gfx_queue,
           VkBuffer dst_buffer,
           VkBuffer src_buffer,
           VkDeviceSize size)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    vkAllocateCommandBuffers(device, &alloc_info, &cmd_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd_buffer, &begin_info);

    VkBufferCopy copy_region{};
    copy_region.size = size;
    copy_region.dstOffset = 0;
    copy_region.srcOffset = 0;
    vkCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, &copy_region);

    vkEndCommandBuffer(cmd_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;

    vkQueueSubmit(gfx_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(gfx_queue);
    vkFreeCommandBuffers(device, command_pool, 1, &cmd_buffer);
}
