#pragma once
#include <array>
#include "DG_Include.h"
#include "DG_StringIdCRC32.h"
#include "DG_Transform.h"

namespace DG
{
struct SpawnGameObjectMessage
{
    StringId Entity;
};

class GameObject
{
   public:
    GameObject(StringId model) : _modelId(model){};

    StringId GetModelId() const { return _modelId; }

   private:
    Transform _transform;
    StringId _modelId;
};

class GameWorld
{
    enum
    {
        GameObjectBufferSize = 256
    };

   public:
    GameWorld() = default;

    void AddGameObject(const GameObject& gameObject);

    // ToDO: Make private again
   public:
    u32 _currentIndex = 0;
    std::array<GameObject, GameObjectBufferSize> _gameObjects;
};
}  // namespace DG
