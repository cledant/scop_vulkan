#include "Engine.hpp"

#include "glm/gtc/matrix_transform.hpp"

Engine::Engine()
  : _io_manager()
  , _camera()
  , _event_handler()
  , _perspective_data()
  , _nb_frame(0)
  , _fps_count_timeref()
  , _str_fps("0")
{}

void
Engine::init(EngineOptions const &opts)
{
    IOManagerWindowCreationOption win_opts{ 0,
                                            0,
                                            opts.fullscreen,
                                            0,
                                            DEFAULT_WIN_SIZE,
                                            "scop",
                                            VK_MAKE_VERSION(1, 0, 0),
                                            "scop_engine",
                                            VK_MAKE_VERSION(1, 0, 0) };

    _event_handler.setCamera(&_camera);
    _event_handler.setIOManager(&_io_manager);
    _event_handler.setPerspectiveData(&_perspective_data);
    _event_handler.setInvertYAxis(opts.invert_y_axis);
    _io_manager.createWindow(std::move(win_opts));
    _perspective_data.near_far = DEFAULT_NEAR_FAR;
    _perspective_data.fov = DEFAULT_FOV;
    _perspective_data.ratio = _io_manager.getWindowRatio();
    _camera.setPerspective(glm::perspective(glm::radians(_perspective_data.fov),
                                            _perspective_data.ratio,
                                            _perspective_data.near_far.x,
                                            _perspective_data.near_far.y));
    _camera.setPosition(START_POS);
    _fps_count_timeref = std::chrono::steady_clock::now();
}

void
Engine::run()
{
    while (!_io_manager.shouldClose()) {
        _io_manager.clear();
        _event_handler.processEvents(_io_manager.getEvents());
        _io_manager.render();
        _compute_fps();
    }
    _io_manager.deleteWindow();
}

void
Engine::_compute_fps()
{
    ++_nb_frame;
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = now - _fps_count_timeref;
    if (diff.count() > 1.0f) {
        _str_fps = std::to_string(_nb_frame);
        _nb_frame = 0;
        _fps_count_timeref = now;
    }
}