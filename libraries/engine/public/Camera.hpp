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
    Camera();
    ~Camera() = default;
    Camera(const Camera &src) = default;
    Camera &operator=(Camera const &rhs) = default;
    Camera(Camera &&src) = default;
    Camera &operator=(Camera &&rhs) = default;

    void update_position(glm::ivec3 const &mov, float coeff);
    void update_front(glm::vec2 const &offsets, float coeff);
    void update_matricies();

    // Setters
    void setPosition(glm::vec3 const &pos);
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

    uint8_t _updated;
    glm::vec3 _pos;
    glm::vec3 _world_up;
    glm::vec3 _right;
    glm::vec3 _up;
    glm::vec3 _front;

    glm::mat4 _view;
    glm::mat4 _perspective;
    glm::mat4 _perspec_mult_view;

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
    std::array<glm::vec4, F_NB_PLANE> _frustum_planes;
    std::array<glm::vec4, F_NB_PLANE> _abs_frustum_planes;
    inline void _extractFrustumPlanes();

    float _mouse_sensitivity;
    float _movement_speed;

    float _yaw;
    float _pitch;
};

#endif // SCOP_VULKAN_CAMERA_HPP
