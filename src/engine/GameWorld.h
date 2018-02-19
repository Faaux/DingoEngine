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
    struct Input
    {
        float Right = 0.f;
        float Forward = 0.f;
        float Up = 0.f;
        float MouseWheel = 0.f;
        bool MouseLeftDown = false;
        bool MouseRightDown = false;
        bool MouseMiddleDown = false;
        bool MouseLeftPressed = false;
        bool MouseRightPressed = false;
        bool MouseMiddlePressed = false;
        f32 MouseDeltaX = 0;
        f32 MouseDeltaY = 0;
        f32 MouseX = 0;
        f32 MouseY = 0;
        f32 ScreenWidth = 0;
        f32 ScreenHeight = 0;
    };

    GameWorld() = default;
    ~GameWorld() = default;

    void Startup(u8* worldMemory, s32 worldMemorySize);
    void Shutdown();

    template <typename T, typename... Args>
    T* CreateActor(Args&&... args);
    void DestroyActor(Actor* actor);
    const std::vector<Actor*>& GetAllActors() const;

    Camera* GetActiveCamera();
    vec3 GetMouseRay() const;
    const Input& GetLastInput() const;

    void Update(float dtSeconds);
    void SetInput(Input input);

   private:
    template <typename T>
    T* CreateComponent(Actor* actor);
    void DestroyComponent(BaseComponent* component);

    Camera _camera;  // ToDo(Faaux)(Default): Remove and put into component
    bool _isNewInput;
    Input _input;
    StackAllocator _actorMemory;
    StackAllocator _worldMemory;
    PhysicsWorld _physicsWorld;
    Clock _worldClock;
    bool _isShutdown = false;

    std::vector<Actor*> _actors;
    std::unordered_map<TypeId, BaseComponentStorage*> _componentStorages;
};

template <typename T, typename... Args>
T* GameWorld::CreateActor(Args&&... args)
{
    static_assert(std::is_base_of<Actor, T>::value, "T not derived from Actor");

    Assert(!_isShutdown);
    auto result = _actorMemory.PushAndConstruct<T>(this, std::forward<Args>(args)...);
    _actors.push_back(result);
    return result;
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
