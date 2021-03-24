#ifndef SCOP_VULKAN_VULKANMODELRENDERPASS_HPP
#define SCOP_VULKAN_VULKANMODELRENDERPASS_HPP

#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"

class VulkanModelRenderPass final
{
  public:
    VulkanModelRenderPass() = default;
    ~VulkanModelRenderPass() = default;
    VulkanModelRenderPass(VulkanModelRenderPass const &src) = delete;
    VulkanModelRenderPass &operator=(VulkanModelRenderPass const &rhs) = delete;
    VulkanModelRenderPass(VulkanModelRenderPass &&src) = delete;
    VulkanModelRenderPass &operator=(VulkanModelRenderPass &&rhs) = delete;

    void init(VulkanInstance const &vkInstance,
              VulkanSwapChain const &swapChain);
    void resize(VulkanSwapChain const &swapChain);
    void clear();

    std::vector<VkFramebuffer> framebuffers;
    VkFormat depthFormat{};
    VkImage depthImage{};
    VkDeviceMemory depthImgMemory{};
    VkImageView depthImgView{};
    VkRenderPass renderPass{};

  private:
    VkDevice _device{};
    VkPhysicalDevice _physical_device{};
    VkCommandPool _command_pool{};
    VkQueue _gfx_queue{};

    inline void _create_render_pass(VulkanSwapChain const &swapChain);
    inline void _create_depth_resources(VulkanSwapChain const &swapChain);
    inline void _create_framebuffers(VulkanSwapChain const &swapChain);
};

#endif // SCOP_VULKAN_VULKANMODELRENDERPASS_HPP
