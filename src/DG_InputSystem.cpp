#include "DG_InputSystem.h"
#include <cstring>
#include "DG_Include.h"
#include "imgui_impl_sdl_gl3.h"

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
    for (Key& key : _keys)
    {
        key.Reset();
    }
    SDL_memset(_textInput, 0, SDL_TEXTINPUTEVENT_TEXT_SIZE);
    SDL_Event event;
    u32 currentIndex = 0;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSdlGL3_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
        {
            _quitRequested = true;
        }
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        {
            Key& key = _keys[event.key.keysym.scancode];

            if (event.type == SDL_KEYDOWN)
            {
                if (!key._isDown) key._wasPressed = true;
                key._isDown = true;
            }
            else
            {
                if (key._isDown) key._wasReleased = true;
                key._isDown = false;
            }
        }
        else if (event.type == SDL_TEXTINPUT)
        {
            strcat(_textInput, event.text.text);
        }
        else if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                // Resize Window and rebuild render pipeline
            }
        }
    }
}

bool InputSystem::IsQuitRequested() const { return _quitRequested; }
}  // namespace DG
