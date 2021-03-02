#ifndef SCOP_VULKAN_PERSPECTIVE_HPP
#define SCOP_VULKAN_PERSPECTIVE_HPP

#include <glm/glm.hpp>

struct Perspective final
{
    float ratio{};
    glm::vec2 near_far{};
    float fov{};
};

#endif // SCOP_VULKAN_PERSPECTIVE_HPP
