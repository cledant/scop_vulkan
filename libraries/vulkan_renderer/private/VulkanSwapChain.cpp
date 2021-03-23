#include <VulkanSwapChain.hpp>

#include <array>
#include <stdexcept>

#include "VulkanImage.hpp"
#include "VulkanSwapChainUtils.hpp"
#include "VulkanPhysicalDevice.hpp"

void
VulkanSwapChain::init(VulkanInstance const &vkInstance,
                       uint32_t fb_w,
                       uint32_t fb_h)
{
    _device = vkInstance.device;
    _physical_device = vkInstance.physicalDevice;
    _surface = vkInstance.surface;
    _create_swap_chain(fb_w, fb_h);
    _create_image_view();
}

void
VulkanSwapChain::resize(uint32_t fb_w, uint32_t fb_h)
{
    oldSwapChainNbImg = currentSwapChainNbImg;
    clear();
    _create_swap_chain(fb_w, fb_h);
    _create_image_view();
}

void
VulkanSwapChain::clear()
{
    for (auto iv : swapChainImageViews) {
        vkDestroyImageView(_device, iv, nullptr);
    }
    vkDestroySwapchainKHR(_device, swapChain, nullptr);
}

void
VulkanSwapChain::_create_swap_chain(uint32_t fb_w, uint32_t fb_h)
{
    // Creating swap chain
    VkExtent2D actual_extent = { fb_w, fb_h };

    auto scs = getSwapChainSupport(_physical_device, _surface, actual_extent);
    if (!scs.isValid()) {
        throw std::runtime_error("VulkanRenderPass: SwapChain error");
    }

    uint32_t nb_img = scs.capabilities.minImageCount + 1;
    if (scs.capabilities.maxImageCount > 0 &&
        nb_img > scs.capabilities.maxImageCount) {
        nb_img = scs.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = _surface;
    create_info.minImageCount = nb_img;
    create_info.imageFormat = scs.surface_format.value().format;
    create_info.imageColorSpace = scs.surface_format.value().colorSpace;
    create_info.imageExtent = scs.extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    DeviceRequirement dr{};
    getDeviceQueues(_physical_device, _surface, dr);
    uint32_t queue_family_indices[] = { dr.present_queue_index.value(),
                                        dr.graphic_queue_index.value() };
    if (dr.present_queue_index.value() != dr.graphic_queue_index.value()) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }
    create_info.preTransform = scs.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = scs.present_mode.value();
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = nullptr;
    if (vkCreateSwapchainKHR(_device, &create_info, nullptr, &swapChain) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderPass: Failed to create swap chain");
    }

    // Retrieving img buffer + keeping info
    uint32_t nb_img_sc;
    vkGetSwapchainImagesKHR(_device, swapChain, &nb_img_sc, nullptr);
    swapChainImages.resize(nb_img_sc);
    currentSwapChainNbImg = nb_img_sc;
    vkGetSwapchainImagesKHR(
      _device, swapChain, &nb_img_sc, swapChainImages.data());
    swapChainExtent = scs.extent;
    swapChainImageFormat = scs.surface_format.value().format;
}

void
VulkanSwapChain::_create_image_view()
{
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); ++i) {
        swapChainImageViews[i] = createImageView(swapChainImages[i],
                                                 swapChainImageFormat,
                                                 1,
                                                 _device,
                                                 VK_IMAGE_ASPECT_COLOR_BIT);
    }
}
