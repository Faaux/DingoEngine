#pragma once
#include "DG_Include.h"

namespace DG
{
class Transform
{
   public:
    Transform(const vec3& p = vec3(), const vec3& r = vec3(), const vec3& s = vec3(1, 1, 1))
        : pos(p), rot(r), scale(s)
    {
        RecalculateModelMatrix();
    }

    void SetTransform(vec3 p, glm::quat quat)
    {
        rot = glm::eulerAngles(quat);
        pos = p;
        RecalculateModelMatrix();
    }

    void RecalculateModelMatrix()
    {
        // ToDo: Cache this?
        mat4 posMatrix = translate(pos);
        mat4 rotXMatrix = rotate(rot.x, vec3(1, 0, 0));
        mat4 rotYMatrix = rotate(rot.y, vec3(0, 1, 0));
        mat4 rotZMatrix = rotate(rot.z, vec3(0, 0, 1));
        mat4 scaleMatrix = glm::scale(scale);
        mat4 rotMatrix = rotZMatrix * rotYMatrix * rotXMatrix;
        modelMatrix = posMatrix * rotMatrix * scaleMatrix;
    }

    mat4& GetModelMatrix() { return modelMatrix; }
    const mat4& GetModelMatrix() const { return modelMatrix; }

    vec3& getPos() { return pos; }
    vec3& getRot() { return rot; }
    vec3& getScale() { return scale; }

    mat4 modelMatrix;
    vec3 pos;
    vec3 rot;
    vec3 scale;

   private:
};

}  // namespace DG
