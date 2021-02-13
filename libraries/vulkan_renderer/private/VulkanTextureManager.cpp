#include "VulkanTextureManager.hpp"

#include <stdexcept>

#include "VulkanImage.hpp"

void
VulkanTextureManager::init(VulkanInstance const &vkInstance)
{
    _device = vkInstance.device;
    _physical_device = vkInstance.physicalDevice;
    _gfx_queue = vkInstance.graphicQueue;
    _command_pool = vkInstance.commandPool;
}

void
VulkanTextureManager::clear()
{
    for (auto &it : _textures) {
        vkDestroySampler(_device, it.second.texture_sampler, nullptr);
        vkDestroyImageView(_device, it.second.texture_img_view, nullptr);
        vkDestroyImage(_device, it.second.texture_img, nullptr);
        vkFreeMemory(_device, it.second.texture_img_memory, nullptr);
    }
    _textures.clear();
}

void
VulkanTextureManager::loadTexture(std::string const &texturePath)
{
    auto existing_tex = _textures.find(texturePath);
    if (existing_tex != _textures.end()) {
        return;
    }

    Texture tex{};

    _create_texture_image(texturePath, tex.texture_img, tex.texture_img_memory);
    tex.texture_img_view = _create_texture_image_view(tex.texture_img);
    tex.texture_sampler = _create_texture_sampler();
    _textures.emplace(texturePath, tex);
}

bool
VulkanTextureManager::getTexture(std::string const &texturePath, Texture &tex)
{
    auto existing_tex = _textures.find(texturePath);
    if (existing_tex != _textures.end()) {
        tex = existing_tex->second;
        return (true);
    }
    return (false);
}

Texture
VulkanTextureManager::loadAndGetTexture(std::string const &texturePath)
{
    auto existing_tex = _textures.find(texturePath);
    if (existing_tex != _textures.end()) {
        return (existing_tex->second);
    }

    Texture tex{};

    _create_texture_image(texturePath, tex.texture_img, tex.texture_img_memory);
    tex.texture_img_view = _create_texture_image_view(tex.texture_img);
    tex.texture_sampler = _create_texture_sampler();
    _textures.emplace(texturePath, tex);
    return (tex);
}

void
VulkanTextureManager::_create_texture_image(std::string const &texturePath,
                                            VkImage &texture_img,
                                            VkDeviceMemory &texture_img_memory)
{
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    int img_w = 0;
    int img_h = 0;

    loadTextureInBuffer(_physical_device,
                        _device,
                        texturePath,
                        staging_buffer,
                        staging_buffer_memory,
                        img_w,
                        img_h);
    createImage(_device,
                texture_img,
                img_w,
                img_h,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    allocateImage(_physical_device,
                  _device,
                  texture_img,
                  texture_img_memory,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    transitionImageLayout(_device,
                          _command_pool,
                          _gfx_queue,
                          texture_img,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(_device,
                      _command_pool,
                      _gfx_queue,
                      staging_buffer,
                      texture_img,
                      img_w,
                      img_h);
    transitionImageLayout(_device,
                          _command_pool,
                          _gfx_queue,
                          texture_img,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_buffer_memory, nullptr);
}

VkImageView
VulkanTextureManager::_create_texture_image_view(VkImage texture_img)
{
    return (createImageView(_device,
                            texture_img,
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_ASPECT_COLOR_BIT));
}

VkSampler
VulkanTextureManager::_create_texture_sampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_physical_device, &properties);
    float aniso = (properties.limits.maxSamplerAnisotropy > 16.0f)
                    ? 16.0f
                    : properties.limits.maxSamplerAnisotropy;

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = aniso;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    VkSampler texture_sampler;
    if (vkCreateSampler(_device, &sampler_info, nullptr, &texture_sampler) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VkTextureManager: failed to create texture sampler");
    }
    return (texture_sampler);
}