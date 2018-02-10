/**
 *  @file    BaseComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "DG_Include.h"
#include "Type.h"

namespace DG
{
class GameWorld;
class Actor;

class BaseComponent : public TypeBase
{
    DECLARE_CLASS_TYPE(BaseComponent, TypeBase)
   public:
    BaseComponent(Actor* actor) : _actor(actor) {}

    Actor* GetOwningActor() const { return _actor; }

   private:
    Actor* _actor;
};

}  // namespace DG
