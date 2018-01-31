#pragma once
#include "DG_Include.h"
#include "DG_Mesh.h"
#include "DG_ResourceManager.h"

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
