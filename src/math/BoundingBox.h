/**
 *  @file    BoundingBox.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    08 February 2018
 */

#pragma once
#include "GLMInclude.h"
#include "Transform.h"

namespace DG
{
struct AABB
{
    vec3 Min;
    vec3 Max;
};

AABB TransformAABB(const AABB& aabb, const Transform& transform);

AABB CombineAABB(const AABB& f, const AABB& s);
}  // namespace DG
