#ifndef SCOP_VULKAN_VULKANTEXTUREMANAGER_HPP
#define SCOP_VULKAN_VULKANTEXTUREMANAGER_HPP

#include <unordered_map>
#include <string>

#include <vulkan/vulkan.h>

#include "VulkanInstance.hpp"

struct Texture final
{
    VkImage texture_img{};
    VkDeviceMemory texture_img_memory{};
    VkImageView texture_img_view{};
    VkSampler texture_sampler{};
    int32_t width{};
    int32_t height{};
    uint32_t mip_level{};
};

class VulkanTextureManager final
{
  public:
    VulkanTextureManager() = default;
    ~VulkanTextureManager() = default;
    VulkanTextureManager(VulkanTextureManager const &src) = delete;
    VulkanTextureManager(VulkanTextureManager &&src) = delete;
    VulkanTextureManager &operator=(VulkanTextureManager const &rhs) = delete;
    VulkanTextureManager &operator=(VulkanTextureManager &&rhs) = delete;

    void init(VulkanInstance const &vkInstance);
    void clear();
    void loadTexture(std::string const &texturePath);
    bool getTexture(std::string const &texturePath, Texture &tex);
    Texture loadAndGetTexture(std::string const &texturePath);

  private:
    VkDevice _device{};
    VkPhysicalDevice _physical_device{};
    VkQueue _gfx_queue{};
    VkCommandPool _command_pool{};
    std::unordered_map<std::string, Texture> _textures;

    inline VkImage _create_texture_image(std::string const &texturePath,
                                         VkDeviceMemory &texture_img_memory,
                                         int32_t &tex_img_w,
                                         int32_t &tex_img_h,
                                         uint32_t &mip_level);
    inline VkImageView _create_texture_image_view(VkImage texture_img,
                                                  uint32_t mip_level);
    inline VkSampler _create_texture_sampler(uint32_t mip_level);
};

#endif // SCOP_VULKAN_VULKANTEXTUREMANAGER_HPP
