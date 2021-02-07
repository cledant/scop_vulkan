#include "VkImage.hpp"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "VkMemory.hpp"
#include "VkCommandBuffer.hpp"

VkDeviceSize
loadTextureInBuffer(VkPhysicalDevice physical_device,
                    VkDevice device,
                    std::string const &filepath,
                    VkBuffer &tex_buffer,
                    VkDeviceMemory &tex_buffer_memory,
                    int &tex_w,
                    int &tex_h)
{
    int img_chan;
    auto pixels =
      stbi_load(filepath.c_str(), &tex_w, &tex_h, &img_chan, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("VkImage: failed to load image: " + filepath);
    }
    VkDeviceSize img_size = tex_w * tex_h * 4;

    createBuffer(
      device, tex_buffer, img_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(physical_device,
                   device,
                   tex_buffer,
                   tex_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(device, tex_buffer_memory, 0, img_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(img_size));
    vkUnmapMemory(device, tex_buffer_memory);
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
    VkMemoryRequirements mem_req{};
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

void
transitionImageLayout(VkDevice device,
                      VkCommandPool command_pool,
                      VkQueue gfx_queue,
                      VkImage image,
                      VkFormat format,
                      VkImageLayout old_layout,
                      VkImageLayout new_layout)
{
    auto cmd_buffer = beginSingleTimeCommands(device, command_pool);
    (void)format;

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
               new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::runtime_error("VkImage: unsupported layout transition");
    }

    vkCmdPipelineBarrier(cmd_buffer,
                         source_stage,
                         destination_stage,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);
    endSingleTimeCommands(device, command_pool, cmd_buffer, gfx_queue);
}

void
copyBufferToImage(VkDevice device,
                  VkCommandPool command_pool,
                  VkQueue gfx_queue,
                  VkBuffer buffer,
                  VkImage image,
                  uint32_t width,
                  uint32_t height)
{
    auto cmd_buffer = beginSingleTimeCommands(device, command_pool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };
    vkCmdCopyBufferToImage(cmd_buffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);
    endSingleTimeCommands(device, command_pool, cmd_buffer, gfx_queue);
}

VkImageView
createImageView(VkDevice device,
                VkImage image,
                VkFormat format,
                VkImageAspectFlags aspect_flags)
{
    VkImageView img_view{};

    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = aspect_flags;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &create_info, nullptr, &img_view) !=
        VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: failed to create image view");
    }
    return (img_view);
}

VkFormat
findSupportedFormat(VkPhysicalDevice physical_device,
                    std::vector<VkFormat> const &candidates,
                    VkImageTiling tiling,
                    VkFormatFeatureFlags features)
{
    for (auto const &it : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, it, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return (it);
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return (it);
        }
    }
    throw std::runtime_error("VkImage: failed to find supported format");
}

bool
hasStencilComponent(VkFormat format)
{
    return (format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            format == VK_FORMAT_D24_UNORM_S8_UINT);
}