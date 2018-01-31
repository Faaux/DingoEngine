#pragma once
#include "DG_Include.h"
#include "DG_ResourceManager.h"
#include "DG_Shader.h"

namespace DG::graphics
{
class ShaderManager : public ResourceManager<Shader>
{
   public:
    ShaderManager() = default;
    Shader* LoadOrGet(StringId id, const char* shaderName);
};
}  // namespace DG::graphics
