#include "ShaderManager.h"

namespace DG::graphics
{
Shader* ShaderManager::LoadOrGet(StringId id, const char* shaderName)
{
    return RegisterAndConstruct(id, shaderName);
}
}  // namespace DG::graphics
