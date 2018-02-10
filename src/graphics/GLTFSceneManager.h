/**
 *  @file    GLTFSceneManager.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "DG_Include.h"
#include "Mesh.h"
#include "ResourceManager.h"

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
