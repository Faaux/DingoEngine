#include <SDL/SDL.h>
#include "DG_Include.h"
#include "DG_SDLHelper.h"

namespace DG
{
	bool GameIsRunning = true;
	SDL_Window* Window;

	bool InitSDL()
	{
		if (SDL_Init(0) < 0)
		{
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
			return false;
		}
#if _DEBUG
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_ERROR);
#endif
		SDL_LogSetOutputFunction(LogOutput, nullptr);
		return true;
	}

	bool InitWindow()
	{
		Window = SDL_CreateWindow("Dingo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
		if (Window == nullptr)
		{
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Window could not be created! SDL Error: %s\n", SDL_GetError());
			return false;
		}
		return true;
	}

	void PollEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				GameIsRunning = false;
			}
			else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
			{
				// Key input!
			}
			else if (event.type == SDL_TEXTINPUT)
			{
				// Some text input
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

	void Cleanup()
	{
		SDL_Quit();
	}
}

int main(int , char* [])
{
	using namespace DG;
	if (!InitSDL())
		return -1;

	if (!InitWindow())
		return -1;

	// When in fullscreen dont minimize when loosing focus! (Borderless windowed)
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	r32 deltaTime;
	u64 lastTime;
	u64 currentFrame = 1;

	u64 currentTime = SDL_GetPerformanceCounter();
	u64 cpuFrequency = SDL_GetPerformanceFrequency();	

	SDL_Log("Output example");
	SDL_LogVerbose(0,"Verbose Logging");
	SDL_LogDebug(0, "Debug Logging");
	SDL_LogInfo(0, "Info Logging");
	SDL_LogWarn(0, "Warn Logging");
	SDL_LogError(0, "Error Logging");
	SDL_LogCritical(0, "Critical Logging");

	SDL_Log("Init Done ---------------------");
	SDL_Log("");
	SDL_Log("");

	while(GameIsRunning)
	{
		PollEvents();
		// Frame Begin
		lastTime = currentTime;
		currentTime = SDL_GetPerformanceCounter();
		deltaTime = static_cast<r32>(currentTime - lastTime) * 1000.f / static_cast<r32>(cpuFrequency);

		
		// Frame End
		currentFrame++;
	}


	Cleanup();
	return 0;
}