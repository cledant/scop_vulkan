#include "VkImage.hpp"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "VkMemory.hpp"

VkDeviceSize
loadImage(VkPhysicalDevice physical_device,
          VkDevice device,
          std::string const &filepath,
          VkBuffer &image_buffer,
          VkDeviceMemory &image_buffer_memory,
          int &img_w,
          int &img_h)
{
    int img_chan;
    auto pixels =
      stbi_load(filepath.c_str(), &img_w, &img_h, &img_chan, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("VkImage: failed to load image: " + filepath);
    }
    VkDeviceSize img_size = img_w * img_h * 4;

    createBuffer(
      device, image_buffer, img_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(physical_device,
                   device,
                   image_buffer,
                   image_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(device, image_buffer_memory, 0, img_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(img_size));
    vkUnmapMemory(device, image_buffer_memory);
    stbi_image_free(pixels);
    return (img_size);
}

void
createImage(VkDevice device,
            VkImage &image,
            uint32_t width,
            uint32_t height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("VkImage: failed to create image");
    }
}

void
allocateImage(VkPhysicalDevice physical_device,
              VkDevice device,
              VkImage &image,
              VkDeviceMemory &image_memory,
              VkMemoryPropertyFlags properties)
{
    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(device, image, &mem_req);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex =
      findMemoryType(physical_device, mem_req.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &alloc_info, nullptr, &image_memory) !=
        VK_SUCCESS) {
        throw std::runtime_error("VkImage: failed to allocate image memory");
    }
    vkBindImageMemory(device, image, image_memory, 0);
}
