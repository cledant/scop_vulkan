#ifndef SCOP_VULKAN_ENGINE_HPP
#define SCOP_VULKAN_ENGINE_HPP

#include "EngineOptions.hpp"
#include "IOManager.hpp"
#include "Camera.hpp"
#include "EventHandler.hpp"
#include "Perspective.hpp"
#include "VulkanRenderer.hpp"
#include "Model.hpp"

class Engine final
{
  public:
    Engine() = default;
    ~Engine() = default;
    Engine(Engine const &src) = delete;
    Engine &operator=(Engine const &rhs) = delete;
    Engine(Engine &&src) = delete;
    Engine &operator=(Engine &&rhs) = delete;

    void init(EngineOptions const &opts);
    void run();

  private:
    static constexpr glm::vec2 const DEFAULT_NEAR_FAR =
      glm::vec2(0.1f, 5000.0f);
    static constexpr float const DEFAULT_FOV = 45.0f;
    static constexpr glm::vec3 const START_POS = glm::vec3(0.0f, 0.0f, 2.0f);
    static constexpr float const START_YAW = -90.0f;
    static constexpr float const START_PITCH = 0.0f;

    inline void _compute_fps();

    IOManager _io_manager;
    VulkanRenderer _vk_renderer;
    Camera _camera;
    EventHandler _event_handler;
    Perspective _perspective_data{};
    Model _model;

    // Fps related
    uint64_t _nb_frame{};
    std::chrono::steady_clock::time_point _fps_count_timeref{};
    std::string _str_fps = "0";
};

#endif // SCOP_VULKAN_ENGINE_HPP