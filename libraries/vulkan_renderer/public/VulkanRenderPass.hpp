#ifndef SCOP_VULKAN_VULKANRENDERPASS_HPP
#define SCOP_VULKAN_VULKANRENDERPASS_HPP

#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanInstance.hpp"

class VulkanRenderPass final
{
  public:
    VulkanRenderPass() = default;
    ~VulkanRenderPass() = default;
    VulkanRenderPass(VulkanRenderPass const &src) = delete;
    VulkanRenderPass &operator=(VulkanRenderPass const &rhs) = delete;
    VulkanRenderPass(VulkanRenderPass &&src) = delete;
    VulkanRenderPass &operator=(VulkanRenderPass &&rhs) = delete;

    void init(VulkanInstance const &vkInstance,
                        uint32_t fb_w,
                        uint32_t fb_h);
    void resize(uint32_t fb_w, uint32_t fb_h);
    void clear();

    uint32_t oldSwapChainNbImg{};
    uint32_t currentSwapChainNbImg{};
    VkSwapchainKHR swapChain{};
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkFormat depthFormat{};
    VkImage depthImage{};
    VkDeviceMemory depthImgMemory{};
    VkImageView depthImgView{};
    VkRenderPass renderPass{};

  private:
    VkDevice _device{};
    VkPhysicalDevice _physical_device{};
    VkSurfaceKHR _surface{};
    VkCommandPool _command_pool{};
    VkQueue _gfx_queue{};

    inline void _create_swap_chain(uint32_t fb_w, uint32_t fb_h);
    inline void _create_image_view();
    inline void _create_render_pass();
    inline void _create_depth_resources();
    inline void _create_framebuffers();
};

#endif // SCOP_VULKAN_VULKANRENDERPASS_HPP
