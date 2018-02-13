/**
 *  @file    GameWorld.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    07 February 2018
 */

#include "GameWorld.h"

namespace DG
{
void GameWorld::Startup(u8* worldMemory, s32 worldMemorySize)
{
    Assert(!_isShutdown);
    _worldMemory.Init(worldMemory, worldMemorySize);
    _actorMemory.Init(_worldMemory.Push(worldMemorySize / 10, 16), worldMemorySize / 10);
    _physicsWorld.Init(_worldClock);
}

void GameWorld::Shutdown()
{
    Assert(!_isShutdown);
    _physicsWorld.Shutdown();
    _worldMemory.Reset();
    _actorMemory.Reset();
    _isShutdown = true;
}

void GameWorld::DestroyActor(Actor* actor)
{
    Assert(!_isShutdown);
    actor->~Actor();
    _actorMemory.Pop(actor);
}

Camera* GameWorld::GetActiveCamera()
{
    static Camera camera(vec3(1), vec3(0), vec3(0, 1, 0), 45.f, 0.01f, 100.f, 16.f / 9.f);
    return &camera;
}

void GameWorld::Update()
{
    Assert(!_isShutdown);
    _worldClock.Update(1.f / 60.f);
    _physicsWorld.Update();
}

void GameWorld::DestroyComponent(BaseComponent* component)
{
    Assert(!_isShutdown);
    Assert(_componentStorages[component->GetInstanceType()]);
    _componentStorages[component->GetInstanceType()]->DestroyComponent(component);
}
}  // namespace DG
