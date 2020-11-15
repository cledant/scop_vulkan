#ifndef SCOP_IOMANAGER_HPP
#define SCOP_IOMANAGER_HPP

#include <array>
#include <string>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "IOEvents.hpp"

class IOManager final
{
  public:
    IOManager();
    ~IOManager();
    IOManager(IOManager const &src) = delete;
    IOManager &operator=(IOManager const &rhs) = delete;
    IOManager(IOManager &&src) = delete;
    IOManager &operator=(IOManager &&rhs) = delete;

    // Constants
    static constexpr int32_t const WIN_W = 1280;
    static constexpr int32_t const WIN_H = 720;
    static constexpr uint16_t const KEYS_BUFF_SIZE = 1024;
    static constexpr uint16_t const MOUSE_KEYS_BUFF_SIZE = 16;

    // Window related
    void createWindow(std::string &&name);
    void deleteWindow();
    [[nodiscard]] uint8_t wasResized() const;
    void toggleFullscreen();
    [[nodiscard]] uint8_t shouldClose() const;
    void triggerClose() const;
    void toggleMouseExclusive();
    [[nodiscard]] uint8_t isMouseExclusive() const;
    [[nodiscard]] float getWindowRatio() const;
    [[nodiscard]] glm::vec2 getWindowSize() const;

    // Keyboard / Mouse Input related
    [[nodiscard]] IOEvents getEvents() const;
    void resetMouseScroll();

    // Render Related
    void render();
    void clear() const;

  private:
    // Input
    std::array<uint8_t, KEYS_BUFF_SIZE> _keys;
    std::array<uint8_t, MOUSE_KEYS_BUFF_SIZE> _mouse_button;
    glm::vec2 _mouse_position;
    float _mouse_scroll;

    // Window related
    GLFWwindow *_win;
    uint8_t _fullscreen;
    uint8_t _resized;
    int32_t _w;
    int32_t _h;
    int32_t _w_viewport;
    int32_t _h_viewport;
    std::string _win_name;
    uint8_t _mouse_exclusive;

    // Callbacks
    inline void _initCallbacks();
};

#endif // SCOP_IOMANAGER_HPP
