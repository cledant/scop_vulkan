#ifndef SCOP_IOEVENTS_HPP
#define SCOP_IOEVENTS_HPP

#include <cstdint>
#include <array>

#include "glm/glm.hpp"

enum IOEventsTypes
{
    MOUSE_EXCLUSIVE = 0,
    ESCAPE,
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
    NB_IO_EVENTS,
};

struct IOEvents final
{
    std::array<uint8_t, NB_IO_EVENTS> events{};
    glm::vec2 mouse_position{};
    float mouse_scroll{};
};

#endif // SCOP_IOEVENTS_HPP
