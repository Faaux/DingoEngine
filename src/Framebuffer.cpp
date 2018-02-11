/**
 *  @file    Framebuffer.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Framebuffer.h"
#include <glad/glad.h>
#include "InputSystem.h"
#include "Messaging.h"
#include "Texture.h"

namespace DG
{
Framebuffer::Framebuffer(u32 width, u32 height) : _width(width), _height(height) { Initialize(); }

Framebuffer::~Framebuffer() {}

void Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, _width, _height);
}

void Framebuffer::UnBind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Framebuffer::AddDepthTexture(bool linear)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    DepthTexture.InitTexture(0, _width, _height, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT,
                             linear);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthTexture.GetTextureId(), 0);
    _hasDepth = true;
}

void Framebuffer::AddColorTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    ColorTexture.InitTexture(0, _width, _height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           ColorTexture.GetTextureId(), 0);
    _hasColor = true;
}

void Framebuffer::Resize(u32 width, u32 height)
{
    if (width == _width && height == _height)
        return;

    _width = width;
    _height = height;

    Cleanup();
    Initialize();
}

vec2 Framebuffer::GetSize() { return vec2(_width, _height); }

void Framebuffer::Initialize()
{
    glGenFramebuffers(1, &fbo);
    if (_hasColor)
        AddColorTexture();
    if (_hasDepth)
        AddDepthTexture();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Cleanup()
{
    glDeleteFramebuffers(1, &fbo);
    ColorTexture.Cleanup();
    DepthTexture.Cleanup();
}
}  // namespace DG
