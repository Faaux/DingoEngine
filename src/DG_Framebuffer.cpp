#include "DG_Framebuffer.h"
#include <glad/glad.h>
#include "DG_InputSystem.h"
#include "DG_Messaging.h"
#include "DG_Texture.h"

namespace DG
{
Framebuffer::Framebuffer() : _width(1280), _height(720) { Initialize(); }

Framebuffer::~Framebuffer() {}

void Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, _width, _height);
}

void Framebuffer::UnBind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Framebuffer::Resize(u32 width, u32 height)
{
    if (width == _width && height == _height)
        return;

    _width = width;
    _height = height;

    Cleanup();
    Initialize();
}

void Framebuffer::Initialize()
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    Texture.InitTexture(0, _width, _height, GL_RGB);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           Texture.GetTextureId(), 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        // Error
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Error: Framebuffer not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Cleanup()
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    Texture.Cleanup();
}
}  // namespace DG
