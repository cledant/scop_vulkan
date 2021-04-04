#include "IOManager.hpp"

#include <stdexcept>

#define THIS_WIN_PTR static_cast<IOManager *>(glfwGetWindowUserPointer(win))

IOManager::IOManager()
{
    if (!glfwInit()) {
        throw std::runtime_error("Glfw : failed to init");
    }
}

IOManager::~IOManager()
{
    glfwTerminate();
}

// Window related
void
IOManager::createWindow(IOManagerWindowCreationOption &&opts)
{
    if (!_win) {
        if (!glfwVulkanSupported()) {
            throw std::runtime_error("Glfw : Vulkan not supported !");
        }
        _win_size = opts.size;
        _mouse_exclusive = opts.mouse_exclusive;
        _cursor_hidden_on_window = opts.cursor_hidden_on_window;
        glfwWindowHint(GLFW_RESIZABLE, opts.resizable);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _win = glfwCreateWindow(
          _win_size.x, _win_size.y, opts.win_name.c_str(), nullptr, nullptr);
        if (!_win) {
            throw std::runtime_error("Glfw : failed to create window");
        }
        glfwSetWindowPos(_win, 100, 100);
        glfwGetWindowSize(_win, &_win_size.x, &_win_size.y);
        glfwGetFramebufferSize(
          _win, &_framebuffer_size.x, &_framebuffer_size.y);
        glfwSetWindowUserPointer(_win, this);
        _initCallbacks();
        if (opts.fullscreen) {
            toggleFullscreen();
        }
        _apply_mouse_visibility();
    }
}

GLFWwindow *
IOManager::getWindow() const
{
    return (_win);
}

void
IOManager::deleteWindow()
{
    if (!_win) {
        glfwDestroyWindow(_win);
        _win = nullptr;
    }
}

bool
IOManager::wasResized()
{
    auto tmp = _resized;
    _resized = false;
    return (tmp);
}

void
IOManager::toggleFullscreen()
{
    _fullscreen = !_fullscreen;

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor) {
        throw std::runtime_error("Glfw : No primary monitor");
    }

    GLFWvidmode const *mode = glfwGetVideoMode(monitor);
    if (!mode) {
        throw std::runtime_error("Glfw : failed to fetch monitor mode");
    }

    if (_fullscreen) {
        glfwSetWindowMonitor(
          _win, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    } else {
        glfwSetWindowMonitor(_win,
                             nullptr,
                             100,
                             100,
                             DEFAULT_WIN_SIZE.x,
                             DEFAULT_WIN_SIZE.y,
                             GLFW_DONT_CARE);
    }
}

bool
IOManager::shouldClose() const
{
    return (glfwWindowShouldClose(_win));
}

void
IOManager::triggerClose() const
{
    glfwSetWindowShouldClose(_win, 1);
}

void
IOManager::toggleMouseExclusive()
{
    _mouse_exclusive = !_mouse_exclusive;
    _apply_mouse_visibility();
}

void
IOManager::toggleMouseVisibility()
{
    _cursor_hidden_on_window = !_cursor_hidden_on_window;
    _apply_mouse_visibility();
}

bool
IOManager::isMouseExclusive() const
{
    return (_mouse_exclusive);
}

float
IOManager::getWindowRatio() const
{
    return (static_cast<float>(_win_size.x) / static_cast<float>(_win_size.y));
}

glm::ivec2
IOManager::getWindowSize() const
{
    return (_win_size);
}

glm::ivec2
IOManager::getFramebufferSize() const
{
    return (_framebuffer_size);
}

// Keyboard / Mouse Input related
IOEvents
IOManager::getEvents() const
{
    IOEvents io{};

    glfwPollEvents();
    io.events[MOUSE_EXCLUSIVE] = _keys[GLFW_KEY_F4];
    io.events[QUIT] = _keys[GLFW_KEY_F10];
    io.events[FULLSCREEN] = _keys[GLFW_KEY_F8];
    io.events[JUMP] = _keys[GLFW_KEY_SPACE];
    io.events[CROUCH] = _keys[GLFW_KEY_LEFT_SHIFT];
    io.events[FRONT] = _keys[GLFW_KEY_W];
    io.events[BACK] = _keys[GLFW_KEY_S];
    io.events[RIGHT] = _keys[GLFW_KEY_D];
    io.events[LEFT] = _keys[GLFW_KEY_A];
    io.events[LEFT_MOUSE] = _mouse_button[GLFW_MOUSE_BUTTON_LEFT];
    io.events[MIDDLE_MOUSE] = _mouse_button[GLFW_MOUSE_BUTTON_MIDDLE];
    io.events[RIGHT_MOUSE] = _mouse_button[GLFW_MOUSE_BUTTON_RIGHT];
    io.events[OPEN_MODEL] = _keys[GLFW_KEY_F2];
    io.events[SHOW_FPS] = _keys[GLFW_KEY_F7];
    io.events[MODEL_PARAMETERS_EDIT] = _keys[GLFW_KEY_F3];
    io.events[MODEL_INFO] = _keys[GLFW_KEY_F6];
    io.events[DISPLAY_UI] = _keys[GLFW_KEY_F9];
    io.events[ABOUT] = _keys[GLFW_KEY_F1];
    io.events[INVERSE_Y_AXIS] = _keys[GLFW_KEY_F5];
    io.mouse_position = _mouse_position;
    io.mouse_scroll = _mouse_scroll;
    return (io);
}

void
IOManager::resetMouseScroll()
{
    _mouse_scroll = 0.0f;
}

// Vulkan related
VkSurfaceKHR
IOManager::createVulkanSurface(VkInstance instance)
{
    VkSurfaceKHR vk_surface{};
    if (glfwCreateWindowSurface(instance, _win, nullptr, &vk_surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("IOManager: Failed to create window surface");
    }
    return (vk_surface);
}

std::vector<const char *>
IOManager::getRequiredInstanceExtension()
{
    uint32_t nb_glfw_extension = 0;
    char const **glfw_extensions =
      glfwGetRequiredInstanceExtensions(&nb_glfw_extension);
    std::vector<char const *> extensions(glfw_extensions,
                                         glfw_extensions + nb_glfw_extension);
    return (extensions);
}

// Callbacks
void
IOManager::_initCallbacks()
{
    // Keyboard input
    auto keyboard_callback =
      [](GLFWwindow *win, int key, int scancode, int action, int mods) {
          static_cast<void>(scancode);
          static_cast<void>(mods);
          if (key >= 0 && key < KEYS_BUFF_SIZE) {
              if (action == GLFW_PRESS) {
                  THIS_WIN_PTR->_keys[key] = 1;
              } else if (action == GLFW_RELEASE) {
                  THIS_WIN_PTR->_keys[key] = 0;
              }
          }
      };
    glfwSetKeyCallback(_win, keyboard_callback);

    // Mouse position
    auto cursor_position_callback =
      [](GLFWwindow *win, double xpos, double ypos) {
          THIS_WIN_PTR->_mouse_position = glm::vec2(xpos, ypos);
      };
    glfwSetCursorPosCallback(_win, cursor_position_callback);

    // Mouse button input
    auto mouse_button_callback =
      [](GLFWwindow *win, int button, int action, int mods) {
          static_cast<void>(mods);
          if (button >= 0 && button < MOUSE_KEYS_BUFF_SIZE) {
              if (action == GLFW_PRESS)
                  THIS_WIN_PTR->_mouse_button[button] = GLFW_PRESS;
              else if (action == GLFW_RELEASE)
                  THIS_WIN_PTR->_mouse_button[button] = GLFW_RELEASE;
          }
      };
    glfwSetMouseButtonCallback(_win, mouse_button_callback);

    // Mouse Scroll
    auto mouse_scroll_callback =
      [](GLFWwindow *win, double xoffset, double yoffset) {
          static_cast<void>(win);
          THIS_WIN_PTR->_mouse_scroll += xoffset;
          THIS_WIN_PTR->_mouse_scroll += yoffset;
      };
    glfwSetScrollCallback(_win, mouse_scroll_callback);

    // Close
    auto close_callback = [](GLFWwindow *win) {
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    };
    glfwSetWindowCloseCallback(_win, close_callback);

    // Window
    auto window_size_callback = [](GLFWwindow *win, int w, int h) {
        auto prev_size = THIS_WIN_PTR->_win_size;

        THIS_WIN_PTR->_win_size = glm::ivec2(w, h);
        if (prev_size != THIS_WIN_PTR->_win_size) {
            THIS_WIN_PTR->_resized = true;
        }
    };
    glfwSetWindowSizeCallback(_win, window_size_callback);

    // Framebuffer
    auto framebuffer_size_callback = [](GLFWwindow *win, int w, int h) {
        THIS_WIN_PTR->_framebuffer_size = glm::ivec2(w, h);
    };
    glfwSetFramebufferSizeCallback(_win, framebuffer_size_callback);
}

void
IOManager::_apply_mouse_visibility() const
{
    if (_mouse_exclusive) {
        glfwSetInputMode(_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (_cursor_hidden_on_window) {
        glfwSetInputMode(_win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    } else {
        glfwSetInputMode(_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}