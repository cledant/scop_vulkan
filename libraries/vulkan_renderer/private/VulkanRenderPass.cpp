#include <VulkanRenderPass.hpp>

#include <array>
#include <stdexcept>

#include "VulkanImage.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanPhysicalDevice.hpp"

void
VulkanRenderPass::init(VulkanInstance const &vkInstance,
                       uint32_t fb_w,
                       uint32_t fb_h)
{
    _device = vkInstance.device;
    _physical_device = vkInstance.physicalDevice;
    _surface = vkInstance.surface;
    _command_pool = vkInstance.commandPool;
    _gfx_queue = vkInstance.graphicQueue;
    _create_swap_chain(fb_w, fb_h);
    _create_image_view();
    _create_render_pass();
    _create_depth_resources();
    _create_framebuffers();
}

void
VulkanRenderPass::resize(uint32_t fb_w, uint32_t fb_h)
{
    oldSwapChainNbImg = currentSwapChainNbImg;
    clear();
    _create_swap_chain(fb_w, fb_h);
    _create_image_view();
    _create_render_pass();
    _create_depth_resources();
    _create_framebuffers();
}

void
VulkanRenderPass::clear()
{
    size_t i = 0;
    vkDestroyImageView(_device, depthImgView, nullptr);
    vkDestroyImage(_device, depthImage, nullptr);
    vkFreeMemory(_device, depthImgMemory, nullptr);
    for (auto &it : swapChainFramebuffers) {
        vkDestroyFramebuffer(_device, it, nullptr);
        ++i;
    }
    vkDestroyRenderPass(_device, renderPass, nullptr);
    for (auto iv : swapChainImageViews) {
        vkDestroyImageView(_device, iv, nullptr);
    }
    vkDestroySwapchainKHR(_device, swapChain, nullptr);
}

void
VulkanRenderPass::_create_swap_chain(uint32_t fb_w, uint32_t fb_h)
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
VulkanRenderPass::_create_image_view()
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

void
VulkanRenderPass::_create_render_pass()
{
    // Color
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapChainImageFormat;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth
    VkAttachmentDescription depth_attachment{};
    depthFormat =
      findSupportedFormat(_physical_device,
                          { VK_FORMAT_D32_SFLOAT,
                            VK_FORMAT_D32_SFLOAT_S8_UINT,
                            VK_FORMAT_D24_UNORM_S8_UINT },
                          VK_IMAGE_TILING_OPTIMAL,
                          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depth_attachment.format = depthFormat;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency sub_dep{};
    sub_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    sub_dep.dstSubpass = 0;
    sub_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                           VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    sub_dep.srcAccessMask = 0;
    sub_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                           VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    sub_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { color_attachment,
                                                           depth_attachment };
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &sub_dep;

    if (vkCreateRenderPass(_device, &render_pass_info, nullptr, &renderPass) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderPass: failed to create render pass");
    }
}

void
VulkanRenderPass::_create_depth_resources()
{
    depthImage = createImage(_device,
                             swapChainExtent.width,
                             swapChainExtent.height,
                             1,
                             depthFormat,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    allocateImage(_physical_device,
                  _device,
                  depthImage,
                  depthImgMemory,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    depthImgView = createImageView(
      depthImage, depthFormat, 1, _device, VK_IMAGE_ASPECT_DEPTH_BIT);
    transitionImageLayout(_device,
                          _command_pool,
                          _gfx_queue,
                          depthImage,
                          depthFormat,
                          1,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void
VulkanRenderPass::_create_framebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    size_t i = 0;
    for (auto const &it : swapChainImageViews) {
        std::array<VkImageView, 2> sciv = { it, depthImgView };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = renderPass;
        framebuffer_info.attachmentCount = sciv.size();
        framebuffer_info.pAttachments = sciv.data();
        framebuffer_info.width = swapChainExtent.width;
        framebuffer_info.height = swapChainExtent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(
              _device, &framebuffer_info, nullptr, &swapChainFramebuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error(
              "VulkanRenderPass: Failed to create framebuffer");
        }
        ++i;
    }
}