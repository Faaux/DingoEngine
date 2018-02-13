/**
 *  @file    Viewport.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#include "Viewport.h"
#include "engine/Types.h"

namespace DG::graphics
{
void Viewport::Initialize(vec2 position, vec2 size)
{
    Assert(!_isValid);
    _position = position;
    _size = size;
    _buffer.Framebuffer.Initialize((s32)_size.x, (s32)_size.y, true, true, false);
    _isValid = true;
}

void Viewport::Destroy()
{
    Assert(_isValid);
    _buffer.Framebuffer.Shutdown();
    _isValid = false;
}

void Viewport::SetLocation(vec2 position, vec2 size)
{
    Assert(_isValid);
    _position = position;
    _size = size;
    if (_camera)
        _camera->UpdateProjection(size.x, size.y);
}
void Viewport::SetCamera(Camera* camera)
{
    Assert(_isValid);
    _camera = camera;
    _camera->UpdateProjection(_size.x, _size.y);
}

Camera* Viewport::GetCamera() const
{
    Assert(_isValid);
    return _camera;
}

const Camera& Viewport::GetBufferedCamera() const
{
    Assert(_isValid);
    return _buffer.Camera;
}

void Viewport::Apply()
{
    Assert(_isValid);
    _buffer.Framebuffer.Resize((s32)_size.x, (s32)_size.y);
    _buffer.Camera = *_camera;
}
}  // namespace DG::graphics
