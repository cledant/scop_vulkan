#ifndef SCOP_VULKAN_VKRENDERER_HPP
#define SCOP_VULKAN_VKRENDERER_HPP

#include <vector>
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

    void createInstance(char const *app_name,
                        char const *engine_name,
                        uint32_t app_version,
                        uint32_t engine_version,
                        std::vector<char const *> &&required_extensions);
    [[nodiscard]] VkInstance getVkInstance() const;
    void initInstance(VkSurfaceKHR surface, uint32_t fb_w, uint32_t fb_h);
    void clear();

  private:
    VkInstance _instance{};
    VkDebugUtilsMessengerEXT _debug_messenger{};

    VkSurfaceKHR _surface{};
    VkPhysicalDevice _physical_device{};
    VkDevice _device{};
    char _device_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE]{};

    VkQueue _graphic_queue{};
    VkQueue _present_queue{};

    VkSwapchainKHR _swap_chain{};

    VkFormat _swap_chain_image_format{};
    VkExtent2D _swap_chain_extent{};
    std::vector<VkImage> _swap_chain_images;
    std::vector<VkImageView> _swap_chain_image_views;

    inline void _create_instance(
      char const *app_name,
      char const *engine_name,
      uint32_t app_version,
      uint32_t engine_version,
      std::vector<char const *> const &required_extension);
    inline void _setup_vk_debug_msg();
    inline void _select_physical_device();
    inline void _create_graphic_queue();
    inline void _create_swap_chain(uint32_t fb_w, uint32_t fb_h);
    inline void _create_image_view();

    static inline bool _check_validation_layer_support();
};

#endif // SCOP_VULKAN_VKRENDERER_HPP
