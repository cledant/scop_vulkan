#ifndef SCOP_VULKAN_VKRENDERER_HPP
#define SCOP_VULKAN_VKRENDERER_HPP

#include "glm/glm.hpp"

#include <vector>
#include <array>
#include <string>
#include <vulkan/vulkan.h>

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
    void initInstance(VkSurfaceKHR surface, uint32_t fb_w, uint32_t fb_h);
    void resizeInstance(uint32_t fb_w, uint32_t fb_h);
    void clearInstance();
    [[nodiscard]] std::string const &getAppName() const;
    [[nodiscard]] uint32_t getAppVersion() const;
    [[nodiscard]] std::string const &getEngineName() const;
    [[nodiscard]] uint32_t getEngineVersion() const;

    // Render related
    void draw(glm::mat4 const &view_proj_mat);
    void deviceWaitIdle();

  private:
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;

        static std::array<VkVertexInputBindingDescription, 1>
        getBindingDescription()
        {
            std::array<VkVertexInputBindingDescription, 1>
              binding_description{};
            binding_description[0].binding = 0;
            binding_description[0].stride = sizeof(Vertex);
            binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            // binding_description[1].binding = 1;
            // binding_description[1].stride = sizeof(glm::mat4);
            // binding_description[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

            return (binding_description);
        }

        static std::array<VkVertexInputAttributeDescription, 2>
        getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 2>
              attribute_description{};

            attribute_description[0].binding = 0;
            attribute_description[0].location = 0;
            attribute_description[0].offset = 0;
            attribute_description[0].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_description[1].binding = 0;
            attribute_description[1].location = 1;
            attribute_description[1].offset = offsetof(Vertex, color);
            attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            // attribute_description[2].binding = 1;
            // attribute_description[2].location = 2;
            // attribute_description[2].offset = 0;
            // attribute_description[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            return (attribute_description);
        }
    };

    // Uniform related
    struct UniformBufferObject
    {
        glm::mat4 view_proj;
    };

    // Test triangle
    static constexpr std::array<Vertex, 4> const _test_triangle_verticies = {
        { { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
          { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
          { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
          { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } } }
    };
    static constexpr std::array<uint16_t, 6> const _test_triangle_indices = {
        0, 1, 2, 2, 3, 0
    };
    static constexpr std::array<glm::vec3, 1> const _test_triangles_position = {
        { { 0.0f, 0.0f, 0.0f } }
    };

    // Description related
    std::string _app_name;
    std::string _engine_name;
    uint32_t _app_version{};
    uint32_t _engine_version{};

    // Instance Related
    VkInstance _instance{};
    VkSurfaceKHR _surface{};
    VkDebugUtilsMessengerEXT _debug_messenger{};

    // Device related
    VkPhysicalDevice _physical_device{};
    VkDevice _device{};
    char _device_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE]{};

    // Queue related
    VkQueue _graphic_queue{};
    VkQueue _present_queue{};

    // Swap chain related
    VkSwapchainKHR _swap_chain{};
    VkFormat _swap_chain_image_format{};
    VkExtent2D _swap_chain_extent{};
    std::vector<VkImage> _swap_chain_images;
    std::vector<VkImageView> _swap_chain_image_views;
    std::vector<VkFramebuffer> _swap_chain_framebuffers;

    // Pipeline related
    VkRenderPass _render_pass{};
    VkDescriptorSetLayout _descriptor_set_layout{};
    VkPipelineLayout _pipeline_layout{};
    VkPipeline _graphic_pipeline{};

    // Command pool related
    VkCommandPool _command_pool{};
    std::vector<VkCommandBuffer> _command_buffers;

    // Render synchronization related
    static constexpr size_t const MAX_FRAME_INFLIGHT = 2;
    std::vector<VkSemaphore> _image_available_sem;
    std::vector<VkSemaphore> _render_finished_sem;
    std::vector<VkFence> _inflight_fence;
    size_t _current_frame{};
    std::vector<VkFence> _imgs_inflight_fence;

    // Buffer related
    VkBuffer _vertex_buffer{};
    VkDeviceMemory _vertex_buffer_memory{};
    VkBuffer _index_buffer{};
    VkDeviceMemory _index_buffer_memory{};

    // Ubo related
    std::vector<VkBuffer> _uniform_buffers;
    std::vector<VkDeviceMemory> _uniform_buffers_memory;
    VkDescriptorPool _descriptor_pool{};
    std::vector<VkDescriptorSet> _descriptor_sets;

    // Instance init related
    inline void _create_instance(
      std::vector<char const *> const &required_extension);
    inline void _setup_vk_debug_msg();
    inline void _select_physical_device();
    inline void _create_graphic_queue();
    inline void _create_swap_chain(uint32_t fb_w, uint32_t fb_h);
    inline void _create_image_view();
    inline void _create_render_pass();
    inline void _create_descriptor_layout();
    inline void _create_gfx_pipeline();
    inline void _create_framebuffers();
    inline void _create_command_pool();
    inline void _create_vertex_buffer();
    inline void _create_index_buffer();
    inline void _create_uniform_buffers();
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
};

#endif // SCOP_VULKAN_VKRENDERER_HPP
