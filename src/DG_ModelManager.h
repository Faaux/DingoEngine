#pragma once
#include "DG_Include.h"
#include "DG_Mesh.h"
#include "DG_ResourceManager.h"

namespace DG
{
class ModelManager : public ResourceManager<graphics::GraphicsModel>
{
   public:
    ModelManager() = default;
    graphics::GraphicsModel* LoadOrGet(StringId id, graphics::GLTFScene* scene, graphics::Shader* shader);
};
}  // namespace DG
