#ifndef SCOP_VULKAN_UI_HPP
#define SCOP_VULKAN_UI_HPP

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"

class Ui final
{
  public:
    Ui() = default;
    ~Ui() = default;
    Ui(Ui const &src) = delete;
    Ui &operator=(Ui const &rhs) = delete;
    Ui(Ui &&src) = delete;
    Ui &operator=(Ui &&rhs) = delete;

    void init(GLFWwindow *win);
    void clear() const;

    void drawUi();
};

#endif // SCOP_VULKAN_UI_HPP
