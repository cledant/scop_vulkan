#ifndef SCOP_VULKAN_VULKANIMAGE_HPP
#define SCOP_VULKAN_VULKANIMAGE_HPP

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

VkDeviceSize loadTextureInBuffer(VkPhysicalDevice physical_device,
                                 VkDevice device,
                                 std::string const &filepath,
                                 VkBuffer &tex_buffer,
                                 VkDeviceMemory &tex_buffer_memory,
                                 int &tex_w,
                                 int &tex_h);
VkImage createImage(VkDevice device,
                    uint32_t width,
                    uint32_t height,
                    uint32_t mip_level,
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
                           uint32_t mip_level,
                           VkImageLayout old_layout,
                           VkImageLayout new_layout);
void copyBufferToImage(VkDevice device,
                       VkCommandPool command_pool,
                       VkQueue gfx_queue,
                       VkBuffer buffer,
                       VkImage image,
                       uint32_t width,
                       uint32_t height);
VkImageView createImageView(VkImage image,
                            VkFormat format,
                            uint32_t mip_level,
                            VkDevice device,
                            VkImageAspectFlags aspect_flags);
VkFormat findSupportedFormat(VkPhysicalDevice physical_device,
                             std::vector<VkFormat> const &candidates,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features);
bool hasStencilComponent(VkFormat format);
void generateMipmaps(VkPhysicalDevice physical_device,
                     VkDevice device,
                     VkCommandPool command_pool,
                     VkQueue gfx_queue,
                     VkImage image,
                     VkFormat image_format,
                     int32_t tex_width,
                     int32_t tex_height,
                     uint32_t mip_levels);

#endif // SCOP_VULKAN_VULKANIMAGE_HPP
