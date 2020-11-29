#ifndef SCOP_VULKAN_VKRENDERER_HPP
#define SCOP_VULKAN_VKRENDERER_HPP

#include <array>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VkRenderer final
{
  public:
    VkRenderer() = default;
    ~VkRenderer() = default;
    VkRenderer(VkRenderer const &src) = delete;
    VkRenderer &operator=(VkRenderer const &rhs) = delete;
    VkRenderer(VkRenderer &&src) = delete;
    VkRenderer &operator=(VkRenderer &&rhs) = delete;

    void init(char const *app_name,
              char const *engine_name,
              GLFWwindow *win,
              uint32_t app_version,
              uint32_t engine_version);
    void clear();

  private:
#ifdef NDEBUG
    static constexpr bool const ENABLE_VALIDATION_LAYER = false;
#else
    static constexpr bool const ENABLE_VALIDATION_LAYER = true;
#endif
    static constexpr std::array const VALIDATION_LAYERS{
        "VK_LAYER_KHRONOS_validation",
    };
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;

    VkSurfaceKHR _surface;
    VkPhysicalDevice _physical_device;
    VkDevice _device;
    char _device_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];

    VkQueue _graphic_queue;
    VkQueue _present_queue;

    VkSwapchainKHR _swap_chain;
    std::vector<VkImage> _swap_chain_images;
    VkFormat _swap_chain_image_format;
    VkExtent2D _swap_chain_extent;

    inline void _create_instance(char const *app_name,
                                 char const *engine_name,
                                 uint32_t app_version,
                                 uint32_t engine_version);
    inline void _setup_vk_debug_msg();
    inline void _select_physical_device();
    inline void _create_graphic_queue();
    inline void _create_swap_chain();

    static inline bool _check_validation_layer_support();
    static inline std::vector<char const *> _get_required_extensions();
};

#endif // SCOP_VULKAN_VKRENDERER_HPP
