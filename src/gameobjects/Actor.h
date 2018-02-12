/**
 *  @file    Actor.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "components/BaseComponent.h"
#include "engine/GameWorld.h"

namespace DG
{
class Actor : public TypeBase
{
    DECLARE_CLASS_TYPE(Actor, TypeBase)

   public:
    Actor(GameWorld* gameWorld);

    virtual ~Actor();

    template <typename T>
    T* RegisterComponent();

    void DeRegisterComponent(BaseComponent* component);

    BaseComponent* GetFirstComponentOfType(TypeId type, bool orSubtype = true);
    std::vector<BaseComponent*> GetComponentsOfType(TypeId type, bool orSubtype = true);

   private:
    GameWorld* _gameWorld;
    std::vector<BaseComponent*> _components;
};

template <typename T>
T* Actor::RegisterComponent()
{
    T* component = _gameWorld->CreateComponent<T>(this);
    _components.push_back(component);
    return component;
}
}  // namespace DG
