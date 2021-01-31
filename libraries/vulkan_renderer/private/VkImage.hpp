#ifndef SCOP_VULKAN_VKIMAGE_HPP
#define SCOP_VULKAN_VKIMAGE_HPP

#include <string>
#include <vulkan/vulkan.h>

VkDeviceSize loadTextureInBuffer(VkPhysicalDevice physical_device,
                                 VkDevice device,
                                 std::string const &filepath,
                                 VkBuffer &tex_buffer,
                                 VkDeviceMemory &tex_buffer_memory,
                                 int &tex_w,
                                 int &tex_h);
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
void transitionImageLayout(VkDevice device,
                           VkCommandPool command_pool,
                           VkQueue gfx_queue,
                           VkImage image,
                           VkFormat format,
                           VkImageLayout old_layout,
                           VkImageLayout new_layout);
void copyBufferToImage(VkDevice device,
                       VkCommandPool command_pool,
                       VkQueue gfx_queue,
                       VkBuffer buffer,
                       VkImage image,
                       uint32_t width,
                       uint32_t height);

#endif // SCOP_VULKAN_VKIMAGE_HPP
