#include "VkRenderer.hpp"

#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <iostream>
#include <set>

#include "VkDebug.hpp"
#include "VkPhysicalDevice.hpp"
#include "VkSwapChain.hpp"
#include "VkShader.hpp"
#include "VkMemory.hpp"
#include "VkImage.hpp"

void
VkRenderer::createInstance(std::string &&app_name,
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
    _create_instance(required_extensions);
}

VkInstance
VkRenderer::getVkInstance() const
{
    return (_instance);
}

void
VkRenderer::initInstance(VkSurfaceKHR surface, uint32_t fb_w, uint32_t fb_h)
{
    assert(surface);

    _surface = surface;
    _setup_vk_debug_msg();
    _select_physical_device();
    _create_graphic_queue();
    _create_swap_chain(fb_w, fb_h);
    _create_image_view();
    _create_render_pass();
    _create_descriptor_layout();
    _create_gfx_pipeline();
    _create_framebuffers();
    _create_command_pool();
    _create_texture_image();
    _create_texture_image_view();
    _create_texture_sampler();
    _create_vertex_buffer();
    _create_index_buffer();
    _create_uniform_buffers();
    _create_descriptor_pool();
    _create_descriptor_sets();
    _create_command_buffers();
    _create_render_sync_objects();
}

void
VkRenderer::resizeInstance(uint32_t fb_w, uint32_t fb_h)
{
    vkDeviceWaitIdle(_device);
    _clear_swap_chain();
    _create_swap_chain(fb_w, fb_h);
    _create_image_view();
    _create_render_pass();
    _create_gfx_pipeline();
    _create_framebuffers();
    _create_uniform_buffers();
    _create_descriptor_pool();
    _create_descriptor_sets();
    _create_command_buffers();
}

void
VkRenderer::clearInstance()
{
    _clear_swap_chain();
    vkDestroySampler(_device, _texture_sampler, nullptr);
    vkDestroyImageView(_device, _texture_img_view, nullptr);
    vkDestroyImage(_device, _texture_img, nullptr);
    vkFreeMemory(_device, _texture_img_memory, nullptr);
    vkDestroyBuffer(_device, _index_buffer, nullptr);
    vkFreeMemory(_device, _index_buffer_memory, nullptr);
    vkDestroyBuffer(_device, _vertex_buffer, nullptr);
    vkFreeMemory(_device, _vertex_buffer_memory, nullptr);
    for (size_t i = 0; i < MAX_FRAME_INFLIGHT; ++i) {
        vkDestroySemaphore(_device, _image_available_sem[i], nullptr);
        vkDestroySemaphore(_device, _render_finished_sem[i], nullptr);
        vkDestroyFence(_device, _inflight_fence[i], nullptr);
    }
    vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
    vkDestroyCommandPool(_device, _command_pool, nullptr);
    vkDestroyDevice(_device, nullptr);
    if constexpr (ENABLE_VALIDATION_LAYER) {
        destroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
    }
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
}

std::string const &
VkRenderer::getAppName() const
{
    return (_app_name);
}

uint32_t
VkRenderer::getAppVersion() const
{
    return (_app_version);
}

std::string const &
VkRenderer::getEngineName() const
{
    return (_engine_name);
}

uint32_t
VkRenderer::getEngineVersion() const
{
    return (_engine_version);
}

// Render Related
void
VkRenderer::draw(glm::mat4 const &view_proj_mat)
{
    vkWaitForFences(
      _device, 1, &_inflight_fence[_current_frame], VK_TRUE, UINT64_MAX);

    uint32_t img_index;
    auto result = vkAcquireNextImageKHR(_device,
                                        _swap_chain,
                                        UINT64_MAX,
                                        _image_available_sem[_current_frame],
                                        VK_NULL_HANDLE,
                                        &img_index);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return;
    }

    if (_imgs_inflight_fence[img_index] != VK_NULL_HANDLE) {
        vkWaitForFences(
          _device, 1, &_imgs_inflight_fence[img_index], VK_TRUE, UINT64_MAX);
    }
    _imgs_inflight_fence[img_index] = _inflight_fence[_current_frame];

    _update_ubo(img_index, view_proj_mat);

    VkSemaphore wait_sems[] = { _image_available_sem[_current_frame] };
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    VkSemaphore sig_sems[] = { _render_finished_sem[_current_frame] };
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_sems;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &_command_buffers[img_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = sig_sems;

    vkResetFences(_device, 1, &_inflight_fence[_current_frame]);
    if (vkQueueSubmit(
          _graphic_queue, 1, &submit_info, _inflight_fence[_current_frame]) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: Failed to submit draw command buffer");
    }

    VkSwapchainKHR swap_chains[] = { _swap_chain };
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = sig_sems;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &img_index;
    present_info.pResults = nullptr;
    vkQueuePresentKHR(_present_queue, &present_info);
    _current_frame = (_current_frame + 1) % MAX_FRAME_INFLIGHT;
}

void
VkRenderer::deviceWaitIdle()
{
    vkDeviceWaitIdle(_device);
}

// Instance init related
void
VkRenderer::_create_instance(
  std::vector<char const *> const &required_extension)
{
    if (ENABLE_VALIDATION_LAYER && !_check_validation_layer_support()) {
        throw std::runtime_error("VkRenderer: Validation layer not available");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = _app_name.c_str();
    app_info.applicationVersion = _app_version;
    app_info.pEngineName = _engine_name.c_str();
    app_info.engineVersion = _engine_version;
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extension.size());
    create_info.ppEnabledExtensionNames = required_extension.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    if constexpr (ENABLE_VALIDATION_LAYER) {
        setupVkDebugInfo(debug_create_info);
        create_info.enabledLayerCount =
          static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        create_info.pNext = &debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: Failed to create instance");
    }
}

void
VkRenderer::_setup_vk_debug_msg()
{
    if constexpr (!ENABLE_VALIDATION_LAYER) {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT dbg_info{};
    setupVkDebugInfo(dbg_info);

    createDebugUtilsMessengerEXT(
      _instance, &dbg_info, nullptr, &_debug_messenger);
}

void
VkRenderer::_select_physical_device()
{
    uint32_t nb_physical_device = 0;
    vkEnumeratePhysicalDevices(_instance, &nb_physical_device, nullptr);
    if (!nb_physical_device) {
        throw std::runtime_error("VkRenderer: No physical device");
    }

    std::vector<VkPhysicalDevice> devices(nb_physical_device);
    vkEnumeratePhysicalDevices(_instance, &nb_physical_device, devices.data());

    _physical_device = selectBestDevice(devices, _surface);
    if (_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("VkRenderer: No Suitable device found");
    }
    getDeviceName(_device_name, _physical_device);
    std::cout << "Device: " << _device_name << std::endl;
}

void
VkRenderer::_create_graphic_queue()
{
    auto dfr = getDeviceRequirement(_physical_device, _surface);
    std::set<uint32_t> queue_families = { dfr.graphic_queue_index.value(),
                                          dfr.present_queue_index.value() };
    std::vector<VkDeviceQueueCreateInfo> vec_queue_create_info;

    // Graphic queue info
    float queue_priority = 1.0f;
    for (auto const &it : queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = it;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        vec_queue_create_info.emplace_back(queue_create_info);
    }

    // Device info
    VkPhysicalDeviceFeatures physical_device_features{};
    physical_device_features.geometryShader = VK_TRUE;
    physical_device_features.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = vec_queue_create_info.data();
    device_create_info.queueCreateInfoCount = vec_queue_create_info.size();
    device_create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
    device_create_info.enabledExtensionCount = DEVICE_EXTENSIONS.size();
    if constexpr (ENABLE_VALIDATION_LAYER) {
        device_create_info.enabledLayerCount =
          static_cast<uint32_t>(VALIDATION_LAYERS.size());
        device_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        device_create_info.enabledLayerCount = 0;
    }
    device_create_info.pEnabledFeatures = &physical_device_features;

    // Device creation
    if (vkCreateDevice(
          _physical_device, &device_create_info, nullptr, &_device) !=
        VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: Failed to create logical device");
    }
    vkGetDeviceQueue(
      _device, dfr.graphic_queue_index.value(), 0, &_graphic_queue);
    vkGetDeviceQueue(
      _device, dfr.present_queue_index.value(), 0, &_present_queue);
}

void
VkRenderer::_create_swap_chain(uint32_t fb_w, uint32_t fb_h)
{
    // Creating swap chain
    VkExtent2D actual_extent = { fb_w, fb_h };

    auto scs = getSwapChainSupport(_physical_device, _surface, actual_extent);
    if (!scs.isValid()) {
        throw std::runtime_error("VkRenderer: SwapChain error");
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
    if (vkCreateSwapchainKHR(_device, &create_info, nullptr, &_swap_chain) !=
        VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: Failed to create swap chain");
    }

    // Retrieving img buffer + keeping info
    uint32_t nb_img_sc;
    vkGetSwapchainImagesKHR(_device, _swap_chain, &nb_img_sc, nullptr);
    _swap_chain_images.resize(nb_img_sc);
    vkGetSwapchainImagesKHR(
      _device, _swap_chain, &nb_img_sc, _swap_chain_images.data());
    _swap_chain_extent = scs.extent;
    _swap_chain_image_format = scs.surface_format.value().format;
}

void
VkRenderer::_create_image_view()
{
    _swap_chain_image_views.resize(_swap_chain_images.size());
    for (size_t i = 0; i < _swap_chain_images.size(); ++i) {
        _swap_chain_image_views[i] = createImageView(
          _device, _swap_chain_images[i], _swap_chain_image_format);
    }
}

void
VkRenderer::_create_render_pass()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format = _swap_chain_image_format;
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

    VkSubpassDependency sub_dep{};
    sub_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    sub_dep.dstSubpass = 0;
    sub_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    sub_dep.srcAccessMask = 0;
    sub_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    sub_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &sub_dep;

    if (vkCreateRenderPass(
          _device, &render_pass_info, nullptr, &_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: failed to create render pass");
    }
}

void
VkRenderer::_create_descriptor_layout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_layout_binding;

    if (vkCreateDescriptorSetLayout(
          _device, &layout_info, nullptr, &_descriptor_set_layout) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: failed to create descriptor set layout");
    }
}

void
VkRenderer::_create_gfx_pipeline()
{
    // Shaders
    auto vert_shader =
      loadShader(_device, "resources/shaders/trivial/trivial.vert.spv");
    auto frag_shader =
      loadShader(_device, "resources/shaders/trivial/trivial.frag.spv");

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
    viewport.height = _swap_chain_extent.height;
    viewport.width = _swap_chain_extent.width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = _swap_chain_extent;

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
    rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
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
    if (vkCreatePipelineLayout(
          _device, &pipeline_layout_info, nullptr, &_pipeline_layout) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: Failed to create pipeline layout");
    }

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
    gfx_pipeline_info.pDepthStencilState = nullptr;
    gfx_pipeline_info.pColorBlendState = &color_blending_info;
    gfx_pipeline_info.pDynamicState = nullptr;
    gfx_pipeline_info.layout = _pipeline_layout;
    gfx_pipeline_info.renderPass = _render_pass;
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
          "VkRenderer: Failed to create graphic pipeline");
    }

    vkDestroyShaderModule(_device, vert_shader, nullptr);
    vkDestroyShaderModule(_device, frag_shader, nullptr);
}

void
VkRenderer::_create_framebuffers()
{
    _swap_chain_framebuffers.resize(_swap_chain_image_views.size());

    size_t i = 0;
    for (auto const &it : _swap_chain_image_views) {
        VkImageView sciv[] = { it };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = _render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = sciv;
        framebuffer_info.width = _swap_chain_extent.width;
        framebuffer_info.height = _swap_chain_extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(_device,
                                &framebuffer_info,
                                nullptr,
                                &_swap_chain_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error(
              "VkRenderer: Failed to create framebuffer");
        }
        ++i;
    }
}

void
VkRenderer::_create_command_pool()
{
    DeviceRequirement dr{};
    getDeviceQueues(_physical_device, _surface, dr);

    VkCommandPoolCreateInfo command_pool_info{};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = dr.graphic_queue_index.value();
    command_pool_info.flags = 0;

    if (vkCreateCommandPool(
          _device, &command_pool_info, nullptr, &_command_pool) != VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: Failed to create command pool");
    }
}

void
VkRenderer::_create_texture_image()
{
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    int img_w = 0;
    int img_h = 0;

    loadTextureInBuffer(_physical_device,
                        _device,
                        "resources/texture/texture.jpg",
                        staging_buffer,
                        staging_buffer_memory,
                        img_w,
                        img_h);
    createImage(_device,
                _texture_img,
                img_w,
                img_h,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    allocateImage(_physical_device,
                  _device,
                  _texture_img,
                  _texture_img_memory,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    transitionImageLayout(_device,
                          _command_pool,
                          _graphic_queue,
                          _texture_img,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(_device,
                      _command_pool,
                      _graphic_queue,
                      staging_buffer,
                      _texture_img,
                      img_w,
                      img_h);
    transitionImageLayout(_device,
                          _command_pool,
                          _graphic_queue,
                          _texture_img,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_buffer_memory, nullptr);
}

void
VkRenderer::_create_texture_image_view()
{
    _texture_img_view =
      createImageView(_device, _texture_img, VK_FORMAT_R8G8B8A8_SRGB);
}

void
VkRenderer::_create_texture_sampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_physical_device, &properties);
    float aniso = (properties.limits.maxSamplerAnisotropy > 16.0f)
                    ? 16.0f
                    : properties.limits.maxSamplerAnisotropy;

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = aniso;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    if (vkCreateSampler(_device, &sampler_info, nullptr, &_texture_sampler) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: failed to create texture sampler");
    }
}

void
VkRenderer::_create_vertex_buffer()
{
    VkDeviceSize size =
      sizeof(_test_triangle_verticies[0]) * _test_triangle_verticies.size();

    // CPU => GPU transfer buffer
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    createBuffer(
      _device, staging_buffer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(_physical_device,
                   _device,
                   staging_buffer,
                   staging_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data{};
    vkMapMemory(_device, staging_buffer_memory, 0, size, 0, &data);
    memcpy(data, _test_triangle_verticies.data(), size);
    vkUnmapMemory(_device, staging_buffer_memory);

    // GPU only memory
    createBuffer(_device,
                 _vertex_buffer,
                 size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    allocateBuffer(_physical_device,
                   _device,
                   _vertex_buffer,
                   _vertex_buffer_memory,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(_device,
               _command_pool,
               _present_queue,
               _vertex_buffer,
               staging_buffer,
               size);

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_buffer_memory, nullptr);
}

void
VkRenderer::_create_index_buffer()
{
    VkDeviceSize size =
      sizeof(_test_triangle_indices[0]) * _test_triangle_indices.size();

    // CPU => GPU transfer buffer
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    createBuffer(
      _device, staging_buffer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocateBuffer(_physical_device,
                   _device,
                   staging_buffer,
                   staging_buffer_memory,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data{};
    vkMapMemory(_device, staging_buffer_memory, 0, size, 0, &data);
    memcpy(data, _test_triangle_indices.data(), size);
    vkUnmapMemory(_device, staging_buffer_memory);

    // GPU only memory
    createBuffer(_device,
                 _index_buffer,
                 size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    allocateBuffer(_physical_device,
                   _device,
                   _index_buffer,
                   _index_buffer_memory,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(_device,
               _command_pool,
               _present_queue,
               _index_buffer,
               staging_buffer,
               size);

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_buffer_memory, nullptr);
}

void
VkRenderer::_create_uniform_buffers()
{
    VkDeviceSize size = sizeof(UniformBufferObject);

    _uniform_buffers.resize(_swap_chain_framebuffers.size());
    _uniform_buffers_memory.resize(_swap_chain_framebuffers.size());

    for (size_t i = 0; i < _swap_chain_framebuffers.size(); ++i) {
        createBuffer(_device,
                     _uniform_buffers[i],
                     size,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        allocateBuffer(_physical_device,
                       _device,
                       _uniform_buffers[i],
                       _uniform_buffers_memory[i],
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

void
VkRenderer::_create_descriptor_pool()
{
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = _swap_chain_framebuffers.size();
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = _swap_chain_framebuffers.size();

    if (vkCreateDescriptorPool(
          _device, &pool_info, nullptr, &_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: failed to create descriptor pool");
    }
}

void
VkRenderer::_create_descriptor_sets()
{
    std::vector<VkDescriptorSetLayout> layouts(_swap_chain_framebuffers.size(),
                                               _descriptor_set_layout);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = _descriptor_pool;
    alloc_info.descriptorSetCount = _swap_chain_framebuffers.size();
    alloc_info.pSetLayouts = layouts.data();

    _descriptor_sets.resize(_swap_chain_framebuffers.size());
    if (vkAllocateDescriptorSets(
          _device, &alloc_info, _descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error(
          "VkRenderer: failed to create descriptor sets");
    }

    for (size_t i = 0; i < _swap_chain_framebuffers.size(); ++i) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = _uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = _descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr;
        descriptor_write.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(_device, 1, &descriptor_write, 0, nullptr);
    }
}

void
VkRenderer::_create_command_buffers()
{
    _command_buffers.resize(_swap_chain_framebuffers.size());

    VkCommandBufferAllocateInfo cb_allocate_info{};
    cb_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cb_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cb_allocate_info.commandPool = _command_pool;
    cb_allocate_info.commandBufferCount = _command_buffers.size();

    if (vkAllocateCommandBuffers(
          _device, &cb_allocate_info, _command_buffers.data()) != VK_SUCCESS) {
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
        VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
        VkRenderPassBeginInfo rp_begin_info{};
        rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin_info.renderPass = _render_pass;
        rp_begin_info.framebuffer = _swap_chain_framebuffers[i];
        rp_begin_info.renderArea.offset = { 0, 0 };
        rp_begin_info.renderArea.extent = _swap_chain_extent;
        rp_begin_info.clearValueCount = 1;
        rp_begin_info.pClearValues = &clear_color;

        // Vertex related values
        VkBuffer vertex_buffer[] = { _vertex_buffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBeginRenderPass(it, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(
          it, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphic_pipeline);
        vkCmdBindVertexBuffers(it, 0, 1, vertex_buffer, offsets);
        vkCmdBindIndexBuffer(it, _index_buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(it,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                _pipeline_layout,
                                0,
                                1,
                                &_descriptor_sets[i],
                                0,
                                nullptr);
        vkCmdDrawIndexed(it, _test_triangle_indices.size(), 1, 0, 0, 0);
        vkCmdEndRenderPass(it);

        if (vkEndCommandBuffer(it) != VK_SUCCESS) {
            throw std::runtime_error(
              "VkRenderer: Failed to record command Buffer");
        }
        ++i;
    }
}

void
VkRenderer::_create_render_sync_objects()
{
    _image_available_sem.resize(MAX_FRAME_INFLIGHT);
    _render_finished_sem.resize(MAX_FRAME_INFLIGHT);
    _inflight_fence.resize(MAX_FRAME_INFLIGHT);
    _imgs_inflight_fence.resize(_swap_chain_images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo sem_info{};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAME_INFLIGHT; ++i) {
        if (vkCreateSemaphore(
              _device, &sem_info, nullptr, &_image_available_sem[i]) !=
              VK_SUCCESS ||
            vkCreateSemaphore(
              _device, &sem_info, nullptr, &_render_finished_sem[i]) !=
              VK_SUCCESS ||
            vkCreateFence(_device, &fence_info, nullptr, &_inflight_fence[i]) !=
              VK_SUCCESS) {
            throw std::runtime_error("VkRender: failed to create semaphores");
        }
    }
}

// Clean related
void
VkRenderer::_clear_swap_chain()
{
    size_t i = 0;
    for (auto &it : _swap_chain_framebuffers) {
        vkDestroyBuffer(_device, _uniform_buffers[i], nullptr);
        vkFreeMemory(_device, _uniform_buffers_memory[i], nullptr);
        vkDestroyFramebuffer(_device, it, nullptr);
        ++i;
    }
    vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
    vkFreeCommandBuffers(
      _device, _command_pool, _command_buffers.size(), _command_buffers.data());
    vkDestroyPipeline(_device, _graphic_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    vkDestroyRenderPass(_device, _render_pass, nullptr);
    for (auto iv : _swap_chain_image_views) {
        vkDestroyImageView(_device, iv, nullptr);
    }
    vkDestroySwapchainKHR(_device, _swap_chain, nullptr);
}

// Dbg related
bool
VkRenderer::_check_validation_layer_support()
{
    uint32_t nb_avail_layers;
    vkEnumerateInstanceLayerProperties(&nb_avail_layers, nullptr);

    std::vector<VkLayerProperties> avail_layers(nb_avail_layers);
    vkEnumerateInstanceLayerProperties(&nb_avail_layers, avail_layers.data());

    for (auto const &it_val_layer : VALIDATION_LAYERS) {
        bool layer_found = false;

        for (auto const &it_avail_layer : avail_layers) {
            if (!std::strcmp(it_avail_layer.layerName, it_val_layer)) {
                layer_found = true;
                break;
            }
        }
        if (!layer_found) {
            return (false);
        }
    }
    return (true);
}

void
VkRenderer::_update_ubo(uint32_t img_index, glm::mat4 const &view_proj_mat)
{
    UniformBufferObject ubo = { view_proj_mat };
    ubo.view_proj[1][1] = -ubo.view_proj[1][1];

    void *data = nullptr;
    vkMapMemory(
      _device, _uniform_buffers_memory[img_index], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(_device, _uniform_buffers_memory[img_index]);
}
