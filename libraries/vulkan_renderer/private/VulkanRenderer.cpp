#include "VulkanRenderer.hpp"

#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>

#include "glm/gtc/matrix_transform.hpp"

#include "VulkanDebug.hpp"
#include "VkSwapChain.hpp"
#include "VkShader.hpp"
#include "VkMemory.hpp"
#include "VkCommandBuffer.hpp"
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
    // Pipeline + Model + model texture related
    _create_descriptor_layout();
    _create_gfx_pipeline();
    _tex = _tex_manager.loadAndGetTexture("resources/texture/texture.jpg");
    _init_instances_matrices();
    _create_gfx_buffer();
    _create_descriptor_pool();
    _create_descriptor_sets();

    // Drawing related
    _create_command_buffers();
    _sync.init(_vk_instance, _render_pass.swapChainFramebuffers.size());
}

void
VulkanRenderer::resize(uint32_t win_w, uint32_t win_h)
{
    vkDeviceWaitIdle(_vk_instance.device);
    _render_pass.resize(win_w, win_h);

    // Pipeline + Model + model texture related
    _create_gfx_pipeline();
    if (_render_pass.oldSwapChainNbImg != _render_pass.currentSwapChainNbImg) {
        _sync.init(_vk_instance, _render_pass.swapChainFramebuffers.size());
        vkDestroyBuffer(_vk_instance.device, _gfx_buffer, nullptr);
        vkFreeMemory(_vk_instance.device, _gfx_memory, nullptr);
        vkDestroyDescriptorPool(_vk_instance.device, _descriptor_pool, nullptr);
        _create_gfx_buffer();
        _create_descriptor_pool();
        _create_descriptor_sets();
    }

    // Drawing related
    _create_command_buffers();
}

void
VulkanRenderer::clear()
{
    _sync.clear();
    _render_pass.clear();
    vkDestroyDescriptorPool(_vk_instance.device, _descriptor_pool, nullptr);
    vkDestroyPipeline(_vk_instance.device, _graphic_pipeline, nullptr);
    vkDestroyPipelineLayout(_vk_instance.device, _pipeline_layout, nullptr);
    vkFreeCommandBuffers(_vk_instance.device,
                         _vk_instance.commandPool,
                         _command_buffers.size(),
                         _command_buffers.data());
    vkDestroyBuffer(_vk_instance.device, _gfx_buffer, nullptr);
    vkFreeMemory(_vk_instance.device, _gfx_memory, nullptr);
    vkDestroyDescriptorSetLayout(
      _vk_instance.device, _descriptor_set_layout, nullptr);
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

    if (_sync.inflightFence[img_index] != VK_NULL_HANDLE) {
        vkWaitForFences(_vk_instance.device,
                        1,
                        &_sync.inflightFence[img_index],
                        VK_TRUE,
                        UINT64_MAX);
    }
    _sync.inflightFence[img_index] = _sync.inflightFence[_sync.currentFrame];

    _update_ubo(img_index, view_proj_mat);

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
          "VkRenderer: Failed to submit draw command buffer");
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
    _sync.currentFrame = (_sync.currentFrame + 1) % _sync.MAX_FRAME_INFLIGHT;
}

void
VulkanRenderer::deviceWaitIdle()
{
    vkDeviceWaitIdle(_vk_instance.device);
}

void
VulkanRenderer::_create_descriptor_layout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler_layout_binding.pImmutableSamplers = nullptr;

    std::array bindings{ ubo_layout_binding, sampler_layout_binding };

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = bindings.size();
    layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_vk_instance.device,
                                    &layout_info,
                                    nullptr,
                                    &_descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: failed to create descriptor set layout");
    }
}

void
VulkanRenderer::_create_gfx_pipeline()
{
    // Shaders
    auto vert_shader =
      loadShader(_vk_instance.device, "resources/shaders/model/model.vert.spv");
    auto frag_shader =
      loadShader(_vk_instance.device, "resources/shaders/model/model.frag.spv");

    VkPipelineShaderStageCreateInfo vert_shader_info{};
    vert_shader_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_info.module = vert_shader;
    vert_shader_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_info{};
    frag_shader_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_info.module = frag_shader;
    frag_shader_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stage_info[] = { vert_shader_info,
                                                            frag_shader_info };

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    auto binding_description = Vertex::getBindingDescription();
    auto attribute_description = Vertex::getAttributeDescriptions();
    vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount =
      binding_description.size();
    vertex_input_info.pVertexBindingDescriptions = binding_description.data();
    vertex_input_info.vertexAttributeDescriptionCount =
      attribute_description.size();
    vertex_input_info.pVertexAttributeDescriptions =
      attribute_description.data();

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
    input_assembly_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    // Viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.height = _render_pass.swapChainExtent.height;
    viewport.width = _render_pass.swapChainExtent.width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = _render_pass.swapChainExtent;

    VkPipelineViewportStateCreateInfo viewport_state_info{};
    viewport_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = &scissor;
    viewport_state_info.viewportCount = 1;
    viewport_state_info.pViewports = &viewport;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer_info{};
    rasterizer_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_info.depthBiasClamp = VK_FALSE;
    rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_info.lineWidth = 1.0f;
    // rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
    // rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_info.cullMode = VK_CULL_MODE_NONE;
    rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_info.depthBiasEnable = VK_FALSE;
    rasterizer_info.depthBiasConstantFactor = 0.0f;
    rasterizer_info.depthBiasClamp = 0.0f;
    rasterizer_info.depthBiasSlopeFactor = 0.0f;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling_info{};
    multisampling_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_info.sampleShadingEnable = VK_FALSE;
    multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_info.minSampleShading = 1.0f;
    multisampling_info.pSampleMask = nullptr;
    multisampling_info.alphaToCoverageEnable = VK_FALSE;
    multisampling_info.alphaToOneEnable = VK_FALSE;

    // Color Blending
    VkPipelineColorBlendAttachmentState color_blending_attachment{};
    color_blending_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blending_attachment.blendEnable = VK_FALSE;
    color_blending_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blending_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blending_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blending_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blending_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blending_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending_info{};
    color_blending_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending_info.logicOpEnable = VK_FALSE;
    color_blending_info.attachmentCount = 1;
    color_blending_info.pAttachments = &color_blending_attachment;
    color_blending_info.logicOp = VK_LOGIC_OP_COPY;
    color_blending_info.blendConstants[0] = 0.0f;
    color_blending_info.blendConstants[1] = 0.0f;
    color_blending_info.blendConstants[2] = 0.0f;
    color_blending_info.blendConstants[3] = 0.0f;

    // Pipeline layout creation
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &_descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(_vk_instance.device,
                               &pipeline_layout_info,
                               nullptr,
                               &_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: Failed to create pipeline layout");
    }

    // Depth
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};

    // Gfx pipeline creation
    VkGraphicsPipelineCreateInfo gfx_pipeline_info{};
    gfx_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gfx_pipeline_info.stageCount = 2;
    gfx_pipeline_info.pStages = shader_stage_info;
    gfx_pipeline_info.pVertexInputState = &vertex_input_info;
    gfx_pipeline_info.pInputAssemblyState = &input_assembly_info;
    gfx_pipeline_info.pViewportState = &viewport_state_info;
    gfx_pipeline_info.pRasterizationState = &rasterizer_info;
    gfx_pipeline_info.pMultisampleState = &multisampling_info;
    gfx_pipeline_info.pDepthStencilState = &depth_stencil;
    gfx_pipeline_info.pColorBlendState = &color_blending_info;
    gfx_pipeline_info.pDynamicState = nullptr;
    gfx_pipeline_info.layout = _pipeline_layout;
    gfx_pipeline_info.renderPass = _render_pass.renderPass;
    gfx_pipeline_info.subpass = 0;
    gfx_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    gfx_pipeline_info.basePipelineIndex = -1;
    if (vkCreateGraphicsPipelines(_vk_instance.device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &gfx_pipeline_info,
                                  nullptr,
                                  &_graphic_pipeline) != VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: Failed to create graphic pipeline");
    }

    vkDestroyShaderModule(_vk_instance.device, vert_shader, nullptr);
    vkDestroyShaderModule(_vk_instance.device, frag_shader, nullptr);
}

void
VulkanRenderer::_create_gfx_buffer()
{
    VkDeviceSize vertex_size = sizeof(Vertex) * TEST_TRIANGLE_VERTICIES.size();
    VkDeviceSize instance_matrices_size =
      sizeof(glm::mat4) * TEST_TRIANGLE_POS.size();
    VkDeviceSize indices_size = sizeof(uint32_t) * TEST_TRIANGLE_INDICES.size();
    VkDeviceSize uniform_size =
      sizeof(glm::mat4) * _render_pass.swapChainFramebuffers.size();
    VkDeviceSize total_size =
      vertex_size + instance_matrices_size + indices_size + uniform_size;

    // CPU => GPU transfer buffer
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    createBuffer(_vk_instance.device,
                 staging_buffer,
                 total_size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(_vk_instance.physicalDevice,
                   _vk_instance.device,
                   staging_buffer,
                   staging_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Vertices
    void *data{};
    vkMapMemory(
      _vk_instance.device, staging_buffer_memory, 0, vertex_size, 0, &data);
    memcpy(data, TEST_TRIANGLE_VERTICIES.data(), vertex_size);
    vkUnmapMemory(_vk_instance.device, staging_buffer_memory);

    // Instance Matrices
    vkMapMemory(_vk_instance.device,
                staging_buffer_memory,
                vertex_size,
                instance_matrices_size,
                0,
                &data);
    memcpy(data, _translation_matrices.data(), instance_matrices_size);
    vkUnmapMemory(_vk_instance.device, staging_buffer_memory);

    // Indices
    vkMapMemory(_vk_instance.device,
                staging_buffer_memory,
                vertex_size + instance_matrices_size,
                indices_size,
                0,
                &data);
    memcpy(data, TEST_TRIANGLE_INDICES.data(), indices_size);
    vkUnmapMemory(_vk_instance.device, staging_buffer_memory);

    // GPU only memory
    createBuffer(
      _vk_instance.device,
      _gfx_buffer,
      total_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    allocateBuffer(_vk_instance.physicalDevice,
                   _vk_instance.device,
                   _gfx_buffer,
                   _gfx_memory,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(_vk_instance.device,
               _vk_instance.commandPool,
               _vk_instance.presentQueue,
               _gfx_buffer,
               staging_buffer,
               total_size);

    vkDestroyBuffer(_vk_instance.device, staging_buffer, nullptr);
    vkFreeMemory(_vk_instance.device, staging_buffer_memory, nullptr);
}

void
VulkanRenderer::_create_descriptor_pool()
{
    std::array<VkDescriptorPoolSize, 2> pool_size{};
    pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size[0].descriptorCount = _render_pass.swapChainFramebuffers.size();
    pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size[1].descriptorCount = _render_pass.swapChainFramebuffers.size();

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = pool_size.size();
    pool_info.pPoolSizes = pool_size.data();
    pool_info.maxSets = _render_pass.swapChainFramebuffers.size();

    if (vkCreateDescriptorPool(
          _vk_instance.device, &pool_info, nullptr, &_descriptor_pool) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: failed to create descriptor pool");
    }
}

void
VulkanRenderer::_create_descriptor_sets()
{
    std::vector<VkDescriptorSetLayout> layouts(
      _render_pass.swapChainFramebuffers.size(), _descriptor_set_layout);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = _descriptor_pool;
    alloc_info.descriptorSetCount = _render_pass.swapChainFramebuffers.size();
    alloc_info.pSetLayouts = layouts.data();

    _descriptor_sets.resize(_render_pass.swapChainFramebuffers.size());
    if (vkAllocateDescriptorSets(_vk_instance.device,
                                 &alloc_info,
                                 _descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: failed to create descriptor sets");
    }

    VkDeviceSize vertex_size = sizeof(Vertex) * TEST_TRIANGLE_VERTICIES.size();
    VkDeviceSize instance_matrices_size =
      sizeof(glm::mat4) * TEST_TRIANGLE_POS.size();
    VkDeviceSize indices_size = sizeof(uint32_t) * TEST_TRIANGLE_INDICES.size();

    for (size_t i = 0; i < _render_pass.swapChainFramebuffers.size(); ++i) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = _gfx_buffer;
        buffer_info.offset = vertex_size + instance_matrices_size +
                             indices_size + sizeof(ModelUbo) * i;
        buffer_info.range = sizeof(ModelUbo);

        VkDescriptorImageInfo img_info{};
        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        img_info.imageView = _tex.texture_img_view;
        img_info.sampler = _tex.texture_sampler;

        std::array<VkWriteDescriptorSet, 2> descriptor_write{};
        descriptor_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[0].dstSet = _descriptor_sets[i];
        descriptor_write[0].dstBinding = 0;
        descriptor_write[0].dstArrayElement = 0;
        descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write[0].descriptorCount = 1;
        descriptor_write[0].pBufferInfo = &buffer_info;
        descriptor_write[0].pImageInfo = nullptr;
        descriptor_write[0].pTexelBufferView = nullptr;

        descriptor_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[1].dstSet = _descriptor_sets[i];
        descriptor_write[1].dstBinding = 1;
        descriptor_write[1].dstArrayElement = 0;
        descriptor_write[1].descriptorType =
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write[1].descriptorCount = 1;
        descriptor_write[1].pBufferInfo = nullptr;
        descriptor_write[1].pImageInfo = &img_info;
        descriptor_write[1].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(_vk_instance.device,
                               descriptor_write.size(),
                               descriptor_write.data(),
                               0,
                               nullptr);
    }
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
          "VkRenderer: Failed to allocate command buffers");
    }

    size_t i = 0;
    for (auto &it : _command_buffers) {
        VkCommandBufferBeginInfo cb_begin_info{};
        cb_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cb_begin_info.flags = 0;
        cb_begin_info.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(it, &cb_begin_info) != VK_SUCCESS) {
            throw std::runtime_error(
              "VkRenderer: Failed to begin recording command buffer");
        }

        // Begin render pass values
        std::array<VkClearValue, 2> clear_vals{};
        clear_vals[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clear_vals[1].depthStencil = { 1.0f, 0 };
        VkRenderPassBeginInfo rp_begin_info{};
        rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin_info.renderPass = _render_pass.renderPass;
        rp_begin_info.framebuffer = _render_pass.swapChainFramebuffers[i];
        rp_begin_info.renderArea.offset = { 0, 0 };
        rp_begin_info.renderArea.extent = _render_pass.swapChainExtent;
        rp_begin_info.clearValueCount = clear_vals.size();
        rp_begin_info.pClearValues = clear_vals.data();

        // Vertex related values
        VkBuffer vertex_buffer[] = { _gfx_buffer, _gfx_buffer };
        VkDeviceSize offsets[] = {
            0, sizeof(Vertex) * TEST_TRIANGLE_VERTICIES.size()
        };

        vkCmdBeginRenderPass(it, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(
          it, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphic_pipeline);
        vkCmdBindVertexBuffers(it, 0, 2, vertex_buffer, offsets);
        vkCmdBindIndexBuffer(it,
                             _gfx_buffer,
                             sizeof(Vertex) * TEST_TRIANGLE_VERTICIES.size() +
                               sizeof(glm::mat4) * TEST_TRIANGLE_POS.size(),
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(it,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                _pipeline_layout,
                                0,
                                1,
                                &_descriptor_sets[i],
                                0,
                                nullptr);
        vkCmdDrawIndexed(it, 6, 4, 0, 0, 0);
        vkCmdEndRenderPass(it);

        if (vkEndCommandBuffer(it) != VK_SUCCESS) {
            throw std::runtime_error(
              "VkRenderer: Failed to record command Buffer");
        }
        ++i;
    }
}

void
VulkanRenderer::_update_ubo(uint32_t img_index, glm::mat4 const &view_proj_mat)
{
    VkDeviceSize vertex_size = sizeof(Vertex) * TEST_TRIANGLE_VERTICIES.size();
    VkDeviceSize instance_matrices_size =
      sizeof(glm::mat4) * TEST_TRIANGLE_POS.size();
    VkDeviceSize indices_size = sizeof(uint32_t) * TEST_TRIANGLE_INDICES.size();

    ModelUbo ubo = { view_proj_mat };
    ubo.view_proj[1][1] *= -1.0f;

    // CPU => GPU transfer buffer
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    createBuffer(_vk_instance.device,
                 staging_buffer,
                 sizeof(ModelUbo),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(_vk_instance.physicalDevice,
                   _vk_instance.device,
                   staging_buffer,
                   staging_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data = nullptr;
    vkMapMemory(_vk_instance.device,
                staging_buffer_memory,
                0,
                sizeof(ModelUbo),
                0,
                &data);
    memcpy(data, &ubo, sizeof(ModelUbo));
    vkUnmapMemory(_vk_instance.device, staging_buffer_memory);

    // GPU buffer
    VkCommandBuffer cmd_buffer =
      beginSingleTimeCommands(_vk_instance.device, _vk_instance.commandPool);

    VkBufferCopy copy_region{};
    copy_region.size = sizeof(ModelUbo);
    copy_region.dstOffset = vertex_size + instance_matrices_size +
                            indices_size + sizeof(ModelUbo) * img_index;
    copy_region.srcOffset = 0;
    vkCmdCopyBuffer(cmd_buffer, staging_buffer, _gfx_buffer, 1, &copy_region);

    endSingleTimeCommands(_vk_instance.device,
                          _vk_instance.commandPool,
                          cmd_buffer,
                          _vk_instance.graphicQueue);

    vkDestroyBuffer(_vk_instance.device, staging_buffer, nullptr);
    vkFreeMemory(_vk_instance.device, staging_buffer_memory, nullptr);
}

void
VulkanRenderer::_init_instances_matrices()
{
    _translation_matrices.resize(TEST_TRIANGLE_POS.size());

    for (size_t i = 0; i < _translation_matrices.size(); ++i) {
        _translation_matrices[i] = glm::mat4(1.0f);

        _translation_matrices[i] =
          glm::translate(_translation_matrices[i], TEST_TRIANGLE_POS[i]);
        _translation_matrices[i] = glm::rotate(
          _translation_matrices[i], 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        _translation_matrices[i] = glm::rotate(
          _translation_matrices[i], 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        _translation_matrices[i] = glm::rotate(
          _translation_matrices[i], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        _translation_matrices[i] = glm::translate(_translation_matrices[i],
                                                  glm::vec3(0.0f, -1.0f, 0.0f));
        _translation_matrices[i] =
          glm::scale(_translation_matrices[i], glm::vec3(1.0f));
    }
}
