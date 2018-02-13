/**
 *  @file    Framebuffer.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "Texture.h"
#include "engine/Types.h"
#include "math/GLMInclude.h"

namespace DG
{
class Framebuffer
{
   public:
    Framebuffer() = default;
    ~Framebuffer() = default;

    void Initialize(s32 width, s32 height, bool withColor, bool withDepth, bool isDepthLinear);
    void Shutdown();

    void Resize(s32 width, s32 height);
    vec2 GetSize() const;

    void Bind();
    void UnBind();

    graphics::Texture ColorTexture;
    graphics::Texture DepthTexture;

   private:
    void ReInitialize();
    void AddDepthTextureInternal();
    void AddColorTextureInternal();
    void Cleanup();

    s32 _width;
    s32 _height;
    u32 fbo = 0;
    bool _isInitialized = false;
    bool _isDirty = false;
    bool _hasColor = false;
    bool _hasDepth = false;
    bool _hasLinearDepth = false;
};
}  // namespace DG
