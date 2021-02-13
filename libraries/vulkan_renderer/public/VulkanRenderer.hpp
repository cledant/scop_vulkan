#ifndef SCOP_VULKAN_VULKANRENDERER_HPP
#define SCOP_VULKAN_VULKANRENDERER_HPP

#include "glm/glm.hpp"

#include <vector>
#include <array>
#include <string>
#include <vulkan/vulkan.h>

#include "VulkanInstance.hpp"
#include "VulkanTextureManager.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanSync.hpp"
#include "VulkanModelPipeline.hpp"

class VulkanRenderer final
{
  public:
    VulkanRenderer() = default;
    ~VulkanRenderer() = default;
    VulkanRenderer(VulkanRenderer const &src) = delete;
    VulkanRenderer &operator=(VulkanRenderer const &rhs) = delete;
    VulkanRenderer(VulkanRenderer &&src) = delete;
    VulkanRenderer &operator=(VulkanRenderer &&rhs) = delete;

    // Instance related
    void createInstance(std::string &&app_name,
                        std::string &&engine_name,
                        uint32_t app_version,
                        uint32_t engine_version,
                        std::vector<char const *> &&required_extensions);
    [[nodiscard]] VkInstance getVkInstance() const;
    void init(VkSurfaceKHR surface, uint32_t win_w, uint32_t win_h);
    void resize(uint32_t win_w, uint32_t win_h);
    void clear();

    [[nodiscard]] std::string const &getAppName() const;
    [[nodiscard]] uint32_t getAppVersion() const;
    [[nodiscard]] std::string const &getEngineName() const;
    [[nodiscard]] uint32_t getEngineVersion() const;

    // Render related
    void draw(glm::mat4 const &view_proj_mat);
    void deviceWaitIdle();

  private:
    std::string _app_name;
    std::string _engine_name;
    uint32_t _app_version{};
    uint32_t _engine_version{};

    VulkanInstance _vk_instance;
    VulkanTextureManager _tex_manager;
    VulkanRenderPass _render_pass;
    VulkanSync _sync;
    VulkanModelPipeline _model_pipeline;

    // Pipeline + Model + model texture related
    VkDescriptorSetLayout _descriptor_set_layout{};
    VkPipelineLayout _pipeline_layout{};
    VkPipeline _graphic_pipeline{};
    VkBuffer _gfx_buffer{};
    VkDeviceMemory _gfx_memory{};
    Texture _tex;
    std::vector<glm::mat4> _translation_matrices;
    VkDescriptorPool _descriptor_pool{};
    std::vector<VkDescriptorSet> _descriptor_sets;

    // Drawing related
    std::vector<VkCommandBuffer> _command_buffers;

    // Gfx pipeline fct
    inline void _create_descriptor_layout();
    inline void _create_gfx_pipeline();
    inline void _create_gfx_buffer();
    inline void _create_descriptor_pool();
    inline void _create_descriptor_sets();

    // Draw related fct
    inline void _create_command_buffers();

    // Ubo related
    inline void _update_ubo(uint32_t img_index, glm::mat4 const &view_proj_mat);

    // Test instancing
    inline void _init_instances_matrices();
};

#endif // SCOP_VULKAN_VULKANRENDERER_HPP
