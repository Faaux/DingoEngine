#include "DG_GameObject.h"

namespace DG
{
GameWorld::GameWorld(glm::vec3 playerCameraPos)
    : _playerCamera(playerCameraPos, vec3(0), vec3(0, 1, 0), 45.f, 0.01f, 1000.f, 16.f / 9.f)
{
    PhysicsWorld.Init();
    g_MessagingSystem.RegisterCallback<InputMessage>(
        [=](const InputMessage& message) { _lastInputMessage = message; });
}

void GameWorld::Update()
{
    PhysicsWorld.Update();
    if (!g_InGameClock.IsPaused())
    {
        UpdateFreeCameraFromInput(_playerCamera, _lastInputMessage, g_InGameClock);
    }
}

void GameWorld::AddGameObject(const GameObject& gameObject)
{
    Assert(_currentIndex < GameObjectBufferSize);
    _gameObjects[_currentIndex] = gameObject;
    PhysicsWorld.AddModel(_gameObjects[_currentIndex]);
    ++_currentIndex;
}
void GameWorld::Shutdown() { PhysicsWorld.Shutdown(); }
}  // namespace DG
