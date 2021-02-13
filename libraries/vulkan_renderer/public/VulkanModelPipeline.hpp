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

struct VulkanModelInfo
{
    glm::vec3 position{};
    float pitch{};
    float yaw{};
    float roll{};
};

struct ModelPipelineUbo
{
    glm::mat4 view_proj;
};

class VulkanModelPipeline
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
              uint32_t maxModelNb);
    void resize(uint32_t nbImgSwapChain);
    void clear();

    uint32_t addInstance(VulkanModelInfo const &info);
    bool removeInstance(uint32_t index);
    bool updateInstance(uint32_t index, VulkanModelInfo const &info);

    void generateCommands();
    void updateUbo(ModelPipelineUbo const &ubo);

  private:
    // Model related
    glm::vec3 _model_center;

    // Vulkan related
    VkDescriptorSetLayout _descriptor_set_layout{};
    VkPipelineLayout _pipeline_layout{};
    VkPipeline _graphic_pipeline{};
    VkBuffer _gfx_buffer{};
    VkDeviceMemory _gfx_memory{};
    Texture _tex;
    std::vector<glm::mat4> _translation_matrices;
    VkDescriptorPool _descriptor_pool{};
    std::vector<VkDescriptorSet> _descriptor_sets;

    inline void _create_descriptor_layout();
    inline void _create_gfx_pipeline();
    inline void _create_gfx_buffer();
    inline void _create_descriptor_pool();
    inline void _create_descriptor_sets();

    // Instance related
    static uint32_t instance_index;
    uint32_t _max_model_nb{};
    uint32_t _current_model_nb{};
    std::unordered_map<uint32_t, uint32_t> _index_to_buffer_pairing;
    std::vector<VulkanModelInfo> _model_instance_info;
    std::vector<uint32_t> _updated_index;
};

#endif // SCOP_VULKAN_VULKANMODELPIPELINE_HPP
