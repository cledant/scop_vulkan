#include "VulkanMemory.hpp"

#include <stdexcept>
#include <cstring>

#include "VulkanCommandBuffer.hpp"

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
copyBufferOnGpu(VkDevice device,
                VkCommandPool command_pool,
                VkQueue gfx_queue,
                VkBuffer dst_buffer,
                VkBuffer src_buffer,
                VkDeviceSize size)
{
    VkCommandBuffer cmd_buffer = beginSingleTimeCommands(device, command_pool);

    VkBufferCopy copy_region{};
    copy_region.size = size;
    copy_region.dstOffset = 0;
    copy_region.srcOffset = 0;
    vkCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, &copy_region);

    endSingleTimeCommands(device, command_pool, cmd_buffer, gfx_queue);
}

void
copyBufferOnGpu(VkDevice device,
                VkCommandPool command_pool,
                VkQueue gfx_queue,
                VkBuffer dst_buffer,
                VkBuffer src_buffer,
                VkBufferCopy copy_region)
{
    VkCommandBuffer cmd_buffer = beginSingleTimeCommands(device, command_pool);
    vkCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, &copy_region);
    endSingleTimeCommands(device, command_pool, cmd_buffer, gfx_queue);
}

void
copyOnCpuCoherentMemory(VkDevice device,
                        VkDeviceMemory memory,
                        VkDeviceSize offset,
                        VkDeviceSize size,
                        void const *dataToCopy)
{
    void *mapped_data{};

    vkMapMemory(device, memory, offset, size, 0, &mapped_data);
    memcpy(mapped_data, dataToCopy, size);
    vkUnmapMemory(device, memory);
}

void
copyCpuBufferToGpu(VkDevice device,
                   VkPhysicalDevice physicalDevice,
                   VkCommandPool commandPool,
                   VkQueue queue,
                   VkBuffer dstBuffer,
                   void *srcData,
                   VkBufferCopy copyRegion)
{
    // Staging buffer on CPU
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    createBuffer(device,
                 staging_buffer,
                 copyRegion.size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(physicalDevice,
                   device,
                   staging_buffer,
                   staging_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Copy on staging buffer
    copyOnCpuCoherentMemory(
      device, staging_buffer_memory, 0, copyRegion.size, srcData);

    // Copy on GPU
    copyBufferOnGpu(
      device, commandPool, queue, dstBuffer, staging_buffer, copyRegion);

    // Cleaning
    vkDestroyBuffer(device, staging_buffer, nullptr);
    vkFreeMemory(device, staging_buffer_memory, nullptr);
}
