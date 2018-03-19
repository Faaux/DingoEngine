/**
 *  @file    StaticMeshComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    19 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "SceneComponent.h"
#include "main.h"
#include "platform/StringIdCRC32.h"

namespace DG
{
class StaticMeshComponent : public SceneComponent
{
    DECLARE_CLASS_TYPE(StaticMeshComponent, SceneComponent)
   public:
    StaticMeshComponent(Actor* actor, StringId renderableId, Transform transform)
        : SceneComponent(actor), _renderableId(renderableId)
    {
        auto model = g_Managers->ModelManager->Exists(renderableId);
        Assert(model);

        _transform = transform;

        _physicsData = actor->GetGameWorld()->GetPhysicsWorld()->AddStaticModel(
            *model, _transform.GetModelMatrix() * model->meshes[0].localTransform, this);
    }

    StringId GetRenderable() const { return _renderableId; }

   private:
    void* _physicsData;
    DPROPERTY StringId _renderableId = "";
};
}  // namespace DG
