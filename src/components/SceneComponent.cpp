/**
 *  @file    SceneComponent.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#include "SceneComponent.h"

namespace DG
{
mat4 SceneComponent::GetGlobalModelMatrix() const
{
    auto parent = Parent;
    mat4 model = Transform.GetModelMatrix();
    while (parent)
    {
        model = parent->Transform.GetModelMatrix() * model;
        parent = parent->Parent;
    }
    return model;
}
}  // namespace DG
