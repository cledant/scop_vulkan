#include "VulkanModelPipeline.hpp"

#include <stdexcept>

#include "glm/gtc/matrix_transform.hpp"

#include "VulkanShader.hpp"

void
VulkanModelPipeline::init(VulkanInstance const &vkInstance,
                          VulkanRenderPass const &renderPass,
                          Model const &model,
                          VulkanTextureManager &texManager,
                          uint32_t maxModelNb)
{
    (void)model;
    (void)texManager;
    _device = vkInstance.device;
    _max_model_nb = maxModelNb;
    _create_descriptor_layout();
    _create_pipeline_layout();
    _create_gfx_pipeline(renderPass);
}

void
VulkanModelPipeline::resize(VulkanRenderPass const &renderPass)
{
    vkDestroyPipeline(_device, _graphic_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    _create_pipeline_layout();
    _create_gfx_pipeline(renderPass);

    /*    if (renderPass.oldSwapChainNbImg != renderPass.currentSwapChainNbImg)
       { vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
            _create_gfx_buffer();
            _create_descriptor_pool();
            _create_descriptor_sets();
        }*/
}

void
VulkanModelPipeline::clear()
{
    vkDestroyPipeline(_device, _graphic_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
}

uint32_t
VulkanModelPipeline::addInstance(VulkanModelInfo const &info)
{
    (void)info;
    return (instance_index++);
}

bool
VulkanModelPipeline::removeInstance(uint32_t index)
{
    (void)index;
    return (true);
}

bool
VulkanModelPipeline::updateInstance(uint32_t index, VulkanModelInfo const &info)
{
    (void)index;
    (void)info;
    return (true);
}

void
VulkanModelPipeline::generateCommands()
{}

void
VulkanModelPipeline::updateViewPerspectiveMatrix(glm::mat4 const &mat)
{
    (void)mat;
}

std::array<VkVertexInputBindingDescription, 2>
VulkanModelPipeline::_get_binding_description()
{
    std::array<VkVertexInputBindingDescription, 2> binding_description{};
    binding_description[0].binding = 0;
    binding_description[0].stride = sizeof(Vertex);
    binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    binding_description[1].binding = 1;
    binding_description[1].stride = sizeof(glm::mat4);
    binding_description[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return (binding_description);
}

std::array<VkVertexInputAttributeDescription, 9>
VulkanModelPipeline::_get_attribute_description()
{
    std::array<VkVertexInputAttributeDescription, 9> attribute_description{};

    attribute_description[0].binding = 0;
    attribute_description[0].location = 0;
    attribute_description[0].offset = 0;
    attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[1].binding = 0;
    attribute_description[1].location = 1;
    attribute_description[1].offset = offsetof(Vertex, normal);
    attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[2].binding = 0;
    attribute_description[2].location = 2;
    attribute_description[2].offset = offsetof(Vertex, tex_coords);
    attribute_description[2].format = VK_FORMAT_R32G32_SFLOAT;

    attribute_description[3].binding = 0;
    attribute_description[3].location = 1;
    attribute_description[3].offset = offsetof(Vertex, tangent);
    attribute_description[3].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[4].binding = 0;
    attribute_description[4].location = 2;
    attribute_description[4].offset = offsetof(Vertex, bitangent);
    attribute_description[4].format = VK_FORMAT_R32G32_SFLOAT;

    attribute_description[5].binding = 1;
    attribute_description[5].location = 3;
    attribute_description[5].offset = 0;
    attribute_description[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[6].binding = 1;
    attribute_description[6].location = 4;
    attribute_description[6].offset = sizeof(glm::vec4);
    attribute_description[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[7].binding = 1;
    attribute_description[7].location = 5;
    attribute_description[7].offset = sizeof(glm::vec4) * 2;
    attribute_description[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[8].binding = 1;
    attribute_description[8].location = 6;
    attribute_description[8].offset = sizeof(glm::vec4) * 3;
    attribute_description[8].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    return (attribute_description);
}

void
VulkanModelPipeline::_create_descriptor_layout()
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

    if (vkCreateDescriptorSetLayout(
          _device, &layout_info, nullptr, &_descriptor_set_layout) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderer: failed to create descriptor set layout");
    }
}

void
VulkanModelPipeline::_create_pipeline_layout()
{
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &_descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(
          _device, &pipeline_layout_info, nullptr, &_pipeline_layout) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderer: Failed to create pipeline layout");
    }
}

void
VulkanModelPipeline::_create_gfx_pipeline(VulkanRenderPass const &renderPass)
{
    // Shaders
    auto vert_shader =
      loadShader(_device, "resources/shaders/model/model.vert.spv");
    auto frag_shader =
      loadShader(_device, "resources/shaders/model/model.frag.spv");

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
    auto binding_description = _get_binding_description();
    auto attribute_description = _get_attribute_description();
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
    viewport.height = renderPass.swapChainExtent.height;
    viewport.width = renderPass.swapChainExtent.width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = renderPass.swapChainExtent;

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
    gfx_pipeline_info.renderPass = renderPass.renderPass;
    gfx_pipeline_info.subpass = 0;
    gfx_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    gfx_pipeline_info.basePipelineIndex = -1;
    if (vkCreateGraphicsPipelines(_device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &gfx_pipeline_info,
                                  nullptr,
                                  &_graphic_pipeline) != VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanRenderer: Failed to create graphic pipeline");
    }

    vkDestroyShaderModule(_device, vert_shader, nullptr);
    vkDestroyShaderModule(_device, frag_shader, nullptr);
}

void
VulkanModelPipeline::_create_gfx_buffer()
{}

void
VulkanModelPipeline::_create_descriptor_pool()
{}

void
VulkanModelPipeline::_create_descriptor_sets()
{}

uint32_t VulkanModelPipeline::instance_index = 1;