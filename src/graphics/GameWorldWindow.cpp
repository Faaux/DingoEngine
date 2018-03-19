/**
 *  @file    GameWorldWindow.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#include "GameWorldWindow.h"
#include <imgui.h>
#include "engine/Types.h"
#include "imgui/imgui_dock.h"
#include "imgui/imgui_impl_sdl_gl3.h"

namespace DG::graphics
{
void GameWorldWindow::Initialize(const char* windowName, GameWorld* gameWorld)
{
    Assert(!_isValid);
    _buffer.Framebuffer.Initialize((s32)_size.x, (s32)_size.y, true, true, false);
    _gameWorld = gameWorld;
    _windowName = windowName;

    // Register to input messages
    _handle = g_MessagingSystem.RegisterCallback(
        MessageType::RawInput,
        Delegate<void(const Message&)>::From<GameWorldWindow, &GameWorldWindow::RecordMessage>(
            this));

    _isValid = true;
}

void GameWorldWindow::Destroy()
{
    Assert(_isValid);
    _buffer.Framebuffer.Shutdown();
    g_MessagingSystem.UnregisterCallback(_handle);
    _isValid = false;
}

void GameWorldWindow::ApplyRenderState()
{
    Assert(_isValid);
    _buffer.Framebuffer.Resize((s32)_size.x, (s32)_size.y);

    if (_camera)
        _buffer.Camera = *_camera;

    _buffer.Camera.UpdateProjection(_size.x, _size.y);
}

void GameWorldWindow::AddToImgui()
{
    // This runs on the main thread
    if (ImGui::BeginDock("Scene Window", 0, ImGuiWindowFlags_NoResize))
    {
        // ToDo(Faaux)(Default): Should forward to GameWorld in Viewport
        _isActive = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

        // Update viewport
        _position = ImGui::GetCursorScreenPos();
        _size = ImGui::GetContentRegionAvail();

        ImGui::GetWindowDrawList()->AddCallback(RenderBackbufferOfWindow, this);
    }
    ImGui::EndDock();
}

void GameWorldWindow::Update(float dtSeconds)
{
    Assert(_isValid);
    Assert(_gameWorld);
    if (_isActive)
    {
        GameWorld::Input input;
        // Sanitize Mouse Input
        input.MouseX = _lastRawMessage.RawInput.MouseX - _position.x;
        input.MouseY = _lastRawMessage.RawInput.MouseY - _position.y;

        input.Up = _lastRawMessage.RawInput.Up;
        input.Right = _lastRawMessage.RawInput.Right;
        input.Forward = _lastRawMessage.RawInput.Forward;

        input.MouseWheel = _lastRawMessage.RawInput.MouseWheel;
        input.MouseLeftDown = _lastRawMessage.RawInput.MouseLeftDown;
        input.MouseRightDown = _lastRawMessage.RawInput.MouseRightDown;
        input.MouseMiddleDown = _lastRawMessage.RawInput.MouseMiddleDown;
        input.MouseLeftPressed = _lastRawMessage.RawInput.MouseLeftPressed;
        input.MouseRightPressed = _lastRawMessage.RawInput.MouseRightPressed;
        input.MouseMiddlePressed = _lastRawMessage.RawInput.MouseMiddlePressed;
        input.MouseDeltaX = (f32)_lastRawMessage.RawInput.MouseDeltaX;
        input.MouseDeltaY = (f32)_lastRawMessage.RawInput.MouseDeltaY;
        input.ScreenWidth = _size.x;
        input.ScreenHeight = _size.y;

        _gameWorld->SetInput(input);
    }
    _gameWorld->Update(dtSeconds);

    _camera = _gameWorld->GetActiveCamera();
}

void GameWorldWindow::RecordMessage(const Message& message) { _lastRawMessage = message; }
}  // namespace DG::graphics
