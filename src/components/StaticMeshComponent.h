/**
 *  @file    StaticMeshComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    19 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "SceneComponent.h"
#include "platform/StringIdCRC32.h"

namespace DG
{
class StaticMeshComponent : public SceneComponent
{
    DECLARE_CLASS_TYPE(StaticMeshComponent, BaseComponent)
   public:
    StaticMeshComponent(Actor* actor) : SceneComponent(actor) {}

    DPROPERTY StringId RenderableId = "";
};
}  // namespace DG
