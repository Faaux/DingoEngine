/**
*  @file    ModelManager.h
*  @author  Faaux (github.com/Faaux)
*  @date    11 February 2018
*/

#pragma once
#include "DG_Include.h"
#include "Mesh.h"
#include "ResourceManager.h"

namespace DG
{
class ModelManager : public ResourceManager<graphics::GraphicsModel>
{
   public:
    ModelManager() = default;
    graphics::GraphicsModel* LoadOrGet(StringId id, graphics::GLTFScene* scene, graphics::Shader* shader);
};
}  // namespace DG