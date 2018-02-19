/**
 *  @file    InputSystem.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "engine/Messaging.h"
#include "engine/Types.h"
#include "graphics/GameWorldWindow.h"

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
}  // namespace DG
