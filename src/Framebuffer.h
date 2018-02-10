/**
*  @file    Framebuffer.h
*  @author  Faaux (github.com/Faaux)
*  @date    11 February 2018
*/

#pragma once
#include "DG_Include.h"
#include "InputSystem.h"
#include "Messaging.h"
#include "Texture.h"

namespace DG
{
struct WindowSizeMessage;

struct MainBackbufferSizeMessage
{
    vec2 WindowSize = vec2(0);
};

class Framebuffer
{
   public:
    Framebuffer(u32 width, u32 height);
    ~Framebuffer();
    void Bind();
    void UnBind();
    void AddDepthTexture(bool linear = false);
    void AddColorTexture();

    void Resize(u32 width, u32 height);
    vec2 GetSize();

    graphics::Texture ColorTexture;
    graphics::Texture DepthTexture;

   private:
    void Initialize();
    void Cleanup();

    u32 _width;
    u32 _height;
    u32 fbo;
    bool _hasColor = false;
    bool _hasDepth = false;
};
}  // namespace DG
