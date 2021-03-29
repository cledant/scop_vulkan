#include "EventHandler.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <functional>

void
EventHandler::setCamera(Camera *camera)
{
    _camera = camera;
}

void
EventHandler::setIOManager(IOManager *io_manager)
{
    _io_manager = io_manager;
}

void
EventHandler::setPerspectiveData(Perspective *perspective)
{
    _perspective = perspective;
}

void
EventHandler::setVkRenderer(VulkanRenderer *renderer)
{
    _renderer = renderer;
}

void
EventHandler::setUi(Ui *ui)
{
    _ui = ui;
}

void
EventHandler::setInvertYAxis(bool val)
{
    _invert_y_axis = val;
}

void
EventHandler::processEvents(IOEvents const &events)
{
    assert(_camera);
    assert(_io_manager);
    assert(_perspective);
    assert(_renderer);
    assert(_ui);

    // Resetting movement tracking
    _movements = glm::ivec3(0);

    static const std::array<void (EventHandler::*)(), NB_IO_EVENTS>
      keyboard_events = {
          &EventHandler::_mouse_exclusive,
          &EventHandler::_close_win_event,
          &EventHandler::_toggle_fullscreen,
          &EventHandler::_jump,
          &EventHandler::_crouch,
          &EventHandler::_front,
          &EventHandler::_back,
          &EventHandler::_right,
          &EventHandler::_left,
          &EventHandler::_left_mouse,
          &EventHandler::_middle_mouse,
          &EventHandler::_right_mouse,
      };

    // Checking Timers
    auto now = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < ET_NB_EVENT_TIMER_TYPES; ++i) {
        std::chrono::duration<double> time_diff = now - _timers.time_ref[i];
        _timers.timer_diff[i] = time_diff.count();
        _timers.accept_event[i] = (time_diff.count() > _timers.timer_values[i]);
    }

    // Looping over events types
    for (uint32_t i = 0; i < NB_IO_EVENTS; ++i) {
        if (events.events[i]) {
            std::invoke(keyboard_events[i], this);
        }
    }

    // Camera updating
    if (_io_manager->isMouseExclusive()) {
        _updateCamera(events.mouse_position);
    }
    _timers.updated[ET_CAMERA] = 1;

    // Resized window case
    if (_io_manager->wasResized()) {
        // VK Renderer related
        auto fb_size = _io_manager->getFramebufferSize();
        _renderer->resize(fb_size.x, fb_size.y);

        // Perspective related
        _perspective->ratio = _io_manager->getWindowRatio();
        _camera->setPerspective(
          glm::perspective(glm::radians(_perspective->fov),
                           _perspective->ratio,
                           _perspective->near_far.x,
                           _perspective->near_far.y));
    }

    // Update model size
    if (events.mouse_scroll != 0.0f) {
        ModelInstanceInfo model_info{};

        _renderer->getModelInstance(1, model_info);
        model_info.scale += SCALING_PER_SCROLL * glm::vec3(events.mouse_scroll);
        if (model_info.scale.x < SCALING_PER_SCROLL) {
            model_info.scale = glm::vec3(SCALING_PER_SCROLL);
        }
        _renderer->updateModelInstance(1, model_info);
        _io_manager->resetMouseScroll();
    }

    // Setting timers origin
    for (uint32_t i = 0; i < ET_NB_EVENT_TIMER_TYPES; ++i) {
        if (_timers.updated[i]) {
            _timers.time_ref[i] = now;
        }
        _timers.updated[i] = 0;
    }
}

EventHandler::EventTimers::EventTimers()
  : accept_event()
  , updated()
  , time_ref()
  , timer_diff()
  , timer_values()
{
    timer_values[ET_SYSTEM] = SYSTEM_TIMER_SECONDS;
    timer_values[ET_CONFIG] = CONFIG_TIMER_SECONDS;
    timer_values[ET_LEFT_MOUSE] = FAST_ACTION_TIMER_SECONDS;
    timer_values[ET_MIDDLE_MOUSE] = FAST_ACTION_TIMER_SECONDS;
    timer_values[ET_RIGHT_MOUSE] = FAST_ACTION_TIMER_SECONDS;
    timer_values[ET_CAMERA] = TARGET_PLAYER_TICK_DURATION;
}

void
EventHandler::_mouse_exclusive()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _previous_mouse_pos = _mouse_pos;
        _io_manager->toggleMouseExclusive();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_close_win_event()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _io_manager->triggerClose();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_toggle_fullscreen()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _io_manager->toggleFullscreen();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_jump()
{
    _movements.z += 1;
}

void
EventHandler::_crouch()
{
    _movements.z -= 1;
}

void
EventHandler::_front()
{
    _movements.x += 1;
}

void
EventHandler::_back()
{
    _movements.x -= 1;
}

void
EventHandler::_right()
{
    _movements.y += 1;
}

void
EventHandler::_left()
{
    _movements.y -= 1;
}

void
EventHandler::_left_mouse()
{
    if (_timers.accept_event[ET_LEFT_MOUSE]) {
        _timers.accept_event[ET_LEFT_MOUSE] = 0;
        _timers.updated[ET_LEFT_MOUSE] = 1;
    }
}

void
EventHandler::_middle_mouse()
{
    if (_timers.accept_event[ET_LEFT_MOUSE]) {
        _timers.accept_event[ET_LEFT_MOUSE] = 0;
        _timers.updated[ET_LEFT_MOUSE] = 1;
    }
}

void
EventHandler::_right_mouse()
{
    if (_timers.accept_event[ET_LEFT_MOUSE]) {
        _timers.accept_event[ET_LEFT_MOUSE] = 0;
        _timers.updated[ET_LEFT_MOUSE] = 1;
    }
}

void
EventHandler::_updateCamera(glm::vec2 const &mouse_pos)
{
    static bool first_run = true;

    _mouse_pos = mouse_pos;
    if (_invert_y_axis) {
        _mouse_pos.y = -mouse_pos.y;
    }
    if (first_run) {
        _previous_mouse_pos = _mouse_pos;
        first_run = false;
    }
    glm::vec2 offset = _mouse_pos - _previous_mouse_pos;

    if (_movements != glm::ivec3(0)) {
        _camera->updatePosition(_movements,
                                _timers.timer_diff[ET_CAMERA] /
                                  _timers.timer_values[ET_CAMERA]);
    }
    if (offset != glm::vec2(0.0)) {
        _camera->updateFront(offset, 0.5f);
        _previous_mouse_pos = _mouse_pos;
    }
    _camera->updateMatrices();
}
