#pragma once
#include "DG_GameObject.h"
#include "DG_GameWorld.h"
#include "DG_Include.h"

namespace DG
{
class WorldEdit
{
   public:
    WorldEdit();

    void Update();
    GameWorld* GetWorld();

   private:
    GameWorld _gameWorld;
    GameObject* _selectedGameModel;
    InputMessage _lastInputMessage;
    bool _lastInputMessageHandled = true;
};

vec3 GetMouseRayGameClient(const InputMessage& message, const Camera& camera);
}  // namespace DG
