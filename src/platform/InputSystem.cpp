/**
 *  @file    InputSystem.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "InputSystem.h"
#include "engine/Messaging.h"
#include "imgui/imgui_impl_sdl_gl3.h"
#include "math/GLMInclude.h"

namespace DG
{
bool Key::isDown() const { return _isDown; }

bool Key::isUp() const { return !_isDown; }

int Key::wasPressed() const { return _wasPressed; }

int Key::wasReleased() const { return _wasReleased; }

void Key::Reset()
{
    _wasReleased = false;
    _wasPressed = false;
}

void RawInputSystem::Update()
{
    _mouseWheel = 0;
    _mouseLeftDown = _mouseRightDown = _mouseMiddleDown = false;
    _mouseLeftPressed = _mouseRightPressed = _mouseMiddlePressed = false;

    for (Key& key : _keys)
    {
        key.Reset();
    }
    SDL_memset(_textInput, 0, SDL_TEXTINPUTEVENT_TEXT_SIZE);
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSdlGL3_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                _quitRequested = true;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                Key& key = _keys[event.key.keysym.scancode];

                if (event.type == SDL_KEYDOWN)
                {
                    if (!key._isDown)
                        key._wasPressed = true;
                    key._isDown = true;
                }
                else
                {
                    if (key._isDown)
                        key._wasReleased = true;
                    key._isDown = false;
                }
                Message message;
                message.Type = MessageType::RawKey;
                message.RawKey.Scancode = event.key.keysym.scancode;
                message.RawKey.Key = &key;
                message.RawKey.WasCtrlDown =
                    _keys[SDL_SCANCODE_LCTRL].isDown() || _keys[SDL_SCANCODE_RCTRL].isDown();
                message.RawKey.WasAltDown =
                    _keys[SDL_SCANCODE_LALT].isDown() || _keys[SDL_SCANCODE_RALT].isDown();

                g_MessagingSystem.SendNextFrame(message);
            }
            break;
            case SDL_TEXTINPUT:
                SDL_strlcat(_textInput, event.text.text, 32);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    // Resize Window
                    Message message;
                    message.Type = MessageType::RawWindowSize;
                    message.RawWindowSize.Width = event.window.data1;
                    message.RawWindowSize.Height = event.window.data2;

                    g_MessagingSystem.SendNextFrame(message);
                }
                break;
            case SDL_MOUSEWHEEL:
            {
                if (event.wheel.y > 0)
                    _mouseWheel = 1;
                if (event.wheel.y < 0)
                    _mouseWheel = -1;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                    _mouseLeftPressed = true;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    _mouseRightPressed = true;
                if (event.button.button == SDL_BUTTON_MIDDLE)
                    _mouseMiddlePressed = true;
            }
        }
    }

    // Grab Mouse State (additionally to events!)
    s32 mouseX, mouseY;
    u32 mouseMask = SDL_GetMouseState(&mouseX, &mouseY);
    _mouseLeftDown = _mouseLeftPressed || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    _mouseRightDown = _mouseRightPressed || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    _mouseMiddleDown = _mouseMiddlePressed || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;

    s32 mouseDeltaX = mouseX - _mouseX;
    s32 mouseDeltaY = _mouseY - mouseY;
    _mouseX = mouseX;
    _mouseY = mouseY;

    // Fullscreen Toggle
    if ((_keys[SDL_SCANCODE_LALT].isDown() || _keys[SDL_SCANCODE_RALT].isDown()) &&
        _keys[SDL_SCANCODE_RETURN].wasPressed())
    {
        static bool isFullscreen = true;
        Message message;
        message.Type = MessageType::Fullscreen;
        message.Fullscreen.SetFullScreen = isFullscreen;
        isFullscreen = !isFullscreen;
        g_MessagingSystem.SendNextFrame(message);
    }

    // Input (Movement and mouse)
    {
        Message message;
        message.Type = MessageType::RawInput;
        message.RawInput.Forward = 0.f;
        message.RawInput.Right = 0.f;
        message.RawInput.Up = 0.f;

        if (_keys[SDL_SCANCODE_W].isDown())
        {
            message.RawInput.Forward += 1.f;
        }
        if (_keys[SDL_SCANCODE_S].isDown())
        {
            message.RawInput.Forward -= 1.f;
        }
        if (_keys[SDL_SCANCODE_D].isDown())
        {
            message.RawInput.Right += 1.f;
        }
        if (_keys[SDL_SCANCODE_A].isDown())
        {
            message.RawInput.Right -= 1.f;
        }
        if (_keys[SDL_SCANCODE_SPACE].isDown())
        {
            message.RawInput.Up += 1.f;
        }
        if (_keys[SDL_SCANCODE_LSHIFT].isDown())
        {
            message.RawInput.Up -= 1.f;
        }
        message.RawInput.MouseWheel = _mouseWheel;
        message.RawInput.MouseLeftDown = _mouseLeftDown;
        message.RawInput.MouseRightDown = _mouseRightDown;
        message.RawInput.MouseMiddleDown = _mouseMiddleDown;
        message.RawInput.MouseLeftPressed = _mouseLeftPressed;
        message.RawInput.MouseRightPressed = _mouseRightPressed;
        message.RawInput.MouseMiddlePressed = _mouseMiddlePressed;
        message.RawInput.MouseDeltaX = mouseDeltaX;
        message.RawInput.MouseDeltaY = mouseDeltaY;
        message.RawInput.MouseX = _mouseX;
        message.RawInput.MouseY = _mouseY;
        g_MessagingSystem.SendNextFrame(message);
    }
}

bool RawInputSystem::IsQuitRequested() const { return _quitRequested; }

void RawInputSystem::RequestClose() { _quitRequested = true; }
}  // namespace DG
