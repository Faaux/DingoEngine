#pragma once
#include <filesystem>
#include <string>
#include "DG_Include.h"

namespace DG
{
namespace graphics
{
class Shader
{
   public:
    bool Use();
    u32 GetProgramId() const { return _programId; }
    Shader(std::string_view vertexFilename, std::string_view fragmentFilename,
           std::string_view geometryFilename);

    void SetUniform(std::string_view, const int &);
    void SetUniform(std::string_view, const glm::ivec2 &);
    void SetUniform(std::string_view, const glm::ivec3 &);
    void SetUniform(std::string_view, const glm::ivec4 &);

    void SetUniform(std::string_view, const float &);
    void SetUniform(std::string_view, const glm::vec2 &);
    void SetUniform(std::string_view, const glm::vec3 &);
    void SetUniform(std::string_view, const glm::vec4 &);

    void SetUniform(std::string_view, const glm::mat4 &);

   protected:
    void ReloadShader();

   private:
    s32 GetUniformLocation(std::string_view name);
    bool HasGeometryShader() const;

    bool HasSourceChanged();

    bool _isValid = false;
    bool _hasFiles = true;
    u32 _programId = 0;

    std::experimental::filesystem::path _vertexPath;
    std::experimental::filesystem::path _fragmentPath;
    std::experimental::filesystem::path _geometryPath;

    // For HotShader Reload
    std::experimental::filesystem::file_time_type _vertexFileTime;
    std::experimental::filesystem::file_time_type _fragmentFileTime;
    std::experimental::filesystem::file_time_type _geometryFileTime;
};
}  // namespace graphics

}  // namespace DG
