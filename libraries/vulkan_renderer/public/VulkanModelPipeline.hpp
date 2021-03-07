#ifndef SCOP_VULKAN_VULKANMODELPIPELINE_HPP
#define SCOP_VULKAN_VULKANMODELPIPELINE_HPP

#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "glm/glm.hpp"

#include "Model.hpp"
#include "VulkanInstance.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanTextureManager.hpp"
#include "IndexedBuffer.hpp"
#include "ModelInstanceInfo.hpp"
#include "VulkanModelPipelineData.hpp"

class VulkanModelPipeline final
{
  public:
    VulkanModelPipeline() = default;
    ~VulkanModelPipeline() = default;
    VulkanModelPipeline(VulkanModelPipeline const &src) = delete;
    VulkanModelPipeline &operator=(VulkanModelPipeline const &rhs) = delete;
    VulkanModelPipeline(VulkanModelPipeline &&src) = delete;
    VulkanModelPipeline &operator=(VulkanModelPipeline &&rhs) = delete;

    void init(VulkanInstance const &vkInstance,
              VulkanRenderPass const &renderPass,
              Model const &model,
              VulkanTextureManager &texManager,
              VkBuffer systemUbo,
              uint32_t maxModelNb);
    void resize(VulkanRenderPass const &renderPass,
                VulkanTextureManager &texManager,
                VkBuffer systemUbo);
    void clear();

    uint32_t addInstance(ModelInstanceInfo const &info);
    bool removeInstance(uint32_t instanceIndex);
    bool updateInstance(uint32_t instanceIndex, ModelInstanceInfo const &info);
    bool getInstance(uint32_t instanceIndex, ModelInstanceInfo &info);

    void generateCommands(VkCommandBuffer cmdBuffer, size_t descriptorSetIndex);

  private:
    // Model related
    Model const *_model{};

    // Vulkan related
    VkDevice _device{};
    VkPhysicalDevice _physical_device{};
    VkCommandPool _cmd_pool{};
    VkQueue _gfx_queue{};
    VkDescriptorSetLayout _descriptor_set_layout{};
    VkPipelineLayout _pipeline_layout{};
    VkPipeline _graphic_pipeline{};
    VulkanModelPipelineData _pipeline_model;

    // Instance related
    IndexedBuffer<ModelInstanceInfo> _instance_handler;

    inline void _create_descriptor_layout();
    inline void _create_pipeline_layout();
    inline void _create_gfx_pipeline(VulkanRenderPass const &renderPass);
    inline VulkanModelPipelineData _create_pipeline_model(
      Model const &model,
      std::string const &modelFolder,
      VulkanTextureManager &textureManager,
      uint32_t currentSwapChainNbImg);
    inline void _create_descriptor_pool(VulkanRenderPass const &renderPass,
                                        VulkanModelPipelineData &pipelineData);
    inline void _create_descriptor_sets(VulkanRenderPass const &renderPass,
                                        VulkanModelPipelineData &pipelineData,
                                        VkBuffer systemUbo);
    inline void _set_instance_matrix_on_gpu(uint32_t bufferIndex,
                                            ModelInstanceInfo const &info);
};

#endif // SCOP_VULKAN_VULKANMODELPIPELINE_HPP
