#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"

Camera::Camera()
  : _updated(1)
  , _pos(1.0)
  , _world_up(DEFAULT_WORLD_UP)
  , _right(0.0)
  , _up(0.0)
  , _front(0.0)
  , _view(1.0)
  , _perspective(1.0)
  , _perspec_mult_view(1.0)
  , _frustum_planes()
  , _abs_frustum_planes()
  , _mouse_sensitivity(DEFAULT_MOUSE_SENSITIVITY)
  , _movement_speed(DEFAULT_MOVEMENT_SPEED)
  , _yaw(0.0f)
  , _pitch(0.0f)
{}

void
Camera::update_position(glm::ivec3 const &mov, float coeff)
{
    _pos += mov.x * _movement_speed * coeff * _front;
    _pos += mov.y * _movement_speed * coeff * _right;
    _pos += mov.z * _movement_speed * coeff * glm::vec3(0.0f, 1.0f, 0.0f);
    _updated = 1;
}

void
Camera::update_front(glm::vec2 const &offsets, float coeff)
{
    _yaw += offsets.x * _mouse_sensitivity * coeff;
    _pitch += offsets.y * _mouse_sensitivity * coeff;
    if (_pitch > 89.0f)
        _pitch = 89.0f;
    if (_pitch < -89.0f)
        _pitch = -89.0f;
    _updated = 1;
}

void
Camera::update_matricies()
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
    _perspec_mult_view[1][1] *= -1.0f;
    _extractFrustumPlanes();
    _updated = 0;
}

// Setters
void
Camera::setPosition(glm::vec3 const &pos)
{
    _pos = pos;
    _updated = 1;
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
    _updated = 1;
}

void
Camera::setWorldUp(glm::vec3 const &world_up)
{
    _world_up = world_up;
    _updated = 1;
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
    for (auto it : _frustum_planes) {
        it = glm::normalize(it);
        _abs_frustum_planes[i] = glm::abs(_frustum_planes[i]);
        ++i;
    }
}
