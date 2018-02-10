/**
*  @file    ModelManager.cpp
*  @author  Faaux (github.com/Faaux)
*  @date    11 February 2018
*/

#include "ModelManager.h"

namespace DG
{
using namespace DG::graphics;
GraphicsModel* ModelManager::LoadOrGet(StringId id, GLTFScene* scene, Shader* shader)
{
    Assert(scene);
    Assert(shader);

    return RegisterAndConstruct(id, *scene, *shader, id);
}
}  // namespace DG
