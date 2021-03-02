#ifndef SCOP_VULKAN_CAMERA_HPP
#define SCOP_VULKAN_CAMERA_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <array>

#include "glm/glm.hpp"

class Camera final
{
  public:
    Camera() = default;
    ~Camera() = default;
    Camera(const Camera &src) = default;
    Camera &operator=(Camera const &rhs) = default;
    Camera(Camera &&src) = default;
    Camera &operator=(Camera &&rhs) = default;

    void updatePosition(glm::ivec3 const &mov, float coeff);
    void updateFront(glm::vec2 const &offsets, float coeff);
    void updateMatrices();

    // Setters
    void setPosition(glm::vec3 const &pos);
    void setYawPitch(float yaw, float pitch);
    void setWorldUp(glm::vec3 const &world_up);
    void setPerspective(glm::mat4 const &perspective);

    // Getters
    [[nodiscard]] glm::vec3 const &getPosition() const;
    [[nodiscard]] glm::vec3 const &getFront() const;
    [[nodiscard]] glm::mat4 const &getPerspectiveMatrix() const;
    [[nodiscard]] glm::mat4 const &getViewMatrix() const;
    [[nodiscard]] glm::mat4 const &getPerspectiveViewMatrix() const;
    [[nodiscard]] std::array<glm::vec4, 6> const &getFrustumPlanes() const;
    [[nodiscard]] std::array<glm::vec4, 6> const &getAbsFrustumPlanes() const;

  private:
    static float constexpr const DEFAULT_MOVEMENT_SPEED = 0.075f * 2;
    static float constexpr const DEFAULT_MOUSE_SENSITIVITY = 0.05f * 2;
    static constexpr glm::vec3 const DEFAULT_WORLD_UP =
      glm::vec3(0.0f, 1.0f, 0.0f);

    bool _updated = true;
    glm::vec3 _pos = glm::vec3(1.0f);
    glm::vec3 _world_up = DEFAULT_WORLD_UP;
    glm::vec3 _right{};
    glm::vec3 _up{};
    glm::vec3 _front{};

    glm::mat4 _view = glm::mat4(1.0f);
    glm::mat4 _perspective = glm::mat4(1.0f);
    glm::mat4 _perspec_mult_view = glm::mat4(1.0f);

    enum frustumPlane
    {
        F_LEFT = 0,
        F_RIGHT,
        F_BOTTOM,
        F_TOP,
        F_NEAR,
        F_FAR,
        F_NB_PLANE
    };
    std::array<glm::vec4, F_NB_PLANE> _frustum_planes{};
    std::array<glm::vec4, F_NB_PLANE> _abs_frustum_planes{};
    inline void _extractFrustumPlanes();

    float _mouse_sensitivity = DEFAULT_MOUSE_SENSITIVITY;
    float _movement_speed = DEFAULT_MOVEMENT_SPEED;

    float _yaw{};
    float _pitch{};
};

#endif // SCOP_VULKAN_CAMERA_HPP
