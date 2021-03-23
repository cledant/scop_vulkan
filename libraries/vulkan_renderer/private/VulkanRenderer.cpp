#include "VulkanRenderer.hpp"

#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>

#include "VulkanDebug.hpp"
#include "VulkanSwapChainUtils.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanMemory.hpp"
#include "VulkanUboStructs.hpp"

void
VulkanRenderer::createInstance(std::string &&app_name,
                               std::string &&engine_name,
                               uint32_t app_version,
                               uint32_t engine_version,
                               std::vector<char const *> &&required_extensions)
{
    if constexpr (ENABLE_VALIDATION_LAYER) {
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    _app_name = std::move(app_name);
    _app_version = app_version;
    _engine_name = std::move(engine_name);
    _engine_version = engine_version;
    _vk_instance.instance =
      VulkanInstance::createInstance(_app_name,
                                     _engine_name,
                                     _app_version,
                                     _engine_version,
                                     std::move(required_extensions));
}

VkInstance
VulkanRenderer::getVkInstance() const
{
    return (_vk_instance.instance);
}

void
VulkanRenderer::init(VkSurfaceKHR surface, uint32_t win_w, uint32_t win_h)
{
    assert(surface);

    _vk_instance.init(surface);
    _tex_manager.init(_vk_instance);
    _swap_chain.init(_vk_instance, win_w, win_h);
    _sync.init(_vk_instance, _swap_chain.swapChainImageViews.size());
    _create_system_uniform_buffer();
    _ui.init(_vk_instance, _swap_chain);
}

void
VulkanRenderer::resize(uint32_t win_w, uint32_t win_h)
{
    if (win_w <= 0 || win_h <= 0) {
        return;
    }
    vkDeviceWaitIdle(_vk_instance.device);

    _swap_chain.resize(win_w, win_h);
    _sync.resize(_swap_chain.currentSwapChainNbImg);
    vkDestroyBuffer(_vk_instance.device, _system_uniform, nullptr);
    vkFreeMemory(_vk_instance.device, _system_uniform_memory, nullptr);
    _create_system_uniform_buffer();
    _model_pipeline.resize(_swap_chain, _tex_manager, _system_uniform);
    _create_model_command_buffers();
    _ui.resize(_swap_chain);
}

void
VulkanRenderer::clear()
{
    _ui.clear();
    _model_pipeline.clear();
    _sync.clear();
    _swap_chain.clear();
    _tex_manager.clear();
    vkDestroyBuffer(_vk_instance.device, _system_uniform, nullptr);
    vkFreeMemory(_vk_instance.device, _system_uniform_memory, nullptr);
    _vk_instance.clear();
}

std::string const &
VulkanRenderer::getAppName() const
{
    return (_app_name);
}

uint32_t
VulkanRenderer::getAppVersion() const
{
    return (_app_version);
}

std::string const &
VulkanRenderer::getEngineName() const
{
    return (_engine_name);
}

uint32_t
VulkanRenderer::getEngineVersion() const
{
    return (_engine_version);
}

// Model Related
void
VulkanRenderer::loadModel(Model const &model)
{
    _model_pipeline.init(_vk_instance,
                         _swap_chain,
                         model,
                         _tex_manager,
                         _system_uniform,
                         MAX_MODEL_INSTANCE);

    // Drawing related
    _create_model_command_buffers();
}

uint32_t
VulkanRenderer::addModelInstance(ModelInstanceInfo const &info)
{
    auto index = _model_pipeline.addInstance(info);
    _create_model_command_buffers();
    return (index);
}

bool
VulkanRenderer::removeModelInstance(uint32_t index)
{
    auto ret = _model_pipeline.removeInstance(index);
    _create_model_command_buffers();
    return (ret);
}
bool
VulkanRenderer::updateModelInstance(uint32_t index,
                                    ModelInstanceInfo const &info)
{
    return (_model_pipeline.updateInstance(index, info));
}

bool
VulkanRenderer::getModelInstance(uint32_t index, ModelInstanceInfo &info)
{
    return (_model_pipeline.getInstance(index, info));
}

// Render Related
void
VulkanRenderer::draw(glm::mat4 const &view_proj_mat)
{
    vkWaitForFences(_vk_instance.device,
                    1,
                    &_sync.inflightFence[_sync.currentFrame],
                    VK_TRUE,
                    UINT64_MAX);

    uint32_t img_index;
    auto result =
      vkAcquireNextImageKHR(_vk_instance.device,
                            _swap_chain.swapChain,
                            UINT64_MAX,
                            _sync.imageAvailableSem[_sync.currentFrame],
                            VK_NULL_HANDLE,
                            &img_index);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return;
    }

    if (_sync.imgsInflightFence[img_index] != VK_NULL_HANDLE) {
        vkWaitForFences(_vk_instance.device,
                        1,
                        &_sync.imgsInflightFence[img_index],
                        VK_TRUE,
                        UINT64_MAX);
    }
    _sync.imgsInflightFence[img_index] =
      _sync.inflightFence[_sync.currentFrame];

    // Update view_proj matrix
    copyOnCpuCoherentMemory(_vk_instance.device,
                            _system_uniform_memory,
                            img_index * sizeof(SystemUbo) +
                              offsetof(SystemUbo, view_proj),
                            sizeof(glm::mat4),
                            &view_proj_mat);

    VkSemaphore wait_sems[] = { _sync.imageAvailableSem[_sync.currentFrame] };
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    VkSemaphore sig_sems[] = { _sync.renderFinishedSem[_sync.currentFrame] };
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_sems;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = sig_sems;
    auto &ui_cmd_buffer =
      _ui.generateCommandBuffer(img_index, _swap_chain.swapChainExtent);
    std::array<VkCommandBuffer, 2> cmd_buffers = {
        _model_command_buffers[img_index], ui_cmd_buffer
    };
    submit_info.commandBufferCount = cmd_buffers.size();
    submit_info.pCommandBuffers = cmd_buffers.data();
    vkResetFences(
      _vk_instance.device, 1, &_sync.inflightFence[_sync.currentFrame]);
    if (vkQueueSubmit(_vk_instance.graphicQueue,
                      1,
                      &submit_info,
                      _sync.inflightFence[_sync.currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderer: Failed to submit draw command buffer");
    }

    VkSwapchainKHR swap_chains[] = { _swap_chain.swapChain };
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = sig_sems;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &img_index;
    present_info.pResults = nullptr;
    vkQueuePresentKHR(_vk_instance.presentQueue, &present_info);
    _sync.currentFrame =
      (_sync.currentFrame + 1) % VulkanSync::MAX_FRAME_INFLIGHT;
}

void
VulkanRenderer::deviceWaitIdle() const
{
    vkDeviceWaitIdle(_vk_instance.device);
}

void
VulkanRenderer::_create_model_command_buffers()
{
    _model_command_buffers.resize(_swap_chain.swapChainImageViews.size());

    VkCommandBufferAllocateInfo cb_allocate_info{};
    cb_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cb_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cb_allocate_info.commandPool = _vk_instance.modelCommandPool;
    cb_allocate_info.commandBufferCount = _model_command_buffers.size();

    if (vkAllocateCommandBuffers(_vk_instance.device,
                                 &cb_allocate_info,
                                 _model_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderer: Failed to allocate model command buffers");
    }

    size_t i = 0;
    auto const &model_render_pass = _model_pipeline.getVulkanModelRenderPass();
    for (auto &it : _model_command_buffers) {
        VkCommandBufferBeginInfo cb_begin_info{};
        cb_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cb_begin_info.flags = 0;
        cb_begin_info.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(it, &cb_begin_info) != VK_SUCCESS) {
            throw std::runtime_error(
              "VulkanRenderer: Failed to begin recording model command buffer");
        }

        // Begin render pass values
        std::array<VkClearValue, 2> clear_vals{};
        clear_vals[0].color = { { 0.2f, 0.2f, 0.2f, 1.0f } };
        clear_vals[1].depthStencil = { 1.0f, 0 };
        VkRenderPassBeginInfo rp_begin_info{};
        rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin_info.renderPass = model_render_pass.renderPass;
        rp_begin_info.framebuffer = model_render_pass.swapChainFramebuffers[i];
        rp_begin_info.renderArea.offset = { 0, 0 };
        rp_begin_info.renderArea.extent = _swap_chain.swapChainExtent;
        rp_begin_info.clearValueCount = clear_vals.size();
        rp_begin_info.pClearValues = clear_vals.data();

        vkCmdBeginRenderPass(it, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        _model_pipeline.generateCommands(it, i);
        vkCmdEndRenderPass(it);
        if (vkEndCommandBuffer(it) != VK_SUCCESS) {
            throw std::runtime_error(
              "VulkanRenderer: Failed to record model command Buffer");
        }
        ++i;
    }
}

void
VulkanRenderer::_create_system_uniform_buffer()
{
    createBuffer(_vk_instance.device,
                 _system_uniform,
                 sizeof(SystemUbo) * _swap_chain.currentSwapChainNbImg,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    allocateBuffer(_vk_instance.physicalDevice,
                   _vk_instance.device,
                   _system_uniform,
                   _system_uniform_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}
