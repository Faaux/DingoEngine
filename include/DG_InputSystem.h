#pragma once
#include <SDL.h>

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

	class InputSystem
	{
	public:
		void Update();

		bool IsQuitRequested() const;

		Key operator [](int i) const { return _keys[i]; }
		Key& operator [](int i) { return _keys[i]; }
	private:
		char _textInput[SDL_TEXTINPUTEVENT_TEXT_SIZE];
		bool _quitRequested = false;

		Key _keys[SDL_NUM_SCANCODES];
		
	};
}
