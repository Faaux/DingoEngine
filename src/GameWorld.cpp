/**
 *  @file    DG_GameWorld.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    07 February 2018
 */

#include "GameWorld.h"

namespace DG
{
void CopyGameWorld(GameWorld* dest, const GameWorld* source)
{
    /*new (dest) GameWorld(vec3(0, 10, 20), g_InGameClock);
    dest->_currentIndex = source->_currentIndex;
    for (u32 i = 0; i < source->_currentIndex; ++i)
    {
        dest->_gameObjects[i] = source->_gameObjects[i];
        dest->PhysicsWorld.AddModel(dest->_gameObjects[i], false);
    }*/
}

GameWorld::GameWorld()
{
    // ToDo: This leaks for now
    const u32 worldMemorySize = 5 * 1024 * 1024;
    const u32 actorMemorySize = worldMemorySize / 10;
    _worldMemory.Init((u8*)malloc(worldMemorySize), worldMemorySize);
    _actorMemory.Init(_worldMemory.Push(actorMemorySize, 16), actorMemorySize);
    PhysicsWorld.Init(_worldClock);
}

GameWorld::~GameWorld() {}

void GameWorld::Shutdown()
{
    Assert(!_isShutdown);
    _isShutdown = true;
    _worldMemory.Reset();
    _actorMemory.Reset();
}

void GameWorld::DestroyActor(Actor* actor)
{
    Assert(!_isShutdown);
    actor->~Actor();
    _actorMemory.Pop(actor);
}

void GameWorld::Update()
{
    _worldClock.Update(1.f / 60.f);
    PhysicsWorld.Update();
}

Camera& GameWorld::GetPlayerCamera()
{
    static Camera camera(vec3(1), vec3(0), vec3(0, 1, 0), 45.f, 0.01f, 1000.f, 16.f / 9.f);
    return camera;
}

void GameWorld::ReleaseComponent(BaseComponent* component)
{
    Assert(!_isShutdown);
    Assert(_componentStorages[component->GetInstanceType()]);
    _componentStorages[component->GetInstanceType()]->ReleaseComponent(component);
}
}  // namespace DG
