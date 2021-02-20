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

struct ModelInstanceInfo final
{
    glm::vec3 position{};
    float pitch{};
    float yaw{};
    float roll{};
    glm::vec3 scale = glm::vec3(1.0f);
};

struct ModelPipelineUbo
{
    alignas(16) glm::mat4 view_proj{};
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
    void resize(VulkanRenderPass const &renderPass,
                VulkanTextureManager &texManager);
    void clear();

    uint32_t addInstance(ModelInstanceInfo const &info);
    bool removeInstance(uint32_t instanceIndex);
    bool updateInstance(uint32_t instanceIndex, ModelInstanceInfo const &info);

    void generateCommands(VkCommandBuffer cmdBuffer, size_t descriptorSetIndex);
    void updateViewProjMatrix(uint32_t img_index, glm::mat4 const &mat);

  private:
    struct VulkanModelPipelineMesh
    {
        VkBuffer buffer{};
        VkDeviceMemory memory{};
        Texture diffuseTexture{};
        VkDeviceSize verticesSize{};
        VkDeviceSize indicesSize{};
        VkDeviceSize nbIndices{};
        VkDeviceSize instanceMatricesOffset{};
        VkDeviceSize indicesOffset{};
        VkDeviceSize uboOffset{};
        VkDescriptorPool descriptorPool{};
        std::vector<VkDescriptorSet> descriptorSets;
    };

    static std::array<VkVertexInputBindingDescription, 2>
    _get_binding_description();
    static std::array<VkVertexInputAttributeDescription, 9>
    _get_attribute_description();

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
    std::vector<VulkanModelPipelineMesh> _pipeline_meshes;

    inline void _create_descriptor_layout();
    inline void _create_pipeline_layout();
    inline void _create_gfx_pipeline(VulkanRenderPass const &renderPass);
    inline VulkanModelPipelineMesh _create_pipeline_mesh(
      Mesh const &mesh,
      std::string const &modelFolder,
      VulkanTextureManager &textureManager,
      uint32_t currentSwapChainNbImg);
    inline void _create_descriptor_pool(VulkanRenderPass const &renderPass,
                                        VulkanModelPipelineMesh &pipelineMesh);
    inline void _create_descriptor_sets(VulkanRenderPass const &renderPass,
                                        VulkanModelPipelineMesh &pipelineMesh);
    inline void _update_ubo(uint32_t img_index,
                            void const *data,
                            VkDeviceSize dataSize,
                            VkDeviceSize dataOffset);
    inline void _set_instance_matrix_on_gpu(uint32_t bufferIndex,
                                            ModelInstanceInfo const &info);
    static inline glm::mat4 _compute_instance_matrix(
      ModelInstanceInfo const &info);

    // Instance related
    static uint32_t instance_index;
    uint32_t _max_model_nb{};
    uint32_t _current_model_nb{};
    std::unordered_map<uint32_t, uint32_t> _index_to_buffer_pairing;
    std::vector<ModelInstanceInfo> _model_instance_info;
    std::vector<uint32_t> _model_instance_index;
};

#endif // SCOP_VULKAN_VULKANMODELPIPELINE_HPP
