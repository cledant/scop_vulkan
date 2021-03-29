#include <VulkanUiRenderPass.hpp>

#include <array>
#include <stdexcept>

#include "VulkanSwapChainUtils.hpp"

void
VulkanUiRenderPass::init(VulkanInstance const &vkInstance,
                         VulkanSwapChain const &swapChain)
{
    _device = vkInstance.device;
    _create_render_pass(swapChain);
    _create_framebuffers(swapChain);
    _create_no_model_render_pass(swapChain);
}

void
VulkanUiRenderPass::resize(VulkanSwapChain const &swapChain)
{
    size_t i = 0;
    for (auto &it : framebuffers) {
        vkDestroyFramebuffer(_device, it, nullptr);
        ++i;
    }
    vkDestroyRenderPass(_device, renderPass, nullptr);
    vkDestroyRenderPass(_device, noModelRenderPass, nullptr);
    _create_render_pass(swapChain);
    _create_framebuffers(swapChain);
    _create_no_model_render_pass(swapChain);
}

void
VulkanUiRenderPass::clear()
{
    size_t i = 0;
    for (auto &it : framebuffers) {
        vkDestroyFramebuffer(_device, it, nullptr);
        ++i;
    }
    vkDestroyRenderPass(_device, renderPass, nullptr);
    vkDestroyRenderPass(_device, noModelRenderPass, nullptr);
    _device = nullptr;
}

void
VulkanUiRenderPass::_create_render_pass(VulkanSwapChain const &swapChain)
{
    // Color
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapChain.swapChainImageFormat;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = nullptr;

    VkSubpassDependency sub_dep{};
    sub_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    sub_dep.dstSubpass = 0;
    sub_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    sub_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    sub_dep.srcAccessMask = 0;
    sub_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = {
        color_attachment,
    };
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
          "VulkanUiRenderPass: failed to create render pass");
    }
}

void
VulkanUiRenderPass::_create_no_model_render_pass(
  VulkanSwapChain const &swapChain)
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
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = nullptr;

    VkSubpassDependency sub_dep{};
    sub_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    sub_dep.dstSubpass = 0;
    sub_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    sub_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    sub_dep.srcAccessMask = 0;
    sub_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = {
        color_attachment,
    };
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &sub_dep;

    if (vkCreateRenderPass(
          _device, &render_pass_info, nullptr, &noModelRenderPass) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanUiRenderPass: failed to create render pass");
    }
}

void
VulkanUiRenderPass::_create_framebuffers(VulkanSwapChain const &swapChain)
{
    framebuffers.resize(swapChain.swapChainImageViews.size());

    size_t i = 0;
    for (auto const &it : swapChain.swapChainImageViews) {
        std::array<VkImageView, 1> sciv = { it };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = renderPass;
        framebuffer_info.attachmentCount = sciv.size();
        framebuffer_info.pAttachments = sciv.data();
        framebuffer_info.width = swapChain.swapChainExtent.width;
        framebuffer_info.height = swapChain.swapChainExtent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(
              _device, &framebuffer_info, nullptr, &framebuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error(
              "VulkanUiRenderPass: Failed to create framebuffer");
        }
        ++i;
    }
}
