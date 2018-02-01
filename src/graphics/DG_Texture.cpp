#include "DG_Texture.h"

namespace DG::graphics
{
void Texture::InitTexture(const u8* data, const u32 width, const u32 height, u32 type)
{
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    _isValid = true;
}

void Texture::Bind() const
{
    if (!_isValid)
    {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Texture was not valid when binding it.");
    }
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::Cleanup()
{
    if (_isValid)
    {
        glDeleteTextures(1, &textureId);
        _isValid = false;
    }
}
}  // namespace DG::graphics
