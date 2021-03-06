/**
 *  @file    Camera.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Camera.h"
#include "Messaging.h"
#include "imgui/DG_Imgui.h"
#include "math/GLMInclude.h"

namespace DG
{
static glm::quat LookRotation(vec3 forward, vec3 up)
{
    forward = glm::normalize(forward);
    vec3 right = glm::normalize(glm::cross(up, forward));
    up = glm::cross(forward, right);
    float m00 = right.x;
    float m01 = right.y;
    float m02 = right.z;
    float m10 = up.x;
    float m11 = up.y;
    float m12 = up.z;
    float m20 = forward.x;
    float m21 = forward.y;
    float m22 = forward.z;
    float num8 = (m00 + m11) + m22;
    glm::quat quaternion;

    if (num8 > 0.f)
    {
        float num = (float)glm::sqrt(num8 + 1.f);
        quaternion.w = num * 0.5f;
        num = 0.5f / num;
        quaternion.x = (m12 - m21) * num;
        quaternion.y = (m20 - m02) * num;
        quaternion.z = (m01 - m10) * num;
    }
    else if ((m00 >= m11) && (m00 >= m22))
    {
        float num7 = (float)glm::sqrt(((1.f + m00) - m11) - m22);
        float num4 = 0.5f / num7;
        quaternion.x = 0.5f * num7;
        quaternion.y = (m01 + m10) * num4;
        quaternion.z = (m02 + m20) * num4;
        quaternion.w = (m12 - m21) * num4;
    }
    else if (m11 > m22)
    {
        float num6 = (float)glm::sqrt(((1.f + m11) - m00) - m22);
        float num3 = 0.5f / num6;
        quaternion.x = (m10 + m01) * num3;
        quaternion.y = 0.5f * num6;
        quaternion.z = (m21 + m12) * num3;
        quaternion.w = (m20 - m02) * num3;
        return quaternion;
    }
    else
    {
        float num5 = (float)glm::sqrt(((1.f + m22) - m00) - m11);
        float num2 = 0.5f / num5;
        quaternion.x = (m20 + m02) * num2;
        quaternion.y = (m21 + m12) * num2;
        quaternion.z = 0.5f * num5;
        quaternion.w = (m01 - m10) * num2;
    }
    return glm::normalize(quaternion);
    ;
}

Camera::Camera() : Camera(vec3(0, 0, 1), vec3(), vec3(0, 1, 0), 45.f, 0.01f, 100.f, 16.f / 9.f) {}

Camera::Camera(vec3 pos, vec3 lookat, vec3 up, float fov, float near, float far, float aspectRatio)
    : _fov(fov), _near(near), _far(far), _aspectRatio(aspectRatio), _position(pos), _up(up)
{
    RecalculateProjection();
    vec3 forward = pos - lookat;
    Assert(forward != vec3(0));
    _orientation = LookRotation(forward, up);
    RecalculateView();
}

const mat4& Camera::GetViewMatrix() const
{
    if (!_isViewValid)
        RecalculateView();
    return _view;
}

const mat4& Camera::GetProjectionMatrix() const
{
    if (!_isProjectionValid)
        RecalculateProjection();
    return _projection;
}

void Camera::UpdateProjection(float width, float height)
{
    float aspect = width / height;
    if (aspect == _aspectRatio)
        return;
    _aspectRatio = aspect;
    _isProjectionValid = false;
}

const vec3& Camera::GetPosition() const { return _position; }

vec3 Camera::GetRight() const
{
    RecalculateView();
    return glm::normalize(glm::vec3(_view[0][0], _view[1][0], _view[2][0]));
}

vec3 Camera::GetUp() const
{
    RecalculateView();
    return glm::normalize(glm::vec3(_view[0][1], _view[1][1], _view[2][1]));
}

vec3 Camera::GetForward() const
{
    RecalculateView();
    return -glm::normalize(glm::vec3(_view[0][2], _view[1][2], _view[2][2]));
}

const quat& Camera::GetOrientation() const { return _orientation; }

void Camera::Set(vec3 position, quat orientation)
{
    _isViewValid = false;
    _position = position;
    _orientation = orientation;
}

void Camera::Set(vec3 newPosition, vec3 lookAt)
{
    _isViewValid = false;
    _position = newPosition;

    vec3 forward = newPosition - lookAt;
    Assert(forward != vec3(0));
    _orientation = LookRotation(forward, _up);
}

void Camera::RecalculateView() const
{
    if (!_isViewValid)
    {
        _view = glm::inverse(glm::translate(_position) * glm::toMat4(_orientation));
        _isViewValid = true;
    }
}

void Camera::RecalculateProjection() const
{
    if (!_isProjectionValid)
    {
        _projection = glm::perspective(glm::radians(_fov), _aspectRatio, _near, _far);
        _isProjectionValid = true;
    }
}

void UpdateFreeCameraFromInput(Camera& camera, const Clock& clock)
{
    // ToDo(Faaux)(Default): Update routine to world
}
}  // namespace DG
