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
createBuffer(VkPhysicalDevice physical_device,
             VkDevice device,
             VkDeviceSize size,
             VkBufferUsageFlags usage,
             VkMemoryPropertyFlags properties,
             VkBuffer &buffer,
             VkDeviceMemory &buffer_memory)
{
    VkBufferCreateInfo buffer_info{};

    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("VkMemory: Failed to create buffer");
    }

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
