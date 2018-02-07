#pragma once
#include <array>
#include "DG_Camera.h"
#include "DG_Include.h"
#include "DG_Physics.h"
#include "DG_StringIdCRC32.h"
#include "DG_Transform.h"

namespace DG
{
class GameObject
{
   public:
    GameObject(StringId model) : _modelId(model) {}
    GameObject() = default;

    Transform& GetTransform() { return _transform; }
    const Transform& GetTransform() const { return _transform; }
    StringId GetModelId() const { return _modelId; }
    char* GetName() { return _name; }

   private:
    Transform _transform;
    StringId _modelId;
    char _name[256] = {};
};

class GameWorld
{
   public:
    enum
    {
        GameObjectBufferSize = 4096
    };

    GameWorld(glm::vec3 playerCameraPos);

    void Update();
    void AddGameObject(const GameObject& gameObject);
    void Shutdown();

    void SetCamera(const Camera& camera) { _playerCamera = camera; }
    u32 GetGameObjectCount() const { return _currentIndex; }
    GameObject& GetGameObject(u32 index)
    {
        Assert(index < _currentIndex);
        return _gameObjects[index];
    }

    Camera& GetPlayerCamera() { return _playerCamera; }
    PhysicsWorld PhysicsWorld;

   private:
    Camera _playerCamera;
    u32 _currentIndex = 0;
    InputMessage _lastInputMessage;
    std::array<GameObject, GameObjectBufferSize> _gameObjects;
};
}  // namespace DG
