#ifndef SCOP_VULKAN_VULKANMODELPIPELINE_HPP
#define SCOP_VULKAN_VULKANMODELPIPELINE_HPP

#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "glm/glm.hpp"

#include "Model.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanModelRenderPass.hpp"
#include "VulkanTextureManager.hpp"
#include "IndexedBuffer.hpp"
#include "ModelInstanceInfo.hpp"
#include "VulkanModelPipelineData.hpp"
#include "VulkanModelRenderPass.hpp"

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
              VulkanSwapChain const &swapChain,
              Model const &model,
              VulkanTextureManager &texManager,
              VkBuffer systemUbo,
              uint32_t maxModelNb);
    void resize(VulkanSwapChain const &swapChain,
                VulkanTextureManager &texManager,
                VkBuffer systemUbo);
    void clear();

    uint32_t addInstance(ModelInstanceInfo const &info);
    bool removeInstance(uint32_t instanceIndex);
    bool updateInstance(uint32_t instanceIndex, ModelInstanceInfo const &info);
    bool getInstance(uint32_t instanceIndex, ModelInstanceInfo &info);
    VulkanModelRenderPass const &getVulkanModelRenderPass() const;
    bool isInit() const;

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
    VulkanModelRenderPass _pipeline_render_pass;

    // Instance related
    IndexedBuffer<ModelInstanceInfo> _instance_handler;

    inline void _create_descriptor_layout();
    inline void _create_pipeline_layout();
    inline void _create_gfx_pipeline(VulkanSwapChain const &swapChain);
    inline VulkanModelPipelineData _create_pipeline_model(
      Model const &model,
      std::string const &modelFolder,
      VulkanTextureManager &textureManager,
      uint32_t currentSwapChainNbImg);
    inline void _create_descriptor_pool(VulkanSwapChain const &swapChain,
                                        VulkanModelPipelineData &pipelineData);
    inline void _create_descriptor_sets(VulkanSwapChain const &swapChain,
                                        VulkanModelPipelineData &pipelineData,
                                        VkBuffer systemUbo);
    inline void _set_instance_matrix_on_gpu(uint32_t bufferIndex,
                                            ModelInstanceInfo const &info);
};

#endif // SCOP_VULKAN_VULKANMODELPIPELINE_HPP
