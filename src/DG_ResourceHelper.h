#pragma once
#include "DG_Include.h"
#include "DG_Mesh.h"

#include <filesystem>
#include <string>
#include <vector>
#include "DG_ResourceManager.h"

namespace DG
{
extern std::vector<std::experimental::filesystem::path> FoldersToSearch;

std::string SearchForFile(
    const std::string_view& filename,
    const std::vector<std::experimental::filesystem::path>& basePaths = FoldersToSearch);

graphics::GLTFScene* LoadGLTF(const std::string_view& filename);

class GLTFSceneManager : public ResourceManager<graphics::GLTFScene*>
{
   public:
    GLTFSceneManager() = default;
    graphics::GLTFScene* LoadOrGet(StringId id, const char* pathToGltf);
};

class ModelManager : public ResourceManager<graphics::Model>
{
   public:
    ModelManager() = default;
    graphics::Model* LoadOrGet(StringId id, graphics::GLTFScene* scene, graphics::Shader* shader);
};

class ShaderManager : public ResourceManager<graphics::Shader>
{
   public:
    ShaderManager() = default;
    graphics::Shader* LoadOrGet(StringId id, const char* shaderName);
};
}  // namespace DG
