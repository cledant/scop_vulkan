#ifndef SCOP_VULKAN_UI_HPP
#define SCOP_VULKAN_UI_HPP

#include <optional>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"

struct UiEvent
{
    bool newModel = false;
    bool updatedCurrentModel = false;
};

class Ui final
{
  public:
    Ui() = default;
    ~Ui() = default;
    Ui(Ui const &src) = delete;
    Ui &operator=(Ui const &rhs) = delete;
    Ui(Ui &&src) = delete;
    Ui &operator=(Ui &&rhs) = delete;

    static void init(GLFWwindow *win);
    static void clear();

    UiEvent getUiEvent() const;

    // Trigger from keyboard
    void toggleModelInfo();
    void toggleShowFps();
    void toggleAbout();
    void toggleControl();
    void toggleDisplayUi();
    void toggleSelectModel();
    void toggleFullscreen();
    void toggleCameraMvt();
    void toggleModelParam();
    void toggleInvertCameraYAxis();

    void drawUi();

  private:
    bool _show_info_model = false;
    bool _show_info_fps = false;
    bool _about = false;
    bool _controls = false;
    bool _fullscreen = false;
    bool _display_ui = true;
    bool _select_model = false;
    bool _close_app = false;
    bool _toggle_camera_mvt = false;
    bool _model_params = false;
    bool _invert_camera_y_axis = false;

    UiEvent _events{};

    // Menu Bar
    void _draw_menu_bar();

    // Windows
    void _about_window();
    void _info_overview() const;
};

#endif // SCOP_VULKAN_UI_HPP
