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
    void Startup(StackAllocator* allocator);
    void Shutdown();

private:
    GameWorld _gameWorld;
    Actor* _selectedActor;
    bool _lastInputMessageHandled = true;
};
}  // namespace DG
