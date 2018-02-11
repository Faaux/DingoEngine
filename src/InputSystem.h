/**
 *  @file    InputSystem.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include <SDL.h>
#include "DG_Include.h"

namespace DG
{
class Key
{
    friend class RawInputSystem;

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

struct KeyMessage
{
    SDL_Scancode scancode;
    Key* key = nullptr;
    bool wasCtrlDown;
    bool wasAltDown;
};

class RawInputSystem
{
   public:
    void Update();

    bool IsQuitRequested() const;
    void RequestClose();

    Key operator[](int i) const { return _keys[i]; }
    Key& operator[](int i) { return _keys[i]; }

   private:
    float _mouseWheel = 0;
    bool _mouseLeftDown = false;
    bool _mouseRightDown = false;
    bool _mouseMiddleDown = false;
    bool _mouseLeftPressed = false;
    bool _mouseRightPressed = false;
    bool _mouseMiddlePressed = false;
    s32 _mouseX = 0;
    s32 _mouseY = 0;
    char _textInput[SDL_TEXTINPUTEVENT_TEXT_SIZE];
    bool _quitRequested = false;

    Key _keys[SDL_NUM_SCANCODES];
};

class InputSystem
{
   public:
    InputSystem();
    bool IsForwardingToGame = true;
};
}  // namespace DG
