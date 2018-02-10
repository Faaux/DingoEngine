/**
*  @file    ResourceHelper.cpp
*  @author  Faaux (github.com/Faaux)
*  @date    11 February 2018
*/

#include "ResourceHelper.h"
#include <filesystem>
#include "Mesh.h"

namespace DG
{
using namespace DG::graphics;
namespace fs = std::experimental::filesystem;

std::vector<fs::path> FoldersToSearch{fs::path(EXPAND_AND_QUOTE(SOURCEPATH)).append("res"),
                                      fs::path(EXPAND_AND_QUOTE(SOURCEPATH)).append("shaders")};
std::string SearchForFile(const char* filename, const std::vector<fs::path>& basePaths)
{
    if (SDL_strcmp(filename, "") == 0)
        return "";

    fs::path filepath(filename);
    for (auto& folder : basePaths)
    {
        for (auto& p : fs::recursive_directory_iterator(folder))
        {
            auto candidatePath = p.path();
            auto localFilePath = filepath;
            if (candidatePath.filename() == localFilePath.filename())
            {
                // Check if all subdirectoreis fit
                bool candidateHasFilename = candidatePath.has_filename();
                bool fileHasFilename = localFilePath.has_filename();
                bool foundMismatch = false;
                while (candidateHasFilename == fileHasFilename && fileHasFilename)
                {
                    candidatePath = candidatePath.parent_path();
                    localFilePath = localFilePath.parent_path();

                    candidateHasFilename = candidatePath.has_filename();
                    fileHasFilename = localFilePath.has_filename();

                    if (fileHasFilename && candidateHasFilename &&
                        candidatePath.filename() != localFilePath.filename())
                    {
                        foundMismatch = true;
                        break;
                    }
                }

                if (!foundMismatch)
                    return p.path().string();
            }
        }
    }
    SDL_LogWarn(0, "WARNING: Could not find resource '%s'", filename);
    return "";
}

}  // namespace DG
