/**
 *  @file    Actor.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#include "Actor.h"
#include "components/SceneComponent.h"

namespace DG
{
Actor::Actor(GameWorld* gameWorld) : _gameWorld(gameWorld)
{
    _rootSceneComponent = RegisterComponent<SceneComponent>();
}

Actor::~Actor()
{
    for (auto& component : _components)
    {
        _gameWorld->DestroyComponent(component);
    }
}

void Actor::DeRegisterComponent(BaseComponent* component)
{
    const auto it = std::find(_components.begin(), _components.end(), component);
    Assert(it != _components.end());
    _gameWorld->DestroyComponent(component);
    _components.erase(it);
}

GameWorld* Actor::GetGameWorld() const { return _gameWorld; }

SceneComponent* Actor::GetRootSceneComponent() const { return _rootSceneComponent; }

BaseComponent* Actor::GetFirstComponentOfType(TypeId type, bool orSubtype)
{
    for (auto& component : _components)
    {
        if (orSubtype)
        {
            if (component->IsType(type))
                return component;
        }
        else
        {
            if (component->IsTypeOrDerivedType(type))
                return component;
        }
    }
    return nullptr;
}

std::vector<BaseComponent*> Actor::GetComponentsOfType(TypeId type, bool orSubtype)
{
    std::vector<BaseComponent*> result;
    for (auto& component : _components)
    {
        if (orSubtype)
        {
            if (component->IsType(type))
                result.push_back(component);
        }
        else
        {
            if (component->IsTypeOrDerivedType(type))
                result.push_back(component);
        }
    }
    return result;
}
}  // namespace DG
