/**
 *  @file    BoundingBox.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    08 February 2018
 */

#include "BoundingBox.h"
#include "engine/Types.h"

namespace DG
{
AABB TransformAABB(const AABB& aabb, const Transform& transform)
{
    AABB result;
    f32 a, b;

    // Copy box A into min and max array.
    vec3 aMin = aabb.Min;
    vec3 aMax = aabb.Max;

    // Begin at T.
    result.Min = result.Max = transform.GetPosition();

    // Find extreme points by considering product of
    // min and max with each component of M.
    auto& model = transform.GetModelMatrix();
    for (s32 j = 0; j < 3; j++)
    {
        for (s32 i = 0; i < 3; i++)
        {
            a = model[i][j] * aMin[i];
            b = model[i][j] * aMax[i];

            if (a < b)
            {
                result.Min[j] += a;
                result.Max[j] += b;
            }
            else
            {
                result.Min[j] += b;
                result.Max[j] += a;
            }
        }
    }

    return result;
}

AABB CombineAABB(const AABB& f, const AABB& s)
{
    AABB result;
    result.Min.x = glm::min(f.Min.x, s.Min.x);
    result.Min.y = glm::min(f.Min.y, s.Min.y);
    result.Min.z = glm::min(f.Min.z, s.Min.z);

    result.Max.x = glm::max(f.Max.x, s.Max.x);
    result.Max.y = glm::max(f.Max.y, s.Max.y);
    result.Max.z = glm::max(f.Max.z, s.Max.z);
    return result;
}
}  // namespace DG
