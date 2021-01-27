#ifndef SCOP_VULKAN_VKRENDERER_HPP
#define SCOP_VULKAN_VKRENDERER_HPP

#include <vector>
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
    void draw();
    void deviceWaitIdle();

  private:
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

    // Instance init related
    inline void _create_instance(
      std::vector<char const *> const &required_extension);
    inline void _setup_vk_debug_msg();
    inline void _select_physical_device();
    inline void _create_graphic_queue();
    inline void _create_swap_chain(uint32_t fb_w, uint32_t fb_h);
    inline void _create_image_view();
    inline void _create_render_pass();
    inline void _create_gfx_pipeline();
    inline void _create_framebuffers();
    inline void _create_command_pool();
    inline void _create_command_buffers();
    inline void _create_render_sync_objects();

    // Clean related
    inline void _clear_swap_chain();

    // Dbg related
    static inline bool _check_validation_layer_support();
};

#endif // SCOP_VULKAN_VKRENDERER_HPP
