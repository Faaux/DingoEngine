/**
 *  @file    GameWorld.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    07 February 2018
 */

#pragma once
#include <unordered_map>
#include "Camera.h"
#include "components/BaseComponent.h"
#include "components/ComponentStorage.h"
#include "gameobjects/Actor.h"
#include "physics/Physics.h"

namespace DG
{
class GameWorld
{
    friend class Actor;

   public:
    GameWorld();
    ~GameWorld();

    void Shutdown();

    template <typename T, typename... Args>
    T* CreateNewActor(Args&&... args);

    void DestroyActor(Actor* actor);
    void Update();
    Camera& GetPlayerCamera();

    PhysicsWorld PhysicsWorld;

   private:
    Clock _worldClock;

    bool _isShutdown = false;

    template <typename T>
    T* CreateComponent(Actor* actor);

    void ReleaseComponent(BaseComponent* component);

    StackAllocator _actorMemory;
    StackAllocator _worldMemory;
    std::unordered_map<TypeId, BaseComponentStorage*> _componentStorages;
};

template <typename T, typename... Args>
T* GameWorld::CreateNewActor(Args&&... args)
{
    static_assert(std::is_base_of<Actor, T>::value, "T not derived from Actor");

    Assert(!_isShutdown);
    return _actorMemory.PushAndConstruct<T>(this, std::forward<Args>(args)...);
}

template <typename T>
T* GameWorld::CreateComponent(Actor* actor)
{
    static_assert(std::is_base_of<BaseComponent, T>::value, "T not derived from BaseComponent");

    Assert(!_isShutdown);
    if (!_componentStorages[T::GetClassType()])
    {
        // Create Component Storage

        _componentStorages[T::GetClassType()] =
            _worldMemory.PushAndConstruct<ComponentStorage<T>>(&_worldMemory);
    }

    ComponentStorage<T>* storage = (ComponentStorage<T>*)(_componentStorages[T::GetClassType()]);

    // Create component
    T* component = storage->CreateComponent();

    new (component) T(actor);
    return component;
}
}  // namespace DG