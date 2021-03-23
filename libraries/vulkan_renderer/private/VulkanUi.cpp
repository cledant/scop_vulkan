#include "VulkanUi.hpp"

#include <stdexcept>
#include <array>

#include "VulkanCommandBuffer.hpp"

void
VulkanUi::init(VulkanInstance const &vkInstance,
               VulkanSwapChain const &swapChain)
{
    _instance = vkInstance.instance;
    _physicalDevice = vkInstance.physicalDevice;
    _device = vkInstance.device;
    _graphicQueue = vkInstance.graphicQueue;
    _graphicQueueIndex = vkInstance.graphicQueueIndex;
    _render_pass.init(vkInstance, swapChain);
    _init_imgui(swapChain);
    _ui_command_pools.resize(swapChain.currentSwapChainNbImg);
    for (auto &it : _ui_command_pools) {
        it = createCommandPool(_device,
                               _graphicQueueIndex,
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }
    _create_ui_command_buffers(swapChain.currentSwapChainNbImg);
    _load_fonts();
}

void
VulkanUi::resize(VulkanSwapChain const &swapChain)
{
    vkDeviceWaitIdle(_device);
    ImGui_ImplVulkan_Shutdown();
    _render_pass.resize(swapChain);
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    _init_imgui(swapChain);
    for (auto &it : _ui_command_pools) {
        vkDestroyCommandPool(_device, it, nullptr);
    }
    _ui_command_pools.resize(swapChain.currentSwapChainNbImg);
    for (auto &it : _ui_command_pools) {
        it = createCommandPool(_device,
                               _graphicQueueIndex,
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }
    _create_ui_command_buffers(swapChain.currentSwapChainNbImg);
}

void
VulkanUi::clear()
{
    vkDeviceWaitIdle(_device);
    ImGui_ImplVulkan_Shutdown();
    _render_pass.clear();
    for (auto &it : _ui_command_pools) {
        vkDestroyCommandPool(_device, it, nullptr);
    }
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    _instance = nullptr;
    _physicalDevice = nullptr;
    _device = nullptr;
    _graphicQueue = nullptr;
    _graphicQueueIndex = UINT32_MAX;
}

VkCommandBuffer &
VulkanUi::generateCommandBuffer(uint32_t frameIndex, VkExtent2D swapChainExtent)
{
    if (vkResetCommandPool(_device, _ui_command_pools[frameIndex], 0) !=
        VK_SUCCESS) {
        throw std::runtime_error("VulkanUi: failed to reset command pool");
    }
    VkCommandBufferBeginInfo cb_begin_info{};
    cb_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cb_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cb_begin_info.pInheritanceInfo = nullptr;
    if (vkBeginCommandBuffer(_ui_command_buffers[frameIndex], &cb_begin_info) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanUi: Failed to begin recording ui command buffer");
    }

    std::array<VkClearValue, 1> clear_vals{};
    clear_vals[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    VkRenderPassBeginInfo rp_begin_info{};
    rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin_info.renderPass = _render_pass.renderPass;
    rp_begin_info.framebuffer = _render_pass.swapChainFramebuffers[frameIndex];
    rp_begin_info.renderArea.offset = { 0, 0 };
    rp_begin_info.renderArea.extent = swapChainExtent;
    rp_begin_info.clearValueCount = clear_vals.size();
    rp_begin_info.pClearValues = clear_vals.data();
    vkCmdBeginRenderPass(_ui_command_buffers[frameIndex],
                         &rp_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                    _ui_command_buffers[frameIndex]);
    vkCmdEndRenderPass(_ui_command_buffers[frameIndex]);
    if (vkEndCommandBuffer(_ui_command_buffers[frameIndex]) != VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderer: Failed to record model command Buffer");
    }
    return (_ui_command_buffers[frameIndex]);
}

void
VulkanUi::_init_imgui(VulkanSwapChain const &swapChain)
{
    ImGui_ImplVulkan_InitInfo init_info = {};
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(
          _device, &pool_info, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("VulkanUi: failed to create descriptor pool");
    }
    init_info.DescriptorPool = _descriptorPool;
    init_info.Instance = _instance;
    init_info.PhysicalDevice = _physicalDevice;
    init_info.Device = _device;
    init_info.QueueFamily = _graphicQueueIndex;
    init_info.Queue = _graphicQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = swapChain.currentSwapChainNbImg;
    init_info.CheckVkResultFn = [](VkResult err) {
        if (err != VK_SUCCESS) {
            throw std::runtime_error("Imgui: Vulkan operation failed");
        }
    };
    ImGui_ImplVulkan_Init(&init_info, _render_pass.renderPass);
    ImGui_ImplVulkan_SetMinImageCount(2);
}

void
VulkanUi::_load_fonts()
{
    if (vkResetCommandPool(_device, _ui_command_pools[0], 0) != VK_SUCCESS) {
        throw std::runtime_error("VulkanUi: failed to reset command pool");
    }
    auto cmd_buffer = beginSingleTimeCommands(_device, _ui_command_pools[0]);
    ImGui_ImplVulkan_CreateFontsTexture(cmd_buffer);
    endSingleTimeCommands(
      _device, _ui_command_pools[0], cmd_buffer, _graphicQueue);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void
VulkanUi::_create_ui_command_buffers(uint32_t nbSwapChainFrames)
{
    _ui_command_buffers.resize(nbSwapChainFrames);

    for (uint32_t i = 0; i < nbSwapChainFrames; ++i) {
        VkCommandBufferAllocateInfo cb_allocate_info{};
        cb_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cb_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cb_allocate_info.commandPool = _ui_command_pools[i];
        cb_allocate_info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(_device,
                                     &cb_allocate_info,
                                     &_ui_command_buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error(
              "VulkanRenderer: Failed to allocate ui command buffers");
        }
    }
}