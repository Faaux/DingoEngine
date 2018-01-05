#pragma once
#include "DG_Include.h"

namespace DG
{
class Camera
{
   public:
    Camera(vec3 camPos = vec3(), vec3 camTarget = vec3(), vec3 camUp = vec3(0, 1, 0));
    virtual ~Camera();

    // set/update view matrix
    void setView(const vec3& camPos, const vec3& camTarget, const vec3& camUp)
    {
        view = lookAt(camPos, camTarget, camUp);
        camPosition = camPos;
    }

    const mat4& getView() const { return view; }

    const mat4& getProjection() const { return projection; }

    const vec3& getPosition() const { return camPosition; }

   private:
    /* view matrix */
    mat4 view;
    // glm::vec3 viewVector;

    /* projection matrix */
    mat4 projection;

    /* cam position in world space */
    vec3 camPosition;
};
}  // namespace DG
