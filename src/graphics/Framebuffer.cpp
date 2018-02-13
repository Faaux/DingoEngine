/**
 *  @file    Framebuffer.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Framebuffer.h"
#include <glad/glad.h>
#include "Texture.h"

namespace DG
{
void Framebuffer::Initialize(s32 width, s32 height, bool withColor, bool withDepth,
                             bool isDepthLinear)
{
    _width = width;
    _height = height;
    _hasColor = withColor;
    _hasDepth = withDepth;
    _hasLinearDepth = isDepthLinear;
    _isDirty = true;
    _isInitialized = true;
}

void Framebuffer::Shutdown()
{
    Cleanup();
    _isInitialized = false;
}

void Framebuffer::Bind()
{
    if (_isDirty)
    {
        Cleanup();
        ReInitialize();
        _isDirty = false;
    }
    Assert(_isInitialized);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, _width, _height);
}

void Framebuffer::UnBind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Framebuffer::Resize(s32 width, s32 height)
{
    if (width == _width && height == _height)
        return;

    _width = width;
    _height = height;
    _isDirty = true;
}

vec2 Framebuffer::GetSize() const { return vec2(_width, _height); }

void Framebuffer::ReInitialize()
{
    Assert(_isInitialized);
    glGenFramebuffers(1, &fbo);
    if (_hasColor)
        AddColorTextureInternal();
    if (_hasDepth)
        AddDepthTextureInternal();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::AddDepthTextureInternal()
{
    Assert(_isInitialized);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    DepthTexture.InitTexture(0, _width, _height, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT,
                             _hasLinearDepth);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthTexture.GetTextureId(), 0);
}

void Framebuffer::AddColorTextureInternal()
{
    Assert(_isInitialized);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    ColorTexture.InitTexture(0, _width, _height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           ColorTexture.GetTextureId(), 0);
}

void Framebuffer::Cleanup()
{
    assert(_isInitialized);
    glDeleteFramebuffers(1, &fbo);
    ColorTexture.Cleanup();
    DepthTexture.Cleanup();
    fbo = 0;
}
}  // namespace DG
