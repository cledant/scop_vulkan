#ifndef SCOP_VULKAN_VULKANUIRENDERPASS_HPP
#define SCOP_VULKAN_VULKANUIRENDERPASS_HPP

#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"

class VulkanUiRenderPass final
{
  public:
    VulkanUiRenderPass() = default;
    ~VulkanUiRenderPass() = default;
    VulkanUiRenderPass(VulkanUiRenderPass const &src) = delete;
    VulkanUiRenderPass &operator=(VulkanUiRenderPass const &rhs) = delete;
    VulkanUiRenderPass(VulkanUiRenderPass &&src) = delete;
    VulkanUiRenderPass &operator=(VulkanUiRenderPass &&rhs) = delete;

    void init(VulkanInstance const &vkInstance,
              VulkanSwapChain const &swapChain);
    void resize(VulkanSwapChain const &swapChain);
    void clear();

    std::vector<VkFramebuffer> framebuffers;
    VkRenderPass renderPass{};
    VkRenderPass noModelRenderPass{};

  private:
    VkDevice _device{};

    inline void _create_render_pass(VulkanSwapChain const &swapChain);
    inline void _create_no_model_render_pass(VulkanSwapChain const &swapChain);
    inline void _create_framebuffers(VulkanSwapChain const &swapChain);
};

#endif // SCOP_VULKAN_VULKANMODELRENDERPASS_HPP
