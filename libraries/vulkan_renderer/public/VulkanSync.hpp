#ifndef SCOP_VULKAN_VULKANSYNC_HPP
#define SCOP_VULKAN_VULKANSYNC_HPP

#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanInstance.hpp"
#include <VulkanSwapChain.hpp>

class VulkanSync
{
  public:
    VulkanSync() = default;
    ~VulkanSync() = default;
    VulkanSync(VulkanSync const &src) = delete;
    VulkanSync &operator=(VulkanSync const &rhs) = delete;
    VulkanSync(VulkanSync &&src) = delete;
    VulkanSync &operator=(VulkanSync &&rhs) = delete;

    void init(VulkanInstance const &vkInstance, uint32_t nbFramebufferImgs);
    void resize(uint32_t nbFramebufferImgs);
    void clear();

    static constexpr size_t const MAX_FRAME_INFLIGHT = 2;

    size_t currentFrame{};
    std::vector<VkSemaphore> imageAvailableSem;
    std::vector<VkSemaphore> renderFinishedSem;
    std::vector<VkFence> inflightFence;
    std::vector<VkFence> imgsInflightFence;

  private:
    VkDevice _device{};
};

#endif // SCOP_VULKAN_VULKANSYNC_HPP
