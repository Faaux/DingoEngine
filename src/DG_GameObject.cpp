#include "DG_GameObject.h"

namespace DG
{
void GameWorld::Update()
{
    PhysicsWorld.Update(1.0f / 60.f);
    _playerCamera.Update();
}

void GameWorld::AddGameObject(const GameObject& gameObject)
{
    Assert(_currentIndex < GameObjectBufferSize);
    _gameObjects[_currentIndex] = gameObject;
    PhysicsWorld.AddModel(_gameObjects[_currentIndex]);
    ++_currentIndex;
}
void GameWorld::Shutdown()
{
    PhysicsWorld.Shutdown();
}
}  // namespace DG
