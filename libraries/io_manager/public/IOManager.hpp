#ifndef SCOP_VULKAN_IOMANAGER_HPP
#define SCOP_VULKAN_IOMANAGER_HPP

#include <array>
#include <vector>
#include <string>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "IOManagerWindowCreationOption.hpp"
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
    static constexpr uint16_t const KEYS_BUFF_SIZE = 1024;
    static constexpr uint16_t const MOUSE_KEYS_BUFF_SIZE = 32;

    // Window related
    void createWindow(IOManagerWindowCreationOption &&opts);
    void deleteWindow();
    [[nodiscard]] uint8_t wasResized();
    void toggleFullscreen();
    [[nodiscard]] uint8_t shouldClose() const;
    void triggerClose() const;
    void toggleMouseExclusive();
    void toggleMouseVisibility();
    [[nodiscard]] uint8_t isMouseExclusive() const;
    [[nodiscard]] float getWindowRatio() const;
    [[nodiscard]] glm::ivec2 getWindowSize() const;
    [[nodiscard]] glm::ivec2 getFramebufferSize() const;

    // Keyboard / Mouse Input related
    [[nodiscard]] IOEvents getEvents() const;
    void resetMouseScroll();

    // Vulkan related
    VkSurfaceKHR createVulkanSurface(VkInstance instance);
    [[nodiscard]] static std::vector<char const *>
    getRequiredInstanceExtension();

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
    glm::ivec2 _win_size;
    glm::ivec2 _framebuffer_size;

    uint8_t _mouse_exclusive;
    uint8_t _cursor_hidden_on_window;

    // Callbacks
    inline void _initCallbacks();

    // Mouse
    inline void _apply_mouse_visibility() const;
};

#endif // SCOP_VULKAN_IOMANAGER_HPP
