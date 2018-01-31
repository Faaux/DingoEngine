#pragma once
#include "DG_Include.h"
#include "DG_Mesh.h"
#include "DG_ResourceManager.h"

namespace DG
{
class ModelManager : public ResourceManager<graphics::Model>
{
   public:
    ModelManager() = default;
    graphics::Model* LoadOrGet(StringId id, graphics::GLTFScene* scene, graphics::Shader* shader);
};
}  // namespace DG
