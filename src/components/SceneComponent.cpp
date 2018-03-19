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
    auto parent = _parent;
    mat4 model = _transform.GetModelMatrix();
    while (parent)
    {
        model = parent->_transform.GetModelMatrix() * model;
        parent = parent->_parent;
    }
    return model;
}
}  // namespace DG
