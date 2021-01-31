#ifndef SCOP_VULKAN_VKIMAGE_HPP
#define SCOP_VULKAN_VKIMAGE_HPP

#include <vulkan/vulkan.h>

void createImage(VkDevice device,
                 VkImage &image,
                 uint32_t width,
                 uint32_t height,
                 VkFormat format,
                 VkImageTiling tiling,
                 VkImageUsageFlags usage);
void allocateImage(VkPhysicalDevice physical_device,
                   VkDevice device,
                   VkImage &image,
                   VkDeviceMemory &image_memory,
                   VkMemoryPropertyFlags properties);

#endif // SCOP_VULKAN_VKIMAGE_HPP
