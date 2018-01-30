#include "DG_Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include "DG_GraphicsSystem.h"
#include "DG_ResourceHelper.h"

namespace DG::graphics
{
namespace fs = std::experimental::filesystem;
bool CheckAndLogProgramErrors(u32 handle)
{
    GLint success = 0;
    int infoLogLength;
    char text[1024];

    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            glGetProgramInfoLog(handle, infoLogLength, NULL, &text[0]);
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[error] %s\n", text);
        }
        return false;
    }

    return true;
}
bool CheckAndLogShaderErrors(u32 handle)
{
    GLint success = 0;
    int infoLogLength;
    char text[1024];

    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            glGetShaderInfoLog(handle, infoLogLength, NULL, &text[0]);
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[error] %s\n", text);
        }
        return false;
    }

    return true;
}
static bool CheckAndUpdateClocks(const fs::path& path, fs::file_time_type& fileTime)
{
    bool result = false;
    if (path != "" && fs::exists(path))
    {
        auto newFileTime = fs::last_write_time(path);
        if (fileTime != newFileTime)
        {
            result = true;
            fileTime = newFileTime;
        }
    }
    return result;
}

static u32 CompileShaderFromString(s32 i, const GLchar* file)
{
    u32 shaderId = glCreateShader(i);
    Assert(shaderId != 0);
    glShaderSource(shaderId, 1, &file, 0);
    glCompileShader(shaderId);
    if (!CheckAndLogShaderErrors(shaderId))
    {
        glDeleteShader(shaderId);
        shaderId = 0;
    }

    return shaderId;
}
static u32 CompileShaderFromFile(s32 i, std::string_view filename)
{
    std::ifstream t(filename.data());
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto source = buffer.str();
    const GLchar* file = static_cast<const GLchar*>(source.c_str());

    return CompileShaderFromString(i, file);
}

Shader::Shader(const char* shaderName)
{
    std::string shader(shaderName);

    _vertexPath = SearchForFile(shader + ".vs");
    _fragmentPath = SearchForFile(shader + ".fs");
    _geometryPath = SearchForFile(shader + ".gs");

    if (fs::exists(_vertexPath))
        _vertexFileTime = fs::last_write_time(_vertexPath);
    if (fs::exists(_fragmentPath))
        _fragmentFileTime = fs::last_write_time(_fragmentPath);
    if (fs::exists(_geometryPath))
        _geometryFileTime = fs::last_write_time(_geometryPath);

    ReloadShader();
}

bool Shader::Use()
{
    if (HasSourceChanged())
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "HotLoading Shader: %s  %s",
                    _vertexPath.string().c_str(), _fragmentPath.string().c_str());
        ReloadShader();
    }
    if (_isValid)
    {
        glUseProgram(_programId);
        return true;
    }

    return false;
}

void Shader::ReloadShader()
{
    if (_isValid)
        glDeleteProgram(_programId);
    bool isValid = true;
    u32 vertexId;
    u32 fragmentId;
    u32 geometryId = 0;

    vertexId = CompileShaderFromFile(GL_VERTEX_SHADER, _vertexPath.string());
    fragmentId = CompileShaderFromFile(GL_FRAGMENT_SHADER, _fragmentPath.string());
    if (HasGeometryShader())
        geometryId = CompileShaderFromFile(GL_GEOMETRY_SHADER, _geometryPath.string());

    if (vertexId && fragmentId && (!HasGeometryShader() || geometryId))
    {
        _programId = glCreateProgram();
        glAttachShader(_programId, vertexId);
        glAttachShader(_programId, fragmentId);
        if (HasGeometryShader())
            glAttachShader(_programId, geometryId);

        glLinkProgram(_programId);
        if (!CheckAndLogProgramErrors(_programId))
        {
            glDeleteProgram(_programId);
            isValid = false;
        }

        glDetachShader(_programId, vertexId);
        glDetachShader(_programId, fragmentId);
        if (HasGeometryShader())
            glDetachShader(_programId, geometryId);

        glDeleteShader(vertexId);
        glDeleteShader(fragmentId);
        if (HasGeometryShader())
            glDeleteShader(geometryId);
    }
    else
    {
        isValid = false;
    }

    _isValid = isValid;
}

void Shader::SetUniform(std::string_view name, const int& val)
{
    if (!Use())
    {
        return;
    }
    glUniform1i(GetUniformLocation(name), val);
}

void Shader::SetUniform(std::string_view name, const glm::ivec2& val)
{
    if (!Use())
    {
        return;
    }
    glUniform2i(GetUniformLocation(name), val.x, val.y);
}

void Shader::SetUniform(std::string_view name, const glm::ivec3& val)
{
    if (!Use())
    {
        return;
    }
    glUniform3i(GetUniformLocation(name), val.x, val.y, val.z);
}

void Shader::SetUniform(std::string_view name, const glm::ivec4& val)
{
    if (!Use())
    {
        return;
    }
    glUniform4i(GetUniformLocation(name), val.x, val.y, val.z, val.w);
}

void Shader::SetUniform(std::string_view name, const float& val)
{
    if (!Use())
    {
        return;
    }
    glUniform1f(GetUniformLocation(name), val);
}

void Shader::SetUniform(std::string_view name, const glm::vec2& val)
{
    if (!Use())
    {
        return;
    }
    glUniform2f(GetUniformLocation(name), val.x, val.y);
}

void Shader::SetUniform(std::string_view name, const glm::vec3& val)
{
    if (!Use())
    {
        return;
    }
    glUniform3f(GetUniformLocation(name), val.x, val.y, val.z);
}

void Shader::SetUniform(std::string_view name, const glm::vec4& val)
{
    if (!Use())
    {
        return;
    }
    glUniform4f(GetUniformLocation(name), val.x, val.y, val.z, val.w);
}

void Shader::SetUniform(std::string_view name, const glm::mat4& val)
{
    if (!Use())
    {
        return;
    }
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &val[0][0]);
}

s32 Shader::GetUniformLocation(std::string_view name)
{
    if (!Use())
    {
        return -1;
    }

    const s32 location = glGetUniformLocation(_programId, name.data());
    return location;
}

bool Shader::HasGeometryShader() const { return fs::exists(_geometryPath); }

bool Shader::HasSourceChanged()
{
    const bool vChanged = CheckAndUpdateClocks(_vertexPath, _vertexFileTime);
    const bool fChanged = CheckAndUpdateClocks(_fragmentPath, _fragmentFileTime);
    const bool gChanged = CheckAndUpdateClocks(_geometryPath, _geometryFileTime);

    return vChanged || fChanged || gChanged;
}
}  // namespace DG::graphics
