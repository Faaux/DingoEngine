/**
 *  @file    RenderableComponent.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "DG_Include.h"
#include "StringIdCRC32.h"

namespace DG
{
class RenderableComponent : public BaseComponent
{
    DECLARE_CLASS_TYPE(RenderableComponent, BaseComponent)
   public:
    RenderableComponent(Actor* actor, StringId renderableId)
        : BaseComponent(actor), RenderableId(renderableId)
    {
    }

    StringId RenderableId = "";
};
}  // namespace DG
