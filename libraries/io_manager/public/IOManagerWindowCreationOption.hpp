#ifndef SCOP_VULKAN_IOMANAGERWINDOWCREATIONOPTION_HPP
#define SCOP_VULKAN_IOMANAGERWINDOWCREATIONOPTION_HPP

#include <string>
#include <cstdint>

#include "glm/glm.hpp"

static constexpr glm::ivec2 const DEFAULT_WIN_SIZE = glm::ivec2(1280, 720);

struct IOManagerWindowCreationOption final
{
    uint8_t resizable;
    uint8_t cursor_hidden_on_window;
    uint8_t fullscreen;
    uint8_t mouse_exclusive;
    glm::ivec2 size;
    std::string name;
};

#endif // SCOP_VULKAN_IOMANAGERWINDOWCREATIONOPTION_HPP
