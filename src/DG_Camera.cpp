#include "DG_Camera.h"
#include "DG_InputSystem.h"
#include "DG_Messaging.h"
#include "imgui/DG_Imgui.h"

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

Camera::Camera(float fov, float near, float far, float aspectRatio, glm::vec3 pos, glm::vec3 lookAt)
    : _fov(fov),
      _near(near),
      _far(far),
      _position(pos),
      _inputMessageCallback(g_MessagingSystem.RegisterCallback<InputMessage>(
          [=](const InputMessage& message) { HandleInputMessage(message); }))
{
    g_MessagingSystem.RegisterCallback<WindowSizeMessage>([=](const WindowSizeMessage& windowSize) {
        UpdateProjection(windowSize.WindowSize.x, windowSize.WindowSize.y);
    });

    _projectionMatrix = glm::perspective(glm::radians(_fov), aspectRatio, _near, _far);

    //_currentRot = glm::rotation(vec3(0, 1, 0), -glm::normalize(lookAt - pos));

    _currentRot = LookRotation(pos - lookAt, vec3(0, 1, 0));

    CalculateViewMatrix();
}

Camera::~Camera() { g_MessagingSystem.UnRegisterCallback(_inputMessageCallback); }

void Camera::UpdateProjection(f32 width, f32 height)
{
    _projectionMatrix = glm::perspective(glm::radians(_fov), width / height, _near, _far);
}

void Camera::Update()
{
    static float rotationSpeed = 1.7f;
    static float speed = 19.f;

    // Imgui Debug Interface
    TWEAKER_CAT("Camera", F1, "Sensitivity", &rotationSpeed);
    TWEAKER_CAT("Camera", F1, "Movement Speed", &speed);
    TWEAKER_CAT("Camera", F3, "Position", &_position.x);

    if (_lastInputMessage.MouseRight)
    {
        // Update Pos by User Input
        glm::vec2 mouseDelta(_lastInputMessage.MouseDeltaX, _lastInputMessage.MouseDeltaY);
        mouseDelta *= rotationSpeed / 1000.f;

        // glm::quat keyQuat(glm::vec3(mouseDelta.y, -mouseDelta.x, 0));
        glm::quat rotX = glm::angleAxis(-mouseDelta.x, glm::vec3(0.f, 1.f, 0.f));
        glm::quat rotY = glm::angleAxis(mouseDelta.y, _right);

        _currentRot = glm::normalize(rotX * rotY * _currentRot);
    }

    // This is needed to make the weakers work, otherwise we would only need it if 
    // _lastInputMessage.MouseRight == true
    CalculateViewMatrix();

    glm::vec3 dir(0);
    dir += _forward * _lastInputMessage.Forward;
    dir += _right * _lastInputMessage.Right;
    if (_lastInputMessage.MouseRight)
        dir += glm::vec3(0, 1, 0) * _lastInputMessage.Up;

    SetPos(_position + dir * speed * g_InGameClock.GetLastDtSeconds());
}

vec3 Camera::GetPos()
{
    return _position;
}

vec3 Camera::GetForward()
{
    return _forward;
}

void Camera::CalculateViewMatrix()
{
    _viewMatrix = glm::inverse(glm::translate(_position) * glm::toMat4(_currentRot));

    _forward = -glm::normalize(glm::vec3(_viewMatrix[0][2], _viewMatrix[1][2], _viewMatrix[2][2]));
    _right = glm::normalize(glm::vec3(_viewMatrix[0][0], _viewMatrix[1][0], _viewMatrix[2][0]));
}

void Camera::HandleInputMessage(const InputMessage& message) { _lastInputMessage = message; }
}  // namespace DG
