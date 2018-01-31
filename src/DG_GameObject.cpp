#include "DG_GameObject.h"

namespace DG
{
void GameWorld::AddGameObject(const GameObject& gameObject)
{
    Assert(_currentIndex < GameObjectBufferSize);
    _gameObjects[_currentIndex++] = gameObject;
}
}  // namespace DG
