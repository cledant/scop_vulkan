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
    _info_overview();

    if (_select_model) {
        static char filepath[2048] = { 0 };
        auto win_size = ImVec2(400, 80);
        ImGui::SetNextWindowSize(win_size);
        ImGuiWindowFlags window_flags =
          ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
        ImGuiViewport const *viewport = ImGui::GetMainViewport();
        auto viewport_center = viewport->GetCenter();
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = viewport_center.x;
        window_pos.y = viewport_center.y;
        window_pos_pivot.x = 0.5f;
        window_pos_pivot.y = 0.5f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::Begin("Open Model", &_select_model, window_flags);
        ImGuiInputTextFlags input_text_flags =
          ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_NoUndoRedo;
        bool ret = ImGui::InputText(
          "Model Filepath", filepath, IM_ARRAYSIZE(filepath), input_text_flags);
        if (ImGui::IsItemDeactivated() &&
            ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
            _select_model = false;
        }
        bool ret2 = ImGui::Button("Ok");
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            _select_model = false;
        }
        if (ret || ret2) {
            _select_model = false;
        }
        ImGui::End();
    }

    ImGui::Render();
}

void
Ui::_draw_menu_bar()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Model", "F2")) {
                _select_model = !_select_model;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "F12")) {
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
            ImGui::MenuItem(
              "Toggle Camera Movement", "F4", &_toggle_camera_mvt);
            ImGui::Separator();
            ImGui::MenuItem(
              "Inverse Mouse Y Axis", "F5", &_invert_camera_y_axis);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Model Info", "F6", &_show_info_model);
            ImGui::Separator();
            ImGui::MenuItem("Show Fps", "F7", &_show_info_fps);
            ImGui::Separator();
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

void
Ui::_info_overview() const
{
    static constexpr float const PADDING = 10.0f;
    static constexpr ImGuiWindowFlags const WIN_FLAGS =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    static ImVec2 const WIN_POS_PIVOT = { 1.0f, 0.0f };
    static constexpr float const WIN_ALPHA = 0.35f;

    if (_show_info_model || _show_info_fps) {
        ImGuiViewport const *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos{ (work_pos.x + work_size.x - PADDING),
                           (work_pos.y + PADDING) };

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, WIN_POS_PIVOT);
        ImGui::SetNextWindowBgAlpha(WIN_ALPHA);
        if (ImGui::Begin("Info Overview", nullptr, WIN_FLAGS)) {
            if (_show_info_fps) {
                ImGui::Text("Put Avg and current fps here");
            }
            if (_show_info_fps && _show_info_model) {
                ImGui::Separator();
            }
            if (_show_info_model) {
                ImGui::Text("Put Model info here");
            }
            ImGui::End();
        }
    }
}
