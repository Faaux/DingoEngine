#include "DG_Camera.h"
#include "DG_InputSystem.h"
#include "DG_Messaging.h"
#include "imgui/DG_Imgui.h"

namespace DG
{
Camera::Camera(float fov, float near, float far, float aspectRatio, glm::vec3 pos, glm::vec3 lookAt)
    : _fov(fov),
      _near(near),
      _far(far),
      _position(pos),
      _inputMessageCallback(g_MessagingSystem.RegisterCallback<InputMessage>(
          [=](const InputMessage& message) { HandleInputMessage(message); }))
{
    g_MessagingSystem.RegisterCallback<WindowSizeMessage>([=](const WindowSizeMessage& windowSize) {
        _projectionMatrix = glm::perspective(
            glm::radians(_fov), windowSize.WindowSize.x / windowSize.WindowSize.y, _near, _far);
    });

    _projectionMatrix = glm::perspective(glm::radians(_fov), aspectRatio, _near, _far);

    _currentRot = glm::rotation(vec3(0, 0, 1), -glm::normalize(lookAt - pos));

    CalculateViewMatrix();
}

Camera::~Camera() { g_MessagingSystem.UnRegisterCallback(_inputMessageCallback); }

void Camera::Update()
{
    static float rotationSpeed = 4.f;
    static float speed = 2.f;

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

        CalculateViewMatrix();
    }

    glm::vec3 dir(0);
    dir += _forward * _lastInputMessage.Forward;
    dir += _right * _lastInputMessage.Right;
    if (_lastInputMessage.MouseRight)
        dir += glm::vec3(0, 1, 0) * _lastInputMessage.Up;

    SetPos(_position + dir * speed * g_InGameClock.GetLastDtSeconds());
}

void Camera::CalculateViewMatrix()
{
    _viewMatrix = glm::inverse(glm::translate(_position) * glm::toMat4(_currentRot));

    _forward = -glm::normalize(glm::vec3(_viewMatrix[0][2], _viewMatrix[1][2], _viewMatrix[2][2]));
    _right = glm::normalize(glm::vec3(_viewMatrix[0][0], _viewMatrix[1][0], _viewMatrix[2][0]));
}

void Camera::HandleInputMessage(const InputMessage& message) { _lastInputMessage = message; }
}  // namespace DG
