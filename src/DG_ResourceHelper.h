#pragma once
#include "DG_Include.h"
#include "DG_Mesh.h"

#include <filesystem>
#include <string>
#include <vector>

namespace DG
{
extern std::vector<std::experimental::filesystem::path> FoldersToSearch;

std::string SearchForFile(
    const std::string_view& filename,
    const std::vector<std::experimental::filesystem::path>& basePaths = FoldersToSearch);

graphics::GLTFScene* LoadGLTF(const std::string_view& filename);
}  // namespace DG
