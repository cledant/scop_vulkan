#include "VulkanSync.hpp"

#include <stdexcept>

void
VulkanSync::init(VulkanInstance const &vkInstance, uint32_t nbFramebufferImgs)
{
    _device = vkInstance.device;
    imageAvailableSem.resize(MAX_FRAME_INFLIGHT);
    modelRenderFinishedSem.resize(MAX_FRAME_INFLIGHT);
    uiRenderFinishedSem.resize(MAX_FRAME_INFLIGHT);
    inflightFence.resize(MAX_FRAME_INFLIGHT);
    imgsInflightFence.resize(nbFramebufferImgs, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo sem_info{};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAME_INFLIGHT; ++i) {
        if (vkCreateSemaphore(
              _device, &sem_info, nullptr, &imageAvailableSem[i]) !=
              VK_SUCCESS ||
            vkCreateSemaphore(
              _device, &sem_info, nullptr, &modelRenderFinishedSem[i]) !=
              VK_SUCCESS ||
            vkCreateSemaphore(
              _device, &sem_info, nullptr, &uiRenderFinishedSem[i]) !=
              VK_SUCCESS ||
            vkCreateFence(_device, &fence_info, nullptr, &inflightFence[i]) !=
              VK_SUCCESS) {
            throw std::runtime_error("VulkanSync: failed to create semaphores");
        }
    }
}

void
VulkanSync::resize(uint32_t nbFramebufferImgs)
{
    imgsInflightFence.resize(nbFramebufferImgs, VK_NULL_HANDLE);
}

void
VulkanSync::clear()
{
    for (size_t i = 0; i < MAX_FRAME_INFLIGHT; ++i) {
        vkDestroySemaphore(_device, imageAvailableSem[i], nullptr);
        vkDestroySemaphore(_device, modelRenderFinishedSem[i], nullptr);
        vkDestroySemaphore(_device, uiRenderFinishedSem[i], nullptr);
        vkDestroyFence(_device, inflightFence[i], nullptr);
    }
}