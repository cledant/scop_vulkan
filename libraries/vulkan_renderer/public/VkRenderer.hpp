#ifndef SCOP_VULKAN_VKRENDERER_HPP
#define SCOP_VULKAN_VKRENDERER_HPP

#include "glm/glm.hpp"

#include <vector>
#include <array>
#include <string>
#include <vulkan/vulkan.h>

#include "VkTextureManager.hpp"

class VkRenderer final
{
  public:
    VkRenderer() = default;
    ~VkRenderer() = default;
    VkRenderer(VkRenderer const &src) = delete;
    VkRenderer &operator=(VkRenderer const &rhs) = delete;
    VkRenderer(VkRenderer &&src) = delete;
    VkRenderer &operator=(VkRenderer &&rhs) = delete;

    // Instance related
    void createInstance(std::string &&app_name,
                        std::string &&engine_name,
                        uint32_t app_version,
                        uint32_t engine_version,
                        std::vector<char const *> &&required_extensions);
    [[nodiscard]] VkInstance getVkInstance() const;
    void initInstance(VkSurfaceKHR surface);
    void clearInstance();
    [[nodiscard]] std::string const &getAppName() const;
    [[nodiscard]] uint32_t getAppVersion() const;
    [[nodiscard]] std::string const &getEngineName() const;
    [[nodiscard]] uint32_t getEngineVersion() const;

    // Resources Related
    void initResources(uint32_t fb_w, uint32_t fb_h);
    void resizeResources(uint32_t fb_w, uint32_t fb_h);
    void clearResources();

    // Global
    void clearAll();

    // Render related
    void draw(glm::mat4 const &view_proj_mat);
    void deviceWaitIdle();

  private:
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 tex_coord;

        static std::array<VkVertexInputBindingDescription, 2>
        getBindingDescription()
        {
            std::array<VkVertexInputBindingDescription, 2>
              binding_description{};
            binding_description[0].binding = 0;
            binding_description[0].stride = sizeof(Vertex);
            binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            binding_description[1].binding = 1;
            binding_description[1].stride = sizeof(glm::mat4);
            binding_description[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

            return (binding_description);
        }

        static std::array<VkVertexInputAttributeDescription, 7>
        getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 7>
              attribute_description{};

            attribute_description[0].binding = 0;
            attribute_description[0].location = 0;
            attribute_description[0].offset = 0;
            attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;

            attribute_description[1].binding = 0;
            attribute_description[1].location = 1;
            attribute_description[1].offset = offsetof(Vertex, color);
            attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;

            attribute_description[2].binding = 0;
            attribute_description[2].location = 2;
            attribute_description[2].offset = offsetof(Vertex, tex_coord);
            attribute_description[2].format = VK_FORMAT_R32G32_SFLOAT;

            attribute_description[3].binding = 1;
            attribute_description[3].location = 3;
            attribute_description[3].offset = 0;
            attribute_description[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;

            attribute_description[4].binding = 1;
            attribute_description[4].location = 4;
            attribute_description[4].offset = sizeof(glm::vec4);
            attribute_description[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;

            attribute_description[5].binding = 1;
            attribute_description[5].location = 5;
            attribute_description[5].offset = sizeof(glm::vec4) * 2;
            attribute_description[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;

            attribute_description[6].binding = 1;
            attribute_description[6].location = 6;
            attribute_description[6].offset = sizeof(glm::vec4) * 3;
            attribute_description[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            return (attribute_description);
        }
    };

    // Uniform related
    struct UniformBufferObject
    {
        glm::mat4 view_proj;
    };

    // Test triangle
    static constexpr std::array<Vertex, 8> const _test_triangle_verticies = { {
      { { -0.5f, 1.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
      { { 0.5f, 1.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
      { { 0.5f, 1.0f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
      { { -0.5f, 1.0f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
    } };
    static constexpr std::array<uint32_t, 12> const _test_triangle_indices = {
        0, 1, 2, 2, 3, 0,
    };
    static constexpr std::array<glm::vec3, 4> _test_triangle_pos = { {
      { 2.0f, 0.0f, 2.0f },
      { -2.0f, 0.0f, 2.0f },
      { -2.0f, 0.0f, -2.0f },
      { 2.0f, 0.0f, -2.0f },
    } };

    // Description related
    std::string _app_name;
    std::string _engine_name;
    uint32_t _app_version{};
    uint32_t _engine_version{};

    // Instance Related
    VkInstance _instance{};
    VkSurfaceKHR _surface{};
    VkDebugUtilsMessengerEXT _debug_messenger{};
    VkPhysicalDevice _physical_device{};
    VkDevice _device{};
    char _device_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE]{};
    VkQueue _graphic_queue{};
    VkQueue _present_queue{};
    VkCommandPool _command_pool{};

    // Tex management related
    VkTextureManager _tex_manager;

    // Swap chain related
    uint32_t _old_swap_chain_nb_img{};
    uint32_t _current_swap_chain_nb_img{};
    VkSwapchainKHR _swap_chain{};
    VkFormat _swap_chain_image_format{};
    VkExtent2D _swap_chain_extent{};
    std::vector<VkImage> _swap_chain_images;
    std::vector<VkImageView> _swap_chain_image_views;
    std::vector<VkFramebuffer> _swap_chain_framebuffers;
    VkFormat _depth_format{};
    VkImage _depth_image{};
    VkDeviceMemory _depth_img_memory{};
    VkImageView _depth_img_view{};
    VkRenderPass _render_pass{};

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

    // Render synchronization related
    static constexpr size_t const MAX_FRAME_INFLIGHT = 2;
    std::vector<VkSemaphore> _image_available_sem;
    std::vector<VkSemaphore> _render_finished_sem;
    std::vector<VkFence> _inflight_fence;
    size_t _current_frame{};
    std::vector<VkFence> _imgs_inflight_fence;

    // Init related
    inline void _create_instance(
      std::vector<char const *> const &required_extension);
    inline void _setup_vk_debug_msg();
    inline void _select_physical_device();
    inline void _create_present_and_graphic_queue();
    inline void _create_swap_chain(uint32_t fb_w, uint32_t fb_h);
    inline void _create_image_view();
    inline void _create_render_pass();
    inline void _create_descriptor_layout();
    inline void _create_gfx_pipeline();
    inline void _create_framebuffers();
    inline void _create_command_pool();
    inline void _create_depth_resources();
    inline void _create_gfx_buffer();
    inline void _create_descriptor_pool();
    inline void _create_descriptor_sets();
    inline void _create_command_buffers();
    inline void _create_render_sync_objects();

    // Clean related
    inline void _clear_swap_chain();

    // Dbg related
    static inline bool _check_validation_layer_support();

    // Ubo related
    inline void _update_ubo(uint32_t img_index, glm::mat4 const &view_proj_mat);

    // Test instancing
    inline void _init_instances_matrices();
};

#endif // SCOP_VULKAN_VKRENDERER_HPP
