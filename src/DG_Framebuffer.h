#pragma once
#include "DG_Include.h"
#include "DG_InputSystem.h"
#include "DG_Messaging.h"
#include "DG_Texture.h"

namespace DG
{
struct WindowSizeMessage;

class Framebuffer
{
   public:
    Framebuffer(u32 width, u32 height);
    ~Framebuffer();
    void Bind();
    void UnBind();
    void AddDepthTexture();
    void AddColorTexture();

    void Resize(u32 width, u32 height);

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
