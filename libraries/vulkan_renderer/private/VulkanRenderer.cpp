#include "VulkanRenderer.hpp"

#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>

#include "glm/gtc/matrix_transform.hpp"

#include "VulkanDebug.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanCommandBuffer.hpp"
#include "TestModelInstancing.hpp"

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
    _render_pass.init(_vk_instance, win_w, win_h);
    _sync.init(_vk_instance, _render_pass.swapChainFramebuffers.size());
}

void
VulkanRenderer::resize(uint32_t win_w, uint32_t win_h)
{
    vkDeviceWaitIdle(_vk_instance.device);
    _render_pass.resize(win_w, win_h);

    // Pipeline + Model + model texture related
    if (_render_pass.oldSwapChainNbImg != _render_pass.currentSwapChainNbImg) {
        _sync.init(_vk_instance, _render_pass.swapChainFramebuffers.size());
        _model_pipeline.resize(_render_pass, _tex_manager);
    }

    // Drawing related
    _create_command_buffers();
}

void
VulkanRenderer::clear()
{
    _model_pipeline.clear();
    _sync.clear();
    _render_pass.clear();
    _tex_manager.clear();
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
    _model_pipeline.init(
      _vk_instance, _render_pass, model, _tex_manager, MAX_MODEL_INSTANCE);

    // Drawing related
    _create_command_buffers();
}

uint32_t
VulkanRenderer::addModelInstance(ModelInstanceInfo const &info)
{
    return (_model_pipeline.addInstance(info));
}

bool
VulkanRenderer::removeModelInstance(uint32_t index)
{
    return (_model_pipeline.removeInstance(index));
}
bool
VulkanRenderer::updateModelInstance(uint32_t index,
                                    ModelInstanceInfo const &info)
{
    return (_model_pipeline.updateInstance(index, info));
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
                            _render_pass.swapChain,
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

    _model_pipeline.updateViewProjMatrix(img_index, view_proj_mat);

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
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &_command_buffers[img_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = sig_sems;

    vkResetFences(
      _vk_instance.device, 1, &_sync.inflightFence[_sync.currentFrame]);
    if (vkQueueSubmit(_vk_instance.graphicQueue,
                      1,
                      &submit_info,
                      _sync.inflightFence[_sync.currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderer: Failed to submit draw command buffer");
    }

    VkSwapchainKHR swap_chains[] = { _render_pass.swapChain };
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
VulkanRenderer::_create_command_buffers()
{
    _command_buffers.resize(_render_pass.swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo cb_allocate_info{};
    cb_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cb_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cb_allocate_info.commandPool = _vk_instance.commandPool;
    cb_allocate_info.commandBufferCount = _command_buffers.size();

    if (vkAllocateCommandBuffers(_vk_instance.device,
                                 &cb_allocate_info,
                                 _command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderer: Failed to allocate command buffers");
    }

    size_t i = 0;
    for (auto &it : _command_buffers) {
        VkCommandBufferBeginInfo cb_begin_info{};
        cb_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cb_begin_info.flags = 0;
        cb_begin_info.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(it, &cb_begin_info) != VK_SUCCESS) {
            throw std::runtime_error(
              "VulkanRenderer: Failed to begin recording command buffer");
        }

        // Begin render pass values
        std::array<VkClearValue, 2> clear_vals{};
        clear_vals[0].color = { { 0.2f, 0.3f, 0.3f, 1.0f } };
        clear_vals[1].depthStencil = { 1.0f, 0 };
        VkRenderPassBeginInfo rp_begin_info{};
        rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin_info.renderPass = _render_pass.renderPass;
        rp_begin_info.framebuffer = _render_pass.swapChainFramebuffers[i];
        rp_begin_info.renderArea.offset = { 0, 0 };
        rp_begin_info.renderArea.extent = _render_pass.swapChainExtent;
        rp_begin_info.clearValueCount = clear_vals.size();
        rp_begin_info.pClearValues = clear_vals.data();

        vkCmdBeginRenderPass(it, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        _model_pipeline.generateCommands(it, i);
        vkCmdEndRenderPass(it);
        if (vkEndCommandBuffer(it) != VK_SUCCESS) {
            throw std::runtime_error(
              "VulkanRenderer: Failed to record command Buffer");
        }
        ++i;
    }
}
