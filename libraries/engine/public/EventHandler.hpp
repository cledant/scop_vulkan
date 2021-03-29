#ifndef SCOP_VULKAN_EVENTHANDLER_HPP
#define SCOP_VULKAN_EVENTHANDLER_HPP

#include <chrono>

#include "IOEvents.hpp"
#include "Camera.hpp"
#include "IOManager.hpp"
#include "Perspective.hpp"
#include "VulkanRenderer.hpp"
#include "Ui.hpp"
#include "EngineOptions.hpp"

class EventHandler final
{
  public:
    EventHandler() = default;
    ~EventHandler() = default;
    EventHandler(EventHandler const &src) = delete;
    EventHandler &operator=(EventHandler const &rhs) = delete;
    EventHandler(EventHandler &&src) = delete;
    EventHandler &operator=(EventHandler &&rhs) = delete;

    void setCamera(Camera *camera);
    void setIOManager(IOManager *io_manager);
    void setPerspectiveData(Perspective *perspective);
    void setVkRenderer(VulkanRenderer *renderer);
    void setUi(Ui *ui);
    void setInvertYAxis(bool val);

    void processEvents(IOEvents const &events);

  private:
    static constexpr double const TARGET_PLAYER_TICK = 20.0f;
    static constexpr float const SCALING_PER_SCROLL = MINIMAL_MODEL_SCALE;

    // Timer related
    static constexpr double const SYSTEM_TIMER_SECONDS = 1.0;
    static constexpr double const CONFIG_TIMER_SECONDS = 0.5;
    static constexpr double const ACTION_TIMER_SECONDS = 0.5;
    static constexpr double const FAST_ACTION_TIMER_SECONDS = 0.5;
    static constexpr double const TARGET_PLAYER_TICK_DURATION =
      1 / TARGET_PLAYER_TICK;

    enum EventTimersTypes
    {
        ET_SYSTEM = 0,
        ET_CONFIG,
        ET_LEFT_MOUSE,
        ET_MIDDLE_MOUSE,
        ET_RIGHT_MOUSE,
        ET_CAMERA,
        ET_NB_EVENT_TIMER_TYPES
    };

    struct EventTimers final
    {
        EventTimers();
        ~EventTimers() = default;

        std::array<uint8_t, ET_NB_EVENT_TIMER_TYPES> accept_event;
        std::array<uint8_t, ET_NB_EVENT_TIMER_TYPES> updated;
        std::array<std::chrono::steady_clock::time_point,
                   ET_NB_EVENT_TIMER_TYPES>
          time_ref;
        std::array<double, ET_NB_EVENT_TIMER_TYPES> timer_diff;
        std::array<double, ET_NB_EVENT_TIMER_TYPES> timer_values;
    };

    // Event handling functions
    inline void _mouse_exclusive();
    inline void _close_win_event();
    inline void _toggle_fullscreen();
    inline void _jump();
    inline void _crouch();
    inline void _front();
    inline void _back();
    inline void _right();
    inline void _left();
    inline void _left_mouse();
    inline void _middle_mouse();
    inline void _right_mouse();

    // Camera Related
    inline void _updateCamera(glm::vec2 const &mouse_pos);

    Camera *_camera{};
    IOManager *_io_manager{};
    Perspective *_perspective{};
    VulkanRenderer *_renderer{};
    Ui *_ui{};

    EventTimers _timers;

    glm::ivec3 _movements{};
    glm::vec2 _mouse_pos{};
    glm::vec2 _previous_mouse_pos{};

    bool _print_ui = true;
    bool _invert_y_axis = false;
};

#endif // SCOP_VULKAN_EVENTHANDLER_HPP
