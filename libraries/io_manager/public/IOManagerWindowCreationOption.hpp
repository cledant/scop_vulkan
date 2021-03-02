#ifndef SCOP_VULKAN_IOMANAGERWINDOWCREATIONOPTION_HPP
#define SCOP_VULKAN_IOMANAGERWINDOWCREATIONOPTION_HPP

#include <string>
#include <cstdint>

#include "glm/glm.hpp"

static constexpr glm::ivec2 const DEFAULT_WIN_SIZE = glm::ivec2(1280, 720);

struct IOManagerWindowCreationOption final
{
    bool resizable{};
    bool cursor_hidden_on_window{};
    bool fullscreen{};
    bool mouse_exclusive{};
    glm::ivec2 size{};
    std::string win_name;
};

#endif // SCOP_VULKAN_IOMANAGERWINDOWCREATIONOPTION_HPP
