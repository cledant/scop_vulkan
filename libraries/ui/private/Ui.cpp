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
Ui::clear() const
{
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void
Ui::drawUi()
{
    static bool demo = true;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(&demo);
    ImGui::Render();
}
