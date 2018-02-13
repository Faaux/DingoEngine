/**
 *  @file    Texture.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include <glad/glad.h>
#include "engine/Types.h"
namespace DG::graphics
{
class Texture
{
   public:
    // ToDo: Clean this linear flag! This is dirty hacking
    Texture() = default;
    void InitTexture(const u8* data, const u32 width, const u32 height, u32 internalFormat,
                     u32 format, u32 type, bool linear = false);
    void Bind() const;
    void Cleanup();
    bool IsValid() const
    {
        return _isValid;
    }
    GLuint GetTextureId() const
    {
        Assert(_isValid);
        return textureId;
    }

    bool _isValid = false;
    GLuint textureId = 0;
};
}  // namespace DG::graphics
