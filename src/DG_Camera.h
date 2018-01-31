#pragma once
#include "DG_Include.h"
#include "DG_InputSystem.h"
#include "DG_Messaging.h"

namespace DG
{
class Camera
{
   public:
    Camera(float fov, float near, float far, float aspectRatio, glm::vec3 pos, glm::vec3 lookAt);
    ~Camera();
    const glm::mat4& GetProjectionMatrix() const { return _projectionMatrix; }

    const glm::mat4& GetViewMatrix() const { return _viewMatrix; }

    void SetPos(glm::vec3 pos)
    {
        if (_position != pos)
        {
            _position = pos;
            CalculateViewMatrix();
        }
    }

    void Update();

   private:
    void CalculateViewMatrix();
    void HandleInputMessage(const InputMessage& message);

    float _fov, _near, _far;

    InputMessage _lastInputMessage;

    glm::quat _currentRot;
    glm::vec3 _position, _forward, _right;
    glm::mat4 _viewMatrix;
    glm::mat4 _projectionMatrix;
    CallbackHandle<InputMessage> _inputMessageCallback;
};
}  // namespace DG::graphics
