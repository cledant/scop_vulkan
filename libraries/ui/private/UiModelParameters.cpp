#include "UiModelParameters.hpp"

#include <cmath>

#include "imgui.h"

bool
UiModelParameters::draw(bool &open)
{
    static constexpr float const PADDING = 10.0f;
    static constexpr ImGuiWindowFlags const WIN_FLAGS =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
      ImGuiWindowFlags_NoMove;
    static ImVec2 const WIN_POS_PIVOT = { 0.0f, 0.0f };
    bool trigger = false;

    if (open) {
        ImGuiViewport const *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;
        ImVec2 window_pos{ (work_pos.x + PADDING), (work_pos.y + PADDING) };

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, WIN_POS_PIVOT);
        if (ImGui::Begin("Model Parameters", &open, WIN_FLAGS)) {
            ImGui::Text("Orientation");
            trigger |= ImGui::SliderAngle("Yaw",
                                          &_yaw,
                                          -360.0f,
                                          360.0f,
                                          "%.0f degree",
                                          ImGuiSliderFlags_AlwaysClamp);
            trigger |= ImGui::SliderAngle("Tilt",
                                          &_pitch,
                                          -360.0f,
                                          360.0f,
                                          "%.0f degree",
                                          ImGuiSliderFlags_AlwaysClamp);
            trigger |= ImGui::SliderAngle("Roll",
                                          &_roll,
                                          -360.0f,
                                          360.0f,
                                          "%.0f degree",
                                          ImGuiSliderFlags_AlwaysClamp);
            ImGui::Separator();
            ImGui::Text("Scaling");
            trigger |= ImGui::SliderFloat("Scale",
                                          &_scale,
                                          0.001f,
                                          100.0f,
                                          "%.3f",
                                          ImGuiSliderFlags_AlwaysClamp |
                                            ImGuiSliderFlags_Logarithmic);
        }
        ImGui::End();
    }
    return (trigger);
}

void
UiModelParameters::resetAllParams()
{
    _yaw = 0.0f;
    _pitch = 0.0f;
    _roll = 0.0f;
    _scale = 1.0f;
}
float
UiModelParameters::getYaw() const
{
    return (_yaw);
}

float
UiModelParameters::getPitch() const
{
    return (_pitch);
}

float
UiModelParameters::getRoll() const
{
    return (_roll);
}

float
UiModelParameters::getScale() const
{
    return (_scale);
}
