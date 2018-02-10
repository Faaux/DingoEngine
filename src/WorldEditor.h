#pragma once
#include "DG_Include.h"
#include "GameWorld.h"

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
    Actor* _selectedActor;
    InputMessage _lastInputMessage;
    bool _lastInputMessageHandled = true;
};

vec3 GetMouseRayGameClient(const InputMessage& message, const Camera& camera);
}  // namespace DG
