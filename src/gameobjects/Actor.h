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
class SceneComponent;

class Actor : public TypeBase
{
    DECLARE_CLASS_TYPE(Actor, TypeBase)

   public:
    Actor(GameWorld* gameWorld);
    virtual ~Actor();

    template <typename T, typename... Args>
    T* RegisterComponent(Args&&... args);
    void DeRegisterComponent(BaseComponent* component);

    GameWorld* GetGameWorld() const;
    SceneComponent* GetRootSceneComponent() const;
    BaseComponent* GetFirstComponentOfType(TypeId type, bool orSubtype = true);
    std::vector<BaseComponent*> GetComponentsOfType(TypeId type, bool orSubtype = true);

   private:
    GameWorld* _gameWorld;
    SceneComponent* _rootSceneComponent;
    std::vector<BaseComponent*> _components;
};

template <typename T, typename... Args>
T* Actor::RegisterComponent(Args&&... args)
{
    T* component = _gameWorld->CreateComponent<T>(this, std::forward<Args>(args)...);
    _components.push_back(component);
    return component;
}
}  // namespace DG
