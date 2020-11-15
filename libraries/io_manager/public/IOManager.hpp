#ifndef SCOP_VULKAN_IOMANAGER_HPP
#define SCOP_VULKAN_IOMANAGER_HPP

#include <array>
#include <string>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "IOManagerWindowCreationOption.hpp"
#include "IOEvents.hpp"
#include "VkRenderer.hpp"

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
    static constexpr uint16_t const KEYS_BUFF_SIZE = 1024;
    static constexpr uint16_t const MOUSE_KEYS_BUFF_SIZE = 32;

    // Window related
    void createWindow(IOManagerWindowCreationOption &&opts);
    void deleteWindow();
    [[nodiscard]] uint8_t wasResized() const;
    void toggleFullscreen();
    [[nodiscard]] uint8_t shouldClose() const;
    void triggerClose() const;
    void toggleMouseExclusive();
    void toggleMouseVisibility();
    [[nodiscard]] uint8_t isMouseExclusive() const;
    [[nodiscard]] float getWindowRatio() const;
    [[nodiscard]] glm::ivec2 getWindowSize() const;

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
    glm::ivec2 _size;
    glm::ivec2 _viewport_size;
    std::string _app_name;
    std::string _engine_name;
    uint32_t _app_version;
    uint32_t _engine_version;
    uint8_t _mouse_exclusive;
    uint8_t _cursor_hidden_on_window;

    // Vulkan Renderer
    VkRenderer _vk_renderer;

    // Callbacks
    inline void _initCallbacks();

    // Mouse
    inline void _apply_mouse_visibility() const;
};

#endif // SCOP_VULKAN_IOMANAGER_HPP