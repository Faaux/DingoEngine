#pragma once
#include <glad/glad.h>
#include "DG_Include.h"
namespace DG
{
class Texture
{
   public:
    Texture() = default;
    void InitTexture(const u8* data, const u32 width, const u32 height);
    void Bind() const;

   private:
    bool _isValid = false;
    GLuint textureId = 0;
};
}  // namespace DG
