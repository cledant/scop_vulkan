#include "VulkanModelPipeline.hpp"

#include <stdexcept>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/gtc/matrix_transform.hpp"

#include "VulkanShader.hpp"
#include "VulkanMemory.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanUboStructs.hpp"

void
VulkanModelPipeline::init(VulkanInstance const &vkInstance,
                          VulkanRenderPass const &renderPass,
                          Model const &model,
                          VulkanTextureManager &texManager,
                          VkBuffer systemUbo,
                          uint32_t maxModelNb)
{
    _instance_handler.setMaxInstanceNb(maxModelNb);
    _device = vkInstance.device;
    _physical_device = vkInstance.physicalDevice;
    _cmd_pool = vkInstance.commandPool;
    _gfx_queue = vkInstance.graphicQueue;
    _model = &model;
    _create_descriptor_layout();
    _create_pipeline_layout();
    _create_gfx_pipeline(renderPass);
    _pipeline_meshes.resize(model.getMeshList().size());
    auto mesh_list = model.getMeshList();
    for (size_t i = 0; i < _pipeline_meshes.size(); ++i) {
        _pipeline_meshes[i] =
          _create_pipeline_mesh(mesh_list[i],
                                model.getDirectory(),
                                texManager,
                                renderPass.currentSwapChainNbImg);
        _create_descriptor_pool(renderPass, _pipeline_meshes[i]);
        _create_descriptor_sets(renderPass, _pipeline_meshes[i], systemUbo);
    }
}

void
VulkanModelPipeline::resize(VulkanRenderPass const &renderPass,
                            VulkanTextureManager &texManager,
                            VkBuffer systemUbo)
{
    assert(_model);

    vkDestroyPipeline(_device, _graphic_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);

    _create_pipeline_layout();
    _create_gfx_pipeline(renderPass);
    auto mesh_list = _model->getMeshList();
    for (size_t i = 0; i < _pipeline_meshes.size(); ++i) {
        vkDestroyBuffer(_device, _pipeline_meshes[i].buffer, nullptr);
        vkFreeMemory(_device, _pipeline_meshes[i].memory, nullptr);
        vkDestroyDescriptorPool(
          _device, _pipeline_meshes[i].descriptorPool, nullptr);
        _pipeline_meshes[i] =
          _create_pipeline_mesh(mesh_list[i],
                                _model->getDirectory(),
                                texManager,
                                renderPass.currentSwapChainNbImg);
        _create_descriptor_pool(renderPass, _pipeline_meshes[i]);
        _create_descriptor_sets(renderPass, _pipeline_meshes[i], systemUbo);
    }

    auto updater = [&](uint32_t index,
                       ModelInstanceInfo const &inst_info) -> void {
        _set_instance_matrix_on_gpu(index, inst_info);
    };
    _instance_handler.update(updater);
}

void
VulkanModelPipeline::clear()
{
    vkDestroyPipeline(_device, _graphic_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
    for (auto &mesh : _pipeline_meshes) {
        vkDestroyBuffer(_device, mesh.buffer, nullptr);
        vkFreeMemory(_device, mesh.memory, nullptr);
        vkDestroyDescriptorPool(_device, mesh.descriptorPool, nullptr);
    }
}

uint32_t
VulkanModelPipeline::addInstance(ModelInstanceInfo const &info)
{
    auto updater = [&](uint32_t index,
                       ModelInstanceInfo const &inst_info) -> void {
        _set_instance_matrix_on_gpu(index, inst_info);
    };

    return (_instance_handler.addInstance(info, updater));
}

bool
VulkanModelPipeline::removeInstance(uint32_t instanceIndex)
{
    auto updater = [&](uint32_t index,
                       ModelInstanceInfo const &inst_info) -> void {
        _set_instance_matrix_on_gpu(index, inst_info);
    };

    return (_instance_handler.removeInstance(instanceIndex, updater));
}

bool
VulkanModelPipeline::updateInstance(uint32_t instanceIndex,
                                    ModelInstanceInfo const &info)
{
    auto updater = [&](uint32_t index,
                       ModelInstanceInfo const &inst_info) -> void {
        _set_instance_matrix_on_gpu(index, inst_info);
    };

    return (_instance_handler.updateInstance(instanceIndex, info, updater));
}

bool
VulkanModelPipeline::getInstance(uint32_t instanceIndex,
                                 ModelInstanceInfo &info)
{
    return (_instance_handler.getInstance(instanceIndex, info));
}

void
VulkanModelPipeline::generateCommands(VkCommandBuffer cmdBuffer,
                                      size_t descriptorSetIndex)
{
    for (auto &it : _pipeline_meshes) {
        // Vertex related values
        VkBuffer vertex_buffer[] = { it.buffer, it.buffer };
        VkDeviceSize offsets[] = { 0, it.instanceMatricesOffset };

        vkCmdBindPipeline(
          cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphic_pipeline);
        vkCmdBindVertexBuffers(cmdBuffer, 0, 2, vertex_buffer, offsets);
        vkCmdBindIndexBuffer(
          cmdBuffer, it.buffer, it.indicesOffset, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(cmdBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                _pipeline_layout,
                                0,
                                1,
                                &it.descriptorSets[descriptorSetIndex],
                                0,
                                nullptr);
        vkCmdDrawIndexed(cmdBuffer,
                         it.nbIndices,
                         _instance_handler.getCurrentInstanceNb(),
                         0,
                         0,
                         0);
    }
}

std::array<VkVertexInputBindingDescription, 2>
VulkanModelPipeline::_get_input_binding_description()
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
VulkanModelPipeline::_get_input_attribute_description()
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
    attribute_description[3].location = 3;
    attribute_description[3].offset = offsetof(Vertex, tangent);
    attribute_description[3].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[4].binding = 0;
    attribute_description[4].location = 4;
    attribute_description[4].offset = offsetof(Vertex, bitangent);
    attribute_description[4].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[5].binding = 1;
    attribute_description[5].location = 5;
    attribute_description[5].offset = 0;
    attribute_description[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[6].binding = 1;
    attribute_description[6].location = 6;
    attribute_description[6].offset = sizeof(glm::vec4);
    attribute_description[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[7].binding = 1;
    attribute_description[7].location = 7;
    attribute_description[7].offset = sizeof(glm::vec4) * 2;
    attribute_description[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[8].binding = 1;
    attribute_description[8].location = 8;
    attribute_description[8].offset = sizeof(glm::vec4) * 3;
    attribute_description[8].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    return (attribute_description);
}

void
VulkanModelPipeline::_create_descriptor_layout()
{
    VkDescriptorSetLayoutBinding system_ubo_layout_binding{};
    system_ubo_layout_binding.binding = 0;
    system_ubo_layout_binding.descriptorType =
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    system_ubo_layout_binding.descriptorCount = 1;
    system_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    system_ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding model_ubo_layout_binding{};
    model_ubo_layout_binding.binding = 1;
    model_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    model_ubo_layout_binding.descriptorCount = 1;
    model_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    model_ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 2;
    sampler_layout_binding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler_layout_binding.pImmutableSamplers = nullptr;

    std::array bindings{ system_ubo_layout_binding,
                         model_ubo_layout_binding,
                         sampler_layout_binding };

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = bindings.size();
    layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(
          _device, &layout_info, nullptr, &_descriptor_set_layout) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanModelPipeline: failed to create descriptor set layout");
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
          "VulkanModelPipeline: Failed to create pipeline layout");
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
    auto binding_description = _get_input_binding_description();
    auto attribute_description = _get_input_attribute_description();
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
    viewport.y = static_cast<float>(renderPass.swapChainExtent.height);
    viewport.height = -static_cast<float>(renderPass.swapChainExtent.height);
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
          "VulkanModelPipeline: Failed to create graphic pipeline");
    }

    vkDestroyShaderModule(_device, vert_shader, nullptr);
    vkDestroyShaderModule(_device, frag_shader, nullptr);
}

VulkanModelPipeline::VulkanModelPipelineMesh
VulkanModelPipeline::_create_pipeline_mesh(Mesh const &mesh,
                                           std::string const &modelFolder,
                                           VulkanTextureManager &textureManager,
                                           uint32_t currentSwapChainNbImg)
{
    VulkanModelPipelineMesh pipeline_mesh{};

    pipeline_mesh.meshCenter = mesh.center;
    pipeline_mesh.diffuseTexture = textureManager.loadAndGetTexture(
      modelFolder + "/" + mesh.material.tex_diffuse_name);

    pipeline_mesh.verticesSize = sizeof(Vertex) * mesh.vertex_list.size();
    pipeline_mesh.nbIndices = mesh.indices.size();
    pipeline_mesh.indicesSize = sizeof(uint32_t) * mesh.indices.size();
    VkDeviceSize instance_matrices_size =
      sizeof(glm::mat4) * _instance_handler.getMaxInstanceNb();

    pipeline_mesh.instanceMatricesOffset = pipeline_mesh.verticesSize;
    pipeline_mesh.indicesOffset =
      pipeline_mesh.verticesSize + instance_matrices_size;
    pipeline_mesh.uboOffset = pipeline_mesh.verticesSize +
                              instance_matrices_size +
                              pipeline_mesh.indicesSize;
    // UBO offset are required to be aligned with
    // minUniformBufferOffsetAlignment prop
    auto ubo_alignment = getMinUniformBufferOffsetAlignment(_physical_device);
    pipeline_mesh.singleUboSize =
      (sizeof(ModelPipelineUbo) > ubo_alignment)
        ? sizeof(ModelPipelineUbo) + sizeof(ModelPipelineUbo) % ubo_alignment
        : ubo_alignment;
    VkDeviceSize model_ubo_size =
      pipeline_mesh.singleUboSize * currentSwapChainNbImg;
    pipeline_mesh.uboOffset +=
      ubo_alignment - (pipeline_mesh.uboOffset % ubo_alignment);
    VkDeviceSize total_size = pipeline_mesh.uboOffset + model_ubo_size;

    // Ubo values
    ModelPipelineUbo m_ubo = { mesh.material.diffuse,
                               mesh.material.specular,
                               mesh.material.shininess };

    // CPU => GPU transfer buffer
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    createBuffer(
      _device, staging_buffer, total_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(_physical_device,
                   _device,
                   staging_buffer,
                   staging_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Copying data into staging buffer
    copyOnMappedMemory(_device,
                       staging_buffer_memory,
                       0,
                       pipeline_mesh.verticesSize,
                       mesh.vertex_list.data());
    copyOnMappedMemory(_device,
                       staging_buffer_memory,
                       pipeline_mesh.indicesOffset,
                       pipeline_mesh.indicesSize,
                       mesh.indices.data());
    for (size_t i = 0; i < currentSwapChainNbImg; ++i) {
        copyOnMappedMemory(_device,
                           staging_buffer_memory,
                           pipeline_mesh.uboOffset +
                             pipeline_mesh.singleUboSize * i,
                           sizeof(ModelPipelineUbo),
                           &m_ubo);
    }

    // GPU only memory
    createBuffer(
      _device,
      pipeline_mesh.buffer,
      total_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    allocateBuffer(_physical_device,
                   _device,
                   pipeline_mesh.buffer,
                   pipeline_mesh.memory,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(_device,
               _cmd_pool,
               _gfx_queue,
               pipeline_mesh.buffer,
               staging_buffer,
               total_size);

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_buffer_memory, nullptr);

    return (pipeline_mesh);
}

void
VulkanModelPipeline::_create_descriptor_pool(
  VulkanRenderPass const &renderPass,
  VulkanModelPipelineMesh &pipelineMesh)
{
    std::array<VkDescriptorPoolSize, 3> pool_size{};
    pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size[0].descriptorCount = renderPass.currentSwapChainNbImg;
    pool_size[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size[1].descriptorCount = renderPass.currentSwapChainNbImg;
    pool_size[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size[2].descriptorCount = renderPass.currentSwapChainNbImg;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = pool_size.size();
    pool_info.pPoolSizes = pool_size.data();
    pool_info.maxSets = renderPass.currentSwapChainNbImg;

    if (vkCreateDescriptorPool(
          _device, &pool_info, nullptr, &pipelineMesh.descriptorPool) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanModelPipeline: failed to create descriptor pool");
    }
}

void
VulkanModelPipeline::_create_descriptor_sets(
  VulkanRenderPass const &renderPass,
  VulkanModelPipelineMesh &pipelineMesh,
  VkBuffer systemUbo)
{
    std::vector<VkDescriptorSetLayout> layouts(renderPass.currentSwapChainNbImg,
                                               _descriptor_set_layout);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pipelineMesh.descriptorPool;
    alloc_info.descriptorSetCount = renderPass.currentSwapChainNbImg;
    alloc_info.pSetLayouts = layouts.data();

    pipelineMesh.descriptorSets.resize(renderPass.currentSwapChainNbImg);
    if (vkAllocateDescriptorSets(
          _device, &alloc_info, pipelineMesh.descriptorSets.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanModelPipeline: failed to create descriptor sets");
    }

    for (size_t i = 0; i < renderPass.currentSwapChainNbImg; ++i) {
        std::array<VkWriteDescriptorSet, 3> descriptor_write{};

        // System UBO
        VkDescriptorBufferInfo system_buffer_info{};
        system_buffer_info.buffer = systemUbo;
        system_buffer_info.offset = 0;
        system_buffer_info.range = sizeof(SystemUbo);
        descriptor_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[0].dstSet = pipelineMesh.descriptorSets[i];
        descriptor_write[0].dstBinding = 0;
        descriptor_write[0].dstArrayElement = 0;
        descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write[0].descriptorCount = 1;
        descriptor_write[0].pBufferInfo = &system_buffer_info;
        descriptor_write[0].pImageInfo = nullptr;
        descriptor_write[0].pTexelBufferView = nullptr;

        // Model UBO
        VkDescriptorBufferInfo model_buffer_info{};
        model_buffer_info.buffer = pipelineMesh.buffer;
        model_buffer_info.offset =
          pipelineMesh.uboOffset + pipelineMesh.singleUboSize * i;
        model_buffer_info.range = sizeof(ModelPipelineUbo);
        descriptor_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[1].dstSet = pipelineMesh.descriptorSets[i];
        descriptor_write[1].dstBinding = 1;
        descriptor_write[1].dstArrayElement = 0;
        descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write[1].descriptorCount = 1;
        descriptor_write[1].pBufferInfo = &model_buffer_info;
        descriptor_write[1].pImageInfo = nullptr;
        descriptor_write[1].pTexelBufferView = nullptr;

        // Texture
        VkDescriptorImageInfo img_info{};
        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        img_info.imageView = pipelineMesh.diffuseTexture.texture_img_view;
        img_info.sampler = pipelineMesh.diffuseTexture.texture_sampler;
        descriptor_write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[2].dstSet = pipelineMesh.descriptorSets[i];
        descriptor_write[2].dstBinding = 2;
        descriptor_write[2].dstArrayElement = 0;
        descriptor_write[2].descriptorType =
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write[2].descriptorCount = 1;
        descriptor_write[2].pBufferInfo = nullptr;
        descriptor_write[2].pImageInfo = &img_info;
        descriptor_write[2].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(_device,
                               descriptor_write.size(),
                               descriptor_write.data(),
                               0,
                               nullptr);
    }
}

void
VulkanModelPipeline::_set_instance_matrix_on_gpu(uint32_t bufferIndex,
                                                 ModelInstanceInfo const &info)
{
    // Staging buffer on CPU
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    createBuffer(_device,
                 staging_buffer,
                 sizeof(glm::mat4),
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(_physical_device,
                   _device,
                   staging_buffer,
                   staging_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    for (auto const &mesh : _pipeline_meshes) {
        auto instance_mat = _compute_instance_matrix(mesh.meshCenter, info);

        copyOnMappedMemory(
          _device, staging_buffer_memory, 0, sizeof(glm::mat4), &instance_mat);

        VkBufferCopy copy_region{};
        copy_region.size = sizeof(glm::mat4);
        copy_region.dstOffset =
          mesh.instanceMatricesOffset + sizeof(glm::mat4) * bufferIndex;
        copy_region.srcOffset = 0;

        // Copy on GPU
        copyBuffer(_device,
                   _cmd_pool,
                   _gfx_queue,
                   mesh.buffer,
                   staging_buffer,
                   copy_region);
    }

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_buffer_memory, nullptr);
}

glm::mat4
VulkanModelPipeline::_compute_instance_matrix(glm::vec3 const &meshCenter,
                                              ModelInstanceInfo const &info)
{
    auto instance_matrix = glm::mat4(1.0f);
    instance_matrix = glm::scale(instance_matrix, info.scale);
    instance_matrix = glm::translate(instance_matrix, -meshCenter);

    instance_matrix =
      glm::rotate(instance_matrix, info.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    instance_matrix =
      glm::rotate(instance_matrix, info.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    instance_matrix =
      glm::rotate(instance_matrix, info.roll, glm::vec3(0.0f, 0.0f, 1.0f));
    instance_matrix = glm::translate(
      instance_matrix, meshCenter - _model->getCenter() + info.position);

    return (instance_matrix);
}