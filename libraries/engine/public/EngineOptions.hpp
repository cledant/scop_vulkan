#ifndef SCOP_VULKAN_ENGINEOPTIONS_HPP
#define SCOP_VULKAN_ENGINEOPTIONS_HPP

#include <cstdint>
#include <string>

static constexpr float const MINIMAL_MODEL_SCALE = 0.01f;

struct EngineOptions
{
    uint8_t invert_y_axis{};
    uint8_t fullscreen{};
    std::string app_name;
    std::string model_path;
    float scale = 1.0f;
};

#endif // SCOP_VULKAN_ENGINEOPTIONS_HPP