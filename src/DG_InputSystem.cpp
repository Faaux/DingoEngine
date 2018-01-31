#include "DG_InputSystem.h"
#include <cstring>
#include "DG_Include.h"
#include "DG_Messaging.h"
#include "imgui/imgui_impl_sdl_gl3.h"

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

void InputSystem::Update()
{
    _mouseWheel = 0;
    _mouseLeft = _mouseRight = _mouseMiddle = false;

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
                KeyMessage message;
                message.scancode = event.key.keysym.scancode;
                message.key = &key;
                message.wasCtrlDown =
                    _keys[SDL_SCANCODE_LCTRL].isDown() || _keys[SDL_SCANCODE_RCTRL].isDown();
                message.wasAltDown =
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
                    // Resize Window and rebuild render pipeline
                    WindowSizeMessage message;
                    message.WindowSize.x = static_cast<float>(event.window.data1);
                    message.WindowSize.y = static_cast<float>(event.window.data2);
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
                    _mouseLeft = true;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    _mouseRight = true;
                if (event.button.button == SDL_BUTTON_MIDDLE)
                    _mouseMiddle = true;
            }
        }
    }

    // Grab Mouse State (additionally to events!)
    s32 mouseX, mouseY;
    u32 mouseMask = SDL_GetMouseState(&mouseX, &mouseY);
    _mouseLeft = _mouseLeft || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    _mouseRight = _mouseRight || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    _mouseMiddle = _mouseMiddle || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;

    s32 mouseDeltaX = mouseX - _mouseX;
    s32 mouseDeltaY = _mouseY - mouseY;
    _mouseX = mouseX;
    _mouseY = mouseY;

    // Fullscreen Toggle
    if ((_keys[SDL_SCANCODE_LALT].isDown() || _keys[SDL_SCANCODE_RALT].isDown()) &&
        _keys[SDL_SCANCODE_RETURN].wasPressed())
    {
        static bool isFullscreen = true;
        ToggleFullscreenMessage message;
        message.SetFullScreen = isFullscreen;
        isFullscreen = !isFullscreen;
        g_MessagingSystem.SendNextFrame(message);
    }

    // Input (Movement and mouse)
    {
        InputMessage message;
        if (_keys[SDL_SCANCODE_W].isDown())
        {
            message.Forward += 1.f;
        }
        if (_keys[SDL_SCANCODE_S].isDown())
        {
            message.Forward -= 1.f;
        }
        if (_keys[SDL_SCANCODE_D].isDown())
        {
            message.Right += 1.f;
        }
        if (_keys[SDL_SCANCODE_A].isDown())
        {
            message.Right -= 1.f;
        }
        if (_keys[SDL_SCANCODE_SPACE].isDown())
        {
            message.Up += 1.f;
        }
        if (_keys[SDL_SCANCODE_LSHIFT].isDown())
        {
            message.Up -= 1.f;
        }
        message.MouseWheel = _mouseWheel;
        message.MouseLeft = _mouseLeft;
        message.MouseRight = _mouseRight;
        message.MouseMiddle = _mouseMiddle;
        message.MouseDeltaX = mouseDeltaX;
        message.MouseDeltaY = mouseDeltaY;
        g_MessagingSystem.SendNextFrame(message);
    }
}

bool InputSystem::IsQuitRequested() const { return _quitRequested; }
}  // namespace DG
