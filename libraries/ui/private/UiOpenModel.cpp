#include "UiOpenModel.hpp"

#include "imgui.h"

bool
UiOpenModel::drawFilepathWindow(bool &open)
{
    static constexpr ImGuiWindowFlags const WIN_FLAGS =
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    static ImVec2 const WIN_SIZE = ImVec2(400, 80);
    static ImVec2 const WIN_POS_PIVOT = { 0.5f, 0.5f };
    static constexpr ImGuiInputTextFlags const INPUT_TEXT_FLAGS =
      ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_NoUndoRedo;
    bool trigger = false;

    if (open) {
        ImGuiViewport const *viewport = ImGui::GetMainViewport();
        auto viewport_center = viewport->GetCenter();
        ImVec2 window_pos{ viewport_center.x, viewport_center.y };

        ImGui::SetNextWindowSize(WIN_SIZE);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, WIN_POS_PIVOT);
        ImGui::Begin("Open Model", &open, WIN_FLAGS);

        bool keyboard_input_ended = ImGui::InputText("Model Filepath",
                                                     _filepath,
                                                     IM_ARRAYSIZE(_filepath),
                                                     INPUT_TEXT_FLAGS);
        if (_force_focus) {
            ImGui::SetKeyboardFocusHere(0);
            _force_focus = false;
        }
        if (ImGui::IsItemDeactivated() &&
            ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
            open = false;
            _force_focus = true;
        }
        bool ok_pressed = ImGui::Button("Ok");
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            open = false;
            _force_focus = true;
        }
        if (keyboard_input_ended || ok_pressed) {
            open = false;
            trigger = true;
            _force_focus = true;
        }
        ImGui::End();
    }
    return (trigger);
}

void
UiOpenModel::drawErrorWindow(bool &open)
{
    static constexpr ImGuiWindowFlags const WIN_FLAGS =
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoMove;
    static ImVec2 const WIN_POS_PIVOT = { 0.5f, 0.5f };

    if (open) {
        ImGuiViewport const *viewport = ImGui::GetMainViewport();
        auto viewport_center = viewport->GetCenter();
        ImVec2 window_pos{ viewport_center.x, viewport_center.y };

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, WIN_POS_PIVOT);
        ImGui::Begin("Error", &open, WIN_FLAGS);
        ImGui::Text("Failed to load: %s", _filepath);
        ImGui::End();
    }
}

std::string
UiOpenModel::getModelFilepath() const
{
    return (_filepath);
}