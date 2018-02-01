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
    Framebuffer();
    ~Framebuffer();
    void Bind();
    void UnBind();

    void Resize(u32 width, u32 height);

    graphics::Texture Texture;

private:
    void Initialize();
    void Cleanup();

    u32 _width;
    u32 _height;
    u32 fbo;
    u32 rbo;
};
}  // namespace DG
