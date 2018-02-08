/**
 *  @file    DG_Physics.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#pragma once
#include "DG_Clock.h"
#include "DG_Include.h"
#include "DG_Mesh.h"

namespace DG
{
class GameObject;
class PhysicsWorld
{
   public:
    PhysicsWorld() = default;
    bool Init(const Clock& clock);
    void ToggleDebugVisualization();
    void Update();
    void* RayCast(vec3 origin, vec3 unitDir);
    void Shutdown();
    void AddModel(GameObject& obj, bool forEditing);
    void RemoveModel(GameObject& obj);

   private:
    const Clock* _clock;
    bool _outputDebugLines = false;
};
void CookModel(graphics::GraphicsModel* model);
bool InitPhysics();
bool ShutdownPhysics();

}  // namespace DG
