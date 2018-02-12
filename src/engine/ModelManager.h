/**
 *  @file    ModelManager.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "graphics/GraphicsSystem.h"
#include "platform/ResourceManager.h"

namespace DG
{
class ModelManager : public ResourceManager<graphics::GraphicsModel>
{
   public:
    ModelManager() = default;
    graphics::GraphicsModel* LoadOrGet(StringId id, graphics::GLTFScene* scene,
                                       graphics::Shader* shader);
};
}  // namespace DG
