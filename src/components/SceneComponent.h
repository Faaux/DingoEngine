/**
 *  @file    SceneComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "math/Transform.h"
#include "gameobjects/Actor.h"
namespace DG
{
class SceneComponent : public BaseComponent
{
    DECLARE_CLASS_TYPE(SceneComponent, BaseComponent)
   public:
    explicit SceneComponent(Actor* actor) : BaseComponent(actor)
    {
        auto parent = actor->GetRootSceneComponent();
        if (parent != this)
            _parent = parent;
    }

    mat4 GetGlobalModelMatrix() const;

   protected:
    DPROPERTY Transform _transform;
    DPROPERTY SceneComponent* _parent = nullptr;
};
}  // namespace DG
