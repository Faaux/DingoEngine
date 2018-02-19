/**
 *  @file    Camera.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "math/GLMInclude.h"
#include "platform/Clock.h"

namespace DG
{
class Camera
{
   public:
    Camera();
    Camera(vec3 pos, vec3 lookat, vec3 up, float fov, float near, float far, float aspectRatio);
    

    const mat4& GetViewMatrix() const;

    const mat4& GetProjectionMatrix() const;
    void UpdateProjection(float width, float height);
    const vec3& GetPosition() const;
    vec3 GetRight() const;
    vec3 GetUp() const;
    vec3 GetForward() const;
    const quat& GetOrientation() const;
    void Set(vec3 newPosition, quat newOrientation);
    void Set(vec3 newPosition, vec3 lookAt);

   private:
    void RecalculateView() const;
    void RecalculateProjection() const;
    // Cache
    mutable bool _isViewValid = false, _isProjectionValid = false;
    mutable mat4 _projection;
    mutable mat4 _view;

    // Projection
    float _fov = 45.f, _near = 0.01f, _far = 1000.f, _aspectRatio = 16.f / 9.f;

    // View
    quat _orientation;
    vec3 _position;
    vec3 _up;
};

void UpdateFreeCameraFromInput(Camera& camera, const Clock& clock);

}  // namespace DG
