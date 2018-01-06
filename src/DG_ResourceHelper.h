#pragma once
#include <tiny_gltf.h>
#include <filesystem>
#include <string>
#include <vector>
#include "DG_Include.h"
#include "DG_Mesh.h"

namespace DG
{
extern std::vector<std::experimental::filesystem::path> FoldersToSearch;
std::string SearchForFile(
    const std::string_view& filename,
    const std::vector<std::experimental::filesystem::path>& basePaths = FoldersToSearch);
Scene* LoadGLTF(const std::string_view& filename);
}  // namespace DG
