/**
 *  @file    DG_GameWorld.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    07 February 2018
 */

#include "DG_GameWorld.h"

namespace DG
{
GameWorld::GameWorld(glm::vec3 playerCameraPos, const Clock& clock)
    : _clock(&clock),
      _playerCamera(playerCameraPos, vec3(0), vec3(0, 1, 0), 45.f, 0.01f, 1000.f, 16.f / 9.f),
      _callback(g_MessagingSystem.RegisterCallback<InputMessage>([=](const InputMessage& message) {
          _lastInputMessage = message;
          _lastInputMessageHandled = false;
      }))
{
    PhysicsWorld.Init(clock);
}

void GameWorld::Update()
{
    PhysicsWorld.Update();
    if (!_clock->IsPaused() && !_lastInputMessageHandled)
    {
        _lastInputMessageHandled = true;
        UpdateFreeCameraFromInput(_playerCamera, _lastInputMessage, *_clock);
    }
}

void GameWorld::AddGameObject(const GameObject& gameObject, bool forEdit)
{
    Assert(_currentIndex < GameObjectBufferSize);
    _gameObjects[_currentIndex] = gameObject;
    PhysicsWorld.AddModel(_gameObjects[_currentIndex], forEdit);
    ++_currentIndex;
}
void GameWorld::Shutdown()
{
    for (u32 i = 0; i < _currentIndex; ++i)
    {
        PhysicsWorld.RemoveModel(_gameObjects[i]);
    }
    PhysicsWorld.Shutdown();
    g_MessagingSystem.UnRegisterCallback(_callback);
}

void CopyGameWorld(GameWorld* dest, const GameWorld* source)
{
    new (dest) GameWorld(vec3(0, 10, 20), g_InGameClock);
    dest->_currentIndex = source->_currentIndex;
    for (u32 i = 0; i < source->_currentIndex; ++i)
    {
        dest->_gameObjects[i] = source->_gameObjects[i];
        dest->PhysicsWorld.AddModel(dest->_gameObjects[i], false);
    }
}
}  // namespace DG
