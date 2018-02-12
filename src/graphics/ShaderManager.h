/**
 *  @file    ShaderManager.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "Shader.h"
#include "platform/ResourceManager.h"

namespace DG::graphics
{
class ShaderManager : public ResourceManager<Shader>
{
   public:
    ShaderManager() = default;
    Shader* LoadOrGet(StringId id, const char* shaderName);
};
}  // namespace DG::graphics
