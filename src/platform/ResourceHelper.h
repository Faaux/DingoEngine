/**
 *  @file    ResourceHelper.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace DG
{
extern std::vector<std::experimental::filesystem::path> FoldersToSearch;

std::string SearchForFile(
    const char* filename,
    const std::vector<std::experimental::filesystem::path>& basePaths = FoldersToSearch);

}  // namespace DG
