#include "Engine.hpp"

#include "glm/gtc/matrix_transform.hpp"

Engine::Engine()
  : _io_manager()
  , _vk_renderer()
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
    auto cpy_app_name = opts.app_name;
    IOManagerWindowCreationOption win_opts{
        0, 0, opts.fullscreen, 0, DEFAULT_WIN_SIZE, opts.app_name
    };

    _event_handler.setCamera(&_camera);
    _event_handler.setIOManager(&_io_manager);
    _event_handler.setPerspectiveData(&_perspective_data);
    _event_handler.setInvertYAxis(opts.invert_y_axis);
    _io_manager.createWindow(std::move(win_opts));
    _vk_renderer.createInstance(std::move(cpy_app_name),
                                cpy_app_name + "_engine",
                                VK_MAKE_VERSION(1, 0, 0),
                                VK_MAKE_VERSION(1, 0, 0),
                                IOManager::getRequiredInstanceExtension());
    auto fb_size = _io_manager.getFramebufferSize();
    _vk_renderer.initInstance(
      _io_manager.createVulkanSurface(_vk_renderer.getVkInstance()),
      fb_size.x,
      fb_size.y);
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
        _event_handler.processEvents(_io_manager.getEvents());
        _vk_renderer.draw();
        _compute_fps();
    }
    _vk_renderer.deviceWaitIdle();
    _vk_renderer.clearInstance();
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
        std::cout << "Avg Fps = " << _str_fps << std::endl;
        _nb_frame = 0;
        _fps_count_timeref = now;
    }
}