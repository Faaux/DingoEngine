#pragma once
#include "DG_Include.h"

namespace DG
{
class Camera
{
   public:
    Camera(glm::vec3 camPos = glm::vec3(), glm::vec3 camTarget = glm::vec3(), glm::vec3 camUp = glm::vec3(0, 1, 0));
    virtual ~Camera();

    // set/update view matrix
    void setView(const glm::vec3& camPos, const glm::vec3& camTarget, const glm::vec3& camUp)
    {
        view = lookAt(camPos, camTarget, camUp);
        camPosition = camPos;
    }

    const glm::mat4& getView() const { return view; }

    const glm::mat4& getProjection() const { return projection; }

    const glm::vec3& getPosition() const { return camPosition; }

   private:
    /* view matrix */
    glm::mat4 view;
    // glm::vec3 viewVector;

    /* projection matrix */
    glm::mat4 projection;

    /* cam position in world space */
    glm::vec3 camPosition;
};
}  // namespace DG
