/**
 *  @file    WorldEditor.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
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