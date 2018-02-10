/**
 *  @file    SceneComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "DG_Include.h"

namespace DG
{
class SceneComponent : public BaseComponent
{
    DECLARE_CLASS_TYPE(SceneComponent, BaseComponent)
   public:
    SceneComponent(Actor* actor) : BaseComponent(actor) {}
};
}  // namespace DG
