/**
 *  @file    DG_GameWorld.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    07 February 2018
 */

#pragma once
#include "Camera.h"
#include "DG_Include.h"
#include "GameObject.h"

namespace DG
{
class GameWorld
{
    friend void CopyGameWorld(GameWorld* dest, const GameWorld* source);

   public:
    enum
    {
        GameObjectBufferSize = 4096
    };

    GameWorld(glm::vec3 playerCameraPos, const Clock& clock);

    void Update();
    void AddGameObject(const GameObject& gameObject, bool forEdit);
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
    const Clock* _clock;
    Camera _playerCamera;
    u32 _currentIndex = 0;
    InputMessage _lastInputMessage;
    bool _lastInputMessageHandled = true;
    std::array<GameObject, GameObjectBufferSize> _gameObjects;
    CallbackHandle<InputMessage> _callback;
};

void CopyGameWorld(GameWorld* dest, const GameWorld* source);
}  // namespace DG
