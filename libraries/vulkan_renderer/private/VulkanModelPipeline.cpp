#include "VulkanModelPipeline.hpp"

#include "glm/gtc/matrix_transform.hpp"

void
VulkanModelPipeline::init(VulkanInstance const &vkInstance,
                          VulkanRenderPass const &renderPass,
                          Model const &model,
                          VulkanTextureManager &texManager,
                          uint32_t maxModelNb)
{
    (void)vkInstance;
    (void)renderPass;
    (void)model;
    (void)texManager;
    (void)maxModelNb;
}

void
VulkanModelPipeline::resize(uint32_t nbImgSwapChain)
{
    (void)nbImgSwapChain;
}

void
VulkanModelPipeline::clear()
{}

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
VulkanModelPipeline::updateUbo(ModelPipelineUbo const &ubo)
{
    (void)ubo;
}

void
VulkanModelPipeline::_create_descriptor_layout()
{}

void
VulkanModelPipeline::_create_gfx_pipeline()
{}

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