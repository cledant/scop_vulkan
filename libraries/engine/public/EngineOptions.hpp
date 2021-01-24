#ifndef SCOP_VULKAN_ENGINEOPTIONS_HPP
#define SCOP_VULKAN_ENGINEOPTIONS_HPP

#include <cstdint>
#include <string>

struct EngineOptions
{
    uint32_t seed;
    uint8_t invert_y_axis;
    uint8_t fullscreen;
    std::string app_name;
};

#endif // SCOP_VULKAN_ENGINEOPTIONS_HPP