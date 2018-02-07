#pragma once
#include "DG_GameObject.h"
#include "DG_Include.h"

namespace DG
{
class WorldEdit
{
   public:
    WorldEdit(GameWorld* world);

    void Update();
    Camera& GetEditCamera();

   private:
    Camera _camera;
    GameWorld* _world;
    GameObject* _selectedGameModel;
    InputMessage _lastInputMessage;
};
}  // namespace DG
