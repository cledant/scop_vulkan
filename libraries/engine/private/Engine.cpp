#include "Engine.hpp"

#include <cstring>

#include "glm/gtc/matrix_transform.hpp"

void
Engine::init(char const *appName)
{
    assert(appName);
    char engine_name[128] = { 0 };
    std::strcat(engine_name, "_engine");
    IOManagerWindowCreationOption win_opts{
        false, false, false, false, DEFAULT_WIN_SIZE, appName
    };

    _event_handler.setCamera(&_camera);
    _event_handler.setIOManager(&_io_manager);
    _event_handler.setPerspectiveData(&_perspective_data);
    _event_handler.setVkRenderer(&_vk_renderer);
    _event_handler.setUi(&_ui);
    _event_handler.setModel(&_model);
    _io_manager.createWindow(std::move(win_opts));
    Ui::init(_io_manager.getWindow());
    _vk_renderer.createInstance(appName,
                                engine_name,
                                VK_MAKE_VERSION(1, 0, 0),
                                VK_MAKE_VERSION(1, 0, 0),
                                IOManager::getRequiredInstanceExtension());
    auto fb_size = _io_manager.getFramebufferSize();
    _vk_renderer.init(
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
    _camera.setYawPitch(START_YAW, START_PITCH);
    _camera.updateMatrices();
}

void
Engine::run()
{
    while (!_io_manager.shouldClose()) {
        _event_handler.processEvents(_io_manager.getEvents(), _ui.getUiEvent());
        _ui.drawUi();
        _vk_renderer.draw(_camera.getPerspectiveViewMatrix());
    }
    _vk_renderer.deviceWaitIdle();
    _vk_renderer.clear();
    Ui::clear();
    _io_manager.deleteWindow();
}