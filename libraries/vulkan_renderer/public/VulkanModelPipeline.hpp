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
              uint32_t maxModelNb);
    void resize(VulkanRenderPass const &renderPass);
    void clear();

    uint32_t addInstance(VulkanModelInfo const &info);
    bool removeInstance(uint32_t index);
    bool updateInstance(uint32_t index, VulkanModelInfo const &info);

    void generateCommands();
    void updateViewPerspectiveMatrix(glm::mat4 const &mat);

  private:
    struct VulkanModelPipelineMesh
    {
        VkBuffer _gfx_buffer{};
        VkDeviceMemory _gfx_memory{};
        Texture _diffuse_texture{};
        size_t instanceMatricesOffset{};
        size_t indicesOffset{};
        size_t uboOffset{};
        std::vector<VkDescriptorSet> _descriptor_sets;
    };

    static std::array<VkVertexInputBindingDescription, 2>
    _get_binding_description();
    static std::array<VkVertexInputAttributeDescription, 9>
    _get_attribute_description();

    VkDevice _device{};

    // Vulkan related
    VkDescriptorSetLayout _descriptor_set_layout{};
    VkPipelineLayout _pipeline_layout{};
    VkPipeline _graphic_pipeline{};
    VkDescriptorPool _descriptor_pool{};
    std::vector<VulkanModelPipelineMesh> _meshes;

    inline void _create_descriptor_layout();
    inline void _create_pipeline_layout();
    inline void _create_gfx_pipeline(VulkanRenderPass const &renderPass);

    inline void _create_gfx_buffer();
    inline void _create_descriptor_pool();
    inline void _create_descriptor_sets();

    // Instance related
    static uint32_t instance_index;
    uint32_t _max_model_nb{};
    uint32_t _current_model_nb{};
    std::unordered_map<uint32_t, uint32_t> _index_to_buffer_pairing;
    std::vector<VulkanModelInfo> _model_instance_info;
};

#endif // SCOP_VULKAN_VULKANMODELPIPELINE_HPP
