#pragma once
#include <SDL.h>
#include "DG_Include.h"

namespace DG
{
class Key
{
    friend class InputSystem;

   public:
    bool isDown() const;
    bool isUp() const;
    int wasPressed() const;
    int wasReleased() const;

   private:
    void Reset();

    bool _wasPressed = false;
    bool _wasReleased = false;
    bool _isDown = false;
};

struct WindowSizeMessage
{
    vec2 WindowSize = vec2(0);
};

struct ToggleFullscreenMessage
{
    bool SetFullScreen = false;
};

struct InputMessage
{

    float Right = 0.f;
    float Forward = 0.f;
    float Up = 0.f;
    float MouseWheel = 0.f;
    bool MouseLeft = false;
    bool MouseRight = false;
    bool MouseMiddle = false;
    s32 MouseDeltaX = 0;
    s32 MouseDeltaY = 0;
};

struct KeyMessage
{
    SDL_Scancode scancode;
    Key* key = nullptr;
    bool wasCtrlDown;
    bool wasAltDown;
};

class InputSystem
{
   public:
    void Update();

    bool IsQuitRequested() const;
    void RequestClose();

    Key operator[](int i) const { return _keys[i]; }
    Key& operator[](int i) { return _keys[i]; }

   private:
    float _mouseWheel = 0;
    bool _mouseLeft = false;
    bool _mouseRight = false;
    bool _mouseMiddle = false;
    s32 _mouseX = 0;
    s32 _mouseY = 0;
    char _textInput[SDL_TEXTINPUTEVENT_TEXT_SIZE];
    bool _quitRequested = false;

    Key _keys[SDL_NUM_SCANCODES];
};
}  // namespace DG
