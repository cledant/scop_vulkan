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
              uint32_t app_version,
              uint32_t engine_version);
    void clear();

  private:
    // Validation layers related
#ifdef NDEBUG
    static constexpr bool const enable_validation_layer = false;
#else
    static constexpr bool const enable_validation_layer = true;
#endif
    static constexpr std::array const VALIDATION_LAYERS{
        "VK_LAYER_KHRONOS_validation",
    };

    VkInstance instance;

    inline void _create_instance(char const *app_name,
                                 char const *engine_name,
                                 uint32_t app_version,
                                 uint32_t engine_version);

    static inline bool _check_validation_layer_support();
    static inline std::vector<char const *> _get_required_extensions();

    static inline void _setup_debug_info(
      VkDebugUtilsMessengerCreateInfoEXT &create_info);
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    _debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                    VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                    void *pUserData);
};

#endif // SCOP_VULKAN_VKRENDERER_HPP
