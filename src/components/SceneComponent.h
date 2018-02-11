/**
 *  @file    SceneComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "DG_Include.h"
#include "Transform.h"

namespace DG
{
class SceneComponent : public BaseComponent
{
   private:
    DECLARE_CLASS_TYPE(SceneComponent, BaseComponent)
   public:
    explicit SceneComponent(Actor* actor) : BaseComponent(actor) {}

   private:
    DPROPERTY Transform transform;
    DPROPERTY SceneComponent* parent = nullptr;
};
}  // namespace DG
