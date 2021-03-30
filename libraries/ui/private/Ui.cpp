#include "Ui.hpp"

void
Ui::init(GLFWwindow *win)
{
    assert(win);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(win, true);
}

void
Ui::clear()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

UiEvent
Ui::getUiEvent() const
{
    return (_ui_events);
}

void
Ui::toggleModelInfo()
{
    _show_info_model = !_show_info_model;
}

void
Ui::toggleShowFps()
{
    _show_info_fps = !_show_info_fps;
}

void
Ui::toggleAbout()
{
    _about = !_about;
}

void
Ui::toggleControl()
{
    _controls = !_controls;
}

void
Ui::toggleDisplayUi()
{
    _display_ui = !_display_ui;
}

void
Ui::toggleSelectModel()
{
    _select_model = !_select_model;
}

void
Ui::toggleFullscreen()
{
    _fullscreen = !_fullscreen;
}

void
Ui::toggleCameraMvt()
{
    _toggle_camera_mvt = !_toggle_camera_mvt;
}

void
Ui::toggleModelParam()
{
    _model_params = !_model_params;
}

void
Ui::toggleInvertCameraYAxis()
{
    _invert_camera_y_axis = !_invert_camera_y_axis;
}

void
Ui::drawUi()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (!_display_ui) {
        ImGui::Render();
        return;
    }

    _draw_menu_bar();
    _about_window();
    _info_overview.draw(_show_info_fps, _show_info_model);
    _ui_events.events[UET_NEW_MODEL] = _open_model_window.draw(_select_model);

    ImGui::Render();
}

void
Ui::setModelInfo(uint32_t nbVertices, uint32_t nbIndices, uint32_t nbFaces)
{
    _info_overview.setModelInfo(nbVertices, nbIndices, nbFaces);
}

std::string
Ui::getModelFilepath() const
{
    return (_open_model_window.getModelFilepath());
}

void
Ui::_draw_menu_bar()
{
    _ui_events = {};
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Model", "F2")) {
                _select_model = !_select_model;
            }
            ImGui::Separator();
            if ((_ui_events.events[UET_EXIT] =
                   ImGui::MenuItem("Exit", "F12"))) {
                _close_app = !_close_app;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Model Parameters", "F3")) {
                _model_params = !_model_params;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Controls")) {
            _ui_events.events[UET_MOUSE_EXCLUSIVE] = ImGui::MenuItem(
              "Toggle Camera Movement", "F4", &_toggle_camera_mvt);
            ImGui::Separator();
            _ui_events.events[UET_INVERT_MOUSE_AXIS] = ImGui::MenuItem(
              "Inverse Mouse Y Axis", "F5", &_invert_camera_y_axis);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Model Info", "F6", &_show_info_model);
            ImGui::Separator();
            ImGui::MenuItem("Show Fps", "F7", &_show_info_fps);
            ImGui::Separator();
            _ui_events.events[UET_FULLSCREEN] =
              ImGui::MenuItem("Fullscreen", "F8", &_fullscreen);
            ImGui::Separator();
            ImGui::MenuItem("Display UI", "F9", &_display_ui);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About", "F1")) {
                _about = !_about;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void
Ui::_about_window()
{
    static constexpr ImGuiWindowFlags const WIN_FLAGS =
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    static ImVec2 const WIN_SIZE = ImVec2(200, 80);
    static ImVec2 const WIN_POS_PIVOT = { 0.5f, 0.5f };

    if (_about) {
        ImGuiViewport const *viewport = ImGui::GetMainViewport();
        auto viewport_center = viewport->GetCenter();
        ImVec2 window_pos{ viewport_center.x, viewport_center.y };

        ImGui::SetNextWindowSize(WIN_SIZE);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, WIN_POS_PIVOT);
        ImGui::Begin("About", &_about, WIN_FLAGS);
        ImGui::Text("Scop Vulkan");
        ImGui::Separator();
        ImGui::Text("Version / Commit");
        ImGui::End();
    }
}
