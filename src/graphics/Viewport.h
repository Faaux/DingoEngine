/**
 *  @file    Viewport.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#pragma once
#include "Framebuffer.h"
#include "engine/Camera.h"
#include "math/GLMInclude.h"

namespace DG::graphics
{
class Viewport
{
   public:
    void Initialize(vec2 position, vec2 size);
    void Destroy();

    void SetLocation(vec2 position, vec2 size);
    const vec2& GetPosition() const { return _position; }
    const vec2& GetSize() const { return _size; }
    const Framebuffer* GetFramebuffer() const { return &_buffer.Framebuffer; }
    Framebuffer* GetBufferedFramebuffer() { return &_buffer.Framebuffer; }

    void SetCamera(Camera* camera);
    Camera* GetCamera() const;
    const Camera& GetBufferedCamera() const;
    void Apply();

   private:
    struct Buffer
    {
        Framebuffer Framebuffer;
        Camera Camera;
    };
    Buffer _buffer;

    Camera* _camera = nullptr;

    bool _isValid = false;
    vec2 _position;
    vec2 _size;
};
}  // namespace DG::graphics
