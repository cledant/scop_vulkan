#ifndef SCOP_IOEVENTS_HPP
#define SCOP_IOEVENTS_HPP

#include <cstdint>
#include <array>

#include "glm/glm.hpp"

enum IOEventsTypes
{
    MOUSE_EXCLUSIVE = 0,
    QUIT,
    FULLSCREEN,
    JUMP,
    CROUCH,
    FRONT,
    BACK,
    RIGHT,
    LEFT,
    LEFT_MOUSE,
    MIDDLE_MOUSE,
    RIGHT_MOUSE,
    OPEN_MODEL,
    SHOW_FPS,
    MODEL_PARAMETERS_EDIT,
    MODEL_INFO,
    DISPLAY_UI,
    ABOUT,
    INVERSE_Y_AXIS,
    NB_IO_EVENTS,
};

struct IOEvents final
{
    std::array<uint8_t, NB_IO_EVENTS> events{};
    glm::vec2 mouse_position{};
    float mouse_scroll{};
};

#endif // SCOP_IOEVENTS_HPP
