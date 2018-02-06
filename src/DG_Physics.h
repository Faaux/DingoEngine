/**
 *  @file    DG_Physics.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#pragma once
#include "DG_Include.h"
#include "DG_Mesh.h"

namespace DG
{
class GameObject;
class PhysicsWorld
{
   public:
    PhysicsWorld() = default;
    bool Init();
    void ToggleDebugVisualization();
    void Update(f32 timeStep);
    bool RayCast(vec3 origin, vec3 unitDir);
    void CookModel(graphics::GraphicsModel* model);
    void Shutdown();
    void AddModel(GameObject& obj);
    void AddForce(vec3 dir, float strength);

   private:
    bool _outputDebugLines = false;
};

}  // namespace DG
