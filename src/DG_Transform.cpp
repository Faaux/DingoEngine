/**
 *  @file    DG_Transform.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    06 February 2018
 */

#include "DG_Transform.h"

namespace DG
{
Transform::Transform(const mat4& toCopy)
{
    vec3 skew;
    vec4 perspective;
    glm::quat orientation;
    glm::decompose(toCopy, _scale, orientation, _position, skew, perspective);
    _orientation = glm::conjugate(orientation);
}

Transform::Transform(vec3 position, vec3 rotationEuler, vec3 scale)
    : _orientation(rotationEuler), _position(position), _scale(scale)
{
}

Transform::Transform(vec3 position, quat orientation, vec3 scale)
    : _orientation(orientation), _position(position), _scale(scale)
{
}

void Transform::Set(vec3 position, vec3 rotationEuler, vec3 scale)
{
    _isValid = false;
    _orientation = quat(rotationEuler);
    _position = position;
    _scale = scale;
}

void Transform::Set(vec3 position, quat orientation, vec3 scale)
{
    _isValid = false;
    _orientation = orientation;
    _position = position;
    _scale = scale;
}

void Transform::Set(vec3 position, quat orientation)
{
    _isValid = false;
    _orientation = orientation;
    _position = position;
}

void Transform::Set(const mat4& other)
{
    vec3 skew;
    vec4 perspective;
    glm::quat orientation;
    glm::decompose(other, _scale, orientation, _position, skew, perspective);
    _orientation = glm::conjugate(orientation);
}

void Transform::SetPos(vec3 position)
{
    _isValid = false;
    _position = position;
}

void Transform::SetScale(vec3 scale)
{
    _isValid = false;
    _scale = scale;
}

void Transform::SetRotation(vec3 rotationEuler)
{
    _isValid = false;
    _orientation = quat(rotationEuler);
}

void Transform::SetRotation(quat orientation)
{
    _isValid = false;
    _orientation = orientation;
}

const vec3& Transform::GetPosition() const { return _position; }

const vec3& Transform::GetScale() const { return _scale; }

const quat& Transform::GetOrientation() const { return _orientation; }

vec3 Transform::GetEulerRotation() const { return glm::eulerAngles(_orientation); }

const mat4& Transform::GetModelMatrix() const
{
    if (!_isValid)
        UpdateModelMatrix();

    return _model;
}

void Transform::UpdateModelMatrix() const
{
    if (_isValid)
        return;

    _model = glm::translate(_position) * glm::mat4_cast(_orientation) * glm::scale(_scale);

    _isValid = true;
}
}  // namespace DG
