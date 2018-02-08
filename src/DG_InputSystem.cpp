#include "DG_InputSystem.h"
#include <cstring>
#include "DG_Include.h"
#include "DG_Messaging.h"
#include "imgui/imgui_impl_sdl_gl3.h"

namespace DG
{
struct RawInputMessage
{
    float Right = 0.f;
    float Forward = 0.f;
    float Up = 0.f;
    float MouseWheel = 0.f;
    bool MouseLeftDown = false;
    bool MouseRightDown = false;
    bool MouseMiddleDown = false;
    bool MouseLeftPressed = false;
    bool MouseRightPressed = false;
    bool MouseMiddlePressed = false;
    s32 MouseDeltaX = 0;
    s32 MouseDeltaY = 0;
    s32 MouseX = 0;
    s32 MouseY = 0;
};

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
        ToggleFullscreenMessage message;
        message.SetFullScreen = isFullscreen;
        isFullscreen = !isFullscreen;
        g_MessagingSystem.SendNextFrame(message);
    }

    // Input (Movement and mouse)
    {
        RawInputMessage message;
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
        message.MouseLeftDown = _mouseLeftDown;
        message.MouseRightDown = _mouseRightDown;
        message.MouseMiddleDown = _mouseMiddleDown;
        message.MouseLeftPressed = _mouseLeftPressed;
        message.MouseRightPressed = _mouseRightPressed;
        message.MouseMiddlePressed = _mouseMiddlePressed;
        message.MouseDeltaX = mouseDeltaX;
        message.MouseDeltaY = mouseDeltaY;
        message.MouseX = _mouseX;
        message.MouseY = _mouseY;
        g_MessagingSystem.SendNextFrame(message);
    }
}

bool RawInputSystem::IsQuitRequested() const { return _quitRequested; }

void RawInputSystem::RequestClose() { _quitRequested = true; }

InputSystem::InputSystem()
{
    g_MessagingSystem.RegisterCallback<RawInputMessage>([=](const RawInputMessage& m) {
        if (!IsForwardingToGame)
            return;

        static_assert(sizeof(InputMessage) == sizeof(RawInputMessage));
        InputMessage message;
        SDL_memcpy(&message, &m, sizeof(InputMessage));

        bool isVisible = ImGui::BeginChild("Scene Window");
        Assert(isVisible);
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetContentRegionAvail();
        ImGui::EndChild();

        // Sanitize Mouse Input
        message.MouseX = (s32)(m.MouseX - p.x);
        message.MouseY = (s32)(m.MouseY - p.x);

        if (message.MouseX > size.x || message.MouseY > size.y)
        {
            message.MouseLeftPressed = false;
            message.MouseRightPressed = false;
            message.MouseMiddlePressed = false;
        }

        g_MessagingSystem.SendImmediate(message);
    });
}
}  // namespace DG
