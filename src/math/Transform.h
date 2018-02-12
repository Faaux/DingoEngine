/**
 *  @file    Transform.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "GLMInclude.h"

namespace DG
{
class Transform
{
   public:
    Transform() {}

    Transform(const mat4& toCopy);
    Transform(vec3 position, vec3 rotationEuler, vec3 scale);
    Transform(vec3 position, quat orientation, vec3 scale);

    void Set(const Transform& other);
    void Set(vec3 position, vec3 rotationEuler, vec3 scale);
    void Set(vec3 position, quat orientation, vec3 scale);
    void Set(vec3 position, quat orientation);
    void Set(const mat4& other);
    void SetPos(vec3 position);
    void SetScale(vec3 scale);
    void SetRotation(vec3 rotationEuler);
    void SetRotation(quat orientation);

    const vec3& GetPosition() const;
    const vec3& GetScale() const;
    const quat& GetOrientation() const;
    vec3 GetEulerRotation() const;

    const mat4& GetModelMatrix() const;

    friend bool operator==(const Transform& lhs, const Transform& rhs)
    {
        return lhs._orientation == rhs._orientation && lhs._position == rhs._position &&
               lhs._scale == rhs._scale;
    }

    friend bool operator!=(const Transform& lhs, const Transform& rhs) { return !(lhs == rhs); }

   private:
    void UpdateModelMatrix() const;
    mutable bool _isValid = false;
    mutable mat4 _model;
    quat _orientation;
    vec3 _position = vec3(0);
    vec3 _scale = vec3(1);
};

}  // namespace DG
