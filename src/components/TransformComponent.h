/**
 *  @file    TransformComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "DG_Include.h"
#include "Transform.h"

namespace DG
{
class TransformComponent : public BaseComponent
{
    DECLARE_CLASS_TYPE(TransformComponent, BaseComponent)
   public:
    TransformComponent(Actor* actor) : BaseComponent(actor) {}

    operator Transform&() { return transform; }

   private:
    Transform transform;
};
}  // namespace DG
