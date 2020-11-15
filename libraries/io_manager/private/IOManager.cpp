#include "IOManager.hpp"

#include <stdexcept>

#define THIS_WIN_PTR static_cast<IOManager *>(glfwGetWindowUserPointer(win))

IOManager::IOManager()
  : _keys()
  , _mouse_button()
  , _mouse_position(0.0f)
  , _mouse_scroll(0.0f)
  , _win(nullptr)
  , _fullscreen(0)
  , _resized(0)
  , _w(0)
  , _h(0)
  , _w_viewport(0)
  , _h_viewport(0)
  , _win_name()
  , _mouse_exclusive(0)
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
IOManager::createWindow(std::string &&name)
{
    if (!_win) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _win_name = name;
        _win = glfwCreateWindow(640, 480, _win_name.c_str(), nullptr, nullptr);
        if (!_win) {
            throw std::runtime_error("Glfw : failed to create window");
        }
        glfwSetInputMode(_win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwSetWindowPos(_win, 100, 100);
        glfwSetWindowUserPointer(_win, this);
        _initCallbacks();
        glfwMakeContextCurrent(_win);
        glfwSwapInterval(0);
        glfwSetWindowSize(_win, WIN_W, WIN_H);
        _w = WIN_W;
        _h = WIN_H;
        _initCallbacks();
#ifdef NDEBUG
        toggleMouseExclusive();
        toggleFullscreen();
#endif
    }
}

void
IOManager::deleteWindow()
{
    if (!_win) {
        glfwDestroyWindow(_win);
        _win = nullptr;
    }
}

uint8_t
IOManager::wasResized() const
{
    return (_resized);
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
        glfwSetWindowMonitor(
          _win, nullptr, 100, 100, WIN_W, WIN_H, GLFW_DONT_CARE);
    }
}

uint8_t
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
    (_mouse_exclusive)
      ? glfwSetInputMode(_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED)
      : glfwSetInputMode(_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

uint8_t
IOManager::isMouseExclusive() const
{
    return (_mouse_exclusive);
}

float
IOManager::getWindowRatio() const
{
    return (static_cast<float>(_w) / static_cast<float>(_h));
}

glm::vec2
IOManager::getWindowSize() const
{
    return (glm::vec2(_w, _h));
}

// Keyboard / Mouse Input related
IOEvents
IOManager::getEvents() const
{
    IOEvents io = {};

    glfwPollEvents();
    io.events[MOUSE_EXCLUSIVE] = _keys[GLFW_KEY_P];
    io.events[ESCAPE] = _keys[GLFW_KEY_ESCAPE];
    io.events[FULLSCREEN] = _keys[GLFW_KEY_F5];
    io.events[JUMP] = _keys[GLFW_KEY_E];
    io.events[CROUCH] = _keys[GLFW_KEY_Q];
    io.events[FRONT] = _keys[GLFW_KEY_W];
    io.events[BACK] = _keys[GLFW_KEY_S];
    io.events[RIGHT] = _keys[GLFW_KEY_D];
    io.events[LEFT] = _keys[GLFW_KEY_A];
    io.events[LEFT_MOUSE] = _mouse_button[GLFW_MOUSE_BUTTON_LEFT];
    io.events[MIDDLE_MOUSE] = _mouse_button[GLFW_MOUSE_BUTTON_MIDDLE];
    io.events[RIGHT_MOUSE] = _mouse_button[GLFW_MOUSE_BUTTON_RIGHT];
    io.mouse_position = _mouse_position;
    io.mouse_scroll = _mouse_scroll;
    return (io);
}

void
IOManager::resetMouseScroll()
{
    _mouse_scroll = 0.0f;
}

// Render related
void
IOManager::render()
{
    _resized = 0;
    glfwSwapBuffers(_win);
}

void
IOManager::clear() const
{
    // glClearColor(0.086f, 0.317f, 0.427f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
        THIS_WIN_PTR->_h = h;
        THIS_WIN_PTR->_w = w;
        THIS_WIN_PTR->_resized = 1;
    };
    glfwSetWindowSizeCallback(_win, window_size_callback);

    // Framebuffer
    auto framebuffer_size_callback = [](GLFWwindow *win, int w, int h) {
        THIS_WIN_PTR->_w_viewport = w;
        THIS_WIN_PTR->_h_viewport = h;
        // glViewport(0, 0, w, h);
    };
    glfwSetFramebufferSizeCallback(_win, framebuffer_size_callback);
}