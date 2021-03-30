#ifndef SCOP_VULKAN_UI_HPP
#define SCOP_VULKAN_UI_HPP

#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "UiOpenModel.hpp"
#include "UiInfoOverview.hpp"

enum UiEventTypes
{
    UET_NEW_MODEL,
    UET_UPDATE_MODEL_PARAMS,
    UET_EXIT,
    UET_MOUSE_EXCLUSIVE,
    UET_INVERT_MOUSE_AXIS,
    UET_FULLSCREEN,
    UET_TOTAL_NB,
};

struct UiEvent
{
    bool events[UET_TOTAL_NB] = { false };
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

    void init(GLFWwindow *win);
    void clear();

    [[nodiscard]] UiEvent getUiEvent() const;

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

    void setModelInfo(uint32_t nbVertices,
                      uint32_t nbIndices,
                      uint32_t nbFaces);

    [[nodiscard]] std::string getModelFilepath() const;

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

    UiEvent _ui_events{};

    // Menu Bar
    void _draw_menu_bar();

    // Compute fps
    void _compute_fps();
    uint64_t _nb_frame{};
    std::chrono::steady_clock::time_point _avg_fps_time_ref;
    std::chrono::steady_clock::time_point _prev_frame_time_ref;

    // Windows
    void _about_window();
    UiInfoOverview _info_overview;
    UiOpenModel _open_model_window;
};

#endif // SCOP_VULKAN_UI_HPP
