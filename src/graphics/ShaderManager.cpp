/**
 *  @file    ShaderManager.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#include "ShaderManager.h"

namespace DG::graphics
{
Shader* ShaderManager::LoadOrGet(StringId id, const char* shaderName)
{
    return RegisterAndConstruct(id, shaderName);
}
}  // namespace DG::graphics
