#include "DG_Camera.h"

namespace DG
{
Camera::Camera(glm::vec3 camPos, glm::vec3 camTarget, glm::vec3 camUp)
{
    view = glm::lookAt(
        camPos,   // camera position in world space
        camTarget,   // camera target in world space
        camUp);  // camera up vector in world space (to calculate right vector)

    // default projection matrix
    projection = glm::perspective(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);

    camPosition = camPos;
}

Camera::~Camera() {}
}  // namespace DG
