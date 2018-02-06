#pragma once
#include <glad/glad.h>
#include "DG_Include.h"
namespace DG::graphics
{
class Texture
{
   public:
    Texture() = default;
    void InitTexture(const u8* data, const u32 width, const u32 height, u32 internalFormat, u32 format, u32 type);
    void Bind() const;
    void Cleanup();
    GLuint GetTextureId() const
    {
        Assert(_isValid);
        return textureId;
    }

    bool _isValid = false;
    GLuint textureId = 0;
};
}  // namespace DG::graphics
