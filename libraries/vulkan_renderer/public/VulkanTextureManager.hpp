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

    inline void _create_texture_image(std::string const &texturePath,
                                      VkImage &texture_img,
                                      VkDeviceMemory &texture_img_memory);
    inline VkImageView _create_texture_image_view(VkImage texture_img);
    inline VkSampler _create_texture_sampler();
};

#endif // SCOP_VULKAN_VULKANTEXTUREMANAGER_HPP
