#include "Camera.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"

void
Camera::updatePosition(glm::ivec3 const &mov, float coeff)
{
    _pos += mov.x * _movement_speed * coeff * _front;
    _pos += mov.y * _movement_speed * coeff * _right;
    _pos += mov.z * _movement_speed * coeff * glm::vec3(0.0f, 1.0f, 0.0f);
    _updated = true;
}

void
Camera::updateFront(glm::vec2 const &offsets, float coeff)
{
    _yaw += offsets.x * _mouse_sensitivity * coeff;
    _pitch += offsets.y * _mouse_sensitivity * coeff;
    if (_pitch > 89.0f)
        _pitch = 89.0f;
    if (_pitch < -89.0f)
        _pitch = -89.0f;
    _updated = true;
}

void
Camera::updateMatrices()
{
    if (!_updated) {
        return;
    }
    _front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _front.y = sin(glm::radians(_pitch));
    _front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    glm::normalize(_front);
    _right = glm::normalize(glm::cross(_front, _world_up));
    _up = glm::normalize(glm::cross(_right, _front));
    _view = glm::lookAt(_pos, _pos + _front, _up);
    _perspec_mult_view = _perspective * _view;
    _extractFrustumPlanes();
    _updated = false;
}

// Setters
void
Camera::setPosition(glm::vec3 const &pos)
{
    _pos = pos;
    _updated = true;
}

void
Camera::setYawPitch(float yaw, float pitch)
{
    _yaw = yaw;
    _pitch = pitch;
    if (_pitch > 89.0f)
        _pitch = 89.0f;
    if (_pitch < -89.0f)
        _pitch = -89.0f;
    _updated = true;
}

void
Camera::setWorldUp(glm::vec3 const &world_up)
{
    _world_up = world_up;
    _updated = true;
}

void
Camera::setPerspective(glm::mat4 const &perspective)
{
    _perspective = perspective;
    _perspec_mult_view = _perspective * _view;
    _extractFrustumPlanes();
}

// Getters
glm::vec3 const &
Camera::getPosition() const
{
    return (_pos);
}

glm::vec3 const &
Camera::getFront() const
{
    return (_front);
}

glm::mat4 const &
Camera::getPerspectiveMatrix() const
{
    return (_perspective);
}

glm::mat4 const &
Camera::getViewMatrix() const
{
    return (_view);
}

glm::mat4 const &
Camera::getPerspectiveViewMatrix() const
{
    return (_perspec_mult_view);
}

std::array<glm::vec4, 6> const &
Camera::getFrustumPlanes() const
{
    return (_frustum_planes);
}

std::array<glm::vec4, 6> const &
Camera::getAbsFrustumPlanes() const
{
    return (_abs_frustum_planes);
}

void
Camera::_extractFrustumPlanes()
{
    _frustum_planes[F_LEFT] =
      glm::row(_perspec_mult_view, 3) + glm::row(_perspec_mult_view, 0);
    _frustum_planes[F_RIGHT] =
      glm::row(_perspec_mult_view, 3) - glm::row(_perspec_mult_view, 0);
    _frustum_planes[F_BOTTOM] =
      glm::row(_perspec_mult_view, 3) + glm::row(_perspec_mult_view, 1);
    _frustum_planes[F_TOP] =
      glm::row(_perspec_mult_view, 3) - glm::row(_perspec_mult_view, 1);
    _frustum_planes[F_NEAR] =
      glm::row(_perspec_mult_view, 3) + glm::row(_perspec_mult_view, 2);
    _frustum_planes[F_FAR] =
      glm::row(_perspec_mult_view, 3) - glm::row(_perspec_mult_view, 2);
    uint32_t i = 0;
    for (auto &it : _frustum_planes) {
        it = glm::normalize(it);
        _abs_frustum_planes[i] = glm::abs(_frustum_planes[i]);
        ++i;
    }
}
