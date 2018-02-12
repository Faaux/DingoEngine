/**
 *  @file    GLTFSceneManager.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "Mesh.h"
#include "platform/ResourceManager.h"

namespace DG::graphics
{
GLTFScene* LoadGLTF(const char* f);

class GLTFSceneManager : public ResourceManager<GLTFScene*>
{
   public:
    GLTFSceneManager() = default;
    GLTFScene* LoadOrGet(StringId id, const char* pathToGltf);
};
}  // namespace DG::graphics
