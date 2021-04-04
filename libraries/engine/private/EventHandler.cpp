#include "EventHandler.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <functional>

#include "fmt/core.h"

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
EventHandler::setModel(Model *model)
{
    _model = model;
}

void
EventHandler::processEvents(IOEvents const &ioEvents, UiEvent const &uiEvent)
{
    assert(_camera);
    assert(_io_manager);
    assert(_perspective);
    assert(_renderer);
    assert(_ui);
    assert(_model);

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
          &EventHandler::_open_model,
          &EventHandler::_show_fps,
          &EventHandler::_model_parameter_edit,
          &EventHandler::_model_info,
          &EventHandler::_display_ui,
          &EventHandler::_about,
          &EventHandler::_invert_camera_y_axis,
      };

    static const std::array<void (EventHandler::*)(), UET_TOTAL_NB>
      ui_events = {
          &EventHandler::_ui_load_model,
          &EventHandler::_ui_update_model_params,
          &EventHandler::_ui_close_app,
          &EventHandler::_ui_mouse_exclusive,
          &EventHandler::_ui_invert_mouse_y_axis,
          &EventHandler::_ui_fullscreen,
      };

    // Checking Timers
    auto now = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < ET_NB_EVENT_TIMER_TYPES; ++i) {
        std::chrono::duration<double> time_diff = now - _timers.time_ref[i];
        _timers.timer_diff[i] = time_diff.count();
        _timers.accept_event[i] = (time_diff.count() > _timers.timer_values[i]);
    }

    // Looping over io events types
    for (uint32_t i = 0; i < NB_IO_EVENTS; ++i) {
        if (ioEvents.events[i]) {
            std::invoke(keyboard_events[i], this);
        }
    }

    // Looping over ui events type
    for (uint32_t i = 0; i < UET_TOTAL_NB; ++i) {
        if (uiEvent.events[i]) {
            std::invoke(ui_events[i], this);
        }
    }

    // Camera updating
    if (_io_manager->isMouseExclusive()) {
        _update_camera(ioEvents.mouse_position);
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
        _io_manager->toggleMouseExclusive();
        _mouse_pos_skip = true;
        _ui->toggleCameraMvt();
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
        _ui->toggleFullscreen();
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
EventHandler::_open_model()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _ui->toggleSelectModel();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_show_fps()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _ui->toggleShowFps();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_model_parameter_edit()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _ui->toggleModelParam();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_model_info()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _ui->toggleModelInfo();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_display_ui()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _ui->toggleDisplayUi();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_about()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _ui->toggleAbout();
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_invert_camera_y_axis()
{
    if (_timers.accept_event[ET_SYSTEM]) {
        _ui->toggleInvertCameraYAxis();
        _invert_y_axis = !_invert_y_axis;
        _timers.accept_event[ET_SYSTEM] = 0;
        _timers.updated[ET_SYSTEM] = 1;
    }
}

void
EventHandler::_ui_load_model()
{
    Model tmp;
    bool model_parsed = false;

    try {
        tmp.loadModel(_ui->getModelFilepath());
        *_model = std::move(tmp);
        model_parsed = true;
        _renderer->loadModel(*_model);
        _model_index = _renderer->addModelInstance({});
        auto model_info = _model->getModelInfo();
        _ui->setModelInfo(
          model_info.nbVertices, model_info.nbIndices, model_info.nbFaces);
        _ui->resetModelParams();
    } catch (std::exception const &e) {
        fmt::print("{}\n", e.what());
        if (model_parsed) {
            _model_index = 0;
            _ui->setModelInfo(0, 0, 0);
        }
        _ui->setModelLoadingError();
    }
}

void
EventHandler::_ui_update_model_params()
{
    ModelInstanceInfo mi{};
    mi.yaw = _ui->getModelYaw();
    mi.pitch = _ui->getModelPitch();
    mi.roll = _ui->getModelRoll();
    mi.scale = glm::vec3(_ui->getModelScale());
    _renderer->updateModelInstance(_model_index, mi);
}

void
EventHandler::_ui_close_app()
{
    _io_manager->triggerClose();
}

void
EventHandler::_ui_mouse_exclusive()
{
    _io_manager->toggleMouseExclusive();
    _mouse_pos_skip = true;
}

void
EventHandler::_ui_invert_mouse_y_axis()
{
    _invert_y_axis = !_invert_y_axis;
}

void
EventHandler::_ui_fullscreen()
{
    _io_manager->toggleFullscreen();
}

void
EventHandler::_update_camera(glm::vec2 const &mouse_pos)
{
    _mouse_pos = mouse_pos;
    if (_mouse_pos_skip) {
        _previous_mouse_pos = _mouse_pos;
        _mouse_pos_skip = false;
    }

    glm::vec2 offset = _mouse_pos - _previous_mouse_pos;
    if (_invert_y_axis) {
        offset.y = -offset.y;
    }
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
