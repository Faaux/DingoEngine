/**
 *  @file    Physics.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#pragma once
#include "graphics/GraphicsSystem.h"
#include "math/GLMInclude.h"
#include "platform/Clock.h"

namespace DG
{
class PhysicsWorld
{
   public:
    PhysicsWorld() = default;
    bool Init(const Clock& clock);
    void ToggleDebugVisualization();
    void Update();
    void* RayCast(vec3 origin, vec3 unitDir);
    void Shutdown();
    void* AddStaticModel(const graphics::GraphicsModel& model, Transform worldTransform,
                         void* userData);
    void RemoveModel(void* model);

   private:
    const Clock* _clock;
    bool _outputDebugLines = false;
};
void CookModel(const graphics::GraphicsModel& model);
bool InitPhysics();
bool ShutdownPhysics();

}  // namespace DG
