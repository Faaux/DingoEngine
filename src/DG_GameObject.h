#pragma once
#include <array>
#include "DG_Camera.h"
#include "DG_Include.h"
#include "DG_StringIdCRC32.h"
#include "DG_Transform.h"
#include "DG_Physics.h"

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

    GameWorld(glm::vec3 playerCameraPos)
        : _playerCamera(45.f, 0.01f, 10000.f, 1280.f / 720.f, playerCameraPos, vec3(0))
    {
        PhysicsWorld.Init();
    }

    void Update();
    void AddGameObject(const GameObject& gameObject);
    void Shutdown();

    PhysicsWorld PhysicsWorld;
    // ToDO: Make private again
   public:
       
    Camera _playerCamera;
    u32 _currentIndex = 0;
    std::array<GameObject, GameObjectBufferSize> _gameObjects;
};
}  // namespace DG
