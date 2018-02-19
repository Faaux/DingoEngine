/**
 *  @file    GameWorldWindow.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#pragma once
#include "Framebuffer.h"
#include "engine/Camera.h"
#include "engine/GameWorld.h"
#include "engine/Messaging.h"
#include "math/GLMInclude.h"

namespace DG::graphics
{
class GameWorldWindow
{
   public:
    void Initialize(const char* windowName, GameWorld* gameWorld);
    void Destroy();

    Framebuffer* GetFramebuffer() { return &_buffer.Framebuffer; }
    Camera* GetCamera() { return &_buffer.Camera; }

    /**
     * \brief Needs to be called from the render thread
     */
    void ApplyRenderState();
    void AddToImgui();
    void Update(float dtSeconds);
    const vec2& GetPosition() const { return _position; }
    const vec2& GetSize() const { return _size; }

   private:
    void RecordMessage(const Message& message);

    struct Buffer
    {
        Framebuffer Framebuffer;
        Camera Camera;
    };
    Buffer _buffer;

    MessageHandle _handle;
    Message _lastRawMessage;

    const char* _windowName = nullptr;
    GameWorld* _gameWorld = nullptr;
    Camera* _camera = nullptr;

    bool _isValid = false;
    bool _isActive = false;
    vec2 _position;
    vec2 _size;
};
}  // namespace DG::graphics
