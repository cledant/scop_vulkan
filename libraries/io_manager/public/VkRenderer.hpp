#ifndef SCOP_VULKAN_VKRENDERER_HPP
#define SCOP_VULKAN_VKRENDERER_HPP

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
    VkInstance instance;
};

#endif // SCOP_VULKAN_VKRENDERER_HPP
