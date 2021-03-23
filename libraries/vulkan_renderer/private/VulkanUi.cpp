#include "VulkanUi.hpp"

#include <stdexcept>

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
    _commandPool = vkInstance.commandPool;
    _init_imgui(swapChain);
    _load_fonts();
}

void
VulkanUi::resize(VulkanSwapChain const &swapChain)
{
    vkDeviceWaitIdle(_device);
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    _init_imgui(swapChain);
}

void
VulkanUi::clear()
{
    vkDeviceWaitIdle(_device);
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    _instance = nullptr;
    _physicalDevice = nullptr;
    _device = nullptr;
    _graphicQueue = nullptr;
    _graphicQueueIndex = UINT32_MAX;
}

void
VulkanUi::_init_imgui(VulkanSwapChain const &renderPass)
{
    (void)renderPass;
    /*    ImGui_ImplVulkan_InitInfo init_info = {};
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
        pool_info.poolSizeCount =
       static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes)); pool_info.pPoolSizes =
       pool_sizes; if (vkCreateDescriptorPool( _device, &pool_info, nullptr,
       &_descriptorPool) != VK_SUCCESS) { throw std::runtime_error("VulkanUi:
       failed to create descriptor pool");
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
        init_info.ImageCount = renderPass.currentSwapChainNbImg;
        init_info.CheckVkResultFn = [](VkResult err) {
            if (err != VK_SUCCESS) {
                throw std::runtime_error("Imgui: Vulkan operation failed");
            }
        };
        ImGui_ImplVulkan_Init(&init_info, renderPass.renderPass);*/
}

void
VulkanUi::_load_fonts()
{
    /*    auto cmd_buffer = beginSingleTimeCommands(_device, _commandPool);
        ImGui_ImplVulkan_CreateFontsTexture(cmd_buffer);
        endSingleTimeCommands(_device, _commandPool, cmd_buffer, _graphicQueue);
        ImGui_ImplVulkan_DestroyFontUploadObjects();*/
}
