#include "UiInfoOverview.hpp"

#include "imgui.h"

void
UiInfoOverview::draw(bool &fps, bool &model_info)
{
    static constexpr float const PADDING = 10.0f;
    static constexpr ImGuiWindowFlags const WIN_FLAGS =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    static ImVec2 const WIN_POS_PIVOT = { 1.0f, 0.0f };
    static constexpr float const WIN_ALPHA = 0.35f;

    if (model_info || fps) {
        ImGuiViewport const *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos{ (work_pos.x + work_size.x - PADDING),
                           (work_pos.y + PADDING) };

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, WIN_POS_PIVOT);
        ImGui::SetNextWindowBgAlpha(WIN_ALPHA);
        if (ImGui::Begin("Info Overview", nullptr, WIN_FLAGS)) {
            if (fps) {
                ImGui::Text("Put Avg and current fps here");
            }
            if (fps && model_info) {
                ImGui::Separator();
            }
            if (model_info) {
                ImGui::Text("Nb Vertices = %u\nNb Indices = %u\nNb Faces = %u",
                            _nb_vertices,
                            _nb_indices,
                            _nb_faces);
            }
            ImGui::End();
        }
    }
}

void
UiInfoOverview::setAvgFps(float avgFps)
{
    _avg_fps = avgFps;
}

void
UiInfoOverview::setCurrentFps(float currentFps)
{
    _current_fps = currentFps;
}

void
UiInfoOverview::setModelInfo(uint32_t nbVertices,
                             uint32_t nbIndices,
                             uint32_t nbFaces)
{
    _nb_vertices = nbVertices;
    _nb_faces = nbFaces;
    _nb_indices = nbIndices;
}
