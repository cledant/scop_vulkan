#include "Ui.hpp"

#include "fmt/core.h"

void
Ui::init(GLFWwindow *win)
{
    assert(win);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(win, true);
}

void
Ui::clear() const
{
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void
Ui::drawUi()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool model_info = false;
    static bool show_fps = false;
    static bool about_win = false;
    static bool fullscreen = false;
    static bool select_model = false;
    static bool close_app = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Model", "F2")) {
                select_model = !select_model;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Esc", &close_app)) {
                close_app = !close_app;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Model Info", "F3", &model_info);
            ImGui::MenuItem("Show Fps", "F4", &show_fps);
            ImGui::MenuItem("Fullscreen", "F5", &fullscreen);
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("About", "F1", &about_win)) {
        }
        ImGui::EndMainMenuBar();
    }

    if (select_model) {
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
        ImGui::Begin("Open Model", &select_model, window_flags);
        ImGuiInputTextFlags input_text_flags =
          ImGuiInputTextFlags_EnterReturnsTrue |
          ImGuiInputTextFlags_NoUndoRedo | ImGuiInputTextFlags_CharsNoBlank;
        bool ret = ImGui::InputText(
          "Model Filepath", filepath, IM_ARRAYSIZE(filepath), input_text_flags);
        if (ImGui::IsItemDeactivated() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
            select_model = false;
        }
        bool ret2 = ImGui::Button("Ok");
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            select_model = false;
        }
        if (ret || ret2) {
            fmt::print("{}\n", filepath);
            select_model = false;
        }
        ImGui::End();
    }

    if (about_win) {
        auto win_size = ImVec2(200, 80);
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
        ImGui::Begin("About", &about_win, window_flags);
        ImGui::Text("Scop Vulkan");
        ImGui::Separator();
        ImGui::Text("Version / Commit");
        ImGui::End();
    }

    if (model_info || show_fps) {
        float const PAD = 10.0f;
        ImGuiWindowFlags window_flags =
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
          ImGuiWindowFlags_NoSavedSettings |
          ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
          ImGuiWindowFlags_NoMove;
        ImGuiViewport const *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos{};
        ImVec2 window_pos_pivot{};
        window_pos.x = (work_pos.x + work_size.x - PAD);
        window_pos.y = (work_pos.y + PAD);
        window_pos_pivot.x = 1.0f;
        window_pos_pivot.y = 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowBgAlpha(0.35f);
        if (ImGui::Begin("Info Overview", nullptr, window_flags)) {
            if (show_fps) {
                ImGui::Text("Put Avg and current fps here");
            }
            if (show_fps && model_info) {
                ImGui::Separator();
            }
            if (model_info) {
                ImGui::Text("Put Model info here");
            }
            ImGui::End();
        }
    }
    ImGui::Render();
}
