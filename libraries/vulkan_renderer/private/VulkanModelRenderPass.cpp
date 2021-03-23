#include <VulkanModelRenderPass.hpp>

#include <array>
#include <stdexcept>

#include "VulkanImage.hpp"
#include "VulkanSwapChainUtils.hpp"

void
VulkanModelRenderPass::init(VulkanInstance const &vkInstance,
                            VulkanSwapChain const &swapChain)
{
    _device = vkInstance.device;
    _physical_device = vkInstance.physicalDevice;
    _command_pool = vkInstance.modelCommandPool;
    _gfx_queue = vkInstance.graphicQueue;
    _create_render_pass(swapChain);
    _create_depth_resources(swapChain);
    _create_framebuffers(swapChain);
}

void
VulkanModelRenderPass::resize(VulkanSwapChain const &swapChain)
{
    clear();
    _create_render_pass(swapChain);
    _create_depth_resources(swapChain);
    _create_framebuffers(swapChain);
}

void
VulkanModelRenderPass::clear()
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
}

void
VulkanModelRenderPass::_create_render_pass(VulkanSwapChain const &swapChain)
{
    // Color
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapChain.swapChainImageFormat;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
          "VulkanModelRenderPass: failed to create render pass");
    }
}

void
VulkanModelRenderPass::_create_depth_resources(VulkanSwapChain const &swapChain)
{
    depthImage = createImage(_device,
                             swapChain.swapChainExtent.width,
                             swapChain.swapChainExtent.height,
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
VulkanModelRenderPass::_create_framebuffers(VulkanSwapChain const &swapChain)
{
    swapChainFramebuffers.resize(swapChain.swapChainImageViews.size());

    size_t i = 0;
    for (auto const &it : swapChain.swapChainImageViews) {
        std::array<VkImageView, 2> sciv = { it, depthImgView };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = renderPass;
        framebuffer_info.attachmentCount = sciv.size();
        framebuffer_info.pAttachments = sciv.data();
        framebuffer_info.width = swapChain.swapChainExtent.width;
        framebuffer_info.height = swapChain.swapChainExtent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(
              _device, &framebuffer_info, nullptr, &swapChainFramebuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error(
              "VulkanModelRenderPass: Failed to create framebuffer");
        }
        ++i;
    }
}