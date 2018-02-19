/**
 *  @file    SceneComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "math/Transform.h"

namespace DG
{
class SceneComponent : public BaseComponent
{
   private:
    DECLARE_CLASS_TYPE(SceneComponent, BaseComponent)
   public:
    explicit SceneComponent(Actor* actor) : BaseComponent(actor) {}

    mat4 GetGlobalModelMatrix() const;

    DPROPERTY Transform Transform;
    DPROPERTY SceneComponent* Parent = nullptr;
};
}  // namespace DG
