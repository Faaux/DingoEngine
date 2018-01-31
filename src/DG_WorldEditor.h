#pragma once
#include "DG_GameObject.h"
#include "DG_Include.h"

namespace DG
{
class WorldEdit
{
   public:
    WorldEdit(GameWorld& world) : _world(world) {}

    void Update();

   private:
    GameWorld& _world;
    GameObject* _selectedGameModel;
};
}  // namespace DG
